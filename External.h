/*
 * External.h
 *
 * Created: 03.04.2017 21:04:41
 *  Author: Christof
 */



#ifndef EXTERNAL_H_
#define EXTERNAL_H_

#include "CRC_Calc.h"
#include "Serial.h"
#include "MyTimer.h"

extern volatile uint8_t UART0_ring_received;
//extern volatile  char UART0_ring_buffer[UART0_RING_BUFFER_SIZE];
extern volatile  char UART0_ring_buffer[];
extern volatile uint8_t UART1_ring_received;
extern volatile  char UART1_ring_buffer[];
extern volatile  uint8_t sendFree;
extern volatile TIMER MyTimers[];
extern volatile bool nextSendReady;

/*  */
extern Serial debug;


#endif /* EXTERNAL_H_ */
