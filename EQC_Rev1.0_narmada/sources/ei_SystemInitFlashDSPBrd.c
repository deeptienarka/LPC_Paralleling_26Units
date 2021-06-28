//#################################################################################################################
//# 							enArka Instruments proprietary
//# File: ei_SystemInitFlashDSPBrd.c
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.
//# All Rights Reserved
//#
//#################################################################################################################

//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
#include "ExternalVariables.h"
//#################################################################################################################
									/*-------Global Prototypes----------*/
//void ei_vSystemInit();
//void ei_vRetrieveLastSettings();
//#################################################################################################################
									/*-------Local Prototypes----------*/
static void  ei_vInitializeGlobVars();
static void ei_vProductCalibrationCheck();
static void ei_vReadModelSpecs();
static void ei_vRetrievePowerOnTimeInformation();
static Uint16 ei_uiChecksumMatch(Uint16 Input_Array[] ,Uint16 No_Of_Databytes);

//#####################	System Initialisation #####################################################################
// Function Name: void ei_vSystemInit
// Return Type: void
// Arguments:   void
// Description: Initialises the system
//				1. Clock settings for DSP and Flash Initialisation
//				2. Copy Ramfuncs and Flash APIs to RAM
//				2. All pin assignments
//				3. All peripheral assignments
//				4. Interrupts assignments
//				5. Initialisation of global variables
//				6. Initialisation of Onboard ICs
//				7. Calibration routines
//				8. Last settings retrieval from EEPROM
//				9. Check for faults
//				10. Set References
//				11. Enable global interrupts
//#################################################################################################################
void ei_vSystemInit()
{
//  Step 1. Initialise System Control:
//  PLL, WatchDog, enable Peripheral Clocks.
//  This example function is found in the DSP280x_SysCtrl.c file.
    InitSysCtrl();
  	MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
  	MemCopy(&IQmath_loadstart,&IQmath_loadend,&IQmath_runstart);				// Copy IQ math library to RAM
   
//  Step 2. Clear all interrupts and initialise PIE vector table:
//  Disable CPU interrupts 
	DINT;

//  Initialise PIE control registers to their default state.
//  The default state is all PIE interrupts disabled and flags
//  are cleared.  
//  This function is found in the DSP280x_PieCtrl.c file.
    InitPieCtrl();
    InitPieVectTable();
    IER = 0x0000;
    IFR = 0x0000;
    
// 	Call Flash Initialisation to setup flash waitstates
// 	This function must reside in RAM
   	InitFlash();
    
//  Initialise all the Gpios used for this project. This function is found in InitGpio.c
    ei_vPinMapping();
    
//  Initialise all the Gpios used for this project. This function is found in InitPeripherals.c
    ei_vInitPeripherals();
    
    EALLOW;  // This is needed to write to EALLOW protected register
	PieVectTable.EPWM6_INT = &ei_vEPWM6_underflow_ISR;
	PieVectTable.SCIRXINTA = &ei_vSciaFifoRX_ISR;
	PieVectTable.TINT0 = &ei_vCpu_Timer0_ISR;
	EDIS;    // This is needed to disable write to EALLOW protected registers
	PieCtrlRegs.PIEIER3.bit.INTx6 = ENABLE_INT_PIE_LEVEL;	// EPwm6 Underflow scheduler interrupts
	PieCtrlRegs.PIEIER9.bit.INTx1 = ENABLE_INT_PIE_LEVEL;   //Scia Rx interrupts
	PieCtrlRegs.PIEIER1.bit.INTx7 = ENABLE_INT_PIE_LEVEL;	//Timer 0 Interrupt
//
//	IER |= (M_INT1 | M_INT3);	// Enable Group 1, Group 3 Interrupts at the CPU level
	IER |= (M_INT1 | M_INT3 | M_INT9);	// Enable Group 1, Group 3 and Group 9 Interrupts at the CPU level

	StartCpuTimer0();
	
//  Initialise all the global variables used in the project
    ei_vInitializeGlobVars();
    
//	Initialise the State
    ei_vInitState();

    //  Give a 1.0 second delay after booting for the front panel to get power.
    //	This avoids I2C bus getting stuck.
	DELAY_US(500000);
	DELAY_US(500000);

	//  Initialise on board ICs.
	ei_vInitializeAD7705();
	ei_vMax625_X_Y_I2C_initialization(MAX6959A_ADDRESS);
	ei_vMax625_X_Y_I2C_initialization(MAX6958B_ADDRESS);

    //  PSU Start up method
	ei_vRetrievePowerOnTimeInformation();	// Retrieve the power on time
	ei_vProductCalibrationCheck();	// Check for calibration info.
	ei_vRetrieveLastSettings();	// Retrieve last settings from EEPROM.
	ei_vDeviceSpecificCanMailboxInit();	// Mailbox Init based

	// Set PWM References and address, baud rate etc. here depending on last settings.
	ei_vSetReferences();	// Defined in Scheduled_Events.c

	// Hardware latched faults must be ignored till all references set. Therefore fault reset is enabled. Any latched fault occurring
	// before this point because of DSP booting will be ignored. After this fault reset must be disabled.
	// This change initiated because OVP fault was being triggered every time in Auto Restart. The delay is required before disabling reset for the
	// OVP reference set to settle down. An arbitrary 1/2 a second delay has been given.
	DELAY_US(500000);
//	ei_vAdjustOvp();
	DISABLE_FAULT_RESET;

	ei_vCheckFaults();	// This function is defined in Scheduled_Events.c.
	// If any faults are detected the action will be taken by state inside the scheduler.


   	//	Soc External ADC
   	Global_Flags.fg_Active_AD7705_Channel = CH2;
   	Global_Flags.fg_Next_AD7705_Channel = CH2;
   	ei_vSocAD7705(CH2);	// This needs to be here. Will be read in the the scheduler.

//  Reload or restart counters wherever possible
	EPwm6Regs.TBCTR = 0x0000;
    
//  Clear all interrupts before enabling the Global interrupts   	
   	EPwm6Regs.ETCLR.bit.INT = CLEAR_INTERRUPT;
   	SciaRegs.SCIFFRX.bit.RXFFINTCLR = CLEAR_INTERRUPT;
   	
    EINT;
    ERTM;          // Enable Global realtime interrupt DBGM
}

