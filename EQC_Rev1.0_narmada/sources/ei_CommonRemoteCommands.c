//#################################################################################################################
//# 							enArka Instruments proprietary				  									  
//# File: ei_CommonRemoteCommands.c
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
//void ei_vCheckAndSetModelNo();
//void ei_vSetVoltageCurrentForCalib(Uint16 Parameter_To_Be_Set);
//void ei_vRecallLastPSUSettings();

//#####################	Allot model no ############################################################################
// Function Name: CheckAndSetModelNo
// Return Type: void 
// Arguments:   void
// Description: Calculates model no which is used for software's internal calculation.
//#################################################################################################################
void ei_vCheckAndSetModelNo()
{
	if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_10_700_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_10_700_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_10_700;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_20_500_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_20_500_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_20_500;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_30_334_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_30_334_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_30_334;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_40_250_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_40_250_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_40_250;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_50_200_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_50_200_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_50_200;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_60_167_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_60_167_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_60_167;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_80_125_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_80_125_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_80_125;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_100_100_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_100_100_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_100_100;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_150_67_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_150_67_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_150_67;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_300_34_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_300_34_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_300_34;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_400_25_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_400_25_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_400_25;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_600_17_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_600_17_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_600_17;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_450_23_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_450_23_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_450_23;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_68_150_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_68_150_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_68_150;
	}
	else if((_IQ15rmpy(Product_Info.iq_Rated_Voltage,100) == RATED_VOLTAGE_48_27_TIMES100) && (_IQ15rmpy(Product_Info.iq_Rated_Current,100) == RATED_CURRENT_48_27_TIMES100))
	{
		Product_Info.ui_Model_Number = MODEL_NO_48_27;
	}
	else
	{
		Product_Info.ui_Model_Number = 255;
	}
	
	// Calculate these values here so that they are useful in setting voltages and current during the calibration process 
	Product_Info.iq_Voltage_PWM_Gain = _IQ15div(_IQ15(PWM_COUNTS_CORRESPONDING_TO_RATED_VALUE),Product_Info.iq_Rated_Voltage);
	Product_Info.iq_Current_PWM_Gain = _IQ15div(_IQ15(PWM_COUNTS_CORRESPONDING_TO_RATED_VALUE),Product_Info.iq_Rated_Current);
	
	Product_Info.iq_Voltage_Dac_Gain = _IQ15div(_IQ15(DAC_COUNTS_CORRESPONDING_TO_RATED_VALUE),Product_Info.iq_Rated_Voltage);
	Product_Info.iq_Current_Dac_Gain = _IQ15div(_IQ15(DAC_COUNTS_CORRESPONDING_TO_RATED_VALUE),Product_Info.iq_Rated_Current);
	Product_Info.iq_OVP_PWM_Gain = _IQ15div(_IQ15(PWM_COUNTS_CORRESPONDING_TO_OVP_RATED_VALUE),_IQ15mpy(_IQ15(1.1),Product_Info.iq_Rated_Voltage));
}


