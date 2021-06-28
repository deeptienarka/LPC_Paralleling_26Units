//#################################################################################################################
//# 							enArka Instruments proprietary
//# File: ei_EncoderAndReferences.c
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.
//# All Rights Reserved
//#
//#################################################################################################################

//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
#include "ExternalVariables.h"
//#################################################################################################################
// Specific defines
#define ADDRESS_FIELD_CLEAR_WORD										0xFFFF00FF
#define ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR							8
//#################################################################################################################
									/*-------Global Prototypes----------*/
//void ei_vSetReferences();
//void ei_vSetOvpReference();
//void ei_vSetVoltageReference();
//void ei_vSetCurrentReference();
//void ei_vVoltageEncoderOperation();
//void ei_vCurrentEncoderOperation();
//void ei_vAdjustOvp();
//#################################################################################################################
									/*-------Local Prototypes----------*/
static Uint16 ei_uiReadEncoder(Uint16 EQep_X);

//#####################	Calculate and Set references ###############################################################
// Function Name: void ei_vSetReferences
// Return Type: void
// Arguments:   void
// Description: 1. Calculate references based on Encoder values and set the PWMs and DACs
//				2. Update SCI baud rate if changed
//				3. Update PSU address if changed and also update the CAN mailboxes based on the new PSU address set.

// 			Immediate_Update_Flags for Voltage and Current are set whenever:
// 	1. References are changed from Encoder. (Encoder values are modified in the scheduled event)
// 	2. Last settings retrieved.	(Encoder Values modified in the ei_vRetrieveLastSettings() itself)
// 	3. PSU Reset to Safe State. (Encoder Values modified in the ei_vResetPsuToSafeState() itself)
// The setting of the references are done here after checking the above flags

