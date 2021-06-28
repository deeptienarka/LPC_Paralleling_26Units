//#################################################################################################################
//# 							enArka Instruments proprietary
//# File: ei_State.c
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.
//# All Rights Reserved
//#
//#################################################################################################################

//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
#include "ExternalVariables.h"
//#################################################################################################################
									/*------- Global Prototypes ----------*/
// void ei_vCheckFaults();
// void ei_vInitState();
// void ei_vState();
//void ei_vClearPsuStatusStructure();
//void ei_vResetPsuToSafeState();
//#################################################################################################################
									/*-------Local Prototypes----------*/
static void ei_vNextState();
static void ei_vStateCycleFlagsReset();

//#####################	Check for Faults ##########################################################################
// Function Name: ei_vCheckFaults
// Return Type: void
// Arguments:   void
// Description: Check Faults and turn ON if all faults are cleared and PSU is in AutoStart mode.
//				IMPORTANT NOTE: A user trying to switch the output off raises by front panel or from remote
//				raises the out fault.

				// Fault Description
//	NAME	DESCRIPTION						LATCHING	ALM		DISPLAY			RECOVER BY 'SO'			SIGNAL TYPE
//	AC		AC Fail						 	Non Latch	Blink	"AC""FLt"		No (h/w recovers)		Active High
//	OTP		over temperature protection		Non Latch	Blink	"OTP""FLt"		No (h/w	recovers)		Active High
//	FOLD	FoldBack protection		 		Latch		Blink	"FOLD""FLt"		Yes						Software
//	OVP		over voltage protection		 	Latch		Blink	"OUP""FLt"		Yes						Active High
//	SO		Shut Off		 				Non Latch	Off		"SHUT""OFF"		Yes						Active High
//	OFF		Output Off(Front Panel)		 	Latch		Off		"OFF"			No						Software
//	ENA		Enable Open					 	Non Latch	Blink	"ENA""FLt"		No (h/w recovers)		Active High
//	FAN		Fan Fail						Non Latch	Blink	"Fan""Flt"		No (h/w recovers)		Active Low
//	PFC		Pfc dc bus fail					Non Latch	Blink					No (h/w recovers)		Active High
//	COM		Internal Com Fail				Latch		Blink 	BLANK			No (Reboot)				Software
//	CHK		Eeprom Checksum Fail			Latch		Blink	BLANK			No (Reboot)				Software
//	ADC		Onboard ADC fail				Latch		Blink	BLANK			No (Reboot)				Software
//	ILL		Illegal Math Fault				Latch		Blink	BLANK			No (Reboot)				Software
//#################################################################################################################
void ei_vCheckFaults()
{
	// Faults with Optional latching
	////////////////////////////////// OTP FAULT //////////////////////////////////////////////////////////////////
	if(OTP_FAULT)
	{
		ei_vEnqueueFaultRegsInErrorQueue(OTP_FAULT_FLAG,E322);
//		Fault_Regs.FAULT_REGISTER.bit.bt_Otp_Fault = SET;
	}
	else
	{
		if(State.Mode.bt_Start_Mode == SAFE_START)
		{
			if(State.Present_St == EI_ST_STANDBY)	// Fault latched on the display. Fault bit can be cleared only from standby state
			{
				Fault_Regs.FAULT_REGISTER.bit.bt_Otp_Fault = CLEARED;
			}
		}
		else	// Fault bit cleared when the fault clears in the system.
		{
			Fault_Regs.FAULT_REGISTER.bit.bt_Otp_Fault = CLEARED;
		}
	}

	////////////////////////////////// AC FAULT ///////////////////////////////////////////////////////////////////
	if(AC_FAULT)
	{
		ei_vEnqueueFaultRegsInErrorQueue(AC_FAULT_FLAG,E321);
		ei_vWriteLastSettingsEEPROM(Product_Info.ui_Calibration_Status);
//		Fault_Regs.FAULT_REGISTER.bit.bt_AC_Fault = SET;
	}
	else
	{
		if(State.Mode.bt_Start_Mode == SAFE_START)
		{
			if(State.Present_St == EI_ST_STANDBY)	// Fault latched on the display. Fault can be cleared only from standby state
			{
				Fault_Regs.FAULT_REGISTER.bit.bt_AC_Fault = CLEARED;
			}
		}
		else	// Fault bit cleared when the fault clears in the system.
		{
				Fault_Regs.FAULT_REGISTER.bit.bt_AC_Fault = CLEARED;
		}
	}

	////////////////////////////////// INTERLOCK FAULT ////////////////////////////////////////////////////////////
	if(INTERLOCK_FAULT)
	{
		ei_vEnqueueFaultRegsInErrorQueue(INTERLOCK_FAULT_FLAG,E327);
//		Fault_Regs.FAULT_REGISTER.bit.bt_Interlock_Fault = SET;
	}
	else
	{
		if(State.Mode.bt_Start_Mode == SAFE_START)
		{
			if(State.Present_St == EI_ST_STANDBY)	// Fault latched on the display. Fault can be cleared only from standby state
			{
				Fault_Regs.FAULT_REGISTER.bit.bt_Interlock_Fault = CLEARED;
			}
		}
		else	// Fault bit cleared when the fault clears in the system.
		{
				Fault_Regs.FAULT_REGISTER.bit.bt_Interlock_Fault = CLEARED;
		}
	}

	// Faults that are always latched
	////////////////////////////////// OVP FAULT //////////////////////////////////////////////////////////////////
	if(OVP_FAULT)
	{
		ei_vEnqueueFaultRegsInErrorQueue(OVP_FAULT_FLAG,E324);
//		Fault_Regs.FAULT_REGISTER.bit.bt_Ovp_Fault = SET;
	}
	else
	{
		if(State.Present_St == EI_ST_STANDBY)	// Fault latched on the display. Fault can be cleared only from standby state
		{
			Fault_Regs.FAULT_REGISTER.bit.bt_Ovp_Fault = CLEARED;
		}
	}

	////////////////////////////////// FOLDBACK FAULT /////////////////////////////////////////////////////////////
	// Foldback can be enabled for only CC
	if((CC_MODE)&&(OUTPUT_ON))	// CC Mode
	{
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Constant_Current = 1;
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Constant_Voltage = 0;

		// If Fold protection enabled for CC and current rise time expires it's a fault.
		if((State.Mode.bt_FoldBack_Mode == FOLDBACK_ARMED_TO_PROTECT_FROM_CC) && (CURRENT_RISE_TIMER_EXPIRED))
		{
			// wait till foldback delay time completed
			if(FOLDBACK_DELAY_TIME_EXPIRED)
			{
				ei_vEnqueueFaultRegsInErrorQueue(FOLD_FAULT_FLAG,E323);
//				Fault_Regs.FAULT_REGISTER.bit.bt_Fold_Fault = SET;
			}
			else
			{
				INCREMENT_FOLDBACK_DELAY_TIMER;
			}
		}
	}
	else if((CV_MODE)&&(OUTPUT_ON))	// CV Mode
	{
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Constant_Voltage = 1;
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Constant_Current = 0;

		if(State.Present_St == EI_ST_STANDBY)	// Fault latched on the display. Fault can be cleared only from standby state
		{
			Fault_Regs.FAULT_REGISTER.bit.bt_Fold_Fault = CLEARED;
		}
		RESET_FOLD_BACK_DELAY_TIMER;
	}
	else // OUTPUT OFF.
	{
		if(State.Present_St == EI_ST_STANDBY)	// Fault latched on the display. Fault can be cleared only from standby state
		{
			Fault_Regs.FAULT_REGISTER.bit.bt_Fold_Fault = CLEARED;
		}

		// Set both CV and CC bits to 0
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Constant_Current = CLEARED;
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Constant_Voltage = CLEARED;
		RESET_FOLD_BACK_DELAY_TIMER;
	}

	////////////////////////////////// OFF FAULT //////////////////////////////////////////////////////////////////
	if(OUTPUT_OFF_FAULT)
	{
		ei_vEnqueueFaultRegsInErrorQueue(OUT_FAULT_FLAG,E326);
//		Fault_Regs.FAULT_REGISTER.bit.bt_Out_Fault = SET;
	}
	else
	{
		Fault_Regs.FAULT_REGISTER.bit.bt_Out_Fault = CLEARED;
	}

	////////////////////////////////// PFC FAULT //////////////////////////////////////////////////////////////////
	if(PFC_DC_BUS_FAULT)
	{
		ei_vEnqueueFaultRegsInErrorQueue(PFC_FAULT_FLAG,E312);
//		Fault_Regs.FAULT_REGISTER.bit.bt_PFC_DC_Bus_Fault = SET;
	}
	else
	{
		if(State.Present_St == EI_ST_STANDBY)	// Fault latched on the display. Fault can be cleared only from standby state
		{
			Fault_Regs.FAULT_REGISTER.bit.bt_PFC_DC_Bus_Fault = CLEARED;
		}
	}

	////////////////////////////////// FAN FAULT //////////////////////////////////////////////////////////////////
	/*if(FAN_FAIL)
	{
		// wait till Fan fail delay time completed
		if(FANFAIL_DELAY_TIME_EXPIRED)
		{
//			ei_vEnqueueFaultRegsInErrorQueue(FAN_FAULT_FLAG,E309);
			Fault_Regs.FAULT_REGISTER.bit.bt_Fan_Fault = SET;
		}
		else
		{
			INCREMENT_FANFAIL_DELAY_TIMER;
		}
	}
	else
	{
		if(State.Present_St == EI_ST_STANDBY)	// Fault latched on the display. Fault can be cleared only from standby state
		{
			Fault_Regs.FAULT_REGISTER.bit.bt_Fan_Fault = CLEARED;
		}
		RESET_FANFAIL_DELAY_TIMER;
	}*/

	////////////////////////////////// SHUTOFF FAULT //////////////////////////////////////////////////////////////
	// ShutOff Fault bit cleared as soon as it clears from the system
	if(SHUTOFF_FAULT)
	{
		ei_vEnqueueFaultRegsInErrorQueue(SHUTOFF_FAULT_FLAG,E325);
//		Fault_Regs.FAULT_REGISTER.bit.bt_ShutOff_Fault = SET;
	}
	else
	{
		Fault_Regs.FAULT_REGISTER.bit.bt_ShutOff_Fault = CLEARED;
	}

	// The Next 3 Faults are not set here. But we just check for them here to generate the appropriate error message
	if(INTERNAL_COMMUNICATION_FAULT_SET)
	{
		ei_vEnqueueFaultRegsInErrorQueue(INTERNAL_COMM_FAULT_FLAG,E311);
	}

	if(EEPROM_FAILURE_FAULT_SET)
	{
		ei_vEnqueueFaultRegsInErrorQueue(EEPROM_FAILURE_FAULT_FLAG,E308);
	}

	if(ONBOARD_ADC_FAULT_SET)
	{
		ei_vEnqueueFaultRegsInErrorQueue(ONBOARD_ADC_FAULT_FLAG,E310);
	}

			// Set Fault Event Register depending on the Faults occured and enabled
	// The ORing here is important because event bits are only set depending on conditional and enable
	// registers. They are cleared on reading through remote communication.
	Fault_Regs.FAULT_EVENT.all |= (Fault_Regs.FAULT_REGISTER.all & Fault_Regs.FAULT_ENABLE.all);

			// Set Status Conditional register bits here.
	// No fault bit
	if(Fault_Regs.FAULT_EVENT.all != NO_EVENT)	// Some fault and notification enabled for that fault
	{
		// A fault detected clears this bit
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_No_Fault = 0;

		// Set the appropriate status byte bit
		Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_Questionable_Summary = SET;
	}

	// No fault condition or no reporting enabled
	if((Fault_Regs.FAULT_REGISTER.all & Fault_Regs.FAULT_ENABLE.all) == NO_EVENT)
	{
		// No fault detected sets this bit
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_No_Fault = 1;
	}
}

