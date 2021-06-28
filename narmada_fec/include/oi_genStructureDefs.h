/*
 * oi_globalStructures.h
 *
 *  Created on: Oct 13, 2020
 *      Author: enArka
 */

#ifndef INCLUDE_OI_GENSTRUCTUREDEFS_H_
#define INCLUDE_OI_GENSTRUCTUREDEFS_H_
#include <oi_fec_control.h>
#include <oi_scheduler.h>
#include <oi_State.h>

extern fec_state_t fec_state;


extern TIMERS timer;
extern fault_regs_t_h faults_h;
extern fault_regs_t_l faults_l;

extern Control Control_var;
extern AC_FEEDBACK AC_inst_Values;
extern DC_FEEDBACK DC_Avg_Values;

extern ABC_TO_DQ_CONTROL current;
extern ABC_TO_DQ_CONTROL voltage;

extern DQ_TO_ABC_CONTROL mod_Index, del_Mod_Index,mod_Index_ff;


extern Phase_Seq Phase_Sequence;
extern Temp_Meas_t Temp_Meas;

extern PI_CONTROLLER voltage_Loop;

extern GLOBAL_FLAGS Global_Flags;
extern EVENT Event_Array[TOTAL_NO_OF_EVENTS];

extern SYSTEM_STATE State;
extern STATE_TIMERS_t State_timers;

extern CIRCUIT_GAIN CircuitGain;



#endif /* INCLUDE_OI_GENSTRUCTUREDEFS_H_ */
