/*
 */

#include <avr/io.h>
#include "External.h"
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "SPI.h"
#include "RFM69.h"
#include "secrets.h"
#include "ledHardware.h"

enum{QUARZ,CLK2M,CLK32M};

#define SYSCLK CLK32M

#define PLL 0
#define ENCRYPTKEY    "sampleEncryptKey"
#define myID          'L'
#define toID          33
#define NETWORK       '1'

const char RFM69Key[16] = {RFM69KEY};

void init_clock(int sysclk, int pll);

int main(void)
{
    // Insert code
  init_clock(SYSCLK, PLL);

 	PORTA_DIRSET = PIN2_bm | PIN3_bm | PIN4_bm;
	PORTA_OUTSET = 0xff;

	PORTB_DIRSET = 0xff;

	PORTC_DIRSET = PIN1_bm;

	PORTD_DIRSET = PIN0_bm | PIN4_bm | PIN5_bm | PIN7_bm;
	PORTD_DIRCLR = PIN6_bm;
	PORTD_OUTCLR = PIN4_bm | PIN5_bm;

	PORTE_DIRSET = 0xff;
  LEDROTSETUP;
  LEDGRUENSETUP;
  LEDSIGNALSETUP;

  LEDSIGNAL_ON;
  LEDROT_ON;
  _delay_ms(1000);
  init_mytimer();
  SPI_Master_t spiRFM69;

  SPI_MasterInit(&spiRFM69,&(SPID),&(PORTD),false,SPI_MODE_0_gc,SPI_INTLVL_LO_gc,false,SPI_PRESCALER_DIV128_gc);

  RFM69 myRFM(&MyTimers[RFM69_TIMER],&spiRFM69,true);

  PMIC_CTRL = PMIC_LOLVLEX_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm;
  sei();

  debug.open(Serial::BAUD_57600,F_CPU);
  debug.print("\nHallo vom Empfaenger");

  myRFM.initialize(86,myID,NETWORK);
  myRFM.encrypt(RFM69Key);
  //myRFM.readAllRegsCompact();
  debug.print("\nInit fertig\n");
  LEDROT_OFF;
  nextSendReady=true;
  while(1)
  {
    //check for any received packets
/*    if (myRFM.receiveDone())
    {
      LEDROT_ON;
      debug.pformat("[%d]-[%d]-[RX_RSSI: %d]:",myRFM.SENDERID,myRFM.DATALEN,myRFM.RSSI);
      debug.print((char *)myRFM.DATA);

      if (myRFM.ACKRequested())
      {
        myRFM.sendACK();
        debug.print(" - ACK sent\n");
      }

      LEDROT_OFF;
    }*/
    if(nextSendReady)
    {
      LEDSIGNAL_ON;
      myRFM.sendWithRetry('T', "BRL1SF1JTKaffee", 15, 10, 100);
      //myRFM.send('T', "BRL1SF1JTKaffee", 15,false);
      nextSendReady = false;
      LEDSIGNAL_OFF;
    }
    myRFM.sleep();
    while(1)
    {
      _delay_ms(200);
      LEDSIGNAL_TOGGLE;
      LEDROT_TOGGLE;
    }

  }

  return 0;
}

void init_clock(int sysclk, int pll)
{
	CLK_t *mein_clock;
	OSC_t *mein_osc;
	mein_clock = &CLK;
	mein_osc = &OSC;
	switch(sysclk)
	{
		case QUARZ:
			mein_osc->XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
//			mein_osc->XOSCCTRL = OSC_FRQRANGE_12TO16_gc | OSC_XOSCPWR_bm | OSC_XOSCSEL_XTAL_16KCLK_gc;
			mein_osc->CTRL = OSC_XOSCEN_bm | OSC_RC2MEN_bm | OSC_RC32KEN_bm; // schaltet die 32 MHz-Clock ein

			while((mein_osc->STATUS & OSC_XOSCRDY_bm) == 0)			// wartet bis diese stabil
			;
			while((mein_osc->STATUS & OSC_RC32KRDY_bm) == 0)		// wartet bis diese stabil
			;

			if ( (pll>0) & (pll<16) )
			{
				mein_osc->PLLCTRL = OSC_PLLSRC_XOSC_gc | pll;
				mein_osc->CTRL = OSC_PLLEN_bm | OSC_XOSCEN_bm | OSC_RC2MEN_bm | OSC_RC32KEN_bm; // schaltet zusätzlich die PLL ein

				while((mein_osc->STATUS & OSC_PLLRDY_bm) == 0)		// wartet bis diese stabil
				;
				CCP = CCP_IOREG_gc;										// geschuetztes Register freigeben
				mein_clock->CTRL = CLK_SCLKSEL_PLL_gc;					// umschalten auf PLL-Clock
				mein_osc->CTRL = OSC_PLLEN_bm | OSC_XOSCEN_bm | OSC_RC32KEN_bm;
			}
			else
			{
				CCP = CCP_IOREG_gc;										// geschuetztes Register freigeben
				mein_clock->CTRL = CLK_SCLKSEL_XOSC_gc;					// umschalten auf XOSC-Clock
				mein_osc->CTRL = OSC_XOSCEN_bm | OSC_RC32KEN_bm;
			}
		break; // QUARZ
		case CLK2M:
			mein_osc->CTRL = OSC_RC2MEN_bm | OSC_RC32KEN_bm; // schaltet die 2 MHz-Clock ein
			while((mein_osc->STATUS & OSC_RC2MRDY_bm) == 0)  // wartet bis diese stabil
			;
			while((mein_osc->STATUS & OSC_RC32KRDY_bm) == 0)  // wartet bis diese stabil
			;
			CCP = CCP_IOREG_gc;								// geschuetztes Register freigeben
			mein_clock->CTRL = CLK_SCLKSEL_RC2M_gc;		// umschalten auf 2 MHz-Clock
//			CLKSYS_AutoCalibration_Enable(OSC_RC2MCREF_RC32K_gc,false); // OSC_RC32MCREF_bm
		break;
		case CLK32M:
			mein_osc->CTRL = OSC_RC32MEN_bm | OSC_RC2MEN_bm | OSC_RC32KEN_bm; // schaltet die 32 MHz-Clock ein
			while((mein_osc->STATUS & OSC_RC32MRDY_bm) == 0)  // wartet bis diese stabil
			;
			while((mein_osc->STATUS & OSC_RC32KRDY_bm) == 0)  // wartet bis diese stabil
			;
			CCP = CCP_IOREG_gc;								// geschuetztes Register freigeben
			mein_clock->CTRL = CLK_SCLKSEL_RC32M_gc;		// umschalten auf 32 MHz-Clock
			mein_osc->CTRL = OSC_RC32MEN_bm | OSC_RC32KEN_bm;		// abschalten der 2 MHz-Clock
//			CLKSYS_AutoCalibration_Enable(OSC_RC32MCREF_RC32K_gc,false); // OSC_RC32MCREF_bm
		break;
	}
}
