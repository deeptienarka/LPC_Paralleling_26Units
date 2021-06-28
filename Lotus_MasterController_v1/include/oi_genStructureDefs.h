/*
 * oi_globalStructures.h
 *
 *  Created on: Jan 13, 2021
 *      Author: enArka
 */

#ifndef INCLUDE_OI_GENSTRUCTUREDEFS_H_
#define INCLUDE_OI_GENSTRUCTUREDEFS_H_

#include <oi_can.h>
#include <oi_scheduler.h>
#include "oi_state.h"
// Scheduler Structures

extern EVENT Event_Array[TOTAL_NO_OF_EVENTS];
extern GLOBAL_FLAGS Global_Flags;

// CAN Structures

extern CANA_timers_t CANA_timers;
extern CANA_Ui_Regs_R CANA_Ui_Regs;
extern CAN_Read CanaRead,CanbRead;
extern CAN_Node CanNode;
extern Query_Type QueryType;
extern CAN_Recd_Params CANRecdParams;
extern AIx_Data AI_Data[TOTAL_IO_NODE_NUMBER];
extern AI_SensorData1 AI_SensorData;
extern AOx_Data AO_Data[TOTAL_IO_NODE_NUMBER], Set_AO_Data[TOTAL_IO_NODE_NUMBER];
extern THERMODATA Thermo_Data[TOTAL_IO_NODE_NUMBER];
extern IO_FLAG IO_Flags;
extern Site_CANCommands SiteCANCommands;


extern union FAULT_REGS FaultRegs[29];
extern union DC_FAULT_REGS DcFaultRegs;
extern union ACTIVE_NODE_REGS ActiveNodeRegs;
extern union ACTIVE_NODE_REGS1 ActiveNodeRegs1;
extern union NODE_FAULT_REGS NodeFaultRegs;
extern union IO_DO_REG IO_DigitalOutputRegs[TOTAL_IO_NODE_NUMBER], IO_SetDigitalOutputRegs[TOTAL_IO_NODE_NUMBER];
extern union IO_DI_REG IO_DigitalInputRegs[TOTAL_IO_NODE_NUMBER];
extern union AI_ENABLE_REG IO_AnalogInputEnableRegs[TOTAL_IO_NODE_NUMBER], IO_SetAnalogInputEnableRegs[TOTAL_IO_NODE_NUMBER];


// Finite State Machine Structures

extern SYSTEM_STATE State;
extern STATE_TIMERS_t State_timers;

// System Controller Structures

extern SystemParams SystemParam;
#endif /* INCLUDE_OI_GENSTRUCTUREDEFS_H_ */