//#####################	Global vars Init ##########################################################################
// Function Name: ei_vInitializeGlobVars
// Return Type: void
// Arguments:   void
// Description: Initialise Global Variables to a known state
//#################################################################################################################
static void ei_vInitializeGlobVars()
{
	Uint16 i;

	//	Initialise all arrays
	for(i = 0; i < 2; i++)
	{
	   External_Rms_Array[i].iq_Filtered_Value = 0;
	   External_Rms_Array[i].iq_Scaled_Filtered_Value = 0;
	   External_Rms_Array[i].iq_ADC_Result_Regs = 0;
	   External_Rms_Array[i].iq_Full_Scale_Value = 0;
	   External_Rms_Array[i].iq_Normalized_Adc_Values = 0;
	   External_Rms_Array[i].iq_Scaled_Unfiltered_Value = 0;
	   External_Rms_Array[i].iq_Calibrated_Value = 0;
	}

	for(i = 0; i < MAX_UART_ARRAY_SIZE; i++)
	{
		Uart_Rx_Data_Array[i] = 0;
	}

	// 	Initialise other communication vars
	ui_Next = 0;

	// General Variables
	iq8_Temperature = 0;

	//	Product Info Initialisation
	Product_Info.ui_Calibration_Status = FACTORY_DEFAULT;
	Product_Info.iq_Voltage_3V_value = 0;
	Product_Info.iq_Current_3V_value = 0;
	Product_Info.iq_Specified_FS_Current = 0;
	Product_Info.iq_Specified_FS_Voltage = 0;
	Product_Info.iq_Rated_Voltage = 0;
	Product_Info.iq_Rated_Current = 0;
	Product_Info.iq_Min_Ovp_Reference = 0;
	Product_Info.iq_Max_Uvl_Reference = 0;
	Product_Info.ui_Qep_Min_OVP_Count = 0;
	Product_Info.iq_Voltage_PWM_Gain = 0;
	Product_Info.iq_Current_PWM_Gain = 0;
	Product_Info.iq_OVP_PWM_Gain = 0;
	Product_Info.ui_Model_Number = 0;
	Product_Info.iq_Voltage_Ref_Offset = 0;
	Product_Info.iq_Voltage_Ref_Gain = 0;
	Product_Info.iq_Current_Ref_Offset = 0;
	Product_Info.iq_Current_Ref_Gain = 0;
	Product_Info.ui_Current_Rise_Time = 0;
	Product_Info.iq_Voltage_Dac_Gain = 0;
	Product_Info.iq_Current_Dac_Gain = 0;
	Product_Info.iq_Voltage_Display_Offset = 0;
	Product_Info.iq_Voltage_Display_Gain = 0;
	Product_Info.iq_Current_Display_Offset = 0;
	Product_Info.iq_Current_Display_Gain = 0;
	Product_Info.ul_Product_Power_On_Time_In_Minutes = 0;

//	Reference Parameters Initialisation
	Reference.iq_Voltage_Reference = 0;
	Reference.iq_Current_Reference = 0;
	Reference.iq_OVP_Reference = 0;
	Reference.iq_UVL_Reference = 0;
	Reference.ui_Set_BaudRate = 0;
	Reference.ui_Set_Address = 0;
	Reference.iq_OVP_Reference_Error = 0;
	Reference.ui_OVP_Reference_Update_Immediate_Flag = 1;
	Reference.fg_Voltage_Reference_Update_Immediate_Flag = 1;
	Reference.fg_Current_Reference_Update_Immediate_Flag = 1;

	// Event Initialisation
	ui_Interrupt_Number = 0;
	ui_Next_Event_In_Line = 0;
	ui_Event_Initiated = 0;
	Event_Array[0].ui_Scheduled_Occurrence = 4;
	Event_Array[0].p_fn_EventDriver = Event8;
	Event_Array[1].ui_Scheduled_Occurrence = 7;
	Event_Array[1].p_fn_EventDriver = Event7;
	Event_Array[2].ui_Scheduled_Occurrence = 10;
	Event_Array[2].p_fn_EventDriver = Event4;
	Event_Array[3].ui_Scheduled_Occurrence = 14;
	Event_Array[3].p_fn_EventDriver = Event8;
	Event_Array[4].ui_Scheduled_Occurrence = 17;
	Event_Array[4].p_fn_EventDriver = Event7;
	Event_Array[5].ui_Scheduled_Occurrence = 20;
	Event_Array[5].p_fn_EventDriver = Event9;
	Event_Array[6].ui_Scheduled_Occurrence = 24;
	Event_Array[6].p_fn_EventDriver = Event8;
	Event_Array[7].ui_Scheduled_Occurrence = 27;
	Event_Array[7].p_fn_EventDriver = Event7;
	Event_Array[8].ui_Scheduled_Occurrence = 30;
	Event_Array[8].p_fn_EventDriver = Event1;
	Event_Array[9].ui_Scheduled_Occurrence = 34;
	Event_Array[9].p_fn_EventDriver = Event8;
	Event_Array[10].ui_Scheduled_Occurrence = 37;
	Event_Array[10].p_fn_EventDriver = Event7;
	Event_Array[11].ui_Scheduled_Occurrence = 42;
	Event_Array[11].p_fn_EventDriver = Event6;
	Event_Array[12].ui_Scheduled_Occurrence = 44;
	Event_Array[12].p_fn_EventDriver = Event8;
	Event_Array[13].ui_Scheduled_Occurrence = 47;
	Event_Array[13].p_fn_EventDriver = Event7;
	Event_Array[14].ui_Scheduled_Occurrence = 54;
	Event_Array[14].p_fn_EventDriver = Event8;
	Event_Array[15].ui_Scheduled_Occurrence = 57;
	Event_Array[15].p_fn_EventDriver = Event7;
	Event_Array[16].ui_Scheduled_Occurrence = 60;
	Event_Array[16].p_fn_EventDriver = Event5;
	Event_Array[17].ui_Scheduled_Occurrence = 64;
	Event_Array[17].p_fn_EventDriver = Event8;
	Event_Array[18].ui_Scheduled_Occurrence = 67;
	Event_Array[18].p_fn_EventDriver = Event7;
	Event_Array[19].ui_Scheduled_Occurrence = 74;
	Event_Array[19].p_fn_EventDriver = Event8;
	Event_Array[20].ui_Scheduled_Occurrence = 77;
	Event_Array[20].p_fn_EventDriver = Event7;
	Event_Array[21].ui_Scheduled_Occurrence = 82;
	Event_Array[21].p_fn_EventDriver = Event6;
	Event_Array[22].ui_Scheduled_Occurrence = 84;
	Event_Array[22].p_fn_EventDriver = Event8;
	Event_Array[23].ui_Scheduled_Occurrence = 87;
	Event_Array[23].p_fn_EventDriver = Event7;
	Event_Array[24].ui_Scheduled_Occurrence = 91;
	Event_Array[24].p_fn_EventDriver = Event3;
	Event_Array[25].ui_Scheduled_Occurrence = 94;
	Event_Array[25].p_fn_EventDriver = Event8;
	Event_Array[26].ui_Scheduled_Occurrence = 97;
	Event_Array[26].p_fn_EventDriver = Event7;
	Event_Array[27].ui_Scheduled_Occurrence = 104;
	Event_Array[27].p_fn_EventDriver = Event8;
	Event_Array[28].ui_Scheduled_Occurrence = 107;
	Event_Array[28].p_fn_EventDriver = Event7;
	Event_Array[29].ui_Scheduled_Occurrence = 110;
	Event_Array[29].p_fn_EventDriver = Event4;
	Event_Array[30].ui_Scheduled_Occurrence = 114;
	Event_Array[30].p_fn_EventDriver = Event8;
	Event_Array[31].ui_Scheduled_Occurrence = 117;
	Event_Array[31].p_fn_EventDriver = Event7;
	Event_Array[32].ui_Scheduled_Occurrence = 122;
	Event_Array[32].p_fn_EventDriver = Event6;
	Event_Array[33].ui_Scheduled_Occurrence = 124;
	Event_Array[33].p_fn_EventDriver = Event8;
	Event_Array[34].ui_Scheduled_Occurrence = 127;
	Event_Array[34].p_fn_EventDriver = Event7;
	Event_Array[35].ui_Scheduled_Occurrence = 130;
	Event_Array[35].p_fn_EventDriver = Event1;
	Event_Array[36].ui_Scheduled_Occurrence = 134;
	Event_Array[36].p_fn_EventDriver = Event8;
	Event_Array[37].ui_Scheduled_Occurrence = 137;
	Event_Array[37].p_fn_EventDriver = Event7;
	Event_Array[38].ui_Scheduled_Occurrence = 144;
	Event_Array[38].p_fn_EventDriver = Event8;
	Event_Array[39].ui_Scheduled_Occurrence = 147;
	Event_Array[39].p_fn_EventDriver = Event7;
	Event_Array[40].ui_Scheduled_Occurrence = 154;
	Event_Array[40].p_fn_EventDriver = Event8;
	Event_Array[41].ui_Scheduled_Occurrence = 157;
	Event_Array[41].p_fn_EventDriver = Event7;
	Event_Array[42].ui_Scheduled_Occurrence = 162;
	Event_Array[42].p_fn_EventDriver = Event6;
	Event_Array[43].ui_Scheduled_Occurrence = 164;
	Event_Array[43].p_fn_EventDriver = Event8;
	Event_Array[44].ui_Scheduled_Occurrence = 167;
	Event_Array[44].p_fn_EventDriver = Event7;
	Event_Array[45].ui_Scheduled_Occurrence = 174;
	Event_Array[45].p_fn_EventDriver = Event8;
	Event_Array[46].ui_Scheduled_Occurrence = 177;
	Event_Array[46].p_fn_EventDriver = Event7;
	Event_Array[47].ui_Scheduled_Occurrence = 184;
	Event_Array[47].p_fn_EventDriver = Event8;
	Event_Array[48].ui_Scheduled_Occurrence = 187;
	Event_Array[48].p_fn_EventDriver = Event7;
	Event_Array[49].ui_Scheduled_Occurrence = 191;
	Event_Array[49].p_fn_EventDriver = Event3;
	Event_Array[50].ui_Scheduled_Occurrence = 194;
	Event_Array[50].p_fn_EventDriver = Event8;
	Event_Array[51].ui_Scheduled_Occurrence = 197;
	Event_Array[51].p_fn_EventDriver = Event7;
	Event_Array[52].ui_Scheduled_Occurrence = 204;
	Event_Array[52].p_fn_EventDriver = Event8;
	Event_Array[53].ui_Scheduled_Occurrence = 207;
	Event_Array[53].p_fn_EventDriver = Event7;
	Event_Array[54].ui_Scheduled_Occurrence = 210;
	Event_Array[54].p_fn_EventDriver = Event4;
	Event_Array[55].ui_Scheduled_Occurrence = 214;
	Event_Array[55].p_fn_EventDriver = Event8;
	Event_Array[56].ui_Scheduled_Occurrence = 217;
	Event_Array[56].p_fn_EventDriver = Event7;
	Event_Array[57].ui_Scheduled_Occurrence = 224;
	Event_Array[57].p_fn_EventDriver = Event8;
	Event_Array[58].ui_Scheduled_Occurrence = 227;
	Event_Array[58].p_fn_EventDriver = Event7;
	Event_Array[59].ui_Scheduled_Occurrence = 230;
	Event_Array[59].p_fn_EventDriver = Event1;
	Event_Array[60].ui_Scheduled_Occurrence = 234;
	Event_Array[60].p_fn_EventDriver = Event8;
	Event_Array[61].ui_Scheduled_Occurrence = 237;
	Event_Array[61].p_fn_EventDriver = Event7;
	Event_Array[62].ui_Scheduled_Occurrence = 244;
	Event_Array[62].p_fn_EventDriver = Event8;
	Event_Array[63].ui_Scheduled_Occurrence = 247;
	Event_Array[63].p_fn_EventDriver = Event7;
	Event_Array[64].ui_Scheduled_Occurrence = 254;
	Event_Array[64].p_fn_EventDriver = Event8;
	Event_Array[65].ui_Scheduled_Occurrence = 257;
	Event_Array[65].p_fn_EventDriver = Event7;
	Event_Array[66].ui_Scheduled_Occurrence = 264;
	Event_Array[66].p_fn_EventDriver = Event8;
	Event_Array[67].ui_Scheduled_Occurrence = 267;
	Event_Array[67].p_fn_EventDriver = Event7;
	Event_Array[68].ui_Scheduled_Occurrence = 274;
	Event_Array[68].p_fn_EventDriver = Event8;
	Event_Array[69].ui_Scheduled_Occurrence = 277;
	Event_Array[69].p_fn_EventDriver = Event7;
	Event_Array[70].ui_Scheduled_Occurrence = 282;
	Event_Array[70].p_fn_EventDriver = Event6;
	Event_Array[71].ui_Scheduled_Occurrence = 284;
	Event_Array[71].p_fn_EventDriver = Event8;
	Event_Array[72].ui_Scheduled_Occurrence = 287;
	Event_Array[72].p_fn_EventDriver = Event7;
	Event_Array[73].ui_Scheduled_Occurrence = 291;
	Event_Array[73].p_fn_EventDriver = Event3;
	Event_Array[74].ui_Scheduled_Occurrence = 294;
	Event_Array[74].p_fn_EventDriver = Event8;
	Event_Array[75].ui_Scheduled_Occurrence = 297;
	Event_Array[75].p_fn_EventDriver = Event7;
	Event_Array[76].ui_Scheduled_Occurrence = 304;
	Event_Array[76].p_fn_EventDriver = Event8;
	Event_Array[77].ui_Scheduled_Occurrence = 307;
	Event_Array[77].p_fn_EventDriver = Event7;
	Event_Array[78].ui_Scheduled_Occurrence = 310;
	Event_Array[78].p_fn_EventDriver = Event4;
	Event_Array[79].ui_Scheduled_Occurrence = 314;
	Event_Array[79].p_fn_EventDriver = Event8;
	Event_Array[80].ui_Scheduled_Occurrence = 317;
	Event_Array[80].p_fn_EventDriver = Event7;
	Event_Array[81].ui_Scheduled_Occurrence = 322;
	Event_Array[81].p_fn_EventDriver = Event6;
	Event_Array[82].ui_Scheduled_Occurrence = 324;
	Event_Array[82].p_fn_EventDriver = Event8;
	Event_Array[83].ui_Scheduled_Occurrence = 327;
	Event_Array[83].p_fn_EventDriver = Event7;
	Event_Array[84].ui_Scheduled_Occurrence = 330;
	Event_Array[84].p_fn_EventDriver = Event1;
	Event_Array[85].ui_Scheduled_Occurrence = 334;
	Event_Array[85].p_fn_EventDriver = Event8;
	Event_Array[86].ui_Scheduled_Occurrence = 337;
	Event_Array[86].p_fn_EventDriver = Event7;
	Event_Array[87].ui_Scheduled_Occurrence = 344;
	Event_Array[87].p_fn_EventDriver = Event8;
	Event_Array[88].ui_Scheduled_Occurrence = 347;
	Event_Array[88].p_fn_EventDriver = Event7;
	Event_Array[89].ui_Scheduled_Occurrence = 354;
	Event_Array[89].p_fn_EventDriver = Event8;
	Event_Array[90].ui_Scheduled_Occurrence = 357;
	Event_Array[90].p_fn_EventDriver = Event7;
	Event_Array[91].ui_Scheduled_Occurrence = 362;
	Event_Array[91].p_fn_EventDriver = Event6;
	Event_Array[92].ui_Scheduled_Occurrence = 364;
	Event_Array[92].p_fn_EventDriver = Event8;
	Event_Array[93].ui_Scheduled_Occurrence = 367;
	Event_Array[93].p_fn_EventDriver = Event7;
	Event_Array[94].ui_Scheduled_Occurrence = 374;
	Event_Array[94].p_fn_EventDriver = Event8;
	Event_Array[95].ui_Scheduled_Occurrence = 377;
	Event_Array[95].p_fn_EventDriver = Event7;
	Event_Array[96].ui_Scheduled_Occurrence = 384;
	Event_Array[96].p_fn_EventDriver = Event8;
	Event_Array[97].ui_Scheduled_Occurrence = 387;
	Event_Array[97].p_fn_EventDriver = Event7;
	Event_Array[98].ui_Scheduled_Occurrence = 391;
	Event_Array[98].p_fn_EventDriver = Event3;
	Event_Array[99].ui_Scheduled_Occurrence = 394;
	Event_Array[99].p_fn_EventDriver = Event8;
	Event_Array[100].ui_Scheduled_Occurrence = 397;
	Event_Array[100].p_fn_EventDriver = Event7;
	Event_Array[101].ui_Scheduled_Occurrence = 402;
	Event_Array[101].p_fn_EventDriver = Event6;
	Event_Array[102].ui_Scheduled_Occurrence = 404;
	Event_Array[102].p_fn_EventDriver = Event8;
	Event_Array[103].ui_Scheduled_Occurrence = 407;
	Event_Array[103].p_fn_EventDriver = Event7;
	Event_Array[104].ui_Scheduled_Occurrence = 410;
	Event_Array[104].p_fn_EventDriver = Event4;
	Event_Array[105].ui_Scheduled_Occurrence = 414;
	Event_Array[105].p_fn_EventDriver = Event8;
	Event_Array[106].ui_Scheduled_Occurrence = 417;
	Event_Array[106].p_fn_EventDriver = Event7;
	Event_Array[107].ui_Scheduled_Occurrence = 424;
	Event_Array[107].p_fn_EventDriver = Event8;
	Event_Array[108].ui_Scheduled_Occurrence = 427;
	Event_Array[108].p_fn_EventDriver = Event7;
	Event_Array[109].ui_Scheduled_Occurrence = 430;
	Event_Array[109].p_fn_EventDriver = Event1;
	Event_Array[110].ui_Scheduled_Occurrence = 434;
	Event_Array[110].p_fn_EventDriver = Event8;
	Event_Array[111].ui_Scheduled_Occurrence = 437;
	Event_Array[111].p_fn_EventDriver = Event7;
	Event_Array[112].ui_Scheduled_Occurrence = 444;
	Event_Array[112].p_fn_EventDriver = Event8;
	Event_Array[113].ui_Scheduled_Occurrence = 447;
	Event_Array[113].p_fn_EventDriver = Event7;
	Event_Array[114].ui_Scheduled_Occurrence = 454;
	Event_Array[114].p_fn_EventDriver = Event8;
	Event_Array[115].ui_Scheduled_Occurrence = 457;
	Event_Array[115].p_fn_EventDriver = Event7;
	Event_Array[116].ui_Scheduled_Occurrence = 464;
	Event_Array[116].p_fn_EventDriver = Event8;
	Event_Array[117].ui_Scheduled_Occurrence = 467;
	Event_Array[117].p_fn_EventDriver = Event7;
	Event_Array[118].ui_Scheduled_Occurrence = 474;
	Event_Array[118].p_fn_EventDriver = Event8;
	Event_Array[119].ui_Scheduled_Occurrence = 477;
	Event_Array[119].p_fn_EventDriver = Event7;
	Event_Array[120].ui_Scheduled_Occurrence = 484;
	Event_Array[120].p_fn_EventDriver = Event8;
	Event_Array[121].ui_Scheduled_Occurrence = 487;
	Event_Array[121].p_fn_EventDriver = Event7;
	Event_Array[122].ui_Scheduled_Occurrence = 491;
	Event_Array[122].p_fn_EventDriver = Event3;
	Event_Array[123].ui_Scheduled_Occurrence = 494;
	Event_Array[123].p_fn_EventDriver = Event8;
	Event_Array[124].ui_Scheduled_Occurrence = 497;
	Event_Array[124].p_fn_EventDriver = Event7;
	Event_Array[125].ui_Scheduled_Occurrence = 500;
	Event_Array[125].p_fn_EventDriver = Event2;

	//	Display keyscan related vars
	ui_Fault_Led_Previous_State = 0;
	ui_Last_Fault_Displayed = 0;

	// Flags Initialisation
	Global_Flags.fg_Scheduled_Event_Flag = 0;
	Global_Flags.fg_Preview_Key_Operation_Mode = 0;
	Global_Flags.fg_Active_AD7705_Channel = 0;
	Global_Flags.fg_Display_FP_Access_Mode = 0;
	Global_Flags.fg_Next_AD7705_Channel = 0;
	Global_Flags.fg_Out_Key_Operation_Mode = 0;
	Global_Flags.fg_Display_PSU_StartMode = 0;
	Global_Flags.fg_Remote_Key_Operation_Mode = 0;
	Global_Flags.fg_Immediate_Update_Sci_Baud = 1;		// Initial Update True
	Global_Flags.fg_Immediate_Update_PSU_Addr = 1;		// Initial Update True
	Global_Flags.fg_Checksum_Included_In_Uart_Comm = 0;
	Global_Flags.fg_Uart_Comm_with_this_PSU = 0;
	Global_Flags.fg_OVP_Remote_control = 0;
	Global_Flags.fg_UVL_Remote_control = 0;
	Global_Flags.fg_Voltage_Remote_control = 0;
	Global_Flags.fg_Current_Remote_control = 0;
	Global_Flags.fg_Rated_Volt_Entered = 0;
	Global_Flags.fg_Rated_Curr_Entered = 0;
	Global_Flags.fg_Specified_Volt_Entered = 0;
	Global_Flags.fg_Specified_Curr_Entered = 0;
	Global_Flags.fg_Previous_Calibration = 0;
	Global_Flags.fg_Current_Calibration_Done = 0;
	Global_Flags.fg_Voltage_Calibration_Done = 0;
	Global_Flags.fg_Voltage_MV1_Entered = 0;
	Global_Flags.fg_Voltage_MV2_Entered = 0;
	Global_Flags.fg_Current_MC1_Entered = 0;
	Global_Flags.fg_Current_MC2_Entered = 0;
	Global_Flags.fg_Output_Switched_On_From_Uart = 0;
	Global_Flags.fg_Output_Switched_On_From_Can = 0;
	Global_Flags.fg_Onboard_OTP_Fault = 0;
	Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse = 1;
	Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse = 1;
	Global_Flags.fg_Query_For_Opc = 0;
	Global_Flags.fg_Date_Entered = 0;
	Global_Flags.fg_Blink_Fold_Led = 0;
	Global_Flags.fg_State_Of_Display = 1;
	Global_Flags.fg_CAN_Srq_Sent = 0;
	Global_Flags.fg_Uart_SRQ_Tranmsit = 0;

	// Register Initialisation
	//  Fault
	Fault_Regs.FAULT_EVENT.all = 0;
	Fault_Regs.FAULT_ENABLE.all = 0;
	Fault_Regs.FAULT_REGISTER.all = 0;

	//	Operation Condition
	Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all = 0;
	Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all = 0;
	Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.all = 0;

	//	Standard Event
	Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.all = 0;
	Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_ENABLE.all = 0;

	//	Service Request
	Service_Request_Regs.STATUS_BYTE_REGISTER.all = 0;
	Service_Request_Regs.STATUS_BYTE_PREVIOUS_REGISTER.all = 0;
	Service_Request_Regs.SERVICE_REQUEST_ENABLE.all = 0;

	//	Encoder Initialisation
	Encoder.ui_Qep_Address_Count = 0;
	Encoder.ui_Qep_BaudRate_Count = 0;
	Encoder.iq_Qep_Current_Count = 0;
	Encoder.iq_Qep_Voltage_Count = 0;
	Encoder.ui_Qep_OVP_Count = 0;
	Encoder.ui_Qep_UVL_Count = 0;

	Encoder.i_CurrentKnob_Coarse_Count = 0;
	Encoder.i_CurrentKnob_Final_Count = 0;
	Encoder.i_CurrentKnob_Previous_Count = 0;
	Encoder.i_CurrentKnob_Fine_Count = QEP_FULL_ROTATIONS_COUNT/2;

	Encoder.i_VoltageKnob_Coarse_Count = 0;
	Encoder.i_VoltageKnob_Final_Count = 0;
	Encoder.i_VoltageKnob_Previous_Count = 0;
	Encoder.i_VoltageKnob_Fine_Count = QEP_FULL_ROTATIONS_COUNT/2;

	//	Timers Initialisation
	// Timers
	Timers.ui_AD7705_DRDY_Pin_Timer = 0;
	Timers.ui_I2C_State_Timer = 0;
	Timers.ui_OVP_UVL_Timer = 0;
	Timers.ui_SPI_State_Timer = 0;
	Timers.ui_Current_Rise_Timer = 0;
	Timers.ui_FoldBack_Delay_Timer = 0;
	Timers.ui_Preview_Operation_Timer = 0;
	Timers.ui_Key_Preview_Press_Timer = 0;
	Timers.ui_Key_Out_Press_Timer = 0;
	Timers.ui_Addr_Baud_Timer = 0;
	Timers.ui_Key_Remote_Press_Timer = 0;
	Timers.ui_MaxFoldBackDelayInCounts = MIN_PROTECTION_DELAY_IN_COUNTS;
	Timers.ui_Onboard_OTP_Timer = 0;
	Timers.ui_EEpromWriteCycle = 6;
	Timers.ui_Fanfail_Delay_Timer = 0;

	// Output Buffer Initialization
	Transmit_Circular_Buffer.i_Front = EMPTY_QUEUE_INDEX;
	Transmit_Circular_Buffer.i_Rear = EMPTY_QUEUE_INDEX;
	for(i = 0; i < TRANSMIT_BUFFER_SIZE; i++)
	{
		Transmit_Circular_Buffer.a_c_TransmitBuffer[i] = '\0';
	}

	// Error Queue Initialisation
	//	Error_Linear_Queue.i_Front = EMPTY_QUEUE_INDEX;
	Error_Linear_Queue.i_Rear = EMPTY_QUEUE_INDEX;
	for(i = 0; i < ERROR_BUFFER_SIZE; i++)
	{
		// Initialise all the pointers to E00[] = "0,\"No Error\""
		Error_Linear_Queue.a_s_p_ErrorBuffer[i] =  (Int8 * )E00;
	}

	// Receive Buffer Initialisation
	Receive_Circular_Buffer.i_Front = EMPTY_QUEUE_INDEX;
	Receive_Circular_Buffer.i_Rear = EMPTY_QUEUE_INDEX;
	for(i = 0; i < RECEIVE_BUFFER_SIZE; i++)
	{
		Receive_Circular_Buffer.a_c_ReceiveBuffer[i] = 0;
	}

	// CAN variables Initialization
	CANFailTrig = 0;
	CANFailCnt = 0;
	CurRefRampDowncnt = 0;
}

