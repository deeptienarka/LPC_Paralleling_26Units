//#################################################################################################################
//# 							enArka Instruments proprietary
//# File: ei_SpiApp.c
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
#define SPI_DELAY			500
#define AD7705_DELAY		2000000000L
#define SPI_WAIT_UNTIL(x)	{ \
								timeout_cnt1 = 0; \
								while((x)) \
								{ \
									timeout_cnt1++; \
									if (timeout_cnt1 >= SPI_DELAY) \
									{ \
										Fault_Regs.FAULT_REGISTER.bit.bt_Internal_Communication_Fault = 1; \
										ei_vResetPsuToSafeState(); \
										return FAILURE; \
									} \
								} \
							}
#define AD7705_WAIT_UNTIL(x)	{ \
									timeout_cnt1 = 0; \
									while((x)) \
									{ \
										timeout_cnt1++; \
										if (timeout_cnt1 >= AD7705_DELAY) \
										{ \
											Fault_Regs.FAULT_REGISTER.bit.bt_On_Board_Adc_Failure = 1; \
											ei_vResetPsuToSafeState(); \
											return FAILURE; \
										} \
									} \
								}
volatile Uint32 timeout_cnt1;
//#################################################################################################################
									/*-------Global Prototypes----------*/
//void ei_vAD7705SoftwareReset();
//Uint16 ei_vInitializeAD7705();
//void ei_vSocAD7705(Uint16 Channel_no);
//Uint16 ei_vReadAD7705Result(Uint16 Channel_no);
//void ei_vAD7705LPF(Uint16 Channel_no);
//void ei_uiSpiWrite(Uint32 ui_Device, Uint16 ui_NoOfDatabytes, unsigned char a_c_DataArray[]);
//Uint16 ei_uiSpiRead(Uint16 *p_ui_Result);
//void ei_vSwitchSpiClkPolarity(Uint16 Device_Type);
//void ei_vExternalAdcOperation();

//###################################	SPI Baud Rate Settings  ###################################################

//		LSPCLK is input to the SPI module. We operate at 90M/4 = 22.5M LSPCLK.
//
//		SPI Baud Rate = LSPCLK/(SPIBRR + 1)
//
//		Our Settings:
//		SPIBRR: 44
//
//
//		Baud Rate set = 22.5M/(44+1) = 500kbps
//#################################################################################################################

//#######################  An explanantion of AD7705 scheduling ###################################################
//	We have set the o/p data rate at 25Hz or 40ms.That means that results of conversion are available at 40ms.
//	But if we switch the channels, the 1st result alone takes 40ms x 3 = 120ms. Subsequent conversion of the same 
//	channel occur at ouput data rate.
//  In our code we perform 4 conversions of the same channel & switch to the other channel after reading the result 
//	of 4th conversion.
//
//			=> 42,82,122,12 -> channel A2.
//			=> 282,322,362,402 -> channel A1.
//
//	Thus we se that in our scheduling of AD7705 events we read the result of channel A2 conversion at 42nd event. Then 
//	we initiate another conversion of the same channel A2 and read it 40ms later.
//	At the 4th event at 162ns, we read A2 result and switch channel to A1. The first result of A1 is available only 
//	after 120 ms at 282 event. The same process is repeated.
//#################################################################################################################

//#####################	Initialize AD7705 #########################################################################
// Function Name: ei_vAD7705SoftwareReset
// Return Type: void
// Arguments:   void
// Description: Write 32 bits of 1 to reset the AD7705 communication bus.
//#################################################################################################################
void ei_vAD7705SoftwareReset()
{
	unsigned char a_c_SpiTxDataArray[4];

	//	Write 32 bits of 1 to AD7705 to reset it.
	a_c_SpiTxDataArray[0] = 0xFF;
	a_c_SpiTxDataArray[1] = 0xFF;
	a_c_SpiTxDataArray[2] = 0xFF;
	a_c_SpiTxDataArray[3] = 0xFF;

	ei_uiSpiWrite(CS_AD7705, 4, a_c_SpiTxDataArray);
}

