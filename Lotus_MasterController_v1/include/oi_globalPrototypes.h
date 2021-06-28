/*
 * oi_globalPrototypes.h
 *
 *  Created on: Jan 13, 2021
 *      Author: enArka
 */

#ifndef INCLUDE_OI_GLOBALPROTOTYPES_H_
#define INCLUDE_OI_GLOBALPROTOTYPES_H_


// Interrupt Service routine

extern interrupt void epwm1_isr(void);
extern interrupt void canISR(void);

extern void oi_vSystemInit(void);
extern void oi_initPeripherals(void);
extern void oi_globalVar_init(void);

// PWM functions

extern void oi_initEPwm1(void);

// CAN Initialization

extern void oi_CAN_init(void);
extern void oi_CANA_TX_Event_PSU();
extern void oi_CANA_RX_Event_PSU();
extern void oi_CANA_TX_Event_VS();
extern void oi_CANA_RX_Event_VS();

extern void CurrentSetLogic();

extern void Set_Voltage_Node_specific(int32 nodeID , float32 voltage_Value, Uint16 global);
extern void Save_Settings_Node_specific(int32 nodeID);
extern void Set_local_Node_specific(int32 nodeID);
extern void Set_Current_Node_specific(int32 nodeID , float32 current_Value, Uint16 global);

extern void Set_Voltage_Global(float32 voltage_Value);
extern void Set_Current_Global(float32 current_Value);
extern void Query_MeasParams_Node_Specific(int32 nodeID);
extern void Query_Flts_Node_Specific(int32 nodeID);
extern void Query_ProgParams_Node_Specific(int32 nodeID);


extern void Command_IO_Node_Specific_default(int32 nodeID);
extern void Command_IO_Node_Specific_Purge(int32 nodeID, Uint16 Stage);

extern void Query_IO_Node_Specific(int32 nodeID);
extern void Command_IO_Node_Specific_IOPower(int32 nodeID, Uint16 Stage);
extern void Query_MeasCell_VoltParam_Node_Specific(int32 nodeID);

extern void oi_CANA_TX_Event_IO();
extern void oi_CANA_RX_Event_IO();
extern void Check_Snsr_Status();

extern void oi_CANB_Site_Event();


extern void oi_CANA_InitRead_PSU();
extern void oi_CANA_InitRead_VS();
extern void oi_CANA_InitRead_IO();
extern void oi_CANB_InitRead_Site();

extern void oi_Cell_MaxVolt();
extern void oi_Cell_MinVolt();
extern void oi_Cell_MaxVoltInt();
extern void oi_Cell_MinVoltInt();

extern void Turn_ON_Node_specific(int32 nodeID , int32 ON_command, int32 global);
//


// GPIO Initialization
extern void oi_initGpio(void);

// System GPIO Initialization
extern void oi_initsystemGpio(void);

//GPIO initialization to the PWMs
extern void oi_initEPwm1Gpio(void);

// GPIO Initialization
extern void oi_initGpio(void);

// Scheduler events

extern Uint16 oi_Event1();
extern Uint16 oi_Event2();
extern Uint16 oi_Event3();
extern Uint16 oi_Event4();
extern Uint16 oi_Event5();
extern Uint16 oi_Event6();
extern Uint16 oi_control_Event1();
extern Uint16 oi_control_Event2();
extern Uint16 oi_control_Event3();
extern Uint16 oi_control_Event4();
extern Uint16 oi_control_Event5();
extern Uint16 oi_control_Event6();

//

// State events

extern oi_vState();

#endif /* INCLUDE_OI_GLOBALPROTOTYPES_H_ */