//#####################	Product Calibration routines ##############################################################
// Function Name: ei_vProductCalibrationCheck
// Return Type: void
// Arguments:   void
// Description: Psu Calibration routine.
//#################################################################################################################
static void ei_vProductCalibrationCheck()
{
	//TEST_I2CWrite();		// FOR TESTING ONLY

	ei_vReadModelSpecs();
	if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
	{
//		This function is defined in I2CApp.c
//		ei_vOVPCalibration();
		ei_vWriteLastSettingsEEPROM(FACTORY_DEFAULT);
		EEPROM_WRITE_CYCLE_DELAY;
	}
//	ei_vOVPCalibration();

//	ei_vRetrievePowerOnTimeInformation();			// FOR TESTING ONLY
//	ei_vWriteLastSettingsEEPROM(FACTORY_DEFAULT);	// FOR TESTING ONLY
//	EEPROM_WRITE_CYCLE_DELAY;						// FOR TESTING ONLY
}

//#####################	Last settings routine #####################################################################
// Function Name: ei_vRetrieveLastSettings
// Return Type: void
// Arguments:   void
// Description: Retrieve the last saved settings of the PSU from the EEPROM.
//#################################################################################################################
void ei_vRetrieveLastSettings()
{
	//Step 1: Read Last settings value from EEPROM.
	//Step 2: Set global flags depending on last settings.
	Uint16 I2C_Rx_Data_Array[42];
	Uint16 Checksum_Operation_Result = 0;

	while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
	if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, PV_ADDRESS, 42, I2C_Rx_Data_Array)) == FAILURE)
	{
		goto I2C_Fail;
	}

	Checksum_Operation_Result = ei_uiChecksumMatch(I2C_Rx_Data_Array,42);