//#####################	Init State ################################################################################
// Function Name: ei_vInitState
// Return Type: void
// Arguments:   void
// Description: Initialise the PSU to a known state
//#################################################################################################################
void ei_vInitState()
{
	//	Mode variables Initialization
	State.Mode.bt_Output_Status = 0;
	State.Mode.bt_FoldBack_Mode = 0;
	State.Mode.bt_Start_Mode = 0;
	State.Mode.bt_PSU_Control = 0;
	State.Mode.bt_Display_Mode = 0;
	State.Mode.bt_Encoder_Operation = 0;
	State.Mode.bt_Encoder_Rotation_Mode = 0;
	State.Mode.bt_FrontPanel_Access = 0;
	State.Mode.bt_Master_Slave_Setting = 0;
	State.Mode.bt_Multidrop_Mode = 0;
	State.Mode.bt_Power_On_Status_Clear = 1;	// Should be on initially
	State.Mode.bt_Can_SRQ_Switch = 0;

	// State Variables Initialization
	State.Previous_St = EI_ST_STANDBY;
	State.Present_St = EI_ST_STANDBY;
	State.Next_St = EI_ST_STANDBY;
	State.ui_Cycle_Counter = 0;

	// This register bit is set immediately on Power up.This bit set only here. Once we read it and clear,
	// it will be set only when the dsp boots next time.
	Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.bit.bt_Power_On = SET;
}

