/*
 * oi_globalVariables.h
 *
 *  Created on: Sep 29, 2020
 *      Author: enArka
 */

#ifndef INCLUDE_OI_GLOBALVARIABLES_H_
#define INCLUDE_OI_GLOBALVARIABLES_H_

#include "oi_sinGeneration.h"


Uint16 ui_Scheduler_Index;                              // Variable that holds current sine table index. We can calculate theta value by using this variable
Uint16 ui_Next_Event_In_Line;                           // Index to access next event in line
Uint16 ui_Event_Initiated;                              // Index to initiate next event in line

// Control Variables

float32 Inst_AC_Over_Currlmt;
float32 Inst_AC_Current_Envelop;
float32 min_mod_index;
float32 max_mod_index;
float32 com_mod_index;

float32 Vref;
float32 DCBusVoltInst;
float32 del_Mod_Com;
float32 k_com;
float32 k_diff;
Uint16  VDCStatusChkFlg;

float32 inputVector[NO_OF_SAMPLES_IN_ONE_CYCLE];
float32 UNIT_COS_R;
float32 UNIT_SINE_R;

Uint16 CapInterruptAck;
Uint16 FrequencyInHz;
Uint16 DC_UNDER_VOLTAGE_LIMIT;

float32 phase_R;
float32 phase_Y;
float32 phase_B;

// SCI Variables

uint16_t onCommand;
uint16_t rxChar;
uint16_t rxStatus;
uint16_t sci_cnt;
uint16_t sci_msg1;
uint16_t sci_msg2;
uint16_t onStatus;
uint16_t receivedChar;

unsigned char msg1[4], msg2[4];
unsigned char *msg_id;

#endif /* INCLUDE_OI_GLOBALVARIABLES_H_ */