//	Checksum_Operation_Result = 1;
	if(CHECKSUM_SUCCESS)
	{
		// Read Voltage Settings.
		Encoder.i_VoltageKnob_Coarse_Count = I2C_Rx_Data_Array[0] << 8 | (I2C_Rx_Data_Array[1] & LAST_EIGHT_BITS_ONLY);
		Encoder.i_VoltageKnob_Fine_Count = I2C_Rx_Data_Array[2] << 8 | (I2C_Rx_Data_Array[3] & LAST_EIGHT_BITS_ONLY);
		Encoder.iq_Qep_Voltage_Count = _IQ15(Encoder.i_VoltageKnob_Coarse_Count) + _IQ15mpy(_IQ15(Encoder.i_VoltageKnob_Fine_Count - (QEP_FULL_ROTATIONS_COUNT/2)),IQ_FINE_CALCULATION_CONSTANT);

		// Read Current Settings.
		Encoder.i_CurrentKnob_Coarse_Count = I2C_Rx_Data_Array[4] << 8 | (I2C_Rx_Data_Array[5] & LAST_EIGHT_BITS_ONLY);
		Encoder.i_CurrentKnob_Fine_Count = I2C_Rx_Data_Array[6] << 8 | (I2C_Rx_Data_Array[7] & LAST_EIGHT_BITS_ONLY);
		Encoder.iq_Qep_Current_Count = _IQ15(Encoder.i_CurrentKnob_Coarse_Count) + _IQ15mpy(_IQ15(Encoder.i_CurrentKnob_Fine_Count - (QEP_FULL_ROTATIONS_COUNT/2)),IQ_FINE_CALCULATION_CONSTANT);


		Encoder.ui_Qep_OVP_Count = I2C_Rx_Data_Array[8] << 8 | (I2C_Rx_Data_Array[9] & LAST_EIGHT_BITS_ONLY);
		Encoder.ui_Qep_UVL_Count = I2C_Rx_Data_Array[10] << 8 | (I2C_Rx_Data_Array[11] & LAST_EIGHT_BITS_ONLY);

		// Bit Info Set 1
		State.Mode.bt_Output_Status = I2C_Rx_Data_Array[12] & LAST_ONE_BIT_ONLY;
		State.Mode.bt_Start_Mode = (I2C_Rx_Data_Array[12] >> 2) & LAST_ONE_BIT_ONLY;
		State.Mode.bt_PSU_Control = (I2C_Rx_Data_Array[12] >> 3)& LAST_TWO_BITS_ONLY;
		State.Mode.bt_FrontPanel_Access = (I2C_Rx_Data_Array[12] >> 5) & LAST_ONE_BIT_ONLY;
		State.Mode.bt_Master_Slave_Setting = (I2C_Rx_Data_Array[12] >> 6) & LAST_ONE_BIT_ONLY;
		State.Mode.bt_Power_On_Status_Clear = (I2C_Rx_Data_Array[12] >> 7) & LAST_ONE_BIT_ONLY;

		// Bit Info Set 2
		State.Mode.bt_FoldBack_Mode = I2C_Rx_Data_Array[40] & LAST_TWO_BITS_ONLY;
		State.Mode.bt_Can_SRQ_Switch = (I2C_Rx_Data_Array[40] >> 2) & LAST_ONE_BIT_ONLY;

		Encoder.ui_Qep_Address_Count = I2C_Rx_Data_Array[13] << 8 | (I2C_Rx_Data_Array[14] & LAST_EIGHT_BITS_ONLY);
		Encoder.ui_Qep_BaudRate_Count = I2C_Rx_Data_Array[15] << 8 | (I2C_Rx_Data_Array[16] & LAST_EIGHT_BITS_ONLY);

		Timers.ui_MaxFoldBackDelayInCounts =  I2C_Rx_Data_Array[17];

		// Status Register will reflect the State.Mode bits
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Auto_start_Enabled = State.Mode.bt_Start_Mode;	//(Auto = 1, Safe = 0)
		if(State.Mode.bt_FoldBack_Mode != FOLDBACK_DISARMED)
		{
			Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Fold_Enabled = SET;	//(Fold Enabled = 1, Fold Disabled = 0)
		}
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 1;		// Assume Local mode. (Local = 1)

		// If fold enabled for CC
		if(State.Mode.bt_FoldBack_Mode == FOLDBACK_ARMED_TO_PROTECT_FROM_CC)
		{
			Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Current_Foldback_Mode = SET;
		}
		else	// This bit will be cleared when Protect from CV enabled or Fold Protection disabled
		{
			Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Current_Foldback_Mode = CLEARED;
		}

		// If the PSU in local control then these parameters must be false.
		Global_Flags.fg_OVP_Remote_control = Global_Flags.fg_UVL_Remote_control = Global_Flags.fg_Voltage_Remote_control = Global_Flags.fg_Current_Remote_control = FALSE;

		if(State.Mode.bt_PSU_Control != LOCAL)
		{
			Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;	// If not Local change to remote. (Remote = 0)
			// We need to bring all paramters in REMOTE control
			Global_Flags.fg_OVP_Remote_control = Global_Flags.fg_UVL_Remote_control = Global_Flags.fg_Voltage_Remote_control = Global_Flags.fg_Current_Remote_control = TRUE;

			// Copy the IQ variables retrieved from EEPROM
			Reference.iq_Voltage_Reference = ((int32)I2C_Rx_Data_Array[18] << 24) | ((int32)I2C_Rx_Data_Array[19] << 16) | (I2C_Rx_Data_Array[20] << 8) | I2C_Rx_Data_Array[21];
			Reference.iq_Current_Reference = ((int32)I2C_Rx_Data_Array[22] << 24) | ((int32)I2C_Rx_Data_Array[23] << 16) | (I2C_Rx_Data_Array[24] << 8) | I2C_Rx_Data_Array[25];
			Reference.iq_OVP_Reference = 	  ((int32)I2C_Rx_Data_Array[26] << 24) | ((int32)I2C_Rx_Data_Array[27] << 16) | (I2C_Rx_Data_Array[28] << 8) | I2C_Rx_Data_Array[29];
			Reference.iq_UVL_Reference = 	  ((int32)I2C_Rx_Data_Array[30] << 24) | ((int32)I2C_Rx_Data_Array[31] << 16) | (I2C_Rx_Data_Array[32] << 8) | I2C_Rx_Data_Array[33];
		}

		// Set all the enable registers depending on the bt_Power_On_Status_Clear
		if(State.Mode.bt_Power_On_Status_Clear == ON)
		{
			// All registers will be 0 in this case
			Fault_Regs.FAULT_ENABLE.all = 0;
			Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all = 0;
			Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_ENABLE.all = 0;
			Service_Request_Regs.SERVICE_REQUEST_ENABLE.all = 0;
		}
		else
		{
			// Load the values from EEPROM.
			Fault_Regs.FAULT_ENABLE.all = I2C_Rx_Data_Array[34] << 8 | (I2C_Rx_Data_Array[35] & LAST_EIGHT_BITS_ONLY);;
			Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all = I2C_Rx_Data_Array[36] << 8 | (I2C_Rx_Data_Array[37] & LAST_EIGHT_BITS_ONLY);;
			Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_ENABLE.all = I2C_Rx_Data_Array[38];
			Service_Request_Regs.SERVICE_REQUEST_ENABLE.all = I2C_Rx_Data_Array[39];
		}
	}
	else	// CHECKSUM ERROR or I2C read error. EEPROM CORRUPTED
	{
		I2C_Fail: ;	// goto location in case of failure.
		// Load default values
		Encoder.i_VoltageKnob_Coarse_Count = 0;
		Encoder.i_VoltageKnob_Fine_Count = QEP_FULL_ROTATIONS_COUNT/2;
		Encoder.iq_Qep_Voltage_Count = 0;

		Encoder.i_CurrentKnob_Coarse_Count = 0;
		Encoder.i_CurrentKnob_Fine_Count = QEP_FULL_ROTATIONS_COUNT/2;
		Encoder.iq_Qep_Current_Count = 0;

		Encoder.ui_Qep_OVP_Count = QEP_MAX_OVP_COUNT;
		Encoder.ui_Qep_UVL_Count = 0;
		Encoder.ui_Qep_BaudRate_Count = QEP_DEFAULT_BAUD_RATE_COUNT;

		// All 1 Byte Values.
		State.Mode.bt_Output_Status = OFF;
		State.Mode.bt_FoldBack_Mode = FOLDBACK_DISARMED;
		State.Mode.bt_Start_Mode = SAFE_START;
		Encoder.ui_Qep_Address_Count = QEP_DEFAULT_ADDR_COUNT;
		State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
		State.Mode.bt_FrontPanel_Access = FRONTPANEL_LOCKED;
		State.Mode.bt_Master_Slave_Setting = 0;
		State.Mode.bt_Power_On_Status_Clear = ON;
		Timers.ui_MaxFoldBackDelayInCounts =  MIN_PROTECTION_DELAY_IN_COUNTS;

		// Status Register will reflect the State.Mode bits
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Auto_start_Enabled = State.Mode.bt_Start_Mode;	//(Auto = 1, Safe = 0)
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Fold_Enabled = CLEARED;	//(Fold Enabled = 1, Fold Disabled = 0)
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;

		// We need to bring all paramters in REMOTE control
		Global_Flags.fg_OVP_Remote_control = Global_Flags.fg_UVL_Remote_control = Global_Flags.fg_Voltage_Remote_control = Global_Flags.fg_Current_Remote_control = TRUE;

		// Copy the IQ variables retrieved from EEPROM
		Reference.iq_Voltage_Reference = 0;
		Reference.iq_Current_Reference = 0;
		Reference.iq_OVP_Reference = 0;
		Reference.iq_UVL_Reference = 0;

		// notify external world of EEPROM failure
		Fault_Regs.FAULT_REGISTER.bit.bt_Eeprom_Checksum_Failure = 1;
	}

	State.Mode.bt_Encoder_Rotation_Mode = COARSE_ROTATION;