//			Math:
//	1. iq_Voltage_Reference = (Encoder_Voltage_Counts * SVUL) / 575
//	2. iq_Current_Reference = (Encoder_Current_Counts * SCUL) / 575
//	3. iq_OVP_Reference = (Encoder_Voltage_Counts * SVUL) / 575
//	4. ui_Set_Address = Encoder_BaudRate_Count/20;  ===> 20 counts of QEP correspond to every Address value.
//	5. Set_Baudrate = 1200 << pow(2,(Qep_Baudrate_Counts/20))
//#################################################################################################################
void ei_vSetReferences()
{
	//	Perform Division first to be in the IQ range
	if(Global_Flags.fg_Voltage_Remote_control == FALSE)	// PSU in encoder control
	{
		Reference.iq_Voltage_Reference = _IQ15rmpy(_IQ15div(Encoder.iq_Qep_Voltage_Count,_IQ15(QEP_FULL_ROTATIONS_COUNT)),Product_Info.iq_Specified_FS_Voltage);

		/*// Multiply first and divide later
		Reference.iq_Voltage_Reference = _IQ15div(_IQ15mpy(Encoder.iq_Qep_Voltage_Count,Product_Info.iq_Specified_FS_Voltage),_IQ15(QEP_FULL_ROTATIONS_COUNT));*/
	}

	if(Global_Flags.fg_Current_Remote_control == FALSE)	// PSU in encoder control
	{
		//	Perform Division first to be in the IQ range
		Reference.iq_Current_Reference = _IQ15rmpy(_IQ15div(Encoder.iq_Qep_Current_Count,_IQ15(QEP_FULL_ROTATIONS_COUNT)),Product_Info.iq_Specified_FS_Current);

		/*// Multiply first and divide later
		Reference.iq_Current_Reference = _IQ15div(_IQ15mpy(Encoder.iq_Qep_Current_Count,Product_Info.iq_Specified_FS_Current),_IQ15(QEP_FULL_ROTATIONS_COUNT));*/
	}

	if(Global_Flags.fg_OVP_Remote_control == FALSE)		// PSU in encoder control
	{
		//	Perform Division first to be in the IQ range
		Reference.iq_OVP_Reference = _IQ15rmpy(_IQ15div(_IQ15(Encoder.ui_Qep_OVP_Count),_IQ15(QEP_FULL_ROTATIONS_COUNT)),Product_Info.iq_Specified_FS_Voltage);

		/*// Multiply first and divide later
		Reference.iq_OVP_Reference = _IQ15div(_IQ15mpy(_IQ15(Encoder.ui_Qep_OVP_Count),Product_Info.iq_Specified_FS_Voltage),_IQ15(QEP_FULL_ROTATIONS_COUNT));*/
	}

	if(Global_Flags.fg_UVL_Remote_control == FALSE)		// PSU in encoder control
	{
		//	Perform Division first to be in the IQ range
		// UVL Reference Calculated purely for display. No PWMs set
		Reference.iq_UVL_Reference = _IQ15rmpy(_IQ15div(_IQ15(Encoder.ui_Qep_UVL_Count),_IQ15(QEP_FULL_ROTATIONS_COUNT)),Product_Info.iq_Specified_FS_Voltage);

		/*// Multiply first and divide later works out better
		Reference.iq_UVL_Reference = _IQ15div(_IQ15mpy(_IQ15(Encoder.ui_Qep_UVL_Count),Product_Info.iq_Specified_FS_Voltage),_IQ15(QEP_FULL_ROTATIONS_COUNT));*/
	}

	ei_vSwitchSpiClkPolarity(DAC_8552);

	if(Reference.fg_Voltage_Reference_Update_Immediate_Flag == TRUE)
	{

#if 	MODE_OF_OPERATION == EQC
		ei_vCurrentCurveFitting();
		ei_vSetCurrentReference();
#endif
		ei_vSetVoltageReference();
		Reference.fg_Voltage_Reference_Update_Immediate_Flag = FALSE;
	}

	if(Reference.fg_Current_Reference_Update_Immediate_Flag == TRUE)
	{
#if		MODE_OF_OPERATION == EQC
		ei_vVoltageCurveFitting();
		ei_vSetVoltageReference();
#endif
		ei_vSetCurrentReference();
		Reference.fg_Current_Reference_Update_Immediate_Flag = FALSE;
	}

	ei_vSwitchSpiClkPolarity(AD7705);

	ei_vSetOvpReference();

	if(Global_Flags.fg_Immediate_Update_Sci_Baud == TRUE)	// Update Baudrate only when it's changed
	{
		Reference.ui_Set_BaudRate = (BASE_BAUD_RATE) << (Encoder.ui_Qep_BaudRate_Count / BAUD_RATE_CALCULATION_CONSTANT);

		SciaRegs.SCICTL1.bit.SWRESET = HOLD_IN_RESET;	// Hold in Reset

		switch(Reference.ui_Set_BaudRate) // Change the Baud Register depending on the BaudRate set
		{
			case 1200:
			{
				SciaRegs.SCIHBAUD = 0x0009;  // 1200 baud @LSPCLK = 22.5MHz (90 MHz SYSCLK).
				SciaRegs.SCILBAUD = 0x0028;
				break;
			}
			case 2400:
			{
				SciaRegs.SCIHBAUD = 0x0004;  // 2400 baud @LSPCLK = 22.5MHz (90 MHz SYSCLK).
				SciaRegs.SCILBAUD = 0x0093;
				break;
				}
			case 4800:
			{
				SciaRegs.SCIHBAUD = 0x0002;  // 4800 baud @LSPCLK = 22.5MHz (90 MHz SYSCLK).
				SciaRegs.SCILBAUD = 0x0059;
				break;
			}
			case 9600:
			{
				SciaRegs.SCIHBAUD = 0x0001;  // 9600 baud @LSPCLK = 22.5MHz (90 MHz SYSCLK).
				SciaRegs.SCILBAUD = 0x0024;
				break;
			}
			case 19200:
			{
				SciaRegs.SCIHBAUD = 0x0000;  // 19200 baud @LSPCLK = 22.5MHz (90 MHz SYSCLK).
				SciaRegs.SCILBAUD = 0x0091;
				break;
			}
			default:
				break;
		}
		SciaRegs.SCICTL1.bit.SWRESET = 1;	// Release peripheral from reset
		Global_Flags.fg_Immediate_Update_Sci_Baud = FALSE;
	}
	if(Global_Flags.fg_Immediate_Update_PSU_Addr == TRUE)
	{
		Reference.ui_Set_Address = Encoder.ui_Qep_Address_Count / ADDRESS_CALCULATION_CONSTANT;

		// The message id incorporates a field for PSU address. Once the address has been determined as changed
		// from the frontpanel, we change them to incorporate the new address

		// First disable all the relevant mailboxes
		ECanaShadow.CANME.all = ECanaRegs.CANME.all;

		ECanaShadow.CANME.bit.ME11 = 0;
		ECanaShadow.CANME.bit.ME10 = 0;
		ECanaShadow.CANME.bit.ME9 = 0;
		ECanaShadow.CANME.bit.ME8 = 0;
		ECanaShadow.CANME.bit.ME16 = 0;
		ECanaShadow.CANME.bit.ME17 = 0;
		ECanaShadow.CANME.bit.ME31 = 0;

		ECanaRegs.CANME.all = ECanaShadow.CANME.all;

		// Clear the RMP field of all receive Mboxes above
		ECanaShadow.CANRMP.all = ECanaRegs.CANRMP.all;
		ECanaShadow.CANRMP.bit.RMP11 = 1;
		ECanaShadow.CANRMP.bit.RMP10 = 1;
		ECanaShadow.CANRMP.bit.RMP9 = 1;
		ECanaShadow.CANRMP.bit.RMP8 = 1;
		ECanaRegs.CANRMP.all = ECanaShadow.CANRMP.all;

		ECanaShadow.CANME.all = ECanaRegs.CANME.all;

				// MBOX11 New Message ID
		// First make the bits to be changed 0
		ECanaMboxes.MBOX11.MSGID.all &= ADDRESS_FIELD_CLEAR_WORD;
		// Or the New message Id
		ECanaMboxes.MBOX11.MSGID.all |= (((Uint32)Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
		ECanaShadow.CANME.bit.ME11 = 1;		// Enable

				// MBOX10 New Message ID
		// First make the bits to be changed 0
		ECanaMboxes.MBOX10.MSGID.all &= ADDRESS_FIELD_CLEAR_WORD;
		// Or the New message Id
		ECanaMboxes.MBOX10.MSGID.all |= (((Uint32)Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
		ECanaShadow.CANME.bit.ME10 = 1;		// Enable

				// MBOX9 New Message ID
		// First make the bits to be changed 0
		ECanaMboxes.MBOX9.MSGID.all &= ADDRESS_FIELD_CLEAR_WORD;
		// Or the New message Id
		ECanaMboxes.MBOX9.MSGID.all |= (((Uint32)Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
		ECanaShadow.CANME.bit.ME9 = 1;		// Enable

				// MBOX8 New Message ID
		// First make the bits to be changed 0
		ECanaMboxes.MBOX8.MSGID.all &= ADDRESS_FIELD_CLEAR_WORD;
		// Or the New message Id
		ECanaMboxes.MBOX8.MSGID.all |= (((Uint32)Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
		ECanaShadow.CANME.bit.ME8 = 1;		// Enable

				// MBOX16 New Message ID
		// First make the bits to be changed 0
		ECanaMboxes.MBOX16.MSGID.all &= ADDRESS_FIELD_CLEAR_WORD;
		// Or the New message Id
		ECanaMboxes.MBOX16.MSGID.all |= (((Uint32)Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
		ECanaShadow.CANME.bit.ME16 = 1;		// Enable

				// MBOX17 New Message ID
		// First make the bits to be changed 0
		ECanaMboxes.MBOX17.MSGID.all &= ADDRESS_FIELD_CLEAR_WORD;
		// Or the New message Id
		ECanaMboxes.MBOX17.MSGID.all |= (((Uint32)Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
		ECanaShadow.CANME.bit.ME17 = 1;		// Enable

				// MBOX31 New Message ID
		// First make the bits to be changed 0
		ECanaMboxes.MBOX31.MSGID.all &= ADDRESS_FIELD_CLEAR_WORD;
		// Or the New message Id
		ECanaMboxes.MBOX31.MSGID.all |= (((Uint32)Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
		ECanaShadow.CANME.bit.ME31 = 1;		// Enable

		ECanaRegs.CANME.all = ECanaShadow.CANME.all;

		Global_Flags.fg_Immediate_Update_PSU_Addr = FALSE;
	}

	// Set Operation_Complete bit to 1. See ei_uiMatchCommonCommands() comment header for descriptions.
	Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.bit.bt_Operation_Complete = SET;
}

//#####################	OVP Reference ############################################################################
// Function Name: ei_vSetOvpReference
// Return Type: void
// Arguments:   void
// Description: PWM Driver for setting OVP reference.

//				Math:
//		iq_OVP_Reference = (Encoder_Voltage_Counts * SVUL) / 575
//#################################################################################################################
void ei_vSetOvpReference()
{
	// Preconditions for the critical functions.
	VALUE_NEGATIVE_CHECK(Reference.iq_OVP_Reference)
	VALUE_NEGATIVE_CHECK(Product_Info.iq_OVP_PWM_Gain)

	Uint16 ui_Temp;
	if(Reference.ui_OVP_Reference_Update_Immediate_Flag == TRUE)
	{
		// Absolutely critical that the value saturates in case of erreneous calculation. (too low gain)
		ui_Temp = _IQ15rsmpy(_IQ15rmpy(Reference.iq_OVP_Reference,Product_Info.iq_OVP_PWM_Gain),1);

		UINT16_SATURATION(ui_Temp)

		EPwm8Regs.CMPA.half.CMPA = ui_Temp;
	}
	return;
}

//#####################	Voltage Reference #########################################################################
// Function Name: ei_vSetVoltageReference
// Return Type: void
// Arguments:   void
// Description: DAC Driver for setting Voltage Reference.

//				Math:
//		1. iq_Voltage_Reference = (Encoder_Voltage_Counts * SVUL) / 575
//#################################################################################################################
void ei_vSetVoltageReference()
{
	// Preconditions for the critical functions.
	DENOMINATOR_ZERO_NEGATIVE_CHECK(Product_Info.iq_Voltage_Ref_Gain)
	VALUE_NEGATIVE_CHECK(Reference.iq_Voltage_Reference)
	VALUE_NEGATIVE_CHECK(Product_Info.iq_Voltage_Dac_Gain)

	int32 Temp;
	Uint16 ui_DacCounts;
	unsigned char a_c_SpiTxDataArray[3];

	// Voltage using DAC
	a_c_SpiTxDataArray[0] = DAC_LOAD_B | DAC_CHANNEL_B | DAC_NORMAL_MODE;	// Setup DAC B
	// Put Calibrated value in temp
	Temp = _IQ15div((Reference.iq_Voltage_Reference - Product_Info.iq_Voltage_Ref_Offset),Product_Info.iq_Voltage_Ref_Gain);

	// Safety
	IQ_DIVISION_RESULT_NEG_AND_POSITIVE_SATURATION_CHECK(Temp)

	// Because of positive offset
	if(Temp < 0)
	{
		Temp = 0;
	}

	// Absolutely critical that the value saturates in case of erreneous calculation. (too low gain)
	// DAC Counts
	ui_DacCounts = _IQ15rsmpy(_IQ15rmpy(Temp,Product_Info.iq_Voltage_Dac_Gain),1);

	UINT16_SATURATION(ui_DacCounts)

	// Channel 2 DAC
	a_c_SpiTxDataArray[1] = ui_DacCounts >> 8;
	a_c_SpiTxDataArray[2] = (ui_DacCounts) & 0x00FF;
	ei_uiSpiWrite(CS_DAC, 3, a_c_SpiTxDataArray);
}

//#####################	Current Reference #########################################################################
// Function Name: ei_vSetCurrentReference
// Return Type: void
// Arguments:   void
// Description: DAC Driver for setting Voltage Reference.

//				Math:
//		1. iq_Current_Reference = (Encoder_Current_Counts * SCUL) / 575
//#################################################################################################################
void ei_vSetCurrentReference()
{
	// Preconditions for the critical functions.
	DENOMINATOR_ZERO_NEGATIVE_CHECK(Product_Info.iq_Current_Ref_Gain)
	VALUE_NEGATIVE_CHECK(Reference.iq_Current_Reference)
	VALUE_NEGATIVE_CHECK(Product_Info.iq_Current_Dac_Gain)

	int32 Temp;
	Uint16 ui_DacCounts;
	unsigned char a_c_SpiTxDataArray[3];
	// Current using DAC
	a_c_SpiTxDataArray[0] = DAC_LOAD_A | DAC_CHANNEL_A | DAC_NORMAL_MODE;
	// Put Calibrated value in temp
	Temp = _IQ15div((Reference.iq_Current_Reference - Product_Info.iq_Current_Ref_Offset),Product_Info.iq_Current_Ref_Gain);

	// Safety
	IQ_DIVISION_RESULT_NEG_AND_POSITIVE_SATURATION_CHECK(Temp)

	// Because of positive offset
	if(Temp < 0)
	{
		Temp = 0;
	}

	// Put dac counts in temp
	ui_DacCounts = _IQ15rmpy(_IQ15rmpy(Temp,Product_Info.iq_Current_Dac_Gain),1);

	UINT16_SATURATION(ui_DacCounts)

	// Channel 1 DAC
	a_c_SpiTxDataArray[1] = ui_DacCounts >> 8;
	a_c_SpiTxDataArray[2] = (ui_DacCounts) & 0x00FF;
	ei_uiSpiWrite(CS_DAC, 3, a_c_SpiTxDataArray);
}

//#####################	Encoder Operation ########################################################################
// Function Name: void ei_vVoltageEncoderOperation
// Return Type: void
// Arguments:   void
// Description: Read Voltage Encoder According to the PSU state.
				// Encoder Operation
//			Mode						Operation
//			SET_VOLTAGE_CURRENT			Set ProgrammedVoltage between 1.05 * Set_UVL_Value and 0.95 * Set_OVP_Value
//			SET_OVP						Set OVP value between MAX and (0.95^-1) * Programmed voltage
//			SET_UVL						Set UVL value between 0 and (1.05^-1) * Programmed voltage
//			SET_ADDR_BAUD				VoltageKnob(EQep1 sets Addr) and CurrentKnob(EQep2 sets BaudRate)

				// Encoder Rotation mode (Only for SET_VOLTAGE_CURRENT mode)
//			COARSE_ROTATION				Limit EQep b/w OVP_limit and UVL_limit.
//			FINE_ROTATION

			//Math and operational details
// 1. For Voltage Setting, OVP_limit is actual value that the QEP count is allowed to rise to, for a set OVP value.
//	  OVP_limit = 0.95 * Set OVP Value
//	  For OVP setting OVP_limit is the actual value that the QEP count is allowed to fall to, for a set Voltage value.
//	  OVP_limit = (0.95^-1) * Set Voltage Value

// 2. UVL_limit is the actual value that the QEP count is allowed to fall to. for a given UVL value.
//	  UVL_limit = 1.05 * Set UVL Value
//	  For UVL setting UVL_limit is the actual value that the QEP count is allowed to rise to, for a set Voltage value.
//	  UVL_limit = (1.05^-1) * Set Voltage Value

// 3. In COARSE_ROTATION, the count cannot exceed 6 rotations value. The slight extra margin over Full Scale Value
//	  is given only when user presses fine when he's on the Full Scale value.

// 4. In FINE_ROTATION,
//    Final = Coarse + (Fine - 317)/10		(In Fine 317 counts corresponds to only 31 counts. SENSITIVITY DECREASED)
//#################################################################################################################
void ei_vVoltageEncoderOperation()
{
	Uint16 OVP_limit = 0,		// Static not reqd. since these values are calculated every time in the fuction.
		   UVL_limit = 0,
		   Fine_Lower_limit = 0,
		   Fine_Upper_limit = 0;

	Encoder.i_VoltageKnob_Previous_Count = Encoder.i_VoltageKnob_Final_Count; // Copy last value to previous value

	if((FRONTPANEL_IS_UNLOCKED) && (State.Mode.bt_PSU_Control == LOCAL))
	{
		if(State.Mode.bt_Encoder_Operation == SET_VOLTAGE_CURRENT)	// Encoder operation is set to SET_VOLTAGE_CURRENT
		{
			// Calculate limits that restrict Qep counts from 1.05*UVL and 0.95*OVP
			OVP_limit = _IQ15rmpy(_IQ15mpy(_IQ15((Uint32)Encoder.ui_Qep_OVP_Count),_IQ15(MIN_DISTANCE_BELOW_OVP)),1);
			UVL_limit = _IQ15rmpy(_IQ15mpy(_IQ15((Uint32)Encoder.ui_Qep_UVL_Count),_IQ15(MIN_DISTANCE_ABOVE_UVL)),1);

	//		For lower UVL value 1.05 * UVL becomes same as Set UVL Value. To Avoid that:
			if((UVL_limit != 0) && (UVL_limit == Encoder.ui_Qep_UVL_Count))	// Except 0
			{
				UVL_limit += 1; // Set UVL as one count Higher
			}

	//		For lower OVP value 0.95 * OVP becomes same as Set OVP Value. To Avoid that:
			if(OVP_limit == Encoder.ui_Qep_OVP_Count)
			{
				OVP_limit -= 1; // Set UVL as one count Higher
			}
	//		OVP Cannot be greater than full scale rotation count
			if(OVP_limit > QEP_FULL_ROTATIONS_COUNT)
			{
				OVP_limit = QEP_FULL_ROTATIONS_COUNT;
			}

			if(State.Mode.bt_Encoder_Rotation_Mode == COARSE_ROTATION) // Rotation mode Coarse
			{
				Encoder.i_VoltageKnob_Coarse_Count = ei_uiReadEncoder(EQEP1);

				//Limit Coarse Register between UVL and OVP
				if(Encoder.i_VoltageKnob_Coarse_Count <= UVL_limit)
				{
					Encoder.i_VoltageKnob_Coarse_Count = UVL_limit;
					EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count;
					EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				}
				else if(Encoder.i_VoltageKnob_Coarse_Count >= OVP_limit)
				{
					Encoder.i_VoltageKnob_Coarse_Count = OVP_limit;
					EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count;
					EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				}
				else
				{
					asm(" nop");
				}

				// Never go above 576 in coarse.
				if(Encoder.i_VoltageKnob_Coarse_Count > QEP_FULL_ROTATIONS_COUNT)
				{
					// In coarse voltage cannot go above six rotation counts
					Encoder.i_VoltageKnob_Coarse_Count = QEP_FULL_ROTATIONS_COUNT;
					EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count;
					EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
				}
			}
			else // Fine Rotation Mode
			{
				Encoder.i_VoltageKnob_Fine_Count = ei_uiReadEncoder(EQEP1);
				// Calculate Fine Lower and upper limits in the below special cases.
		    	if((Encoder.i_VoltageKnob_Coarse_Count - UVL_limit)  > VOLTAGE_COUNT_DIFFERENCE_FOR_WHICH_FINE_HAS_A_LOWER_LIMIT)
		    	{
		    		// In this case
		    		Fine_Lower_limit = 0;
		    	}
		    	else
		    	{
		    		// Math
		    		// UVL_Limit = Coarse + (Fine - 288)*0.012
		    		// ==> Fine_Lower_limit = 288 + (UVL_Limit - Coarse)/0.012   	,for (coarse - UVL) <= 3
		    		Fine_Lower_limit = (QEP_FULL_ROTATIONS_COUNT/2) + _IQ15rmpy(_IQ15mpy(_IQ15((int32)UVL_limit - Encoder.i_VoltageKnob_Coarse_Count),IQ_FINE_CALCULATION_CONSTANT_INVERSE),1);
		    	}

		    	if((OVP_limit - Encoder.i_VoltageKnob_Coarse_Count) > VOLTAGE_COUNT_DIFFERENCE_FOR_WHICH_FINE_HAS_AN_UPPER_LIMIT)
		    	{
		    		// In this case
		    		Fine_Upper_limit = QEP_FULL_ROTATIONS_COUNT;
		    	}
		    	else
				{
					// Math
					// OVP_limit = Coarse + (Fine - 317)*0.012
					//==> Fine_Upper_limit = 288 + (OVP_limit - Coarse)/0.012 		,for (OVP - Coarse) <= 3
					Fine_Upper_limit = (QEP_FULL_ROTATIONS_COUNT/2) + _IQ15rmpy(_IQ15mpy(_IQ15(OVP_limit - Encoder.i_VoltageKnob_Coarse_Count),IQ_FINE_CALCULATION_CONSTANT_INVERSE),1);
				}

	//			Limit Fine here
				if (Encoder.i_VoltageKnob_Fine_Count < Fine_Lower_limit)
				{
					Encoder.i_VoltageKnob_Fine_Count = Fine_Lower_limit;
					EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Fine_Count;
					EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;

				}
				else if (Encoder.i_VoltageKnob_Fine_Count > Fine_Upper_limit)
				{
					Encoder.i_VoltageKnob_Fine_Count = Fine_Upper_limit;
			 		EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Fine_Count;
					EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;

				}
				else
				{
					asm(" nop");
				}
			}
			// Here Final_Count just checks for change in encoder to implement various timeout logic.
			Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count + (Encoder.i_VoltageKnob_Fine_Count - (QEP_FULL_ROTATIONS_COUNT/2));

			if(Encoder.i_VoltageKnob_Final_Count != Encoder.i_VoltageKnob_Previous_Count)
			{
				Global_Flags.fg_Voltage_Remote_control = FALSE;	// A change in encoder during local mode brings this parameter from remote to local control.
				RESET_PREVIEW_OPERATION_TIMER;					// Value changed. Reset counter
				Reference.fg_Voltage_Reference_Update_Immediate_Flag = TRUE;	// Immediate update Voltage flag set

				// The below logic ensures that when user changes from fine to coarse, the parameter resolution is
				// still available till he changes the encoder again.
				if(Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse == TRUE)
				{
					// Reset fine registers here
					Encoder.i_VoltageKnob_Fine_Count = (QEP_FULL_ROTATIONS_COUNT/2);
					Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse = FALSE;
				}
			}
			else
			{
				if(State.Mode.bt_Display_Mode == DISPLAY_PREVIEW)
				{
					INCREMENT_PREVIEW_OPERATION_TIMER;
				}
			}
			Encoder.iq_Qep_Voltage_Count = _IQ15(Encoder.i_VoltageKnob_Coarse_Count) + _IQ15rmpy(_IQ15(Encoder.i_VoltageKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2),IQ_FINE_CALCULATION_CONSTANT);
		}
		else if(State.Mode.bt_Encoder_Operation == SET_OVP)	// Voltage encoder sets OVP
		{
			// Calculate lower limit for the OVP setting.

			if(OUTPUT_OFF)	// Min possible OVP setting when Output is off
			{
				OVP_limit = Product_Info.ui_Qep_Min_OVP_Count;
			}
			else	//(0.95^-1) * Programmed voltage when output is ON
			{
				OVP_limit = _IQ15rmpy(_IQ15mpy(Encoder.iq_Qep_Voltage_Count,_IQ15(MIN_DISTANCE_ABOVE_PROGRAMMED_VOLT)),1);
				if(OVP_limit == _IQ15rmpy(Encoder.iq_Qep_Voltage_Count,1))
				{
					OVP_limit += 1;
				}

				// This value itself must not be less than Min posible OVP count
				if(OVP_limit < Product_Info.ui_Qep_Min_OVP_Count)
				{
					OVP_limit = Product_Info.ui_Qep_Min_OVP_Count;
				}
			}
			Encoder.i_VoltageKnob_Final_Count = ei_uiReadEncoder(EQEP1);	//limited from 0 to OVP_MAX

			if(Encoder.i_VoltageKnob_Final_Count < OVP_limit)	// limit to OVP_limit
			{
				Encoder.i_VoltageKnob_Final_Count = OVP_limit;
			 	EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Final_Count;
				EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
			}

			// Timer will wait for 5 seconds. If value not changed go to SET_VOLTAGE_CURRENT

			if(Encoder.i_VoltageKnob_Final_Count != Encoder.i_VoltageKnob_Previous_Count)
			{
				Global_Flags.fg_OVP_Remote_control = FALSE;	// A change in encoder during local mode brings this parameter from remote to local control.
				RESET_OVP_UVL_TIMER;							// Value changed. Reset counter
				Reference.ui_OVP_Reference_Update_Immediate_Flag = TRUE;
			}
			else
			{
				INCREMENT_OVP_UVL_TIMER;
			}

			// Copy value
			Encoder.ui_Qep_OVP_Count = Encoder.i_VoltageKnob_Final_Count;
		}
		else if(State.Mode.bt_Encoder_Operation == SET_UVL)
		{
			// Calculate upper limit for the UVL setting.

			if(OUTPUT_OFF)
			{
				// Math:
				// OVP is always set first. So when output off UVL can be set to 90.476% of set OVP.
				// UVL_limit = OVP_limit * (0.95 * PV)/(1.05*PV)
				//			 = OVP_limit * .90476

//				UVL_limit = Round_in_Decimal(_IQ15mpy(_IQ15mpy(_IQ15(Encoder.ui_Qep_OVP_Count),_IQ15(0.904761904)),10));
				UVL_limit = _IQ15rmpy(_IQ15mpy(_IQ15(Encoder.ui_Qep_OVP_Count),_IQ15(0.904761904)),1);
			}
			else	// Output Off
			{
				// Limit to (1.05^-1) * programmed voltage when output ON
				UVL_limit = _IQ15rmpy(_IQ15rmpy(Encoder.iq_Qep_Voltage_Count,_IQ15(MIN_DISTANCE_BELOW_PROGRAMMED_VOLT)),1);

				// For low values of Qep_Voltage_Counts (1.05^-1) * Counts becomes equal to counts.
				// In that case
				if((UVL_limit == _IQ15rmpy(Encoder.iq_Qep_Voltage_Count,1)) && (UVL_limit != 0))	//
				{
					UVL_limit -= 1;
				}
			}

			// UVL_limit must never exceed the Max_possible_UVL_count
			if(UVL_limit > QEP_MAX_UVL_COUNT)
			{
				UVL_limit = QEP_MAX_UVL_COUNT;
			}

			Encoder.i_VoltageKnob_Final_Count = ei_uiReadEncoder(EQEP1);	//limited from 0 to OVP_MAX

			if(Encoder.i_VoltageKnob_Final_Count > UVL_limit)
			{
				Encoder.i_VoltageKnob_Final_Count = UVL_limit;
			 	EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Final_Count;
				EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
			}

			// Timer will wait for 5 seconds. If value not changed go to SET_VOLTAGE_CURRENT

			if(Encoder.i_VoltageKnob_Final_Count != Encoder.i_VoltageKnob_Previous_Count)
			{
				Global_Flags.fg_UVL_Remote_control = FALSE;	// A change in encoder during local mode brings this parameter from remote to local control.
				RESET_OVP_UVL_TIMER;						// Value changed. Reset counter
			}
			else
			{
				INCREMENT_OVP_UVL_TIMER;
			}

			// Copy value
			Encoder.ui_Qep_UVL_Count = Encoder.i_VoltageKnob_Final_Count;
		}
		else if(State.Mode.bt_Encoder_Operation == SET_ADDR_BAUD)
		{
			// Voltage encoder sets address
			Encoder.i_VoltageKnob_Final_Count = EQep1Regs.QPOSCNT;

			if((Encoder.i_VoltageKnob_Final_Count < BASE_ADDRESS_QEP_COUNT) || (Encoder.i_VoltageKnob_Final_Count > QEP_MAX_ADDRESS_COUNT))
			{
				Encoder.i_VoltageKnob_Final_Count = BASE_ADDRESS_QEP_COUNT;	// Purposeful rollover
				EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Final_Count;
				EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
			}

			// Timer will wait for 5 seconds. If value not changed go to SET_VOLTAGE_CURRENT

			if(Encoder.i_VoltageKnob_Final_Count != Encoder.i_VoltageKnob_Previous_Count)
			{
				RESET_ADDR_BAUD_TIMER;		// Value changed. Reset counter
			}
			else
			{
				INCREMENT_ADDR_BAUD_TIMER;
			}

			// We need to update address if previous/constant and present/constant are different
			if((Encoder.i_VoltageKnob_Final_Count / ADDRESS_CALCULATION_CONSTANT) != (Encoder.i_VoltageKnob_Previous_Count / ADDRESS_CALCULATION_CONSTANT))
			{
				Global_Flags.fg_Immediate_Update_PSU_Addr = TRUE;
			}

			Encoder.ui_Qep_Address_Count = Encoder.i_VoltageKnob_Final_Count;
		}
	}
	else	// Could be local lockout or remote or Frontpanel locked
	{
		// Check whether the Qep registers value has changed. This is determined by using Encoder_Final and Encoder_Previous counts.
		// First calculate Final registers depending on the present state of Qep Count register.

		// For State_Voltage_Current mode
		if(State.Mode.bt_Encoder_Operation == SET_VOLTAGE_CURRENT)
		{
			int16 i_TempCount;
			i_TempCount = EQep1Regs.QPOSCNT;	// Copy the register to temporary variable

			// Depending on Rotation mode calculation of Final Count will vary.
			if(State.Mode.bt_Encoder_Rotation_Mode == COARSE_ROTATION)
			{
				// In coarse mode temp used in place of coarse.
				Encoder.i_VoltageKnob_Final_Count = i_TempCount + (Encoder.i_VoltageKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2);
			}
			else	// Fine rotaion
			{
				// In Fine mode temp used in place of Fine.
				Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count + (i_TempCount - QEP_FULL_ROTATIONS_COUNT/2);
			}
		}
		else
		{
			// For other encoder modes like Ovp, Uvl etc.. directly copy reister to final. No math involved
			Encoder.i_VoltageKnob_Final_Count = EQep1Regs.QPOSCNT;
		}

		// Check the calculated final count with previous count for any change.
		if(Encoder.i_VoltageKnob_Previous_Count != Encoder.i_VoltageKnob_Final_Count)
		{
			// Value changed. Copy previous value to present Final
			Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Previous_Count;

			// Now the Qep counts register must be set to their previous state as well.
			// For Set_Voltage_Current Mode.
			if(State.Mode.bt_Encoder_Operation == SET_VOLTAGE_CURRENT)
			{
				if(State.Mode.bt_Encoder_Rotation_Mode == COARSE_ROTATION)
				{
				// In coarse rotation mode the Qep register will be set to Previous coarse value
					EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count;
				}
				else	// Fine rotaion
				{
				// In fine rotation mode the Qep register will be set to Previous Fine value
					EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Fine_Count;
				}
			}
			else
			{
//				In all other modes qep register set to final value.
				EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Final_Count;
			}
			EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;	// Initiate change

			// Value changed. Copy back previous value and display LFP
			if(State.Mode.bt_FrontPanel_Access == FRONTPANEL_LOCKED)
				Global_Flags.fg_Display_FP_Access_Mode = SET;

		}

		// The timers are incremented in current encoder.
	}

}

//#####################	Encoder Operation ########################################################################
// Function Name: void ei_vCurrentEncoderOperation
// Return Type: void
// Arguments:   void
// Description: Read Current Encoder According to the PSU state.
				// Encoder Operation
//			Mode						Operation
//			SET_VOLTAGE_CURRENT			Set ProgrammedVoltage between 1.05 * Set_UVL_Value and 0.95 * Set_OVP_Value
//			SET_OVP						No operation
//			SET_UVL						No operation
//			SET_ADDR_BAUD				VoltageKnob(EQep1 sets Addr) and CurrentKnob(EQep2 sets BaudRate)

				// Encoder Rotation mode (Only for SET_VOLTAGE_CURRENT mode)
//			COARSE_ROTATION				Limit EQep 0 and six_rotations_count
//			FINE_ROTATION							--do--

//				// Math
// 1. IN FINE ROTATION,
//	  One Full Rotation corresponds to 0.2% of full scale. 3 Rotations allowed on either side of the set value.
//#################################################################################################################
void ei_vCurrentEncoderOperation()
{
	Uint16 Fine_Lower_limit = 0,
		   Fine_Upper_limit = 0;

	Encoder.i_CurrentKnob_Previous_Count = Encoder.i_CurrentKnob_Final_Count; // Copy last value to previous value

	if((FRONTPANEL_IS_UNLOCKED) && (State.Mode.bt_PSU_Control == LOCAL))
	{
		if(State.Mode.bt_Encoder_Operation == SET_VOLTAGE_CURRENT)	// Encoder operation is set to SET_VOLTAGE_CURRENT
		{
			if(State.Mode.bt_Encoder_Rotation_Mode == COARSE_ROTATION)		// Rotation mode coarse
		    {
		    	Encoder.i_CurrentKnob_Coarse_Count = ei_uiReadEncoder(EQEP2);
		    }
		    else	// Rotation mode fine
		    {
		    	Encoder.i_CurrentKnob_Fine_Count = ei_uiReadEncoder(EQEP2);

		    	// Calculate Fine Lower and upper limits in the below special cases.
		    	if(Encoder.i_CurrentKnob_Coarse_Count > CURRENT_COARSE_COUNT_FOR_WHICH_FINE_HAS_A_LOWER_LIMIT)
		    	{
		    		// In this case we can subtract upto 28 counts from coarse ==> 0 to 287 allowed for Fine count
		    		Fine_Lower_limit = 0;
		    	}
		    	else
		    	{
		    		// Math
		    		// 0 = Coarse + (Fine - 288)*0.012
		    		// ==> Fine_Lower_limit = 288 - Coarse/0.012   ,for coarse <= 3
		    		Fine_Lower_limit = (QEP_FULL_ROTATIONS_COUNT/2) - _IQ15rmpy(_IQ15rmpy(_IQ15(Encoder.i_CurrentKnob_Coarse_Count),IQ_FINE_CALCULATION_CONSTANT_INVERSE),1);
		    	}

		    	if(Encoder.i_CurrentKnob_Coarse_Count < CURRENT_COARSE_COUNT_FOR_WHICH_FINE_HAS_AN_UPPER_LIMIT)
		    	{
		    		// In this case we can add upto 28 counts to coarse ==> 287 to 575 allowed for Fine count
		    		Fine_Upper_limit = QEP_FULL_ROTATIONS_COUNT;
		    	}
		    	else
				{
					// Math
					// 576 = Coarse + (Fine - 288)*0.012
					//==> Fine_Upper_limit = (576 - Coarse)/0.012 + 288	,for Coarse >= 573
					Fine_Upper_limit = (QEP_FULL_ROTATIONS_COUNT/2) + _IQ15rmpy(_IQ15rmpy(_IQ15(QEP_FULL_ROTATIONS_COUNT - Encoder.i_CurrentKnob_Coarse_Count),IQ_FINE_CALCULATION_CONSTANT_INVERSE),1);
				}

				//	Limit Fine here
				if (Encoder.i_CurrentKnob_Fine_Count < Fine_Lower_limit)
				{
					Encoder.i_CurrentKnob_Fine_Count = Fine_Lower_limit;
					EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Fine_Count;
					EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;

				}
				else if (Encoder.i_CurrentKnob_Fine_Count > Fine_Upper_limit)
				{
					Encoder.i_CurrentKnob_Fine_Count = Fine_Upper_limit;
			 		EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Fine_Count;
					EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;

				}
				else
				{
					asm(" nop");
				}
		    }

		    // Here Final_Count just checks for change in encoder to implement various timeout logic.
		    Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count + (Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2);

			if(Encoder.i_CurrentKnob_Final_Count != Encoder.i_CurrentKnob_Previous_Count)
			{
				Global_Flags.fg_Current_Remote_control = FALSE;	// A change in encoder during local mode brings this parameter from remote to local control.

				// For accurate time calculation, in Current encoder we only reset this timer if value changed.
				// If value is not changed preview timer incremented in Voltage encoder operation only. Incrementing the timer
				// in 1 place only is important otherwise preview mode will be enable only for half the specified time.
				RESET_PREVIEW_OPERATION_TIMER;		// Value changed. Reset counter
				Reference.fg_Current_Reference_Update_Immediate_Flag = TRUE;	// Immediate update Current flag set

				// The below logic ensures that when user changes from fine to coarse, the parameter resolution is
				// still available till he changes the encoder again.
				if(Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse == TRUE)
				{
					// Reset fine registers here
					Encoder.i_CurrentKnob_Fine_Count = (QEP_FULL_ROTATIONS_COUNT/2);
					Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse = FALSE;
				}
			}
		    Encoder.iq_Qep_Current_Count = _IQ15(Encoder.i_CurrentKnob_Coarse_Count) + _IQ15rmpy(_IQ15(Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2),IQ_FINE_CALCULATION_CONSTANT);
		}
		else if(State.Mode.bt_Encoder_Operation == SET_ADDR_BAUD)
		{
			Encoder.i_CurrentKnob_Final_Count = EQep2Regs.QPOSCNT;

			if(Encoder.i_CurrentKnob_Final_Count > QEP_MAX_BAUDRATE_COUNT)
			{
				Encoder.i_CurrentKnob_Final_Count = 0;	// Purposeful rollover
				EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Final_Count;
				EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
			}

			// Timer will wait for 5 seconds. If value not changed go to SET_VOLTAGE_CURRENT
			// We increment it only in the Voltage encoder operation.
			if(Encoder.i_CurrentKnob_Final_Count != Encoder.i_CurrentKnob_Previous_Count)
			{
				RESET_ADDR_BAUD_TIMER;		// Value changed. Reset counter
			}

			// We need to update baudrate if previous%constant and present%constant are different
			if((Encoder.i_CurrentKnob_Final_Count / BAUD_RATE_CALCULATION_CONSTANT) != (Encoder.i_CurrentKnob_Previous_Count / BAUD_RATE_CALCULATION_CONSTANT))
			{
				Global_Flags.fg_Immediate_Update_Sci_Baud = TRUE;
			}

			Encoder.ui_Qep_BaudRate_Count = Encoder.i_CurrentKnob_Final_Count;
		}
	}
	else	// Could be local lockout or remote or Frontpanel locked
	{
		// Check whether the Qep registers value has changed. This is determined by using Encoder_Final and Encoder_Previous counts.
		// First calculate Final registers depending on the present state of Qep Count register.

		// For State_Voltage_Current mode
		if(State.Mode.bt_Encoder_Operation == SET_VOLTAGE_CURRENT)
		{
			int16 i_TempCount;
			i_TempCount = EQep2Regs.QPOSCNT;	// Copy the register to temporary variable

			// Depending on Rotation mode calculation of Final Count will vary.
			if(State.Mode.bt_Encoder_Rotation_Mode == COARSE_ROTATION)
			{
				// In coarse mode temp used in place of coarse.
				Encoder.i_CurrentKnob_Final_Count = i_TempCount + (Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2);
			}
			else	// Fine rotaion
			{
				// In Fine mode temp used in place of Fine.
				Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count + (i_TempCount - QEP_FULL_ROTATIONS_COUNT/2);
			}
		}
		else if(State.Mode.bt_Encoder_Operation == SET_ADDR_BAUD)	// OVP, UVl has no effect on current encoders.
		{
			// For other encoder modes like Ovp, Uvl etc.. directly copy reister to final. No math involved
			Encoder.i_CurrentKnob_Final_Count = EQep2Regs.QPOSCNT;
		}
		else
		{
			// no operation in OVP/UVL
			asm("	nop");
		}

		// Check the calculated final count with previous count for any change.
		if(Encoder.i_CurrentKnob_Previous_Count != Encoder.i_CurrentKnob_Final_Count)
		{
			// Value changed. Copy previous value to present Final
			Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Previous_Count;


			// Now the Qep counts register must be set to their previous state as well.
			// For Set_Voltage_Current Mode.
			if(State.Mode.bt_Encoder_Operation == SET_VOLTAGE_CURRENT)
			{
				if(State.Mode.bt_Encoder_Rotation_Mode == COARSE_ROTATION)
				{
					// In coarse rotation mode the Qep register will be set to Previous coarse value
					EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Coarse_Count;
				}
				else	// Fine rotaion
				{
					// In fine rotation mode the Qep register will be set to Previous Fine value
					EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Fine_Count;
				}
			}
			else
			{
//				In all other modes qep register set to final value.
				EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Final_Count;
			}
			EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;

			// Value changed. Copy back previous value and display LFP
			// Set the LFP Display flag
			if(State.Mode.bt_FrontPanel_Access == FRONTPANEL_LOCKED)
				Global_Flags.fg_Display_FP_Access_Mode = SET;
		}
		// Now depending on the state increment the following timers
		if(State.Mode.bt_Display_Mode == DISPLAY_PREVIEW)
		{
			INCREMENT_PREVIEW_OPERATION_TIMER;
		}
		if((State.Mode.bt_Encoder_Operation == SET_OVP) || (State.Mode.bt_Encoder_Operation == SET_UVL))
		{
			INCREMENT_OVP_UVL_TIMER;
		}
		if(State.Mode.bt_Encoder_Operation == SET_ADDR_BAUD)
		{
			INCREMENT_ADDR_BAUD_TIMER;
		}
	}

	// On OVP_UVL time out
	if(OVP_UVL_TIMER_EXPIRED)
	{
		RESET_OVP_UVL_TIMER;
		// Take PSU back to set_voltage_current state
		State.Mode.bt_Encoder_Operation = SET_VOLTAGE_CURRENT;

		// Reset the Qeps to latest value of voltage and current
		EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count; // Reset Voltage counter to Coarse count
		EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
		Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count + (Encoder.i_VoltageKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
		Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution

		EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Coarse_Count; // Reset Current counter to Coarse count
		EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
		Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count + (Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
		Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution
	}

	// On Addr_Baud time out
	if(ADDR_BAUD_TIMER_EXPIRED)
	{
		RESET_ADDR_BAUD_TIMER;
		// Take PSU back to set_voltage_current state
		State.Mode.bt_Encoder_Operation = SET_VOLTAGE_CURRENT;

		// Reset the Qeps to latest value of voltage and current
		EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count; // Reset Voltage counter to Coarse count
		EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
		Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count + (Encoder.i_VoltageKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
		Global_Flags.fg_Voltage_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution

		EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Coarse_Count; // Reset Current counter to Coarse count
		EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
		Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count + (Encoder.i_CurrentKnob_Fine_Count - QEP_FULL_ROTATIONS_COUNT/2); // Important to maintain last local reference
		Global_Flags.fg_Current_Mode_Changed_From_Fine_To_Coarse = TRUE;	// To maintain last set local reference resolution
	}

	// On Preview time out.
	if(PREVIEW_OPEARTION_TIMER_EXPIRED)
	{
		RESET_PREVIEW_OPERATION_TIMER;

		// Take PSU back to Display Output state
		State.Mode.bt_Display_Mode = DISPLAY_OUTPUT;
	}
}

//#####################	Read Encoder  #############################################################################
// Function Name: void ReadEncoder
// Return Type: void
// Arguments:   void
// Description: Read Qep Count register and.
//				VoltageKnob Count limited b/w 0 and MAX_POSSIBLE_OVP_COUNT
//				CurrentKnob Count limited b/w 0 and 575 (Count corresponding to 6 rotations)
//#################################################################################################################
static Uint16 ei_uiReadEncoder(Uint16 EQep_X)
{
	Uint16 Counts = 0;

//	Simply Return the count value limited from 0 to count corresponding to Maximaum Possible OVP
	if(EQep_X == EQEP1)
	{
		Counts = EQep1Regs.QPOSCNT;

//		This is for Clockwise rotation with 100 counts safety net for overflow
		if((Counts > QEP_MAX_OVP_COUNT) && (Counts <= (QEP_MAX_OVP_COUNT + 100)))
		{
			Counts = QEP_MAX_OVP_COUNT;
			EQep1Regs.QPOSINIT = Counts;						// COUNT Initialise Reg = Max Count Value
			EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;   // Initialise PosCtr with value in COUNT Initialise Reg
		}
//		This is for Anticlockwise rotation with 100 counts safety net for underflow
		else if((Counts > (QEP_MAX_OVP_COUNT + 100)) && (Counts <= (QEP_MAX_OVP_COUNT + QEP_SAFETY_NET)))
		{
			Counts = 0;
			EQep1Regs.QPOSINIT = Counts; 					// COUNT Initialise Reg = Min Count Value
			EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;   // Initialise PosCtr with value in COUNT Initialise Reg
		}
		else
		{
			asm(" nop");
		}
	}

//  Simply return the count value from limited from 0 to count corresponding to Six rotations
	else if(EQep_X == EQEP2)
	{
		Counts = EQep2Regs.QPOSCNT;

//		This is for Clockwise rotation with 100 counts safety net for overflow
		if((Counts > QEP_FULL_ROTATIONS_COUNT) && (Counts <= QEP_FULL_ROTATIONS_COUNT + 100))
		{
			Counts = QEP_FULL_ROTATIONS_COUNT;
			EQep2Regs.QPOSINIT = Counts;		// COUNT Initialise Reg = Max Count Value
			EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;   // Initialise PosCtr with value in COUNT Initialise Reg
		}
//		This is for Anticlockwise rotation and underflow
		else if((Counts > (QEP_FULL_ROTATIONS_COUNT + 100)) && (Counts <= QEP_FULL_ROTATIONS_COUNT + QEP_SAFETY_NET))
		{
			Counts = 0;
			EQep2Regs.QPOSINIT = Counts; 					// COUNT Initialise Reg = Min Count Value
			EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;   // Initialise PosCtr with value in COUNT Initialise Reg
		}
		else
		{
			asm(" nop");
		}
	}
	return Counts;
}

//#####################	Adjust OVP ################################################################################
// Function Name: ei_vAdjustOvp
// Return Type: void
// Arguments:   void
//				1. Take 32 ADC samples of OVP feedback.
//				2. Avearge them.
//				3. Calculate error and compensate
//#################################################################################################################
void ei_vAdjustOvp()
{
	// Not adjusting OVP for now
	Reference.ui_OVP_Reference_Update_Immediate_Flag = FALSE;

	if(Reference.ui_OVP_Reference_Update_Immediate_Flag == TRUE)
	{
		int16 i;
		Uint16 ui_num_of_ovp_samples;
		int32 iq_Temp, iq_MeasuredOvp;
		iq_MeasuredOvp = 0;
		ui_num_of_ovp_samples = 32;

		// Mesure OVP_ref_fb. Take 32 samples

		for(i = ui_num_of_ovp_samples-1; i >= 0; i--)
		{
			ADC_SOC(SOC_6);
			while(AdcRegs.ADCINTFLG.bit.ADCINT1 == 0);
			AdcRegs.ADCINTFLGCLR.bit.ADCINT1 = SET_1_TO_CLEAR;
			iq_MeasuredOvp += _IQ15toQ12(AdcResult.ADCRESULT6);
		}

		iq_MeasuredOvp >>= 5;	// Divide by no. of samples

		iq_MeasuredOvp = _IQ15mpy(iq_MeasuredOvp,_IQ15(3.3));
		iq_MeasuredOvp = _IQ15mpy(iq_MeasuredOvp,_IQ15div(IQ_FULL_SCALE_OVP,_IQ15(CONTROL_VOLTAGE_CORRESPONDING_TO_FULL_SCALE_OVP)));

		// Calculate Error
		Reference.iq_OVP_Reference_Error = iq_MeasuredOvp - Reference.iq_OVP_Reference;

		if(Reference.iq_OVP_Reference_Error > 0)
		{
			iq_Temp =	Reference.iq_OVP_Reference - Reference.iq_OVP_Reference_Error;
		}
		else
		{
			iq_Temp =	Reference.iq_OVP_Reference + Reference.iq_OVP_Reference_Error;
		}

		// Calculate and set the new reference
		iq_Temp = _IQ15rmpy(_IQ15rmpy(iq_Temp,Product_Info.iq_OVP_PWM_Gain),1);
		EPwm8Regs.CMPA.half.CMPA = iq_Temp;
		Reference.ui_OVP_Reference_Update_Immediate_Flag = FALSE;
	}
}
