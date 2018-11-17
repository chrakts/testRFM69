/*
 * Globals.cpp
 *
 * Created: 19.03.2017 09:24:37
 *  Author: Christof
 */
#define EXTERNALS_H_

#include <stdint.h>

#include "Serial.h"
#include "MyTimer.h"
#include "Communication.h"
#include "CRC_Calc.h"

char Compilation_Date[] = __DATE__;
char Compilation_Time[] = __TIME__;

volatile bool nextSendReady=false;

// Timerwert 16 entspricht 100ms
volatile TIMER MyTimers[MYTIMER_NUM]= {
  {TM_STOP,RESTART_NO,4,0,NULL},
  {TM_START,RESTART_YES,4,0,NULL},
  {TM_STOP,RESTART_NO,100,0,NULL}		// Timeout-Timer
};

Serial debug(0);
//Communication cmulti(1,"DF",8);