//	Conditions based on last settings.
	// 1. If in Local lockout mode then bring PSU to Non Latched Remote
	if(State.Mode.bt_PSU_Control == LOCAL_LOCKOUT)
	{
		State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
	}

	// 2. If last setting was OUT on and Auto Start, then OUT on. Else OUT off
	if((State.Mode.bt_Start_Mode == AUTO_RESTART) && (State.Mode.bt_Output_Status == ON))
	{
		State.Mode.bt_Output_Status = ON;
	}
	else
	{
		State.Mode.bt_Output_Status = OFF;
	}

	// Reset Qep Registers
	Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count + (Encoder.i_VoltageKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
	EQep1Regs.QPOSCNT = Encoder.i_VoltageKnob_Coarse_Count;
	EQep1Regs.QPOSINIT = QEP_CTR_SOFTWARE_INIT;

	Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count + (Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
	EQep2Regs.QPOSCNT = Encoder.i_CurrentKnob_Coarse_Count;
	EQep2Regs.QPOSINIT = QEP_CTR_SOFTWARE_INIT;

	Reference.ui_OVP_Reference_Update_Immediate_Flag = TRUE;
	Reference.fg_Voltage_Reference_Update_Immediate_Flag = TRUE;
	Reference.fg_Current_Reference_Update_Immediate_Flag = TRUE;
}

//#####################	Read model specs ##########################################################################
// Function Name: ReadModelSpecs
// Return Type: void
// Arguments:   void
// Description: Read the model specs from EEPROM by reading Rated, Specified and Calibration parameters.

//				Math
//				1. Voltage_3V_Value = (iq_Rated_Voltage * 3)/2.29
//				2. Current_3V_Value = (iq_Rated_Current * 3)/2.29
//				3. iq_Voltage_PWM_Gain = PWM_Counts_Corresponding_to_Rated_Voltage/ iq_Rated_Voltage = 1735/ iq_Rated_Voltage
//				4. iq_OVP_PWM_Gain = PWM_Counts_Corresponding_to_Rated_Voltage/(1.10 * iq_Rated_Voltage) = 6939/iq_Rated_Voltage	(For OVP alone 2.29 corresponds to 110% of RatedVoltage.)
//				5. iq_Current_PWM_Gain = PWM_Counts_Corresponding_to_Rated_Current/ iq_Rated_Voltage = 1735/ iq_Rated_Current
//				6. Min_OVP_Counts = (MIN_OVP_VOLTAGE * 575)/SVUL

//				External ADC
//				1. Rated Voltage corresponds to 1.145( = 2.29/2) in AD7705.
//					=>	Full_Scale_Voltage ( = 65535 counts or 1.2497 V) = 1.2497 * SVUL/1.145

//				2. Same as above except Full_Scale_Voltage = 1.2497 * SCUL/1.145

//				Internal ADC
//				1. Current and voltage iq_Full_Scale_Value corresponds to 3V Value.
//				2. For OVP alone iq_Full_Scale_Value corresponds to (3V Value * 1.1)
//
// 				Min_Ovp_Reference and Min_Ovp_Counts not stored in EEPROM so we use a switch case for Model no. to calculate these
//				parameters.
//#################################################################################################################
static void ei_vReadModelSpecs()
{
	Uint16 I2C_Rx_Data_Array[47];
	int32 Temp_Storage;
	Uint16 Checksum_Operation_Result = 0;

	// Read till the 1st checksum
	while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
	if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, MODEL_NO_ADDR, 47, I2C_Rx_Data_Array)) == FAILURE)
	{
		goto Default_Model;
	}

	// First operate on 1st 30 bytes till Current gain
	Checksum_Operation_Result = ei_uiChecksumMatch(I2C_Rx_Data_Array,47);
	if((CHECKSUM_SUCCESS) || (I2C_Rx_Data_Array[0] == DEFAULT_1V_1A_MODEL))	// Ignore checksum result for a default power supply.
	{
		if(I2C_Rx_Data_Array[0] != DEFAULT_1V_1A_MODEL)
		{
			// Product number
			Product_Info.ui_Model_Number = I2C_Rx_Data_Array[0];

			// For Rated Voltage
			Temp_Storage = (I2C_Rx_Data_Array[1] >> 4) * 100 + (I2C_Rx_Data_Array[1] & LAST_FOUR_BITS) * 10 + (I2C_Rx_Data_Array[2] >> 4);
			Temp_Storage = _IQ15(Temp_Storage);
			Temp_Storage += (_IQ15div(_IQ15((I2C_Rx_Data_Array[2] & LAST_FOUR_BITS)),_IQ15(10))
							 + _IQ15div(_IQ15((I2C_Rx_Data_Array[3] >> 4)),_IQ15(100))
							 + _IQ15div(_IQ15((I2C_Rx_Data_Array[3] & LAST_FOUR_BITS)),_IQ15(1000))); // 3 digits after decimal point
			Product_Info.iq_Rated_Voltage = Temp_Storage;

			// For Rated current
			Temp_Storage = (I2C_Rx_Data_Array[4] >> 4) * 100 + (I2C_Rx_Data_Array[4] & LAST_FOUR_BITS) * 10 + (I2C_Rx_Data_Array[5] >> 4);
			Temp_Storage = _IQ15(Temp_Storage);
			Temp_Storage += (_IQ15div(_IQ15((I2C_Rx_Data_Array[5] & LAST_FOUR_BITS)),_IQ15(10))
							 + _IQ15div(_IQ15((I2C_Rx_Data_Array[6] >> 4)),_IQ15(100))
							 + _IQ15div(_IQ15((I2C_Rx_Data_Array[6] & LAST_FOUR_BITS)),_IQ15(1000))); // 3 digits after decimal point
			Product_Info.iq_Rated_Current = Temp_Storage;

			// For SVUL
			Temp_Storage = (I2C_Rx_Data_Array[7] >> 4) * 100 + (I2C_Rx_Data_Array[7] & LAST_FOUR_BITS) * 10 + (I2C_Rx_Data_Array[8] >> 4);
			Temp_Storage = _IQ15(Temp_Storage);
			Temp_Storage += (_IQ15div(_IQ15((I2C_Rx_Data_Array[8] & LAST_FOUR_BITS)),_IQ15(10))
							 + _IQ15div(_IQ15((I2C_Rx_Data_Array[9] >> 4)),_IQ15(100))
							 + _IQ15div(_IQ15((I2C_Rx_Data_Array[9] & LAST_FOUR_BITS)),_IQ15(1000))); // 3 digits after decimal point
			Product_Info.iq_Specified_FS_Voltage = Temp_Storage;

			// For SCUL
			Temp_Storage = (I2C_Rx_Data_Array[10] >> 4) * 100 + (I2C_Rx_Data_Array[10] & LAST_FOUR_BITS) * 10 + (I2C_Rx_Data_Array[11] >> 4);
			Temp_Storage = _IQ15(Temp_Storage);
			Temp_Storage += (_IQ15div(_IQ15((I2C_Rx_Data_Array[11] & LAST_FOUR_BITS)),_IQ15(10))
							 + _IQ15div(_IQ15((I2C_Rx_Data_Array[12] >> 4)),_IQ15(100))
							 + _IQ15div(_IQ15((I2C_Rx_Data_Array[12] & LAST_FOUR_BITS)),_IQ15(1000))); // 3 digits after decimal point
			Product_Info.iq_Specified_FS_Current = Temp_Storage;
/*			// For Rated Voltage
			Temp_Storage = (I2C_Rx_Data_Array[1] >> 4) * 100 + (I2C_Rx_Data_Array[1] & 0x000F) * 10 + (I2C_Rx_Data_Array[2] >> 4);
			Temp_Storage = _IQ15(Temp_Storage);
			Temp_Storage += (_IQ15mpy(_IQ15((I2C_Rx_Data_Array[2] & 0x000F)),_IQ15(0.1))
							 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[3] >> 4)),_IQ15(0.01))
							 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[3] & 0x000F)),_IQ15(0.001))); // 3 digits after decimal point
			Product_Info.iq_Rated_Voltage = Temp_Storage;

			// For Rated current
			Temp_Storage = (I2C_Rx_Data_Array[4] >> 4) * 100 + (I2C_Rx_Data_Array[4] & 0x000F) * 10 + (I2C_Rx_Data_Array[5] >> 4);
			Temp_Storage = _IQ15(Temp_Storage);
			Temp_Storage += (_IQ15mpy(_IQ15((I2C_Rx_Data_Array[5] & 0x000F)),_IQ15(0.1))
							 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[6] >> 4)),_IQ15(0.01))
							 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[6] & 0x000F)),_IQ15(0.001))); // 3 digits after decimal point
			Product_Info.iq_Rated_Current = Temp_Storage;

			// For SVUL
			Temp_Storage = (I2C_Rx_Data_Array[7] >> 4) * 100 + (I2C_Rx_Data_Array[7] & 0x000F) * 10 + (I2C_Rx_Data_Array[8] >> 4);
			Temp_Storage = _IQ15(Temp_Storage);
			Temp_Storage += (_IQ15mpy(_IQ15((I2C_Rx_Data_Array[8] & 0x000F)),_IQ15(0.1))
							 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[9] >> 4)),_IQ15(0.01))
							 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[9] & 0x000F)),_IQ15(0.001))); // 3 digits after decimal point
			Product_Info.iq_Specified_FS_Voltage = Temp_Storage;

			// For SCUL
			Temp_Storage = (I2C_Rx_Data_Array[10] >> 4) * 100 + (I2C_Rx_Data_Array[10] & 0x000F) * 10 + (I2C_Rx_Data_Array[11] >> 4);
			Temp_Storage = _IQ15(Temp_Storage);
			Temp_Storage += (_IQ15mpy(_IQ15((I2C_Rx_Data_Array[11] & 0x000F)),_IQ15(0.1))
							 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[12] >> 4)),_IQ15(0.01))
							 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[12] & 0x000F)),_IQ15(0.001))); // 3 digits after decimal point
			Product_Info.iq_Specified_FS_Current = Temp_Storage;*/

			// Retrieve Calibration Values
			Product_Info.iq_Voltage_Ref_Offset = ((int32)I2C_Rx_Data_Array[13] << 24) | ((int32)I2C_Rx_Data_Array[14] << 16) | (I2C_Rx_Data_Array[15] << 8) | I2C_Rx_Data_Array[16];
			Product_Info.iq_Voltage_Ref_Gain = ((int32)I2C_Rx_Data_Array[17] << 24) | ((int32)I2C_Rx_Data_Array[18] << 16) | (I2C_Rx_Data_Array[19] << 8) | I2C_Rx_Data_Array[20];
			Product_Info.iq_Voltage_Display_Offset = ((int32)I2C_Rx_Data_Array[21] << 24) | ((int32)I2C_Rx_Data_Array[22] << 16) | (I2C_Rx_Data_Array[23] << 8) | I2C_Rx_Data_Array[24];
			Product_Info.iq_Voltage_Display_Gain = ((int32)I2C_Rx_Data_Array[25] << 24) | ((int32)I2C_Rx_Data_Array[26] << 16) | (I2C_Rx_Data_Array[27] << 8) | I2C_Rx_Data_Array[28];
			Product_Info.iq_Current_Ref_Offset = ((int32)I2C_Rx_Data_Array[29] << 24) | ((int32)I2C_Rx_Data_Array[30] << 16) | (I2C_Rx_Data_Array[31] << 8) | I2C_Rx_Data_Array[32];
			Product_Info.iq_Current_Ref_Gain = ((int32)I2C_Rx_Data_Array[33] << 24) | ((int32)I2C_Rx_Data_Array[34] << 16) | (I2C_Rx_Data_Array[35] << 8) | I2C_Rx_Data_Array[36];
			Product_Info.iq_Current_Display_Offset = ((int32)I2C_Rx_Data_Array[37] << 24) | ((int32)I2C_Rx_Data_Array[38] << 16) | (I2C_Rx_Data_Array[39] << 8) | I2C_Rx_Data_Array[40];
			Product_Info.iq_Current_Display_Gain = ((int32)I2C_Rx_Data_Array[41] << 24) | ((int32)I2C_Rx_Data_Array[42] << 16) | (I2C_Rx_Data_Array[43] << 8) | I2C_Rx_Data_Array[44];
			Product_Info.ui_Calibration_Status = ONE_TIME_CALIBRATED;
		}
		else	// Uncalibrated factory default
		{
			goto Default_Model;
		}

		switch(Product_Info.ui_Model_Number)
		{

			case MODEL_NO_10_700:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_10_700),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_10_700);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_10_700_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_20_500:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_20_500),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_20_500);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_20_500_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_30_334:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_30_334),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_30_334);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_30_334_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_40_250:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_40_250),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_40_250);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_40_250_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_50_200:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_50_200),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_50_200);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_50_200_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_60_167:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_60_167),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_60_167);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_60_167_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_68_150:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_68_150),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_68_150);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_68_150_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_80_125:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_80_125),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_80_125);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_80_125_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_100_100:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_100_100),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_100_100);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_100_100_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_150_67:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_150_67),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_150_67);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_150_67_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_300_34:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_300_34),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_300_34);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_300_34_CURRENT_RISE_TIME;
				break;
			}

			case MODEL_NO_400_25:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_400_25),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_400_25);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_400_25_CURRENT_RISE_TIME;
				break;
			}
			case MODEL_NO_600_17:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_600_17),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_600_17);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_600_17_CURRENT_RISE_TIME;
				break;
			}

			// Custom Models start from here
			case MODEL_NO_450_23:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_450_23),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_450_23);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_450_23_CURRENT_RISE_TIME;
				break;
			}
			case MODEL_NO_48_27:
			{
				// Min OVP in Qep Counts and iq Calculation
				Product_Info.ui_Qep_Min_OVP_Count = _IQ15rmpy(_IQ15div(_IQ15(MIN_OVP_48_27),Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
				Product_Info.iq_Min_Ovp_Reference = _IQ15(MIN_OVP_48_27);

				//Retrieve Model Dependent Timers
				Product_Info.ui_Current_Rise_Time = MODEL_48_27_CURRENT_RISE_TIME;
				break;
			}


			default:
			{
				break;
			}
		}
	}
	else	// CHECKSUM ERROR. EEPROM FAILED
	{
		// Notify EEPROM error through external communication
		Fault_Regs.FAULT_REGISTER.bit.bt_Eeprom_Checksum_Failure = 1;

		// GOTO Location for default model
		Default_Model: ;

		Product_Info.ui_Calibration_Status = FACTORY_DEFAULT;

		// Load Default Values
		Product_Info.ui_Model_Number = DEFAULT_1V_1A_MODEL;
		Product_Info.iq_Rated_Voltage = _IQ15(RATED_VOLTAGE_1_1);
		Product_Info.iq_Rated_Current = _IQ15(RATED_CURRENT_1_1);

		Product_Info.iq_Specified_FS_Voltage = _IQ15(RATED_VOLTAGE_1_1);
		Product_Info.iq_Specified_FS_Current = _IQ15(RATED_CURRENT_1_1);

		// Min OVP Counts Calculation
		Product_Info.ui_Qep_Min_OVP_Count = 0;
		Product_Info.iq_Min_Ovp_Reference = 0;

		// Max UVL Counts Calculation
		Product_Info.iq_Max_Uvl_Reference = 0;				// No calculation for default.

		//Default Calibration Values
		Product_Info.iq_Voltage_Ref_Offset = 0;
		Product_Info.iq_Voltage_Ref_Gain = _IQ15(UNITY);
		Product_Info.iq_Current_Ref_Offset = 0;
		Product_Info.iq_Current_Ref_Gain = _IQ15(UNITY);
		Product_Info.iq_Voltage_Display_Offset = 0;
		Product_Info.iq_Voltage_Display_Gain = _IQ15(UNITY);
		Product_Info.iq_Current_Display_Offset = 0;
		Product_Info.iq_Current_Display_Gain = _IQ15(UNITY);

		Product_Info.ui_Current_Rise_Time = 0;

		Product_Info.ui_Calibration_Status = FACTORY_DEFAULT;
	}

	// Calculate Voltage and Current PWM Gain required for CMPA and CMPB values.

	// Gain calcultaion using 3V math.
	Product_Info.iq_Voltage_3V_value = _IQ15mpy(Product_Info.iq_Rated_Voltage,_IQ15(CONVERSION_FACTOR_2P29V_TO_3V));
	Product_Info.iq_Current_3V_value = _IQ15mpy(Product_Info.iq_Rated_Current,_IQ15(CONVERSION_FACTOR_2P29V_TO_3V));

	// Gain calculation using 2.2902V math. Provides better programming accuracy.
	Product_Info.iq_Voltage_PWM_Gain = _IQ15div(_IQ15(PWM_COUNTS_CORRESPONDING_TO_RATED_VALUE),Product_Info.iq_Rated_Voltage);
	Product_Info.iq_Current_PWM_Gain = _IQ15div(_IQ15(PWM_COUNTS_CORRESPONDING_TO_RATED_VALUE),Product_Info.iq_Rated_Current);
	Product_Info.iq_OVP_PWM_Gain = _IQ15div(_IQ15(PWM_COUNTS_CORRESPONDING_TO_OVP_RATED_VALUE),_IQ15mpy(_IQ15(1.1),Product_Info.iq_Rated_Voltage));

	// Dac gains will have to be in IQ15 only to accommodate 65536
	Product_Info.iq_Voltage_Dac_Gain = _IQ15div(_IQ15(DAC_COUNTS_CORRESPONDING_TO_RATED_VALUE),Product_Info.iq_Rated_Voltage);
	Product_Info.iq_Current_Dac_Gain = _IQ15div(_IQ15(DAC_COUNTS_CORRESPONDING_TO_RATED_VALUE),Product_Info.iq_Rated_Current);

	// Internal Adc Initialization