//#####################	State Operations ##########################################################################
// Function Name: ei_vState
// Return Type: void
// Arguments:   void
// Description: This function is a scheduled event that is responsible for transition
//				of states. Various flags that determine the state changes are set/reset
//				in different functions. Based on the information from these routines, this function determines
//				if the present state needs to be continued or a different state needs to be taken. Moreover,
//				changeover to the reset state during faults is also done in this routine.
//#################################################################################################################
void ei_vState()
{
	//	Determine the Next State depending on the flags set/reset in the rest of the code
	ei_vNextState();

	switch(State.Present_St)
	{
		case EI_ST_ARMED_POWER:
		{
			if(State.ui_Cycle_Counter == ONE_CYCLE_COUNT)
			{
				// Reset all flags and variables as required

				// First we need to check whether the output is within limits
				Uint16 Upper_limit = 0;	// Local variables to check bounds
				Uint16 Lower_limit = 0;
				unsigned char a_c_SpiTxDataArray[3];

				// limit Voltage to 1.05 times UVL if it is Out of Bounds
				// Integer comparisons are more effective for counts than iq comparisons
				Upper_limit = _IQ15rmpy(_IQ15mpy(_IQ15(Encoder.ui_Qep_OVP_Count),_IQ15(MIN_DISTANCE_BELOW_OVP)),1);
				Lower_limit = _IQ15rmpy(_IQ15mpy(_IQ15(Encoder.ui_Qep_UVL_Count),_IQ15(MIN_DISTANCE_ABOVE_UVL)),1);

				if((_IQ15int(Encoder.iq_Qep_Voltage_Count) > Upper_limit) || (_IQ15int(Encoder.iq_Qep_Voltage_Count) < Lower_limit))
				{
					Encoder.i_VoltageKnob_Coarse_Count = Lower_limit;
					Encoder.i_VoltageKnob_Fine_Count = QEP_FULL_ROTATIONS_COUNT/2;			// Default Value
					Encoder.iq_Qep_Voltage_Count = _IQ15(Encoder.i_VoltageKnob_Coarse_Count);	// IQ15 value
					EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count;
					EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;

					//On a violation take the PV state to local, so that the Reference is set from Qep counts for PV
					Global_Flags.fg_Voltage_Remote_control = FALSE;

					ei_vSwitchSpiClkPolarity(DAC_8552);
					// This step is important because by the time the references are changed after output on,OVP might be triggerred by the hardware
					//EPwm2Regs.CMPA.half.CMPA = 0;
					a_c_SpiTxDataArray[0] = DAC_LOAD_B | DAC_CHANNEL_B | DAC_NORMAL_MODE;	// Setup DAC B
					a_c_SpiTxDataArray[1] = 0;
					a_c_SpiTxDataArray[2] = 0;
					ei_uiSpiWrite(CS_DAC, 3, a_c_SpiTxDataArray);
					ei_vSwitchSpiClkPolarity(AD7705);
				}

				// Reset other state variables
				State.Mode.bt_Encoder_Operation = SET_VOLTAGE_CURRENT;	// Change operation to set_voltage_current
				State.Mode.bt_Encoder_Rotation_Mode = COARSE_ROTATION;	// Change rotation to coarse

				EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count;	// Reset Voltage counter to Coarse count
				EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count + (Encoder.i_VoltageKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
				Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution

				EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Coarse_Count; // Reset Current counter to Coarse count
				EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count + (Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
				Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution

				// Update Output references Immediately
				Reference.fg_Voltage_Reference_Update_Immediate_Flag = TRUE;
				Reference.fg_Current_Reference_Update_Immediate_Flag = TRUE;
			}
			else if(State.ui_Cycle_Counter == TWO_CYCLES_COUNT)
			{
				// when output switched ON. Reset current rise timer.
				RESET_CURRENT_RISE_TIMER;

				DISABLE_FAULT_RESET;
				SWITCH_OUTPUT_ON;
				PULL_PS_OK_HIGH;

				// The output has been switched on from Remote(Uart).
				if(Global_Flags.fg_Output_Switched_On_From_Uart == TRUE)
				{
					Global_Flags.fg_Output_Switched_On_From_Uart = FALSE;	// Clear the flag
					if(State.Mode.bt_PSU_Control == LOCAL)
					{
						State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
					}
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
				}

				// The output has been switched on from Remote(CAN).
				if(Global_Flags.fg_Output_Switched_On_From_Can == TRUE)
				{
					Global_Flags.fg_Output_Switched_On_From_Can = FALSE;	// Clear the flag
					if(State.Mode.bt_PSU_Control == LOCAL)
					{
						State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
					}
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
				}
			}
			else
			{
				asm("	nop");
			}
			break;
		}

		// There is functionally nothing different between fault state and standby state right now. After pressing Out in
		// fault state, the PSU goes into Standby State. So for both states same operation can be performed.
		case EI_ST_FAULT:
		case EI_ST_STANDBY:
		{
			if(State.ui_Cycle_Counter == ONE_CYCLE_COUNT)
			{
				SWITCH_OUTPUT_OFF;
				PULL_PS_OK_LOW;
				ENABLE_FAULT_RESET;

				// In case display turned off through remote by User always turn it back on during a fault for notification.
				// Display does not turn back ON for output fault.
				// For all other faults it turns back on to display the fault. It's the responsibility of the user to turn it back OFF again.
				if(ANY_FAULT_OTHER_THAN_OUT_FAULT)
				{
					Global_Flags.fg_State_Of_Display = ON;
				}

				/*if((Fault_Regs.FAULT_REGISTER.all & OTP_FAULT_FLAG) == OTP_FAULT_FLAG)
				{
					// OTP Fault. Fan runs at full speed.
					FULL_SPEED;
				}
				else
				{
					// Other faults. Fan runs at base speed.

					// If at all any faults other than OTP only will come here
					if(ANY_FAULTS)
					{
						BASE_SPEED;
					}
				}*/

				// Important to avoid inadvertent fold faults because it was observed that CC signal received by DSP
				// when o/p off.
				RESET_CURRENT_RISE_TIMER;
			}
			else if(State.ui_Cycle_Counter == TWO_CYCLES_COUNT)
			{
				// Reset all the Modes and encoder variables to default when power switched off.
				State.Mode.bt_Encoder_Operation = SET_VOLTAGE_CURRENT;	// Change operation to set_voltage_current
				State.Mode.bt_Encoder_Rotation_Mode = COARSE_ROTATION;	// Change rotation to coarse

				EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count; 	// Reset Voltage counter to Coarse count
				EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count + (Encoder.i_VoltageKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
				Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution

				EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Coarse_Count; // Reset Current counter to Coarse count
				EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count + (Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
				Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution
			}
			else	// every cycle after second check if error response to be generated for the Output on command
			{
				// Respond to remote commands. If this command received when PSU is in Fault/Standby, then
				// we need to reply with Output cannot be switched on error
				if(Global_Flags.fg_Output_Switched_On_From_Uart == TRUE)
				{
					// Output cannot be turned on. Push the appropriate error in the queue.
					ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E307);
					Global_Flags.fg_Output_Switched_On_From_Uart = FALSE;	// Clear the flags after response
				}

				if(Global_Flags.fg_Output_Switched_On_From_Can == TRUE)
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_TO_INDICATE_OUTPUT_CANNOT_SWITCH_ON;

					// Transmit response
					ECanaMboxes.MBOX16.MSGCTRL.bit.DLC = 2;
					ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
					ECanaShadow.CANTRS.bit.TRS16 = 1;
					ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;

					Global_Flags.fg_Output_Switched_On_From_Can = FALSE;	// Clear the flags after response
				}
			}

			break;
		}
		default:
			break;
	}

	//	Do not count more than 1 second in any state because there is no use.
	if(State.ui_Cycle_Counter <= TEN_CYCLES_COUNT)
	{
		INCREMENT_STATE_CYCLE_TIMER;
	}
}

//#####################	Next State ################################################################################
// Function Name: ei_vNextState
// Return Type: void
// Arguments:   void
// Description: Determines the next state depending on the various flags set/reset in different locations.
					// States Implemented
//			EI_ST_STANDBY
//			EI_ST_ARMED_POWER
//			EI_ST_FAULT
//#################################################################################################################
static void ei_vNextState()
{
	switch(State.Present_St)
	{
		case EI_ST_STANDBY:
		{
			// From STANDBY to (ARMED_POWER)
					// to ARMED_POWER when there are no faults
			if((ANY_FAULTS) == NONE)
			{
				State.Next_St = EI_ST_ARMED_POWER;
			}
			break;
		}

		case EI_ST_FAULT:
		{
			// From FAULT to (ARMED_POWER or STANDBY)
					// to ARMED_POWER when there are no faults
					// to STANDBY when OUT Button Pressed from front panel or remote
			if((ANY_FAULTS) == NONE)
			{
				State.Next_St = EI_ST_ARMED_POWER;
			}
			else	// Some Fault exists
			{
				if(State.Mode.bt_Output_Status == OFF)
				{
					State.Next_St = EI_ST_STANDBY;
				}
			}
			break;
		}

		case EI_ST_ARMED_POWER:
		{
			// From ARMED_POWER to (FAULT)
					// To FAULT when a FAULT occurs
			if((ANY_FAULTS) == NONE)
			{
				State.Next_St = EI_ST_ARMED_POWER;
			}
			else	// Some Fault exists
			{
				State.Next_St = EI_ST_FAULT;
			}
			break;
		}
	}

// If the State.Next remains the same as State.Current no action needs to be taken
// However if two above mentioned states are different, the PSU is disabled
// and the state change flags, which track the time after a state change are reset
// to their initial states.
	if (State.Next_St == State.Present_St)
	{
		asm ("NOP");
	}
	else
	{
		ei_vStateCycleFlagsReset();
	}
	State.Previous_St = State.Present_St;
	State.Present_St = State.Next_St;
}

//#####################	Reset State timer #########################################################################
// Function Name: ei_vStateCycleFlagsReset
// Return Type: void
// Arguments:   void
// Description: Resets the state cycle counter flag whenever the state changes.
//#################################################################################################################
static void ei_vStateCycleFlagsReset()
{
	RESET_STATE_CYCLE_TIMER;
}

//#####################	Clear Status ##############################################################################
// Function Name: ei_vClearPsuStatusStructure
// Return Type: void
// Arguments:   void
// Description: Clears the following items
//
//			  	1. Fault Event Registers
//			  	2. Standard Event Status Registers
//			  	3. Status Byte Register
//			  	4. Operation condition event register
//			  	5. Error Queue
//			  	6. Input Buffer
//				7. Output Buffer

//	To Clear Error Queue and Input buffer, Uart interrupt must be disabled to avoid SHARED DATA PROBLEM

//	Note: This function is called in both SciApp.c (by SCPI) and ECanApp.c (CAN BYTES). Even though CAN has got
//  nothing to do with most of the register cleared here, it still goes and modified these registers. It should not
//  cause any problems, but we just need to be aware that the SCPI register can be cleared through CAN also.
//#################################################################################################################
void ei_vClearPsuStatusStructure()
{
	Fault_Regs.FAULT_EVENT.all = 0;
	Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.all = 0;
	Service_Request_Regs.STATUS_BYTE_REGISTER.all = 0;
	Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all = 0;

	// Flush Transmit Buffer
	Transmit_Circular_Buffer.i_Front = EMPTY_QUEUE_INDEX;
	Transmit_Circular_Buffer.i_Rear = EMPTY_QUEUE_INDEX;

	// Disable the SCI interrupt here to avoid SHARED DATA PROBLEM
	SciaRegs.SCIFFRX.bit.RXFFIENA = DISABLE_INT;//1

	// Wait 10 Cycles to allow any flagged interrupts to get serviced
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");

	// Flush Input Buffer
	Receive_Circular_Buffer.i_Front = EMPTY_QUEUE_INDEX;
	Receive_Circular_Buffer.i_Rear = EMPTY_QUEUE_INDEX;

	// Flush Error Queue
	Uint16 i;
	//	Error_Linear_Queue.i_Front = EMPTY_QUEUE_INDEX;
	Error_Linear_Queue.i_Rear = EMPTY_QUEUE_INDEX;
	for(i = 0; i < ERROR_BUFFER_SIZE; i++)
	{
		// Initialise all the pointers to E00[] = "0,\"No Error\""
		Error_Linear_Queue.a_s_p_ErrorBuffer[i] =  (Int8 * )E00;
	}

	SciaRegs.SCIFFRX.bit.RXFFIENA = ENABLE_INT;//1
}

//#####################	PSU in safe state #########################################################################
// Function Name: ei_vResetPsuToSafeState
// Return Type: void
// Arguments:   void
// Description: Brings PSU to a known state.
//
//#################################################################################################################
void ei_vResetPsuToSafeState()
{
	// 1. Reset Mode variables
	State.Mode.bt_Encoder_Operation = SET_VOLTAGE_CURRENT;		// Default encoder operation
	State.Mode.bt_Encoder_Rotation_Mode = COARSE_ROTATION;		// Default encoder rotation mode
	State.Mode.bt_Display_Mode = DISPLAY_OUTPUT;				// Defult display value
	State.Mode.bt_FoldBack_Mode = FOLDBACK_DISARMED;			// Disable Foldback
	State.Mode.bt_Power_On_Status_Clear = ON;					// Clear Enable when system reboots
	State.Mode.bt_FrontPanel_Access = FRONTPANEL_UNLOCKED;		// Unlock FrontPanel in case unlocked.

	// 2. Reset Fine, coarse and final registers
	Encoder.i_VoltageKnob_Fine_Count = QEP_FULL_ROTATIONS_COUNT/2;
	Encoder.i_VoltageKnob_Coarse_Count = 0;
	Encoder.i_VoltageKnob_Final_Count = 0;

	Encoder.i_CurrentKnob_Fine_Count = QEP_FULL_ROTATIONS_COUNT/2;
	Encoder.i_CurrentKnob_Coarse_Count = 0;
	Encoder.i_CurrentKnob_Final_Count = 0;

	// 3. Set Voltage and current to 0
	Encoder.iq_Qep_Voltage_Count = 0;
	EQep1Regs.QPOSINIT = Encoder.iq_Qep_Voltage_Count;		// Copy final value back into Qep
	EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;

	Encoder.iq_Qep_Current_Count = 0;
	EQep2Regs.QPOSINIT = Encoder.iq_Qep_Current_Count;		// Copy final value back into Qep
	EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;

	// 4. Turn off Output
	State.Mode.bt_Output_Status = OFF;

	// 5. Put PSU in remote non latched mode and all parameters will take value from the Qep Registers.
	State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
	Global_Flags.fg_OVP_Remote_control = Global_Flags.fg_UVL_Remote_control = Global_Flags.fg_Voltage_Remote_control = Global_Flags.fg_Current_Remote_control = FALSE;

	// 6. Start mode is safe start
	State.Mode.bt_Start_Mode = SAFE_START;

	// 7. Set OVP and UVL to MAX and 0 respectively
	Encoder.ui_Qep_OVP_Count = QEP_MAX_OVP_COUNT;
	Encoder.ui_Qep_UVL_Count = 0;

	Reference.ui_OVP_Reference_Update_Immediate_Flag = TRUE;
	Reference.fg_Voltage_Reference_Update_Immediate_Flag = TRUE;
	Reference.fg_Current_Reference_Update_Immediate_Flag = TRUE;

	// 8. Update Status registers
	Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Auto_start_Enabled = 0;
	Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Fold_Enabled = 0;
	Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
}
