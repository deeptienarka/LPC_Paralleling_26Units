//#################################################################################################################
//# 							enArka Instruments proprietary
//# File: ei_DisplayKeyscan.c
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
//void ei_vMax625_X_Y_I2C_initialization(Uint16 MAX_Init_Device_address);
//void ei_vKeyProcessing();
//void ei_vDisplayOperation();
//void TestI2CWrite();
//#################################################################################################################
									/*-------Local Prototypes----------*/
static void ei_vUpdateKeyscanEvents(Uint16 Press, Uint16 Debounce);
static Uint16 ei_uiGetNextFault();
static void ei_vDisplayFault(Uint16 New_Fault);
static Uint16 ei_uiDisplayVoltageCurrent(Uint16 Display_Driver_Address);
static Uint16 ei_uiDisplayOVP(Uint16 Display_Driver_Address);
static Uint16 ei_uiDisplayUVL(Uint16 Display_Driver_Address);
static Uint16 ei_uiDisplayOtherParameters(Uint16 Display_Driver_Address);
static Uint16 ei_uiDisplayAddrBaud(Uint16 Display_Driver_Address);
static void ei_uiSetFrontPanelIndicator(Uint16 Display_Driver_Address, Uint16 Point_Location, Uint16 Fault_Led);
static Uint16 ei_uiDisplayDigitMapping(Uint16 Digit_To_Be_Displayed);
static void ei_vBlankDisplay(Uint16 Display_Driver_Address);

//#####################	Max625_X_Y_I2C_initialization #############################################################
// Function Name: void ei_vMax625_X_Y_I2C_initialization
// Return Type: void
// Arguments:   Uint16 MAX_Init_Device_address
// Description: MAX6958/59 Initialization routine. Pass Slave Address. 	(MAX6958A: 0x39)	(MAX6959B: 0x38)
//
//
				//Register map of the device
// 1. Configuration Register (0x04)	Address in bracket.
//		D7:X
//		D6:X
//		D5:R (1 Clears all data and segment registers. Transient bit)
//		D4:X
//		D3:X
//		D2:X
//		D1:D (Device Readback  0: MAX6958, 1: MAX6959)
//		D0:S (1: Normal operation. 0: Shutdown Operation)

// 2. Scan limit register (0x03)
//		D7 to D2: X
//		D1	  D0
//		0	  0 (Active: Dig 0 and Seg 0,4)
// 		0	  1 (Active: Dig 0,1 and Seg 0,1,4,5)
// 		1	  0 (Active: Dig 0,1,2 and Seg 0,1,2,4,5,6)
// 		1	  1 (Active: all Digs and Segs

// 3. Global Intensity Register (0x02)
//     Operating at 1.44mA, 4/64 Duty cycle (Data: 0x04)

// 4. Decode Mode register (0x01)
//		D7-D4: X
// 		D3-D0: (A 1 in any of the location denotes hexadecimal decode mode. We have put 0s in all location
//				which means that each bit in the digit register corresponds to the led you want to light up.)
//

// 5. Digit0 to Digit3 (0x20 to 0x23)
//		Each register corresponds to one 7 segment Led. Each location in the register corresponds to one Led.
// 		D7:X
//		D6:Seg a
//		D5:Seg b
//		D4:Seg c
//		D3:Seg d
//		D2:Seg e
//		D1:Seg f
//		D0:Seg g

// 6. Segments (0x24)
//		D7:Seg 7 (DIG3 dp)
//		D6:Seg 6 (DIG2 dp)
//		D5:Seg 5 (DIG1 dp)
//		D4:Seg 4 (DIG0 dp)
//		D3:Seg 3
//		D2:Seg 2
//		D1:Seg 1
//		D0:Seg 0

// 7. Port Configuration Register (0x06)
//		D7	D6	D5
//		0	0	0 (IRQ/SEG9 is logic 0 o/p)
//		0	0	1 (IRQ/SEG9 is logic 1 o/p)
//		0	1	0 (IRQ/SEG9 is active low IRQ)
//		0	1	1 (IRQ/SEG9 is active high IRQ)
//		1	X	X (IRQ/SEG9 is segment driver) We Use in this mode
//		D4:INPUT 2 (0: Logic input/ 1: Keyscan)
//		D3:INPUT 1 (0: Logic input/ 1: Keyscan)
//		D2:INPUT 2 logic level (read only: not used)
//		D1:INPUT 1 logic level (read only: not used)
//		D0:IRQ status (read only: not used)

// 8. Key Debounced register (0x08) Read only
//		D7:Key7
//		D6:Key6
//		D5:Key5
//		D4:Key4
//		D3:Key3
//		D2:Key2
//		D1:Key1
//		D0:Key0

// 8. Key Pressed register (0x0C) Read only
//		D7:Key7
//		D6:Key6
//		D5:Key5
//		D4:Key4
//		D3:Key3
//		D2:Key2
//		D1:Key1
//		D0:Key0

// 9. Display Test register (0x07)
//		D7-D1:X
//		D0:Mode (0: Normal/1: Display Test)
//#################################################################################################################
void ei_vMax625_X_Y_I2C_initialization(Uint16 MAX_Init_Device_address)
{
	Uint16 I2C_Tx_Data_Array[1];

//	address_I2C = CONFIGURATION_REGISTER_ADDRESS;
	I2C_Tx_Data_Array[0] = SHUTDOWN_MAXIC ;
	ei_uiI2CWrite(MAX_Init_Device_address, CONFIGURATION_REGISTER_ADDRESS, 1, I2C_Tx_Data_Array);

	I2C_Tx_Data_Array[0] = 0x00 | NORMAL_OPERATION | GLOBAL_CLEAR_MAX ;
	ei_uiI2CWrite(MAX_Init_Device_address, CONFIGURATION_REGISTER_ADDRESS, 1, I2C_Tx_Data_Array);

	I2C_Tx_Data_Array[0] = ENABLE_DIG0_TO_DIG3 ;
	ei_uiI2CWrite(MAX_Init_Device_address, SCAN_LIMIT_REGISTER_ADDRESS, 1, I2C_Tx_Data_Array);

	I2C_Tx_Data_Array[0] = LEVEL_SELECTOR ;
	ei_uiI2CWrite(MAX_Init_Device_address, INTENSITY_REGISTER_ADDRESS, 1, I2C_Tx_Data_Array);

	I2C_Tx_Data_Array[0] = NO_HEXADECIMAL_DECODE_MODE_IN_DIGIT_REGISTER ;
	ei_uiI2CWrite(MAX_Init_Device_address, DECODE_MODE_REGISTER_ADDRESS, 1, I2C_Tx_Data_Array);

	I2C_Tx_Data_Array[0] = 0x0F ; 	// According to the SRS all indicator led's must be on during start up
	ei_uiI2CWrite(MAX_Init_Device_address, DIGIT_DISPLAY_DECIMAL_POINT_AND_SEGMENT_LED_ADDRESS, 1, I2C_Tx_Data_Array);

	if (MAX_Init_Device_address == MAX6959A_ADDRESS)
	{
		I2C_Tx_Data_Array[0] = 0x00 | SEGMENT9_FUNCTION | ENABLE_KEYSCAN0 | ENABLE_KEYSCAN1 ;
		ei_uiI2CWrite(MAX_Init_Device_address, PORT_CONFIGURATION_REGISTER_ADDRESS, 1, I2C_Tx_Data_Array);
	}

	// for testing current consumption by FP. Light up everything
//	I2C_Tx_Data_Array[0] = 0x7F;
//	I2C_Write(MAX_Init_Device_address, DIGIT_DISPLAY_BEGIN_ADDRESS, 1, I2C_Tx_Data_Array);
//	I2C_Write(MAX_Init_Device_address, DIGIT_DISPLAY_BEGIN_ADDRESS + 1, 1, I2C_Tx_Data_Array);
//	I2C_Write(MAX_Init_Device_address, DIGIT_DISPLAY_BEGIN_ADDRESS + 2, 1, I2C_Tx_Data_Array);
//	I2C_Write(MAX_Init_Device_address, DIGIT_DISPLAY_BEGIN_ADDRESS + 3, 1, I2C_Tx_Data_Array);
//	I2C_Tx_Data_Array[0] = 0xFF ;
//	I2C_Write(MAX_Init_Device_address, DIGIT_DISPLAY_DECIMAL_POINT_AND_SEGMENT_LED_ADDRESS, 1, I2C_Tx_Data_Array);
//	if(MAX_Init_Device_address == MAX6958B_ADDRESS
//	{
//		for(;;);
//	}
}

//#####################	Key Press Detection #######################################################################
// Function Name: void ei_vKeyProcessing
// Return Type: void
// Arguments:   void
// Description: Detects key presses. MAX6959 detects both Key Debounce and Key Presses.

//	NAME		BOARD NOTATION		DEVICE DATASHEET NOT.		REGISTER VALUE
//	KEY_FINE	S1					Key5						0x0020
//	KEY_OUT		S2					Key6						0x0040
//	KEY_PREV	S3					Key7						0x0080
//	KEY_FOLD	S4					Key1						0x0002
//	KEY_REMOTE	S5					Key2						0x0004
//	KEY_OVP_UVL	S6					Key3						0x0008