//	Internal_Rms_Array[VOLTAGE_REF_FEEDBACK].iq_Full_Scale_Value = Product_Info.iq_Voltage_3V_value;
//	Internal_Rms_Array[VOLTAGE_OVER_VOLTAGE_REF_FEEDBACK].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Voltage_3V_value,_IQ15(1.1));
//	Internal_Rms_Array[VOLTAGE_ACTUAL_FEEDBACK].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Voltage_3V_value,_IQ15(CONVERSION_FACTOR_1P145V_TO_3V));
//
//	Internal_Rms_Array[CURRENT_REF_FEEDBACK].iq_Full_Scale_Value = Product_Info.iq_Current_3V_value;
//	Internal_Rms_Array[CURRENT_ACTUAL_FEEDBACK].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Current_3V_value,_IQ15(CONVERSION_FACTOR_1P145V_TO_3V));

	// External Adc Initialization
	External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Rated_Current, _IQ15(CONVERSION_FACTOR_1P145V_TO_1P25V));
	External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Rated_Voltage, _IQ15(CONVERSION_FACTOR_1P145V_TO_1P25V));

	// Max UVL possible is 0.95 * SVUL
	Product_Info.iq_Max_Uvl_Reference = _IQ15rmpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(0.95));
}

//#####################	Retrieve Time On Value ####################################################################
// Function Name: ei_vRetrievePowerOnTimeInformation
// Return Type: void
// Arguments:   void
// Description: Read the previously stored time On value from EEPROM location. It is a set of values stored at the memory
//              locations immediately after last settings checksum.