//-----------------------------------------------------------------------------------------------------------------
// Reset and initialize the device.
// Function Name: ei_vInitializeAD7705
// Return Type: Uint16
// Arguments:   void
// Description: Initialize the device to set up conversions.
//				Every write or read from AD705 must be proceeded by a write to the communication register.
//
				// Register map
// 1. Communincation
//		D7: 0/DRDY(Active low) (Read status of DRDY pin. Write 0 to enable next 7 values to be latched
//		D6(RS2)	D5(RS1) D4(RS0) [Register selection]
//		0		0		0		Comm. Reg
//		0		0		1		Setup Reg
//		0		1		0		Clock Reg
//		0		1		1		Data Reg
//		1		0		0		Test Reg
//		1		0		1		No operation
//		1		1		0		Offset Reg
//		1		1		1		Gain Reg
//		D3:R/~W (0: Write/ 1: Read)
//		D2:STBY (0: Normal/ 1:STBY)
//		D1(CH1)	D0(CH2)	[Channel]
//		0		0		AIN1+/AIN1-
//		0		1		AIN2+/AIN2-
//		1		0		AIN1-/AIN1-
//		1		1		AIN1+/AIN2-

// 2. Setup Register
//		D7(MD1)	D6(MD0)	[ADC Mode]
//		0		0		Normal mode
//		0		1		Self Calibration mode
//		1		0		Zero Scale Cal. mode
//		1		1		Full Scale Cal. mode
//		D5(G2)	D4(G1)	D3(G0)	[Gain Setting]
//		0		0		0		1
//		0		0		1		2
//		0		1		0		4
//		0		1		1		8
//		1		0		0		16
//		1		0		1		32
//		1		1		0		64
//		1		1		1		128
//		D2:(~B/U)	[0: Bipolar, 1: Unipolar]
//		D1:(BUF)	[0: Unbuffered/ 1: Unbuffered]
//		D0:FSYNC	[0: Normal/ 1: Filter off]

// 3. Clock Register
//		D7-D5	0
//		D4: CLKDIS	[0:Enable MCLKOUT/ 1: Disable MCLKOUT]
//		D3: CLKDIV	[0:Divide by 1/1:Divide by 2]
//		D2: CLK		[0:MCLK = 1M/1:MCLK = 2M]
//		D2[CLK]	D1[FS1]	D0[FS0]	[Output Data Rate]
//		0		0		0		20Hz
//		0		0		1		25Hz
//		0		1		0		100Hz
//		0		1		1		200Hz
//		1		0		0		50Hz
//		1		0		1		60Hz
//		1		1		0		250Hz
//		1		1		1		500Hz
//------------------------------  DAC IC --------------------------------------------------------------------------
//	DAC 8552 uses a 24 bit register for it's complete operation. SPI writes to DAC must be of 3 bytes everytime.

//					Table 1: 24 bit register setup for DAC
//	DB23 																				DB12
//	0 	0 	LDB 	LDA 	X 	Buffer_Select 	PD1 	PD0 	D15 	D14 	D13 	D12

//	DB11 																				DB0
//	D11 D10 D9 		D8 		D7 	D6 				D5 		D5 		D3 		D2 		D1 		D0

