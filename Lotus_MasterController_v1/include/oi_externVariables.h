/*
 * oi_externVariables.h
 *
 *  Created on: Jan 13, 2021
 *      Author: enArka
 */


#ifndef INCLUDE_OI_EXTERNVARIABLES_H_
#define INCLUDE_OI_EXTERNVARIABLES_H_

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_can.h"
#include "driverlib/can.h"

extern Uint16 ui_Scheduler_Index;                              // Variable that holds current sine table index. We can calculate theta value by using this variable
extern Uint16 ui_Next_Event_In_Line;                           // Index to access next event in line
extern Uint16 ui_Event_Initiated;                              // Index to initiate next event in line



// CAN related variables

extern volatile uint32_t txMsgCount;
extern volatile uint32_t rxMsgCount;
extern volatile uint32_t errorFlag;
extern uint16_t txMsgData[8];
extern uint16_t txMsgData1[2];
extern Uint16 NodePresent, PrevNodePresent;


extern int32 temp5[8],frac[8];
//
// Globals
//

extern uint16_t RxMsgData[8];
extern uint16_t RxMsgData1[8];
extern uint16_t RxMsgData_VSInt[8];
extern uint16_t RxMsgData_IO[8];
extern uint16_t RxMsgData_Site[8];
extern uint16_t txMsgData_Site[8];

extern Uint32 cnt1, cnt2;
extern Uint16 trig1, trig2;
extern Uint16 fsmCounter;

extern Uint32 x;
extern Uint16 i;
extern Uint16 queryType;
extern Uint16 canQueryCount;
extern Uint16 statCheckCnt;
extern Uint16 IOCANcounter;
extern Uint16 CurrentCnt;
extern Uint16 PurgeCnt;
extern Uint16 StandByCnt;



extern Uint32 SYSTEM_ON;

extern Uint16 Site_Controller_On_Command_Received;
extern Uint16 h2ResHlthyChkCnt;
extern Uint16 h2ResHlthyChkLmt;
#endif /* INCLUDE_OI_EXTERNVARIABLES_H_ */