// The first location in this set stores a byte key PRODUCT_NOT_SWITCHED_ON_FOR_FIRST_TIME which tells the uC
// whether the product has already been switched on before. Every time an AC fail occurs the DSP writes this KEY along with
// the present count of timer.
// When PSU switched on for the very first time this value will be 0xFF (because of new EEPROM). In that case
// the timer is initialised to 0. The key is set to PRODUCT_NOT_SWITCHED_ON_FOR_FIRST_TIME.

// The TIMER itself is stored in 4 bytes after the Key which makes it a Uint32 value. The stored value denotes
// the no. of minutes the PSU has remained ON.
// Theoretical limit : (2^32 - 1) minutes is approx. 49029 decades!!!!!!

// The final value stores the checksum of the above 5 locations. ( The timer set is different to last settings
//	and hence both sets have different checksums stored separately.

// Another important thing: In case of data corruption because of checksum failure we initialise timer To a
// BASE_TIME_ON_VALUE_IN_CASE_OF_DATA_FAILURE. If the user reads a time ON value equal to or greater than this value
// then the information is not reliable.
// This fault denotes that either EEPROM corrupted or I2C bus fails.
//#################################################################################################################
static void ei_vRetrievePowerOnTimeInformation()
{
	Uint16 a_ui_I2cRxDataArray[6],
		   a_ui_I2cTxDataArray[6],
		   Checksum_Operation_Result;

	while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
	if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS,POWER_ON_TIME_STORAGE_ADDR,6,a_ui_I2cRxDataArray)) == FAILURE)
	{
		// FAILURE CONDIION.. COMMUNICATION WITH EEPROM TYPICALLY NEVER FAILS...
		Product_Info.ul_Product_Power_On_Time_In_Minutes = BASE_TIME_ON_VALUE_IN_CASE_OF_DATA_FAILURE;
		return;
	}

	Checksum_Operation_Result = ei_uiChecksumMatch(a_ui_I2cRxDataArray,6);
	if(CHECKSUM_SUCCESS)
	{
		// Check if the product is not being switched ON for the first time. Just a safety. If
		// checksum has passed then it means that time values have already been entered in the EEPROM previous time
		if(a_ui_I2cRxDataArray[0] == PRODUCT_NOT_SWITCHED_ON_FOR_FIRST_TIME)
		{
			// Read the Previously stored time on value
			Product_Info.ul_Product_Power_On_Time_In_Minutes = ((Uint32)a_ui_I2cRxDataArray[1] << 24) | ((Uint32)a_ui_I2cRxDataArray[2] << 16) | (a_ui_I2cRxDataArray[3] << 8) | a_ui_I2cRxDataArray[4];
		}
	}
	else	// A Checksum failure
	{
		// A checksum failure could mean that PSU switched on for first time (or new EEPROM).
		// Check if the product is being switched ON for the first time
		if(PRODUCT_SWITCHED_ON_FOR_FIRST_TIME)	//if(a_ui_I2cRxDataArray[0] == 0xFF)
		{
			// The product has been switched for the first time.
			// Write the following values in EEPROM.

//			The key that tells the code that the PSU is not new, when switched On next time
			a_ui_I2cTxDataArray[0] = PRODUCT_NOT_SWITCHED_ON_FOR_FIRST_TIME;

			// Reset the timer to 0
			Product_Info.ul_Product_Power_On_Time_In_Minutes = 0;

			// All locations 0
			a_ui_I2cTxDataArray[1] = 0x00;
			a_ui_I2cTxDataArray[2] = 0x00;
			a_ui_I2cTxDataArray[3] = 0x00;
			a_ui_I2cTxDataArray[4] = 0x00;

			// Checksum is the same as key because all the remaining locations are 0
			a_ui_I2cTxDataArray[5] = PRODUCT_NOT_SWITCHED_ON_FOR_FIRST_TIME;

			// Write this value in EEPROM
			ei_uiI2CWrite(EEPROM_24LC256_ADDRESS,POWER_ON_TIME_STORAGE_ADDR,6,a_ui_I2cTxDataArray);
			EEPROM_WRITE_CYCLE_DELAY;

		}
		else	// Now this denotes a true EEPROM Failure.
		{
			Product_Info.ul_Product_Power_On_Time_In_Minutes = BASE_TIME_ON_VALUE_IN_CASE_OF_DATA_FAILURE;
			// Notify EEPROM error through external communication
			Fault_Regs.FAULT_REGISTER.bit.bt_Eeprom_Checksum_Failure = 1;
		}
	}
}

//#####################	Checksum matcher ##########################################################################
// Function Name: ChecksumMatch
// Return Type: Uint16
// Arguments:   Uint16 Input_Array[] ,Uint16 No_Of_Databytes
// Description: Takes an input array(that should be read from EEPROM) in which the last location must contain the
//				last 2 nibbles of the sum of all other array elements.
// 				It calculates the sum independently and checks to see if the value matches with the one in array.
//				It returns 1 or 0 depending on success and failure of the match.
//#################################################################################################################
static Uint16 ei_uiChecksumMatch(Uint16 Input_Array[] ,Uint16 No_Of_Databytes)
{
	Uint16 i = 0;
	Uint16 Checksum = 0;

	for( i = 0; i < No_Of_Databytes - 1; i++)
	{
		Checksum += Input_Array[i];
	}

	if(Input_Array[No_Of_Databytes - 1] == (Checksum & 0x00FF))
	{
		return SUCCESS;
	}
	else
	{
		return FAILURE;
	}
}