//					Table 2: Control Matrix
//	D23 	D22 	D21 	D20 	D19 	D18 	D17 	D16 	D15 	D14 	D13–D0
//									Don't 	Buffer 									MSB-2...
//	 Reserved 		Load B Load A 	Care 	Select 	PD1 	PD0 	MSB		MSB-1	LSB
//											0 = A,
//	Always Write 0  						1 = B 												DESCRIPTION
//	0 		0 		0 		0 		X 		# 		0 		0 		----------Data-----------   WR Buffer # w/Data
//	0 		0 		0 		0 		X 		# 		See Table3  	---------- X ------------	WR Buffer # w/Power-down Command
//	0 		0 		0 		1 		X 		# 		0 		0 		----------Data-----------	WR Buffer # w/Data and Load DAC A
//	0 		0 		0 		1 		X 		0 		See Table3  	---------- X ------------	WR Buffer A w/Power-Down Command and LOAD DAC A																	WR Buffer A w/Power-Down Command and LOAD DAC A
//																								(DAC A Powered Down)
//	0 		0 		0 		1 		X 		1 		See Table3		---------- X ------------	WR Buffer B w/Power-Down Command and LOAD DAC A
//	0 		0 		1 		0 		X 		# 		0 		0 		----------Data-----------	WR Buffer # w/Data and Load DAC B
//	0 		0 		1 		0 		X 		0 		See Table3 		---------- X ------------ 	WR Buffer A w/Power-Down Command and LOAD DAC B
//	0 		0 		1 		0 		X 		1 		See Table3 		---------- X ------------	WR Buffer B w/Power-Down Command and LOAD DAC B
//																								(DAC B Powered Down)
//	0 		0 		1 		1 		X 		# 		0 		0 		----------Data----------- 	WR Buffer # w/Data and Load DACs A and B
//	0 		0 		1 		1 		X 		0 		See Table3 		----------- X -----------	WR Buffer A w/Power-Down Command and Load DACs A and
//																								B (DAC A Powered Down)
//	0 		0 		1 		1 		X 		1 		See Table3 		----------- X------------	WR Buffer B w/Power-Down Command and Load DACs A and
//																								B (DAC B Powered Down)

//					Table 3. Power-Down Commands
//	D17 	D16
//	PD1 	PD0 		OUTPUT IMPEDANCE POWER DOWN COMMANDS
//	0 		1 			1kW
//	1 		0 			100kW
//	1 		1 			High Impedance
//#################################################################################################################
Uint16 ei_vInitializeAD7705()
{
//	Step 1: Software reset of the spi bus of the device.
	ei_vAD7705SoftwareReset();

//  A 20ms Delay here to allow AD7705 to become stable when 1MHz clock given. Might not be actually required because
//	it is mandatory when using a crystal to clock AD7705. But here we use a PWM pin to provide clock directly
    DELAY_US(STARTUP_TIME_DELAY);

    unsigned char a_c_SpiTxDataArray[1];

//  Step 2: Write to Clock register
    a_c_SpiTxDataArray[0] = AD7705_CLOCK_REGISTER | AD7705_WRITE;
	if(ei_uiSpiWrite(CS_AD7705, 1, a_c_SpiTxDataArray) == FAILURE)
	{
		return FAILURE;
	}

	a_c_SpiTxDataArray[0] = AD7705_DISABLE_MCLK_OUT | AD7705_CLKDIV_1 | AD7705_MCLK_1MHz | AD7705_OUTPUT_DATA_RATE_25;
	if(ei_uiSpiWrite(CS_AD7705, 1, a_c_SpiTxDataArray) == FAILURE)
	{
		return FAILURE;
	}

//	Step 3: Write to Setup register and self calibration for channel 1.
	a_c_SpiTxDataArray[0] = AD7705_SETUP_REGISTER | AD7705_WRITE | AD7705_CH1;
	if(ei_uiSpiWrite(CS_AD7705, 1, a_c_SpiTxDataArray) == FAILURE)
	{
		return FAILURE;
	}

	a_c_SpiTxDataArray[0] = AD7705_SELF_CALIBRATION | AD7705_UNITY_GAIN | ADD7705_UNIPOLAR | AD7705_UNBUFFERED_INPUT | AD7705_CLEAR_FSYNC;
	if(ei_uiSpiWrite(CS_AD7705, 1, a_c_SpiTxDataArray) == FAILURE)
	{
		return FAILURE;
	}

	DELAY_US(128);		// Ignore the DRDY pin for 128 us
//	while(DRDY_PIN != AD7705_READY_TO_BE_READ);
	AD7705_WAIT_UNTIL(DRDY_PIN != AD7705_READY_TO_BE_READ)
	if(ei_vReadAD7705Result(CH1) == FAILURE)
	{
		return FAILURE;
	}

//	Step 4: Do a self calibration for channel 2.
	a_c_SpiTxDataArray[0] = AD7705_SETUP_REGISTER | AD7705_WRITE | AD7705_CH2;
	if(ei_uiSpiWrite(CS_AD7705, 1, a_c_SpiTxDataArray) == FAILURE)
	{
		return FAILURE;
	}

	a_c_SpiTxDataArray[0] = AD7705_SELF_CALIBRATION | AD7705_UNITY_GAIN | ADD7705_UNIPOLAR | AD7705_UNBUFFERED_INPUT | AD7705_CLEAR_FSYNC;
	if(ei_uiSpiWrite(CS_AD7705, 1, a_c_SpiTxDataArray) == FAILURE)
	{
		return FAILURE;
	}

	DELAY_US(128);		// Ignore the DRDY pin for 128 us
//	while(DRDY_PIN != AD7705_READY_TO_BE_READ);
	AD7705_WAIT_UNTIL(DRDY_PIN != AD7705_READY_TO_BE_READ)
	if(ei_vReadAD7705Result(CH2) == FAILURE)
	{
		return FAILURE;
	}
	
	return SUCCESS;
}