//#################################################################################################################
void ei_vKeyProcessing()
{
	Uint16 Key_Pressed_data, Key_Debounced_data;
	Uint16 I2C_Rx_Data_Array[1];

	/*Key_Pressed_data = 0;
	Key_Debounced_data = 0;*/

	if((ei_uiI2CRead(MAX6959A_ADDRESS, KEY_DEBOUNCE_REGISTER_ADDRESS, 1, I2C_Rx_Data_Array)) == FAILURE)
	{
		return;
	}
	Key_Debounced_data = I2C_Rx_Data_Array[0];

	if((ei_uiI2CRead(MAX6959A_ADDRESS, KEY_PRESS_REGISTER_ADDRESS, 1, I2C_Rx_Data_Array)) == FAILURE)
	{
		return;
	}
	Key_Pressed_data = I2C_Rx_Data_Array[0];

	// To avoid any key press misses because some keys inside use only Key_Pressed_Data for detection.
	// Key_Debounced_Data would have already detected a press in case Key_Pressed_Data has missed it.
	if((Key_Debounced_data & Key_Pressed_data) != Key_Debounced_data)
	{
		Key_Pressed_data = Key_Debounced_data;
	}

    ei_vUpdateKeyscanEvents(Key_Pressed_data,Key_Debounced_data);
}

//#####################	Mode update depending on keyscan #########################################################
// Function Name: ei_vUpdateKeyscanEvents
// Return Type: void
// Arguments:   Uint16 Press, Uint16 Debounce
// Description: Update the state of the PSU depending on the KeyPresses and KeyDebounce

				// Key and Their Functions
// BUTTON	FUNCTION													OUTPUT_OFF		REM		LOCAL		TIMEOUT		LOCKOUT		LFP
// Out		Primary: Toggles Output On Off								Yes				no		yes			no			no			no
//			Secondary: Toggle b/w AST and SAF for 3 secs				Yes				no		yes			timed		no			no
//					   Mode selected is the one being displayed
//			when button released.

// Fine		Encoder Coarse/Fine Operation								Yes				no		yes			no			no			no

// Ovp/Uvl	1. Press once to set 'OVP' using volt encoder				Yes				view	yes			yes			view		view
//			2. Press again to set 'UVL' using volt.
//					Current encoder disabled

// Fold		Press to toggle foldback mode								Yes				no		Yes			no			no			no

// Prev		Primary: Toggle display mode b/w o/p and Prev.
//					 Prev will be displayed for 3 seconds
//			till no change and return to display output.				Yes				view	yes			Yes			view		view
//			Secondary: Toggle b/w LFP and UFP for 3 seconds.			yes				no		yes			timed		no			yes
//						Mode selected is the one being displayed
//			when button released.

