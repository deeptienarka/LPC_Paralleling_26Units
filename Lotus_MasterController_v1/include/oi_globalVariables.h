/*
 * oi_globalVariables.h
 *
 *  Created on: Sep 29, 2020
 *      Author: enArka
 */

#ifndef INCLUDE_OI_GLOBALVARIABLES_H_
#define INCLUDE_OI_GLOBALVARIABLES_H_

#include "driverlib/can.h"             // Standard CAN module include file
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_can.h"

Uint16 ui_Scheduler_Index;                              // Variable that holds current sine table index. We can calculate theta value by using this variable
Uint16 ui_Next_Event_In_Line;                           // Index to access next event in line
Uint16 ui_Event_Initiated;                              // Index to initiate next event in line

// CAN related variables

//tCANMsgObject sTXCANMessage;
//tCANMsgObject sRXCANMessage,sRXCANMessage1;

uint16_t RxMsgData[8] = {0};
uint16_t RxMsgData1[8] = {0};
uint16_t RxMsgData_IO[8] = {0};
uint16_t RxMsgData_Site[8] = {0};
uint16_t txMsgData_Site[8] = {0};

uint16_t RxMsgData_VSInt[6] = {0};

//
// Globals
//
volatile uint32_t txMsgCount = 0;
volatile uint32_t rxMsgCount = 0;
volatile uint32_t errorFlag = 0;
uint16_t txMsgData[8];
uint16_t txMsgData1[2];


Uint16 turnOn_All_PSU,indivisual_control_PSU, Set_Voltage,Set_Current;
Uint16 turnOn_PSU1, turnOn_PSU2, turnOn_PSU3;

float32 Voltage_Set = 26.36;
int32 temp5[8],frac[8];
Uint32 SYSTEM_ON;

Uint16 Site_Controller_On_Command_Received = 0;
Uint16 PurgeToIO = 0;
Uint16 h2ResHlthyChkCnt = 0;
Uint16 h2ResHlthyChkLmt = 4;
#endif /* INCLUDE_OI_GLOBALVARIABLES_H_ */
