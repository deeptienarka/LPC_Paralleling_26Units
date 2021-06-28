/*
 * oi_externVariables.h
 *
 *  Created on: Sep 29, 2020
 *      Author: enArka
 */

#ifndef INCLUDE_OI_EXTERNVARIABLES_H_
#define INCLUDE_OI_EXTERNVARIABLES_H_

#include "oi_sinGeneration.h"

extern float32 inputVector[NO_OF_SAMPLES_IN_ONE_CYCLE];
extern float32 UNIT_COS_R;
extern float32 UNIT_SINE_R;


extern Uint16 FrequencyInHz;
extern Uint16 DC_UNDER_VOLTAGE_LIMIT;

extern Uint16 ui_Scheduler_Index;                              // Variable that holds current sine table index. We can calculate theta value by using this variable
extern Uint16 ui_Next_Event_In_Line;                           // Index to access next event in line
extern Uint16 ui_Event_Initiated;                              // Index to initiate next event in line

// Control Variables

extern float32 Inst_AC_Over_Currlmt;
extern float32 Inst_AC_Current_Envelop;
extern float32 min_mod_index;
extern float32 max_mod_index;
extern float32 com_mod_index;

extern float32 Vref;
extern float32 DCBusVoltInst;
extern float32 del_Mod_Com;
extern float32 k_com;
extern float32 k_diff;
extern Uint16  VDCStatusChkFlg;
extern float32 phase_R;
extern float32 phase_Y;
extern float32 phase_B;

// PLL Variables
extern Uint16 CapInterruptAck;

// SCI Variables

extern uint16_t onCommand;
extern uint16_t rxChar;
extern uint16_t rxStatus;
extern uint16_t sci_cnt;
extern uint16_t sci_msg1;
extern uint16_t sci_msg2;
extern uint16_t onStatus;
extern uint16_t receivedChar;

extern unsigned char msg1[4], msg2[4];
extern unsigned char *msg_id;




#endif /* INCLUDE_OI_EXTERNVARIABLES_H_ */