// Rem/Loc	Primary: Go to local if PSU is in remote					Yes				yes		yes			no			no			no
//			Secondary: Press for 3 seconds to set baud rate & Addr		yes				view	yes			yes			view		view
//			Voltage Encoder: BaudRate
//			Current Encoder: Address
//#################################################################################################################
static void ei_vUpdateKeyscanEvents(Uint16 Press, Uint16 Debounce)
{
	///////////////////// OUT KEY//////////////////////////////////////////////////////////////////////////////
	if((Press & KEY_OUT_PRESSED) == KEY_OUT_PRESSED)
	{
		if((FRONTPANEL_IS_UNLOCKED) && (State.Mode.bt_PSU_Control == LOCAL))
		{
			INCREMENT_KEY_OUT_PRESSED_TIMER;					// Increment this button timer everytime you detect it as pressed

			// Key Pressed and held for more than 3 seconds in the allowed modes.
			if((State.Mode.bt_PSU_Control == LOCAL) && (OUT_KEY_SECONDARY_OPERATION_DETECTED))
			{
				if(NO_FAULT_OTHER_THAN_OUT_FAULT)
				{
					RESET_KEY_OUT_PRESSED_TIMER;								// Reset the timer
					Global_Flags.fg_Out_Key_Operation_Mode = SECONDARY_OPERATION;	// Put this flag in secondary mode so that primary function is disabled
					State.Mode.bt_Start_Mode ^= 1;									// Toggle Start mode between AutoRestart and SafeStart
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Auto_start_Enabled ^= 1;	// Toggle Status bit.
				}
				else
				{
					// Just reset the timer but take no action
					RESET_KEY_OUT_PRESSED_TIMER;
				}
			}
		}
		else	// Could be local lockout or remote or Frontpanel locked
		{
			if(State.Mode.bt_FrontPanel_Access == FRONTPANEL_LOCKED)
				Global_Flags.fg_Display_FP_Access_Mode = SET;
		}
	}

	//	When the above key is released or not pressed enter the condition below
	if((Press & KEY_OUT_PRESSED) != KEY_OUT_PRESSED)
	{
		// 1. When key not pressed the value of the ui_Key_Out_Press_Timer is 0
		// 2. Will not come in the below IF, if code has fallen in above block's else
		if((Timers.ui_Key_Out_Press_Timer > 0) && (Global_Flags.fg_Out_Key_Operation_Mode == PRIMARY_OPERATION))
		{
			// Come here only when the key is pressed for less than 3 seconds
			State.Mode.bt_Output_Status ^= 1;		// Toggle Output Mode

			if(State.Mode.bt_Output_Status == ON)
			{
			    sci_fecOnCommand = 1;
			}
			else
			{
			    sci_fecOnCommand = 0;
			}

			RESET_KEY_OUT_PRESSED_TIMER;		// This timer must be reset everytime the display mode toggled
		}

		// The two operations below must be performed whenever the key is released.
		if(Timers.ui_Key_Out_Press_Timer > 0)
		{
			RESET_KEY_OUT_PRESSED_TIMER;							// Reset the timer
			Global_Flags.fg_Out_Key_Operation_Mode = PRIMARY_OPERATION;	// Go to primary
		}
	}

	// ALL OTHER KEYS WILL NOT OPERATE WHEN ANY OTHER FAULT's THERE.
	if(NO_FAULT_OTHER_THAN_OUT_FAULT)
	{

		//	To check whether the Out Key is in secondary mode or not
		if(Global_Flags.fg_Out_Key_Operation_Mode == SECONDARY_OPERATION)
		{
			Global_Flags.fg_Display_PSU_StartMode = SET;				// Display AST/SAF on the voltage display because we are in secondary mode
		}

		///////////////////// FINE/COARSE KEY//////////////////////////////////////////////////////////////////////////////
		if((Debounce & KEY_FINE_PRESSED) == KEY_FINE_PRESSED)
		{
			if((FRONTPANEL_IS_UNLOCKED) && (State.Mode.bt_PSU_Control == LOCAL))
			{
				if(State.Mode.bt_Encoder_Operation == SET_VOLTAGE_CURRENT)
				{
					State.Mode.bt_Encoder_Rotation_Mode ^= 1;	// Toggle Encoder Rotation Mode

					if (State.Mode.bt_Encoder_Rotation_Mode == COARSE_ROTATION)	// We just changed to coarse mode
					{
						EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count; // Reset Voltage counter to Coarse count
						EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
						Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count + (Encoder.i_VoltageKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
						Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution

						EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Coarse_Count; // Reset Current counter to Coarse count
						EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
						Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count + (Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
						Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution
					}
					else 	// We just changed to Fine mode
					{
						// Just copy back previous fine values. This maintains the refernces set when just toggled between
						// Fine and Coarse without rotating encoders.
						EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Fine_Count;
						EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
						Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse = FALSE;

						EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Fine_Count; // Reset Current counter to last Fine count
						EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
						Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse = FALSE;
					}
				}
			}
			else // Could be local lockout or remote or Frontpanel locked
			{
				if(State.Mode.bt_FrontPanel_Access == FRONTPANEL_LOCKED)
					Global_Flags.fg_Display_FP_Access_Mode = SET;
			}
		}

		///////////////////// OVP/UVL KEY//////////////////////////////////////////////////////////////////////////////
		if (( Debounce & KEY_OVP_UVL_PRESSED) == KEY_OVP_UVL_PRESSED)
		{
			if ((State.Mode.bt_Encoder_Operation != SET_OVP) && (State.Mode.bt_Encoder_Operation != SET_UVL))	// Could be in SET_VOLTAGE_CURRENT or SET_ADDR_BAUD
			{
				RESET_OVP_UVL_TIMER; // Reset Timer
				State.Mode.bt_Encoder_Operation = SET_OVP;      // Change mode to Set OVP
				State.Mode.bt_Display_Mode = DISPLAY_OUTPUT;		// Change to Display output mode

				State.Mode.bt_Encoder_Rotation_Mode = COARSE_ROTATION;     // Change to coarse mode if in Fine mode

	//			Reset counter and final to count corresponding to previous OVP Setting
				EQep1Regs.QPOSINIT = Encoder.ui_Qep_OVP_Count;
				EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				Encoder.i_VoltageKnob_Final_Count = Encoder.ui_Qep_OVP_Count; // This line helps for proper operation of the timeout feature
			}
			else if (State.Mode.bt_Encoder_Operation == SET_OVP)// Already in OVP mode. Change to UVL
			{
				RESET_OVP_UVL_TIMER; // Reset Timer
				State.Mode.bt_Encoder_Operation = SET_UVL;

	//			Reset counter to count corresponding to previous UVL Setting
				EQep1Regs.QPOSINIT = Encoder.ui_Qep_UVL_Count;
				EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				Encoder.i_VoltageKnob_Final_Count = Encoder.ui_Qep_UVL_Count; // This line helps for proper operation of the timeout feature
			}
			else //Already in UVL State.Mode. Take it to set Voltage_Current State.Mode.
			{
				State.Mode.bt_Encoder_Operation = SET_VOLTAGE_CURRENT; // Change to SET_VOLTAGE_CURRENT Mode

	//			Reset the voltage and current counter to previous values.
				EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count; //Reset Voltage counter to Coarse count
				EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count + (Encoder.i_VoltageKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
				Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution

				EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Coarse_Count; // Reset Current counter to Coarse count
				EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count + (Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
				Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution
			}
		}

		/////////////////////// FOLD KEY//////////////////////////////////////////////////////////////////////////////
		if((Debounce & KEY_FOLD_PRESSED) == KEY_FOLD_PRESSED)
		{
			if((FRONTPANEL_IS_UNLOCKED) && (State.Mode.bt_PSU_Control == LOCAL))
			{
				if(State.Mode.bt_FoldBack_Mode == FOLDBACK_DISARMED)
				{
					State.Mode.bt_FoldBack_Mode = FOLDBACK_ARMED_TO_PROTECT_FROM_CC;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Fold_Enabled = SET;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Current_Foldback_Mode = SET;
				}
				else //if(State.Mode.bt_FoldBack_Mode == FOLDBACK_ARMED_TO_PROTECT_FROM_CC)
				{
					State.Mode.bt_FoldBack_Mode = FOLDBACK_DISARMED;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Fold_Enabled = CLEARED;
				}
			}
			else // Could be local lockout or remote or Frontpanel locked
			{
				if(State.Mode.bt_FrontPanel_Access == FRONTPANEL_LOCKED)
					Global_Flags.fg_Display_FP_Access_Mode = SET;
			}
		}

		///////////////////// PREV KEY////////////////////////////////////////////////////////////////////////////////
		if((Press & KEY_PREVIEW_PRESSED) == KEY_PREVIEW_PRESSED)
		{
			INCREMENT_KEY_PREVIEW_PRESSED_TIMER;					// Increment this button timer everytime you detect it as pressed

			if(PREVIEW_KEY_SECONDARY_OPERATION_DETECTED)			// Key Pressed and held for more than 3 seconds
			{
				RESET_KEY_PREVIEW_PRESSED_TIMER;							// Reset the timer
				Global_Flags.fg_Preview_Key_Operation_Mode = SECONDARY_OPERATION;	// Put this flag in secondary mode so that primary function is disabled

				if(State.Mode.bt_PSU_Control == LOCAL)	// This operation should work only when in local mode
				{
					State.Mode.bt_FrontPanel_Access ^= 1;								// Toggle Frontpanel Access from Lock to Unlock
				}
			}
		}

	//	When the above key is released or not pressed enter the condition below
		if((Press & KEY_PREVIEW_PRESSED) != KEY_PREVIEW_PRESSED)
		{
			// When key not pressed the value of the ui_Key_Preview_Press_Timer is 0
			if((Timers.ui_Key_Preview_Press_Timer > 0) && (Global_Flags.fg_Preview_Key_Operation_Mode == PRIMARY_OPERATION))
			{
				// Come here only when the key is pressed for less than 3 seconds
				State.Mode.bt_Display_Mode ^= 1;			// Toggle display mode between Preview and Output
				RESET_PREVIEW_OPERATION_TIMER;		// This timer must be reset everytime the display mode toggled

				// If PSU is not in SET_VOLTAGE_CURRENT mode, then the PSU must remain in Display Output mode
				if(State.Mode.bt_Encoder_Operation != SET_VOLTAGE_CURRENT)
				{
					State.Mode.bt_Display_Mode = DISPLAY_OUTPUT;
				}
			}

			// The two operations below must be performed whenever the key is released.
			if(Timers.ui_Key_Preview_Press_Timer > 0)
			{
				RESET_KEY_PREVIEW_PRESSED_TIMER;							// Reset the timer
				Global_Flags.fg_Preview_Key_Operation_Mode = PRIMARY_OPERATION;	// Go to primary
			}
		}

	//	To check whether the Prev Key is in secondary mode or not
		if(Global_Flags.fg_Preview_Key_Operation_Mode == SECONDARY_OPERATION)
		{
			Global_Flags.fg_Display_FP_Access_Mode = SET;				// Display LFP/UFP on the voltage display because we are in secondary mode
		}

		///////////////////// REM/LOC KEY//////////////////////////////////////////////////////////////////////////////
		if((Press & KEY_REMOTE_PRESSED) == KEY_REMOTE_PRESSED)
		{
			INCREMENT_KEY_REMOTE_PRESSED_TIMER;	// Increment the timer everytime this button is detected as pressed

			RESET_ADDR_BAUD_TIMER;				// Reset this timer to avoid timeout if key held for more than (3+5)seconds.

			if(REMOTE_KEY_SECONDARY_OPERATION_DETECTED)	// Key held for more than 3 seconds
			{
				RESET_KEY_REMOTE_PRESSED_TIMER;			// Reset this every time a 3 second press detected

	//			This Key is not operation toggle like other keys' secondary function.
	//			Once it is set, it remains set for as long as you hold it.
	//			So every 3 seconds it cannot change it's state.
				if(Global_Flags.fg_Remote_Key_Operation_Mode == PRIMARY_OPERATION)
				{
					if(State.Mode.bt_Encoder_Operation != SET_ADDR_BAUD)	// In some other mode. Bring to Addr_Baud
					{
						RESET_ADDR_BAUD_TIMER;	// Reset this timer whenever you enter this mode
						State.Mode.bt_Encoder_Operation = SET_ADDR_BAUD;
						State.Mode.bt_Display_Mode = DISPLAY_OUTPUT;		// Change to Display output mode
						Global_Flags.fg_Uart_Comm_with_this_PSU = CLOSED;	// Close the communication whenever you enter this mode.

						State.Mode.bt_Encoder_Rotation_Mode = COARSE_ROTATION;     // Change to coarse mode if in Fine mode

					//	Reset counter 1 and final to count corresponding to previous Address Setting
						EQep1Regs.QPOSINIT = Encoder.ui_Qep_Address_Count;
						EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
						Encoder.i_VoltageKnob_Final_Count = Encoder.ui_Qep_Address_Count; // This line helps for proper operation of the timeout feature

					//	Reset counter 2 and final to count corresponding to previous Baudrate Setting
						EQep2Regs.QPOSINIT = Encoder.ui_Qep_BaudRate_Count;
						EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
						Encoder.i_CurrentKnob_Final_Count = Encoder.ui_Qep_BaudRate_Count; // This line helps for proper operation of the timeout feature
					}
					Global_Flags.fg_Remote_Key_Operation_Mode = SECONDARY_OPERATION;
				}
			}
		}

		//	When the above key is released or not pressed enter the condition below
		if((Press & KEY_REMOTE_PRESSED) != KEY_REMOTE_PRESSED)
		{
			// When key not pressed the value of the ui_Key_Remote_Press_Timer is 0
			if((Timers.ui_Key_Remote_Press_Timer > 0) && (Global_Flags.fg_Remote_Key_Operation_Mode == PRIMARY_OPERATION))
			{
				if(State.Mode.bt_Encoder_Operation == SET_ADDR_BAUD)	// Here a quick press can take the PSU back to SET_VOLTAGE_CURRENT state
				{
					State.Mode.bt_Encoder_Operation = SET_VOLTAGE_CURRENT; // Change to SET_VOLTAGE_CURRENT Mode

			//		Reset the voltage and current counter to previous values.
					EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count; //Reset Voltage counter to Coarse count
					EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
					Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count + (Encoder.i_VoltageKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
					Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution

					EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Coarse_Count; // Reset Current counter to Coarse count
					EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
					Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count + (Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
					Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution
				}
				else	// do normal primary operation
				{
					if((FRONTPANEL_IS_UNLOCKED) && (State.Mode.bt_PSU_Control == NON_LATCHED_REMOTE))
					{
						State.Mode.bt_PSU_Control = LOCAL;
						Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 1;
					}
					else // Could be local lockout or Frontpanel locked
					{
						if(State.Mode.bt_FrontPanel_Access == FRONTPANEL_LOCKED)
							Global_Flags.fg_Display_FP_Access_Mode = SET;
					}
				}
			}

			// The two operations below must be performed whenever the key is released.
			if(Timers.ui_Key_Remote_Press_Timer > 0)
			{
				RESET_KEY_REMOTE_PRESSED_TIMER;								// Reset the timer
				Global_Flags.fg_Remote_Key_Operation_Mode = PRIMARY_OPERATION;	// Go to primary
			}
		}
	}
}

//#####################	Display Driver ############################################################################
// Function Name: void ei_vDisplayOperation
// Return Type: void
// Arguments:   void
// Description: Complete Display operations start from here.

//	In case the State of display is detected as OFF, then this routine will perform all
//	normal operations but just before writing
//#################################################################################################################
void ei_vDisplayOperation()
{
	Uint16 ui_Point_Location_Left = 0;
	Uint16 ui_Point_Location_Right = 0;
	Uint16 Led_Blink = 0;
	Uint16 ui_Fault_To_Be_Displayed;

	ui_Fault_To_Be_Displayed = ei_uiGetNextFault();	// Gets the next fault after sending the last fault.

	if(ui_Fault_To_Be_Displayed != NONE)
	{
		ei_vDisplayFault(ui_Fault_To_Be_Displayed);
	}
	else //No Faults
	{
		ui_Last_Fault_Displayed = NONE;
	}

	if((ui_Last_Fault_Displayed == OUT_FAULT_FLAG) || (ui_Last_Fault_Displayed == NONE))	// Any other fault will display only Fault
	{
		if((Global_Flags.fg_Display_FP_Access_Mode == SET) || (Global_Flags.fg_Display_PSU_StartMode == SET))
		{
			ui_Point_Location_Left = ei_uiDisplayOtherParameters(MAX6959A_ADDRESS);
			ui_Point_Location_Right = ei_uiDisplayOtherParameters(MAX6958B_ADDRESS);
		}
		// Set_Voltage_Current
		else if(State.Mode.bt_Encoder_Operation == SET_VOLTAGE_CURRENT)
		{
			if((OUTPUT_ON) || (State.Mode.bt_Display_Mode == DISPLAY_PREVIEW))
			{
				// Come in when
				// 1. Output ON
						//or
				// 2. Display Mode Preview irrespective of the output
				ui_Point_Location_Left = ei_uiDisplayVoltageCurrent(MAX6959A_ADDRESS);
				ui_Point_Location_Right = ei_uiDisplayVoltageCurrent(MAX6958B_ADDRESS);
			}
		}

		// OVP will work during Output OFF
		else if(State.Mode.bt_Encoder_Operation == SET_OVP)
		{
			ui_Point_Location_Left = ei_uiDisplayOVP(MAX6959A_ADDRESS);
			ui_Point_Location_Right = ei_uiDisplayOVP(MAX6958B_ADDRESS);
		}

		// UVL will work during Output OFF
		else if(State.Mode.bt_Encoder_Operation == SET_UVL)
		{
			ui_Point_Location_Left = ei_uiDisplayUVL(MAX6959A_ADDRESS);
			ui_Point_Location_Right = ei_uiDisplayUVL(MAX6958B_ADDRESS);
		}
		else if(State.Mode.bt_Encoder_Operation == SET_ADDR_BAUD)
		{
			ui_Point_Location_Left = ei_uiDisplayAddrBaud(MAX6959A_ADDRESS);
			ui_Point_Location_Right = ei_uiDisplayAddrBaud(MAX6958B_ADDRESS);
		}
		else
			asm("	nop");
	}

	if(BLINK_LED_FAULTS)	// Use last fault displayed to check for Led Blinking
	{
		Led_Blink = BLINK;
	}

	// Decimal Point and Segment Leds
	if(Global_Flags.fg_State_Of_Display == OFF)
	{
		// When display has been switched OFF by user decimal point is always turned OFF.
		ui_Point_Location_Left = NO_DECIMAL_POINT;
		ui_Point_Location_Right = NO_DECIMAL_POINT;
	}
	ei_uiSetFrontPanelIndicator(MAX6959A_ADDRESS, ui_Point_Location_Left, Led_Blink);
	ei_uiSetFrontPanelIndicator(MAX6958B_ADDRESS, ui_Point_Location_Right, Led_Blink);
}

//#####################	Next Fault ################################################################################
// Function Name: ei_uiGetNextFault
// Return Type: Uint16 Next_Fault
// Arguments:   void
// Description: Calculate and return the next fault to be displayed depending on the last fault displayed.
//#################################################################################################################
static Uint16 ei_uiGetNextFault()
{
	Uint16 New_Fault;
//	if((ANY_FAULTS) == NONE)	Used to check this earlier, But this caused the Output on LED to turn 0.5 seconds later than the display.
	if(OUTPUT_ON)	// Checking for this avoids the problem caused in the last condition
	{
		New_Fault = NONE;
	}
	else // some fault exists
	{
		if(ui_Last_Fault_Displayed == NONE)	// Initially display the first fault that occurs.
		{
			New_Fault  = Fault_Regs.FAULT_REGISTER.all;
		}
		else if(LAST_FAULT_DISPLAYED_WAS_NOT_OUT_FAULT)
		{
			if(LAST_FAULT_CLEARED)
			{
				if(ANY_FAULT_OTHER_THAN_OUT_FAULT)
				{
					New_Fault = Fault_Regs.FAULT_REGISTER.all;	// Get new Fault
				}
				else // No fault other than OUT fault.
				{
					New_Fault = OUT_FAULT_FLAG;
				}
			}
			else // Last Fault not yet cleared
			{
				New_Fault = ui_Last_Fault_Displayed;	// Display same fault
			}
		}
		else	// Last Fault is OUT
		{
			if(ANY_FAULT_OTHER_THAN_OUT_FAULT)
			{
				New_Fault = ANY_FAULT_OTHER_THAN_OUT_FAULT;	// Get new Fault
			}
			else	// There is only OUT Fault
			{
				New_Fault = OUT_FAULT_FLAG;
			}
		}
	}
	return New_Fault;
}

//#####################	Display Fault #############################################################################
// Function Name: ei_vDisplayFault
// Return Type: void
// Arguments:   Uint16 New_Fault
// Description: Display the new fault and return the fault that was displayed. The new fault may contain more than 1
//				faults set so that the displayed value and the argument may not be same.
//#################################################################################################################
static void ei_vDisplayFault(Uint16 New_Fault)
{
	Uint16 Nibble_Array[8];
	Uint16 Nibble_Counter = 0;
	Uint16 I2C_Tx_Data_Array[8];

	// Date: 2/01/2013
	// A bug was found that displayed all zeros on the voltage display and blank on current display when out key is pressed.
	// This happened momentarily before display changed back to Output voltage and current.
	// The bug came because there is a delay between state transition from Standby mode to
	// Armed power mode. Because of that when no faults are there and output yet to be switched on, the code will
	// hit the else condition inside the first if below. To avoid that we start initialise the nibble array to display blank initially.
	Nibble_Array[0] = DISPLAY_BLANK;
	Nibble_Array[1] = DISPLAY_BLANK;
	Nibble_Array[2] = DISPLAY_BLANK;
	Nibble_Array[3] = DISPLAY_BLANK;
	Nibble_Array[4] = DISPLAY_BLANK;
	Nibble_Array[5] = DISPLAY_BLANK;
	Nibble_Array[6] = DISPLAY_BLANK;
	Nibble_Array[7] = DISPLAY_BLANK;

	if((New_Fault & SHUTOFF_FAULT_FLAG) == SHUTOFF_FAULT_FLAG)
	{
		ui_Last_Fault_Displayed = SHUTOFF_FAULT_FLAG;
		New_Fault &= SHUTOFF_FAULT_FLAG; // Clear other faults, if any in New_Fault

		Nibble_Array[3] = 5;
		Nibble_Array[2] = 'H';
		Nibble_Array[1] = 'U';
		Nibble_Array[0] = 'T';
		Nibble_Array[7] = 0;
		Nibble_Array[6] = 'F';
		Nibble_Array[5] = 'F';
		Nibble_Array[4] = DISPLAY_BLANK;
	}
	else if((New_Fault & AC_FAULT_FLAG) == AC_FAULT_FLAG)
	{
		ui_Last_Fault_Displayed = AC_FAULT_FLAG;
		New_Fault &= AC_FAULT_FLAG; // Clear other faults if any in New_Fault

		Nibble_Array[3] = DISPLAY_BLANK;
		Nibble_Array[2] = DISPLAY_BLANK;
		Nibble_Array[1] = 'A';
		Nibble_Array[0] = 'C';
		Nibble_Array[7] = 'F';
		Nibble_Array[6] = 'L';
		Nibble_Array[5] = 'T';
		Nibble_Array[4] = DISPLAY_BLANK;
	}
	else if((New_Fault & FOLD_FAULT_FLAG) == FOLD_FAULT_FLAG)
	{
		ui_Last_Fault_Displayed = FOLD_FAULT_FLAG;
		New_Fault &= FOLD_FAULT_FLAG; // Clear other faults if any in New_Fault

		Nibble_Array[3] = 'F';
		Nibble_Array[2] = 0;
		Nibble_Array[1] = 'L';
		Nibble_Array[0] = 'D';
		Nibble_Array[7] = 'F';
		Nibble_Array[6] = 'L';
		Nibble_Array[5] = 'T';
		Nibble_Array[4] = DISPLAY_BLANK;
	}
	else if((New_Fault & INTERLOCK_FAULT_FLAG) == INTERLOCK_FAULT_FLAG)
	{
		ui_Last_Fault_Displayed = INTERLOCK_FAULT_FLAG;
		New_Fault &= INTERLOCK_FAULT_FLAG; // Clear other faults if any in New_Fault

		Nibble_Array[3] = DISPLAY_BLANK;
		Nibble_Array[2] = 1;
		Nibble_Array[1] = 'N';
		Nibble_Array[0] = 'L';
		Nibble_Array[7] = 'F';
		Nibble_Array[6] = 'L';
		Nibble_Array[5] = 'T';
		Nibble_Array[4] = DISPLAY_BLANK;
	}
	else if((New_Fault & OTP_FAULT_FLAG) == OTP_FAULT_FLAG)
	{
		ui_Last_Fault_Displayed = OTP_FAULT_FLAG;
		New_Fault &= OTP_FAULT_FLAG; // Clear other faults if any in New_Fault

		Nibble_Array[3] = DISPLAY_BLANK;
		Nibble_Array[2] = 0;
		Nibble_Array[1] = 'T';
		Nibble_Array[0] = 'P';
		Nibble_Array[7] = 'F';
		Nibble_Array[6] = 'L';
		Nibble_Array[5] = 'T';
		Nibble_Array[4] = DISPLAY_BLANK;
	}
	else if((New_Fault & OVP_FAULT_FLAG) == OVP_FAULT_FLAG)
	{
		ui_Last_Fault_Displayed = OVP_FAULT_FLAG;
		New_Fault &= OVP_FAULT_FLAG; // Clear other faults if any in New_Fault

		Nibble_Array[3] = DISPLAY_BLANK;
		Nibble_Array[2] = 0;
		Nibble_Array[1] = 'U';
		Nibble_Array[0] = 'P';
		Nibble_Array[7] = 'F';
		Nibble_Array[6] = 'L';
		Nibble_Array[5] = 'T';
		Nibble_Array[4] = DISPLAY_BLANK;
	}
	else if((New_Fault & CAN_COMM_FAULT_FLAG) == CAN_COMM_FAULT_FLAG)
	{
        ui_Last_Fault_Displayed = CAN_COMM_FAULT_FLAG;
        New_Fault &= CAN_COMM_FAULT_FLAG; // Clear other faults if any in New_Fault

        Nibble_Array[3] = DISPLAY_BLANK;
        Nibble_Array[2] = 'C';
        Nibble_Array[1] = 'A';
        Nibble_Array[0] = 'N';
        Nibble_Array[7] = 'F';
        Nibble_Array[6] = 'L';
        Nibble_Array[5] = 'T';
        Nibble_Array[4] = DISPLAY_BLANK;
	}
	/*else if((New_Fault & FAN_FAULT_FLAG) == FAN_FAULT_FLAG)
	{
		ui_Last_Fault_Displayed = FAN_FAULT_FLAG;
		New_Fault &= FAN_FAULT_FLAG; // Clear other faults if any in New_Fault

		Nibble_Array[3] = DISPLAY_BLANK;
		Nibble_Array[2] = 'F';
		Nibble_Array[1] = 'A';
		Nibble_Array[0] = 'N';
		Nibble_Array[7] = 'F';
		Nibble_Array[6] = 'L';
		Nibble_Array[5] = 'T';
		Nibble_Array[4] = DISPLAY_BLANK;
	}*/
	else if(((New_Fault & OUT_FAULT_FLAG) == OUT_FAULT_FLAG) || ((New_Fault & PFC_FAULT_FLAG) == PFC_FAULT_FLAG))	// Always check this last for accurate operation
	{
		ui_Last_Fault_Displayed = OUT_FAULT_FLAG;
		New_Fault &= OUT_FAULT_FLAG; // Clear other faults if any in New_Fault

		Nibble_Array[3] = DISPLAY_BLANK;
		Nibble_Array[2] = 0;
		Nibble_Array[1] = 'F';
		Nibble_Array[0] = 'F';
		Nibble_Array[7] = DISPLAY_BLANK;
		Nibble_Array[6] = DISPLAY_BLANK;
		Nibble_Array[5] = DISPLAY_BLANK;
		Nibble_Array[4] = DISPLAY_BLANK;
	}
	else //Could be any of the fault that don't require user notification.
	{
//		Display will go blank in this case and blink only LED. And if it's an I2C bus failure there can be no user notification at all
//      since display itself is I2C driven.
		ui_Last_Fault_Displayed = UNDISPLAYED_FAULTS;

		// In case of AC fail, PFC fault was found to be detected first. To override PFC for AC we check again for AC fail here.
		if((Fault_Regs.FAULT_REGISTER.all & AC_FAULT_FLAG) == AC_FAULT_FLAG)
		{
			ui_Last_Fault_Displayed = AC_FAULT_FLAG;
			New_Fault &= AC_FAULT_FLAG; // Clear other faults if any in New_Fault

			Nibble_Array[3] = DISPLAY_BLANK;
			Nibble_Array[2] = DISPLAY_BLANK;
			Nibble_Array[1] = 'A';
			Nibble_Array[0] = 'C';
			Nibble_Array[7] = 'F';
			Nibble_Array[6] = 'L';
			Nibble_Array[5] = 'T';
			Nibble_Array[4] = DISPLAY_BLANK;
		}
	}

	// Do not display fault when
	// 1. Out Fault and Encoder operation is not Set_Voltage_Current
						//or
	// 2. Out Fault and (Display mode is Preview/FrontPanel_Access/Start_Mode
	if((ui_Last_Fault_Displayed == OUT_FAULT_FLAG) && ((State.Mode.bt_Encoder_Operation != SET_VOLTAGE_CURRENT)||(State.Mode.bt_Display_Mode == DISPLAY_PREVIEW)||(Global_Flags.fg_Display_FP_Access_Mode == SET)||(Global_Flags.fg_Display_PSU_StartMode == SET))) // Purely time saving tactic. Do not display off when not required
	{
		return;
	}

	else	// We need to display the fault only.
	{
		//  Convert the Character to be displayed to appropriate value for Digit Register and put it on the I2C Bus
	   	Nibble_Counter = 0;
	   	while(Nibble_Counter < 2*NUMBER_OF_DIGITS_DISPLAY)
	   	{
	   		I2C_Tx_Data_Array[Nibble_Counter] = ei_uiDisplayDigitMapping(Nibble_Array[Nibble_Counter]);
			Nibble_Counter++;
	   	}

		if(Global_Flags.fg_State_Of_Display == OFF)
		{
			ei_vBlankDisplay(MAX6959A_ADDRESS);
			ei_vBlankDisplay(MAX6958B_ADDRESS);
		}
		else
		{
			ei_uiI2CWrite(MAX6959A_ADDRESS, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, I2C_Tx_Data_Array);
			ei_uiI2CWrite(MAX6958B_ADDRESS, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, &I2C_Tx_Data_Array[4]);
		}
	}
}

//#####################	Display Voltage Current ###################################################################
// Function Name: ei_uiDisplayVoltageCurrent
// Return Type: Uint16 Display_Driver_Address
// Arguments:   Uint16 Decimal_digit_Point
// Description: Display voltage & current in SET_VOLTAGE_CURRENT mode.
//#################################################################################################################
static Uint16 ei_uiDisplayVoltageCurrent(Uint16 Display_Driver_Address)
{
	Uint16 DecimalShiftMultiplier = 0;
	int32 DisplayValue = 0;
	Uint16 Decimal_digit_Point = 0;
	Uint16 Nibble_Counter = 0;
	Uint16 Nibble_Array[4];
	Uint16 I2C_Tx_Data_Array[4];

//		For Preview use Reference values calculated by the software
	if(State.Mode.bt_Display_Mode == DISPLAY_PREVIEW)
	{
		if(Display_Driver_Address == MAX6959A_ADDRESS) // Voltage Encoder
		{
   	    	DisplayValue = Reference.iq_Voltage_Reference;	// Copy Preview Voltage
		}
   		else // Current Encoder
   		{
   			DisplayValue = Reference.iq_Current_Reference;	// Copy preview Current
   		}
	}
	else // Display Output. Take AD7705 values.
	{
		if(Display_Driver_Address == MAX6959A_ADDRESS) // Voltage Encoder
		{
   	    	DisplayValue = External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Calibrated_Value;	// Copy Display Voltage
		}
   		else // Current Encoder
   		{
   			DisplayValue = External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Calibrated_Value;	// Copy Display Current
   		}
	}

	// Check the Range of Number to determine location of decimal point. All values to be converted to four digits
	// and location of the decimal point.

	// The following ifs give five digit nos which is rounded and converted to four digits.
	if ( _IQ15int(DisplayValue) >= 0 && _IQ15int(DisplayValue) < TWO_DIGIT)
   	{
   		DecimalShiftMultiplier = 1000;
   		Decimal_digit_Point = DIGIT3_DECIMAL_POINT;
   	}
   	else if ( _IQ15int(DisplayValue) >= TWO_DIGIT && _IQ15int(DisplayValue) < THREE_DIGIT)
   	{
   		DecimalShiftMultiplier = 100;
   		Decimal_digit_Point = DIGIT2_DECIMAL_POINT;
   	}
   	else if ( _IQ15int(DisplayValue) >= THREE_DIGIT && _IQ15int(DisplayValue) < FOUR_DIGIT)
   	{
   		DecimalShiftMultiplier = 10;
    	Decimal_digit_Point = DIGIT1_DECIMAL_POINT;
   	}

   	// Bug observed on 25-02-2013. Do not use rmpy here. It will make 9.9999 look 0.000 because the DecimalShiftMultiplier used will be for 1 digit but result become 2 digit.
//	DisplayValue = _IQ15mpy(DisplayValue,DecimalShiftMultiplier);

   	// Bug observed after making the changes above: Without using rmpy the display shows 8.339 for 8.3399 which is
   	// clearly not accepted. Rounding after the 4th significant digit is required.
   	// But rounding will cause 9.9999 to look like 0.000, 99.999 to look like 00.00 etc. as mentioned above.

   	// So the solution is to use rmpy and use and additional if to see if Display value has jumped up one place.
   	// For eg. when working with 9.9999 we multiply 1000 and round it. This leads to display value being changed to
   	// 10000 but the code will display only last 4 digits and put the decimal point after the 1st digit.

   	// Thus 2 things are required
   	// 1. Bring back display value to 4 digit.
   	// 2. Shift decimal point to next number.

   	DisplayValue = _IQ15rmpy(DisplayValue,DecimalShiftMultiplier);

   	// Check if Display value has become five digit because of rounding
   	if(DisplayValue == FIVE_DIGIT)
   	{
   		// Make it four digit
   		DisplayValue = FOUR_DIGIT;

   		// Shift the decimal point location right by 1
   		Decimal_digit_Point >>= 1;

		// But never allow digit 0 to light up it's decimal point in case of 999.99
		// being displayed as "1000."
		if(Decimal_digit_Point == DIGIT0_DECIMAL_POINT)
		{
			Decimal_digit_Point = NO_DECIMAL_POINT;
		}
   	}

	//Store the 4 digits in an array. Nibble_Array[3] stores the Most significant digit.
	Nibble_Counter =0;
	while(Nibble_Counter < NUMBER_OF_DIGITS_DISPLAY)
	{
		Nibble_Array[Nibble_Counter] = DisplayValue % 10;
		DisplayValue = DisplayValue /10;
		Nibble_Counter++;
	}

	//  Convert the Character to be displayed to appropriate value for Digit Register and put it on the I2C Bus
   	Nibble_Counter = 0;
   	while(Nibble_Counter < NUMBER_OF_DIGITS_DISPLAY)
   	{
   		I2C_Tx_Data_Array[Nibble_Counter] = ei_uiDisplayDigitMapping(Nibble_Array[Nibble_Counter]);
		Nibble_Counter++;
   	}

   	if(Global_Flags.fg_State_Of_Display == OFF)
	{
		ei_vBlankDisplay(Display_Driver_Address);
	}
	else
	{
		ei_uiI2CWrite(Display_Driver_Address, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, I2C_Tx_Data_Array);
	}

	return Decimal_digit_Point;
}

//#####################	Display OVP ###############################################################################
// Function Name: ei_uiDisplayUVL
// Return Type: Uint16 Display_Driver_Address
// Arguments:   Uint16 Decimal_digit_Point
// Description: Display OVP setting and 'OVP'.
//#################################################################################################################
static Uint16 ei_uiDisplayOVP(Uint16 Display_Driver_Address)
{
	Uint16 DecimalShiftMultiplier = 0;
	int32 DisplayValue = 0;
	Uint16 Decimal_digit_Point = 0;
	Uint16 Nibble_Counter = 0;
	Uint16 Nibble_Array[4];
	Uint16 I2C_Tx_Data_Array[4];

	if(Display_Driver_Address == MAX6959A_ADDRESS) // Voltage Encoder will display set OVP
	{
   	   	DisplayValue = Reference.iq_OVP_Reference;

   	   	// Check the Range of Number to determine location of decimal point. All values to be converted to four digits
   		// and location of the decimal point.

   		// The following ifs give five digit nos which is rounded and converted to four digits.
		if ( _IQ15int(DisplayValue) >= 0 && _IQ15int(DisplayValue) < TWO_DIGIT)
   		{
   			DecimalShiftMultiplier = 1000;
   			Decimal_digit_Point = DIGIT3_DECIMAL_POINT;
   		}
   		else if ( _IQ15int(DisplayValue) >= TWO_DIGIT && _IQ15int(DisplayValue) < THREE_DIGIT)
   		{
   			DecimalShiftMultiplier = 100;
   			Decimal_digit_Point = DIGIT2_DECIMAL_POINT;
   		}
   		else if ( _IQ15int(DisplayValue) >= THREE_DIGIT && _IQ15int(DisplayValue) < FOUR_DIGIT)
   		{
   			DecimalShiftMultiplier = 10;
        	Decimal_digit_Point = DIGIT1_DECIMAL_POINT;
   		}

   		DisplayValue = _IQ15rmpy(DisplayValue,DecimalShiftMultiplier);

   		// Check if Display value has become five digit because of rounding
	   	if(DisplayValue == FIVE_DIGIT)
	   	{
	   		// Make it four digit
	   		DisplayValue = FOUR_DIGIT;

	   		// Shift the decimal point location right by 1
	   		Decimal_digit_Point >>= 1;

			// But never allow digit 0 to light up it's decimal point in case of 999.99
			// being displayed as "1000."
			if(Decimal_digit_Point == DIGIT0_DECIMAL_POINT)
			{
				Decimal_digit_Point = NO_DECIMAL_POINT;
			}
	   	}

		//Store the 4 digits in an array. Nibble_Array[3] stores the Most significant digit.
		Nibble_Counter =0;
		while(Nibble_Counter < NUMBER_OF_DIGITS_DISPLAY)
		{
			Nibble_Array[Nibble_Counter] = DisplayValue % 10;
			DisplayValue = DisplayValue /10;
			Nibble_Counter++;
		}
	}
	else 	// MAX6958 will display " OUP"
	{
		Nibble_Array[3] = DISPLAY_BLANK;
		Nibble_Array[2] = 0;
		Nibble_Array[1] = 'U';
		Nibble_Array[0] = 'P';
	}

	//  Convert the Character to be displayed to appropriate value for Digit Register and put it on the I2C Bus
   	Nibble_Counter = 0;
   	while(Nibble_Counter < NUMBER_OF_DIGITS_DISPLAY)
   	{
   		I2C_Tx_Data_Array[Nibble_Counter] = ei_uiDisplayDigitMapping(Nibble_Array[Nibble_Counter]);
		Nibble_Counter++;
   	}

   	if(Global_Flags.fg_State_Of_Display == OFF)
	{
		ei_vBlankDisplay(Display_Driver_Address);
	}
	else
	{
		ei_uiI2CWrite(Display_Driver_Address, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, I2C_Tx_Data_Array);
	}

	return Decimal_digit_Point;
}

//#####################	Display UVL ###############################################################################
// Function Name: ei_uiDisplayUVL
// Return Type: Uint16 Display_Driver_Address
// Arguments:   Uint16 Decimal_digit_Point
// Description: Display UVL setting and " UVL".
//#################################################################################################################
static Uint16 ei_uiDisplayUVL(Uint16 Display_Driver_Address)
{
	Uint16 DecimalShiftMultiplier = 0;
	int32 DisplayValue = 0;
	Uint16 Decimal_digit_Point = 0;
	Uint16 Nibble_Counter = 0;
	Uint16 Nibble_Array[4];
	Uint16 I2C_Tx_Data_Array[4];

	if(Display_Driver_Address == MAX6959A_ADDRESS) // Voltage Encoder will display set OVP
	{
   	   	DisplayValue = Reference.iq_UVL_Reference;

   	   	// Check the Range of Number to determine location of decimal point. All values to be converted to four digits
   		// and location of the decimal point.

   		// The following ifs give five digit nos which is rounded and converted to four digits.
		if ( _IQ15int(DisplayValue) >= 0 && _IQ15int(DisplayValue) < TWO_DIGIT)
   		{
   			DecimalShiftMultiplier = 1000;
   			Decimal_digit_Point = DIGIT3_DECIMAL_POINT;
   		}
   		else if ( _IQ15int(DisplayValue) >= TWO_DIGIT && _IQ15int(DisplayValue) < THREE_DIGIT)
   		{
   			DecimalShiftMultiplier = 100;
   			Decimal_digit_Point = DIGIT2_DECIMAL_POINT;
   		}
   		else if ( _IQ15int(DisplayValue) >= THREE_DIGIT && _IQ15int(DisplayValue) < FOUR_DIGIT)
   		{
   			DecimalShiftMultiplier = 10;
        	Decimal_digit_Point = DIGIT1_DECIMAL_POINT;
   		}

   		DisplayValue = _IQ15rmpy(DisplayValue,DecimalShiftMultiplier);

   		// Check if Display value has become five digit because of rounding
	   	if(DisplayValue == FIVE_DIGIT)
	   	{
	   		// Make it four digit
	   		DisplayValue = FOUR_DIGIT;

	   		// Shift the decimal point location right by 1
	   		Decimal_digit_Point >>= 1;

			// But never allow digit 0 to light up it's decimal point in case of 999.99
			// being displayed as "1000."
			if(Decimal_digit_Point == DIGIT0_DECIMAL_POINT)
			{
				Decimal_digit_Point = NO_DECIMAL_POINT;
			}
	   	}

		//Store the 4 digits in an array. Nibble_Array[3] stores the Most significant digit.
		Nibble_Counter =0;
		while(Nibble_Counter < NUMBER_OF_DIGITS_DISPLAY)
		{
			Nibble_Array[Nibble_Counter] = DisplayValue % 10;
			DisplayValue = DisplayValue /10;
			Nibble_Counter++;
		}
	}
	else 	// MAX6958 will display " UUL"
	{
		Nibble_Array[3] = DISPLAY_BLANK;
		Nibble_Array[2] = 'U';
		Nibble_Array[1] = 'U';
		Nibble_Array[0] = 'L';
	}

	//  Convert the Character to be displayed to appropriate value for Digit Register and put it on the I2C Bus
   	Nibble_Counter = 0;
   	while(Nibble_Counter < NUMBER_OF_DIGITS_DISPLAY)
   	{
   		I2C_Tx_Data_Array[Nibble_Counter] = ei_uiDisplayDigitMapping(Nibble_Array[Nibble_Counter]);
		Nibble_Counter++;
   	}
	if(Global_Flags.fg_State_Of_Display == OFF)
	{
		ei_vBlankDisplay(Display_Driver_Address);
	}
	else
	{
		ei_uiI2CWrite(Display_Driver_Address, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, I2C_Tx_Data_Array);
	}

	return Decimal_digit_Point;
}

//#####################	Display AddrBaud ##################################################################
// Function Name: ei_uiDisplayAddrBaud
// Return Type: Uint16 Display_Driver_Address
// Arguments:   Uint16 Decimal_digit_Point
// Description: Display Address and BaudRate. Accept Display Driver's address and return
//				location of the Decimap point to calling function. (Always zero here)
//
//				Voltage Display: Address from 0 to 30
//				Current display: 1200,2400,4800,9600,19200
//#################################################################################################################
static Uint16 ei_uiDisplayAddrBaud(Uint16 Display_Driver_Address)
{
	Uint16 Nibble_Counter = 0;
	Uint16 Nibble_Array[4];
	Uint16 I2C_Tx_Data_Array[4];

	if(Display_Driver_Address == MAX6959A_ADDRESS)	// Address
	{
		if(Reference.ui_Set_Address < TWO_DIGIT)	// Without leading zeroes
		{
			Nibble_Array[3] = DISPLAY_BLANK;
			Nibble_Array[2] = DISPLAY_BLANK;
			Nibble_Array[1] = DISPLAY_BLANK;
			Nibble_Array[0] = Reference.ui_Set_Address;
		}
		else	// Two digit address
		{
			Nibble_Array[3] = DISPLAY_BLANK;
			Nibble_Array[2] = DISPLAY_BLANK;
			Nibble_Array[1] = Reference.ui_Set_Address / 10;	//Tenth Place
			Nibble_Array[0] = Reference.ui_Set_Address % 10;	//Unit Place
		}
	}
	else	// MAX6958 will display BaudRate
	{
		if(Reference.ui_Set_BaudRate > FIVE_DIGIT)
		{
			// Only 1 such value 19200. Display 1920
			Nibble_Array[3] = 1;
			Nibble_Array[2] = 9;
			Nibble_Array[1] = 2;
			Nibble_Array[0] = 0;
		}
		else // a four digit BaudRate. Can be accomodated
		{
			Nibble_Array[3] = Reference.ui_Set_BaudRate / 1000;		// Thousandth place
			Nibble_Array[2] = (Reference.ui_Set_BaudRate / 100) % 10;	// Hundredth place
			Nibble_Array[1] = (Reference.ui_Set_BaudRate / 10) % 10;	// Tenth Place
			Nibble_Array[0] = Reference.ui_Set_BaudRate % 10;			// Unit place
		}
	}

	//  Convert the Character to be displayed to appropriate value for Digit Register and put it on the I2C Bus
   	Nibble_Counter = 0;
   	while(Nibble_Counter < NUMBER_OF_DIGITS_DISPLAY)
   	{
   		I2C_Tx_Data_Array[Nibble_Counter] = ei_uiDisplayDigitMapping(Nibble_Array[Nibble_Counter]);
		Nibble_Counter++;
   	}

   	if(Global_Flags.fg_State_Of_Display == OFF)
	{
		ei_vBlankDisplay(Display_Driver_Address);
	}
	else
	{
		ei_uiI2CWrite(Display_Driver_Address, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, I2C_Tx_Data_Array);
	}

	return NO_DECIMAL_POINT;	// This is always returned as 0
}

//#####################	Display Other Parameters ##################################################################
// Function Name: ei_uiDisplayOtherParameters
// Return Type: Uint16 Display_Driver_Address
// Arguments:   Uint16 Decimal_digit_Point
// Description: Display AST/SAF or LFP/UFP depending on flags set. Accept Display Driver's address and return
//				location of the Decimap point to calling function. (Always zero here). Current encoder inside is
//				always blank.
//#################################################################################################################
static Uint16 ei_uiDisplayOtherParameters(Uint16 Display_Driver_Address)
{
	Uint16 Nibble_Counter = 0;
	Uint16 Nibble_Array[4];
	Uint16 I2C_Tx_Data_Array[4];

	if(Display_Driver_Address == MAX6959A_ADDRESS)
	{
		if(Global_Flags.fg_Display_FP_Access_Mode == SET)	// This will have higher priority than AST/SAF. Display this only
		{																							//  when both activated.
			if(State.Mode.bt_FrontPanel_Access == FRONTPANEL_LOCKED)	// Voltage Display will read " LFP"
			{
				Nibble_Array[3] = DISPLAY_BLANK;
				Nibble_Array[2] = 'L';
				Nibble_Array[1] = 'F';
				Nibble_Array[0] = 'P';
			}
			else	// Frontapanel Unlocked. Display " UFP"
			{
				Nibble_Array[3] = DISPLAY_BLANK;
				Nibble_Array[2] = 'U';
				Nibble_Array[1] = 'F';
				Nibble_Array[0] = 'P';
			}
		}
		else if(Global_Flags.fg_Display_PSU_StartMode == SET)
		{
			if(State.Mode.bt_Start_Mode == SAFE_START)	// Voltage Display will read " SAF"
			{
				Nibble_Array[3] = DISPLAY_BLANK;
				Nibble_Array[2] = 5;
				Nibble_Array[1] = 'A';
				Nibble_Array[0] = 'F';
			}
			else	// Auto Restart. Display " AUT"
			{
				Nibble_Array[3] = DISPLAY_BLANK;
				Nibble_Array[2] = 'A';
				Nibble_Array[1] = 'U';
				Nibble_Array[0] = 'T';
			}
		}
		else
			asm("	nop");

	}
	else // MAX6958 always blank here
	{
		Nibble_Array[3] = DISPLAY_BLANK;
		Nibble_Array[2] = DISPLAY_BLANK;
		Nibble_Array[1] = DISPLAY_BLANK;
		Nibble_Array[0] = DISPLAY_BLANK;

	// This must be cleared here only to ensure that the displayed value and the set value are same.
	// Put this in MAX6958 only.
		Global_Flags.fg_Display_FP_Access_Mode = CLEARED;
		Global_Flags.fg_Display_PSU_StartMode = CLEARED;
	}


	//  Convert the Character to be displayed to appropriate value for Digit Register and put it on the I2C Bus
   	Nibble_Counter = 0;
   	while(Nibble_Counter < NUMBER_OF_DIGITS_DISPLAY)
   	{
   		I2C_Tx_Data_Array[Nibble_Counter] = ei_uiDisplayDigitMapping(Nibble_Array[Nibble_Counter]);
		Nibble_Counter++;
   	}

   	if(Global_Flags.fg_State_Of_Display == OFF)
	{
		ei_vBlankDisplay(Display_Driver_Address);
	}
	else
	{
		ei_uiI2CWrite(Display_Driver_Address, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, I2C_Tx_Data_Array);
	}

	return NO_DECIMAL_POINT;	// This is always returned as 0
}

//#####################	Front Panel Indicator #####################################################################
// Function Name: ei_uiSetFrontPanelIndicator
// Return Type: void
// Arguments:   void
// Description: Set Front Panel indicator leds depending on status.

//	NAME		BOARD NOTATION		REGISTER VALUE		CONTROLLING DRIVER
//	CV_LED		D4					0x0008				MAX6959
//	OUT_LED		D3					0x0004				MAX6959
//	FINE_LED	D2					0x0002				MAX6959
//	PREVIEW		D1					0x0001				MAX6959

//	CC_LED		D8					0x0008				MAX6958
//	FOLD_LED	D7					0x0007				MAX6958
//	REMOTE_LED	D6					0x0002				MAX6958
//	FAULT_LED	D5					0x0001				MAX6958


//#################################################################################################################
static void ei_uiSetFrontPanelIndicator(Uint16 Display_Driver_Address, Uint16 Point_Location, Uint16 Fault_Led)
{
	Uint16 I2C_Tx_Data_Array[1];
	I2C_Tx_Data_Array[0] = 0;
	I2C_Tx_Data_Array[0] |= Point_Location;

	if(Display_Driver_Address == MAX6958B_ADDRESS)
	{
		// CC LED
		if(CC_MODE & OUTPUT_ON)
		{
			I2C_Tx_Data_Array[0] |= CC_LED;
		}
		else // CV or output off
		{
			I2C_Tx_Data_Array[0] &= ~CC_LED;
		}

		// FOLD LED. Lights up in case of CC as well as CV protection
		if(State.Mode.bt_FoldBack_Mode != FOLDBACK_DISARMED)
		{
			if(Global_Flags.fg_Blink_Fold_Led == TRUE)
			{
				// Switch Off for the blinking operation of 500ms
				I2C_Tx_Data_Array[0] &= ~FOLD_LED;
				Global_Flags.fg_Blink_Fold_Led = FALSE;
			}
			else
			{
				I2C_Tx_Data_Array[0] |= FOLD_LED;
			}
		}
		else // FOLDBACK DISABLED
		{
			I2C_Tx_Data_Array[0] &= ~FOLD_LED;
		}

		// REMOTE LED
		if((State.Mode.bt_PSU_Control != LOCAL) || (PSU_IN_ANALOG_REMOTE))	// Remote mode (Non_Latched or Local Lockout or Analog remote)
		{
			I2C_Tx_Data_Array[0] |= REMOTE_LED;

			if(PSU_IN_ANALOG_REMOTE)
			{
				// Here we also set Analog remote bit in operation condition register
				Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Analog_Control = SET;
			}
			else	// Not in Analog remote. Could be Local Lockout or Non latched remote
			{
				Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Analog_Control = CLEARED;
			}
		}
		else // Local
		{
			I2C_Tx_Data_Array[0] &= ~REMOTE_LED;

			// In case this bit set
			Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Analog_Control = CLEARED;
		}

		// FAULT LED
		if(Fault_Led == DONT_BLINK)
		{
			I2C_Tx_Data_Array[0] &= ~FAULT_LED;
		}
		else
		{
			if(ui_Fault_Led_Previous_State == ON)
			{
				I2C_Tx_Data_Array[0] &= ~FAULT_LED;
				ui_Fault_Led_Previous_State= OFF;
			}
			else
			{
				I2C_Tx_Data_Array[0] |= FAULT_LED;
				ui_Fault_Led_Previous_State = ON;
			}
		}

	}
	else if(Display_Driver_Address == MAX6959A_ADDRESS)
	{
		// CV LED
		if(CV_MODE & OUTPUT_ON)
		{
			I2C_Tx_Data_Array[0] |= CV_LED;
		}
		else // CC or output off
		{
			I2C_Tx_Data_Array[0] &= ~CV_LED;
		}

		// OUT_LED
		if(OUTPUT_ON)
		{
			I2C_Tx_Data_Array[0] |= OUT_LED;
		}
		else
		{
			I2C_Tx_Data_Array[0] &= ~OUT_LED;
		}

		// FINE_LED
		if(State.Mode.bt_Encoder_Rotation_Mode == FINE_ROTATION)
		{
			I2C_Tx_Data_Array[0] |= FINE_LED;
		}
		else 	// COARSE_ROTATION
		{
			I2C_Tx_Data_Array[0] &= ~FINE_LED;
		}

		// PREVIEW_LED
		if(State.Mode.bt_Display_Mode == DISPLAY_PREVIEW)
		{
			I2C_Tx_Data_Array[0] |= PREVIEW_LED;
		}
		else	// Display_Output
		{
			I2C_Tx_Data_Array[0] &= ~PREVIEW_LED;
		}
	}
	else
	{
		asm("	nop");
	}
	ei_uiI2CWrite(Display_Driver_Address, DIGIT_DISPLAY_DECIMAL_POINT_AND_SEGMENT_LED_ADDRESS, 1, I2C_Tx_Data_Array);
}

//#####################	Digit Mapping for Display #################################################################
// Function Name: ei_uiDisplayDigitMapping
// Return Type: Uint16 Digit_To_Be_Displayed
// Arguments:   void
// Description: Digit Mapping. Accepts an int or char value to be displayed and returns the value to be put in
//				Driver register. The driver is being operated in non hexadecimal decode mode. Therefore each
//				bit in the Driver's DIGIT register corresponds to one LED in that seven segment display.

									// Mapping process
//	CHARACTER 	Bit7(X)		Bit6(a)		Bit5(b)		Bit4(c)		Bit3(d)		Bit2(e)		Bit1(f)		Bit0(g)		HEX
//	0
//	1			0			0			1			1			0			0			0			0			0x30
//	2
//	4
//	5
//	6
//	7
//	8
//	9
//	A
//	B
//	C
//	D
//	E
//	F
//  H			0			0			1			1			0			1			1			1			0x37
//	U
//	P
//	t
//	n
//	L
//	BLANK		0			0			0			0			0			0			0			0			0x00
//#################################################################################################################
static Uint16 ei_uiDisplayDigitMapping(Uint16 Digit_To_Be_Displayed)
{
	Uint16 Equivalent_Digit_Register_Value;

	switch (Digit_To_Be_Displayed)
	{
		case 0:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_0;
			break;
		}

		case 1:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_1;
			break;
		}

		case 2:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_2;
			break;
		}

		case 3:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_3;
			break;
		}

		case 4:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_4;
			break;
		}

		case 5:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_5;
			break;
		}

		case 6:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_6;
			break;
		}

		case 7:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_7;
			break;
		}

		case 8:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_8;
			break;
		}

		case 9:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_9;
			break;
		}

		case 10:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_A;
			break;
		}

		case 11:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_B;
			break;
		}

		case 12:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_C;
			break;
		}

		case 13:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_D;
			break;
		}

		case 14:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_E;
			break;
		}

		case 15:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_F;
			break;
		}

		case 'A':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_A;
			break;
		}

		case 'B':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_B;
			break;
		}

		case 'C':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_C;
			break;
		}

		case 'D':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_D;
			break;
		}
		case 'E':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_E;
			break;
		}
		case 'F':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_F;
			break;
		}
		case 'H':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_H;
			break;
		}
		case 'a':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_A;
			break;
		}
		case 'b':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_B;
			break;
		}
		case 'c':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_C;
			break;
		}
		case 'd':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_D;
			break;
		}
		case 'e':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_E;
			break;
		}

		case 'f':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_F;
			break;
		}
		case 'h':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_H;
			break;
		}
		case 'U':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_U;
			break;
		}

		case 'P':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_P;
			break;
		}

		case 'T':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_T;
			break;
		}

		case 'N':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_N;
			break;
		}

		case 'L':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_L;
			break;
		}

		case 'u':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_U;
			break;
		}

		case 'p':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_P;
			break;
		}

		case 't':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_T;
			break;
		}

		case 'n':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_N;
			break;
		}

		case 'l':
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_L;
			break;
		}

		case DISPLAY_BLANK:
		{
			Equivalent_Digit_Register_Value = DIGIT_REGISTER_MAPPING_FOR_DISPLAY_BLANK;
			break;
		}
	}
	return (Equivalent_Digit_Register_Value);
}

//#####################	Blank Display #############################################################################
// Function Name: ei_vBlankDisplay
// Return Type: void
// Arguments:   Uint16 Display_Driver_Address
// Description: Blanks the Voltage and current display. Power saving feature.
//#################################################################################################################
static void ei_vBlankDisplay(Uint16 Display_Driver_Address)
{
	Uint16 Nibble_Counter = 0;
	Uint16 Nibble_Array[4];
	Uint16 I2C_Tx_Data_Array[4];

	Nibble_Array[3] = DISPLAY_BLANK;
	Nibble_Array[2] = DISPLAY_BLANK;
	Nibble_Array[1] = DISPLAY_BLANK;
	Nibble_Array[0] = DISPLAY_BLANK;

	//  Convert the Character to be displayed to appropriate value for Digit Register and put it on the I2C Bus
   	Nibble_Counter = 0;
   	while(Nibble_Counter < NUMBER_OF_DIGITS_DISPLAY)
   	{
   		I2C_Tx_Data_Array[Nibble_Counter] = ei_uiDisplayDigitMapping(Nibble_Array[Nibble_Counter]);
		Nibble_Counter++;
   	}
   	ei_uiI2CWrite(Display_Driver_Address, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, I2C_Tx_Data_Array);
}