//#####################	Start of Conversion #######################################################################
// Function Name: ei_vSocAD7705
// Return Type: void
// Arguments:   Uint16 Channel_no
// Description: Write to the communication register which channel to be converted next.
//				AIN1+/AIN1-		Current Measure
//				AIN2+/AIN2-		Voltage Measure
//#################################################################################################################
void ei_vSocAD7705(Uint16 Channel_no)
{
	unsigned char a_c_SpiTxDataArray[1];
	switch(Channel_no)
	{
		case CH1:
		{
			a_c_SpiTxDataArray[0] = AD7705_DATA_REGISTER | AD7705_READ | AD7705_CH1;
			if(ei_uiSpiWrite(CS_AD7705, 1, a_c_SpiTxDataArray) == FAILURE)
			{
				return;
			}
			break;
		}
		case CH2:
		{
			a_c_SpiTxDataArray[0] = AD7705_DATA_REGISTER | AD7705_READ | AD7705_CH2;
			if(ei_uiSpiWrite(CS_AD7705, 1, a_c_SpiTxDataArray) == FAILURE)
			{
				return;
			}
			break;
		}
	}
}

//#####################	Read Result ###############################################################################
// Function Name: ei_vReadAD7705Result
// Return Type: Uint16
// Arguments:   void
// Description: Read Channel when DRDY pin low. Return Success or Failure depending on the state of operation
//#################################################################################################################
Uint16 ei_vReadAD7705Result(Uint16 Channel_no)
{
	Uint16 ui_Result;
	ui_Result = 0;
	switch(Channel_no)
	{
		case CH1:
		{
//			while(DRDY_PIN != AD7705_READY_TO_BE_READ);	// Wait for pin to go low to enable read
			AD7705_WAIT_UNTIL(DRDY_PIN != AD7705_READY_TO_BE_READ)	// Wait for pin to go low to enable read
			if(ei_uiSpiRead(&ui_Result) == FAILURE)
			{
				return FAILURE;
			}
			External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_ADC_Result_Regs = ui_Result;
			External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Normalized_Adc_Values = _Q16toIQ26(External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_ADC_Result_Regs);
			break;
		}
		case CH2:
		{
//			while(DRDY_PIN != AD7705_READY_TO_BE_READ);	// Wait for pin to go low to enable read
			AD7705_WAIT_UNTIL(DRDY_PIN != AD7705_READY_TO_BE_READ)	// Wait for pin to go low to enable read
			if(ei_uiSpiRead(&ui_Result) == FAILURE)
			{
				return FAILURE;
			}
			External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_ADC_Result_Regs = ui_Result;
			External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Normalized_Adc_Values = _Q16toIQ26(External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_ADC_Result_Regs);
		}
	}
	return SUCCESS;
}