//#####################	Voltage Current Calibration ###############################################################
// Function Name: ei_vSetVoltageCurrentForCalib
// Return Type: void
// Arguments:   Uint16 Parameter_To_Be_Set
// Description: Sets the output voltage and current for calibration. Takes the command as input.
//				Set voltage to 10% and Current to 50% for Voltage offset calibration 
//				Set voltage to 100% and Current to 100% for Voltage gain calibration 
//				Set voltage to 100% and Current to 10% for Current offset calibration 
//				Set voltage to 100% and Current to 100% for Current gain calibration 
//
//		OVP Reference will be FULL throughout the calibration.
//#################################################################################################################
void ei_vSetVoltageCurrentForCalib(Uint16 Parameter_To_Be_Set)
{
	State.Mode.bt_Output_Status = ON;	// Switch output on
	switch(Parameter_To_Be_Set)
	{
		case VOLTAGE_LOW:
		{
			// 10% , 50%
			Reference.iq_Voltage_Reference = Product_Info.iq_Specified_FS_Voltage/10;
			Reference.iq_Current_Reference = Product_Info.iq_Specified_FS_Current/2;
			Reference.iq_OVP_Reference = _IQ15rmpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(1.1));
			
			Global_Flags.fg_Previous_Calibration = VOLTAGE_LOW;
			break;
		}
		case VOLTAGE_FULL:
		{
			// 100%, 100%
			Reference.iq_Voltage_Reference = Product_Info.iq_Specified_FS_Voltage;
			Reference.iq_Current_Reference = Product_Info.iq_Specified_FS_Current;
			
			// Called already in case VOLTAGE_LOW
//			Reference.iq_OVP_Reference = _IQ15rmpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(1.1));
			
			Global_Flags.fg_Previous_Calibration = VOLTAGE_FULL;
			break;
		}
		case CURRENT_LOW:
		{
			// 100%, 10%
			Reference.iq_Voltage_Reference = Product_Info.iq_Specified_FS_Voltage;
			Reference.iq_Current_Reference = Product_Info.iq_Specified_FS_Current/10;
			
			// Called already in case VOLTAGE_LOW
//			Reference.iq_OVP_Reference = _IQ15rmpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(1.1));
			
			Global_Flags.fg_Previous_Calibration = CURRENT_LOW;
			break;
		}
		case CURRENT_FULL:
		{
			// 100%, 100%
			Reference.iq_Voltage_Reference = Product_Info.iq_Specified_FS_Voltage;
			Reference.iq_Current_Reference = Product_Info.iq_Specified_FS_Current;
			
			// Called already in case VOLTAGE_LOW
//			Reference.iq_OVP_Reference = _IQ15rmpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(1.1));

			Global_Flags.fg_Previous_Calibration = CURRENT_FULL;
			break;
		}
		default:
			break;
	}
	ei_vSetOvpReference();
	
	ei_vSwitchSpiClkPolarity(DAC_8552);
	
	ei_vSetVoltageReference();
	ei_vSetCurrentReference();
	
	ei_vSwitchSpiClkPolarity(AD7705);
}

//#####################	Recall last settings ######################################################################
// Function Name: Uart_Recall_Last_PSU_Settings
// Return Type: void
// Arguments:   void
// Description: Recalls the last settings from the EEPROM.
//				The Following are not changed
//				1. Address
//				2. PSU
//				3. Enable registers. (Note that the bt_Power_On_Status_Clear condition may change but that impact will be seen
//									  only during booting next time. The present values of enable remain undisturbed).

//	And because the enable registers must not be changed. Here we will have to disable SCHEDULER interrupts only in the beginning
//	of the function so that the in between Change of the enable registers because of calling RetieveLastSettings can
//	be corrected after coming out of that function.
//	SCHEDULER Interrupt will be enabled just before leaving the function.
//#################################################################################################################
void ei_vRecallLastPSUSettings()
{
	// Recall last settings from EEPROM
	// Do not change Address and baud rate values so that communications are not disrupted
	
	// Variables to store present address and baudrate values
	Uint16 Temp_Addr = 0;
	Uint16 Temp_Baud = 0;
	Uint16 Temp_FENA,Temp_OENA,Temp_SENA,Temp_SRENA;
	
	// Copy Enable Register values to Temps
	Temp_FENA = Fault_Regs.FAULT_ENABLE.all; 
	Temp_OENA = Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all; 
	Temp_SENA = Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_ENABLE.all; 
	Temp_SRENA = Service_Request_Regs.SERVICE_REQUEST_ENABLE.all; 
	
	// Store the current address and baud rate settings.
	Temp_Addr = Encoder.ui_Qep_Address_Count;
	Temp_Baud = Encoder.ui_Qep_BaudRate_Count;
	
	// Disable Scheduler Interrupt.
	EPwm6Regs.ETSEL.bit.INTEN = DISABLE_INT;         	// Disable EPWM1_INT
	
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
	
	ei_vRetrieveLastSettings();
	
	// Restore the Address and Baudrate settings which might have gotten changed in the above function.
	Encoder.ui_Qep_Address_Count = Temp_Addr;
	Encoder.ui_Qep_BaudRate_Count = Temp_Baud;
	
	// Restore Enable Registers
	// Copy Enable Register values to Temps
	Fault_Regs.FAULT_ENABLE.all = Temp_FENA; 
	Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all = Temp_OENA; 
	Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_ENABLE.all = Temp_SENA; 
	Service_Request_Regs.SERVICE_REQUEST_ENABLE.all = Temp_SRENA;
	
	// Enable Scheduler Interrupt.
	EPwm6Regs.ETSEL.bit.INTEN = ENABLE_INT;         	// Enable EPWM1_INT
}