//#####################	AD7705 LPF ################################################################################
// Function Name: ei_vAD7705LPF
// Return Type: void
// Arguments:   Uint16 Channel_no
// Description: Update results and moving average filter for External ADC.
//#################################################################################################################
void ei_vAD7705LPF(Uint16 Channel_no)
{
	// Preconditions
	// Preconditions for the critical functions.
	DENOMINATOR_ZERO_NEGATIVE_CHECK(Product_Info.iq_Voltage_Display_Gain)
	DENOMINATOR_ZERO_NEGATIVE_CHECK(Product_Info.iq_Current_Display_Gain)
	
	switch(Channel_no)
	{
		case CH1:
		{
			External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Filtered_Value = ((External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Filtered_Value*(NO_OF_AD7705_SAMPLES - 1) + External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Normalized_Adc_Values)>>(RIGHT_SHIFT_FACTOR_FOR_AD7705_LPF));
			External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Scaled_Unfiltered_Value = _IQ15mpy(_IQ26toIQ15(External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Normalized_Adc_Values), External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Full_Scale_Value);
			External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Scaled_Filtered_Value = _IQ15mpy(_IQ26toIQ15(External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Filtered_Value), External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Full_Scale_Value);
			External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Calibrated_Value = _IQ15div((External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Scaled_Filtered_Value - Product_Info.iq_Current_Display_Offset),Product_Info.iq_Current_Display_Gain);
			
			IQ_DIVISION_RESULT_NEG_AND_POSITIVE_SATURATION_CHECK(External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Calibrated_Value)
			
			if(External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Calibrated_Value < 0)
			{
				External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Calibrated_Value = 0;
			}
			break;
		}

		case CH2:
		{
			External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Filtered_Value = ((External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Filtered_Value*(NO_OF_AD7705_SAMPLES - 1) + External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Normalized_Adc_Values)>>(RIGHT_SHIFT_FACTOR_FOR_AD7705_LPF));
			External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Scaled_Unfiltered_Value = _IQ15mpy(_IQ26toIQ15(External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Normalized_Adc_Values), External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Full_Scale_Value);
			External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Scaled_Filtered_Value = _IQ15mpy(_IQ26toIQ15(External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Filtered_Value), External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Full_Scale_Value);
			External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Calibrated_Value = _IQ15div((External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Scaled_Filtered_Value - Product_Info.iq_Voltage_Display_Offset),Product_Info.iq_Voltage_Display_Gain);
			
			IQ_DIVISION_RESULT_NEG_AND_POSITIVE_SATURATION_CHECK(External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Calibrated_Value)
			
			if(External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Calibrated_Value < 0)
			{
				External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Calibrated_Value = 0;
			}
			break;
		}
	}
}
	

//#####################	Spi Write #################################################################################
// Function Name: ei_uiSpiWrite
// Return Type: Uint16
// Arguments:   Uint32 ui_Device, Uint16 ui_NoOfDatabytes, unsigned char a_c_DataArray[]
// Description: Spi write driver. Pass data to be written. Returns Success or Failure after operation completed.
//#################################################################################################################
Uint16 ei_uiSpiWrite(Uint32 ui_Device, Uint16 ui_NoOfDatabytes, unsigned char a_c_DataArray[])
{
	Uint16 Dummy, i;
//	Step 1: Pull Gpio Low Depending on the device no
//  Step 2: Write to SPITX Whatever data needs to be sent. All writes to TXBUF are left justified
//	Step 3: Perform a dummy read after every write

	Dummy = 0;
	i = Dummy;
	i = 0;

	GpioDataRegs.GPACLEAR.all = ui_Device;
	/*if(ui_Device == CS_AD7705)
	{
		CHIP_SELECT_AD7705;
	}
	else //if(ui_Device == CS_AD7705
	{
		CHIP_SELECT_DAC;
	}*/

	while(ui_NoOfDatabytes != 0)
	{
		SPI_WAIT_UNTIL(SpiaRegs.SPISTS.bit.BUFFULL_FLAG ==  1) 				// While the buffer is full.
		SpiaRegs.SPITXBUF = ((Uint16)a_c_DataArray[i]) << 8;				// Left justify the data and transmit it
		SPI_WAIT_UNTIL(SpiaRegs.SPISTS.bit.INT_FLAG ==  0)					// While the buffer is full.
		Dummy = SpiaRegs.SPIRXBUF;											// Dummy read after every write
		ui_NoOfDatabytes--;
		i++;
	}

	GpioDataRegs.GPASET.all = ui_Device;
	/*if(ui_Device == CS_AD7705)
	{
		CHIP_RELEASE_AD7705;
	}
	else //if(ui_Device == CS_AD7705
	{
		CHIP_RELEASE_DAC;
	}*/
	
	return SUCCESS;
}

//#####################	Spi Read ##################################################################################
// Function Name: ei_uiSpiRead
// Return Type: Uint16 Spi_Receive_Data
// Arguments:   Uint16 *p_ui_Result
// Description: Spi write driver. Pass the variable by reference to which the result is copied. Returns Success or failure
//				Depending on operation completed or not.
//#################################################################################################################
Uint16 ei_uiSpiRead(Uint16 *p_ui_Result)//(Uint16 Device_No, Uint16 No_of_DataBytes)
{
//	Step 1: Pull Gpio Low Depending on the device no
//  Step 2: Write to SPITX Whatever data needs to be sent. All writes to TXBUF are left justified
//	Step 3: Transmit Junk data and read the receive Buffer to receive data

	CHIP_SELECT_AD7705;

	SPI_WAIT_UNTIL(SpiaRegs.SPISTS.bit.BUFFULL_FLAG ==  1) 	// While the buffer is full.
	SpiaRegs.SPITXBUF = JUNK;			    // JUNK
	SPI_WAIT_UNTIL(SpiaRegs.SPISTS.bit.INT_FLAG ==  0)
	(*p_ui_Result) = (SpiaRegs.SPIRXBUF) << 8;

	SPI_WAIT_UNTIL(SpiaRegs.SPISTS.bit.BUFFULL_FLAG ==  1) 	// While the buffer is full.
	SpiaRegs.SPITXBUF = JUNK;			    // JUNK
	SPI_WAIT_UNTIL(SpiaRegs.SPISTS.bit.INT_FLAG ==  0)
	(*p_ui_Result) |= (0x00FF & SpiaRegs.SPIRXBUF);

	CHIP_RELEASE_AD7705;
	return SUCCESS;
}

//#####################	Switch clock polarity ##################################################################################
// Function Name: ei_vSwitchSpiClkPolarity
// Return Type: void 
// Arguments:   Uint16 Device_Type
// Description: Switch the SPICLK polarity depending on the SPI device being addressed.
// 	We use 2 SPI based devices in our project now.
//	1. AD7705
//	2. DAC8552

// Now the clock polarity requirements(The idle state of the clock when data not being latched) is opposite.
// DAC8552 requires SPICLK to remain LOW when idle.
// AD7705 requires SPICLK to remain HIGH when idle.

// It was determined after some testing that these states must be strictly followed when communicating to the
// device. Neither of the device will work in the other mode. 
// Thus one solution to rectify this problem is to use a NOT gate in hardware. (In 3U provision exists.)
// 
// The solution we use is that just before addressing the DAC we switch the clock polarity of the SPICLK using
// CLKPOLARITY bit. After addressing DAC we go back to CLKPOLARITY high which is suitable for AD7705.

// Works like a charm!!!

// NOTE: SPI module must be held in reset state before making setting changes. It will be released from reset
// after the polarity is switched.
//#################################################################################################################
void ei_vSwitchSpiClkPolarity(Uint16 Device_Type)
{
	SpiaRegs.SPICCR.bit.SPISWRESET = 0;	 						// Hold in reset
	if(Device_Type == AD7705)
	{
		SpiaRegs.SPICCR.bit.CLKPOLARITY = 1;
	}
	else
	{
		SpiaRegs.SPICCR.bit.CLKPOLARITY = 0;
	}
	SpiaRegs.SPICCR.bit.SPISWRESET = 1;	 						// Release from reset
	DELAY_US(1);												// 1us for stability.
}

//#####################	External Adc Operation  ###################################################################
// Function Name: ei_vExternalAdcOperation
// Return Type: void
// Arguments:   void
// Description: Performs External 16 bit Sigma delta ADC operation and all its Calculations.
//				External ADC is used for Output Display
//#################################################################################################################
void ei_vExternalAdcOperation()
{
	// Read the result and compute LPF for the AD7705
	ei_vReadAD7705Result(Global_Flags.fg_Active_AD7705_Channel);
	ei_vAD7705LPF(Global_Flags.fg_Active_AD7705_Channel);

	// Give the next start of conversion
	ei_vSocAD7705(Global_Flags.fg_Next_AD7705_Channel);
}
