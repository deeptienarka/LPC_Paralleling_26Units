//#################################################################################################################
//# 							enArka Instruments proprietary
//# File: ei_InitGpio.c
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.
//# All Rights Reserved
//#
//#################################################################################################################

//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
//#################################################################################################################
							/*--------Global Prototypes---------*/
//void ei_vPinMapping();
//#################################################################################################################
							/*--------Local Prototypes---------*/
static void ei_vInitEQep1Gpio();	// EQepa(Voltage) 
static void ei_vInitEQep2Gpio();	// EQepb(Current)
static void ei_vInitI2CGpio();
static void ei_vInitSpiaGpio();
static void ei_vInitEPwm2Gpio();
static void ei_vInitEPwm3Gpio();
static void ei_vInitEPwm4Gpio();
static void ei_vInitEPwm5Gpio();
static void ei_vInitEPwm8Gpio();
static void ei_vInitSciaGpio();
static void ei_vInitECanaGpio();
static void ei_vInitGpioPins();
static void ei_vInitAioPins();
//#################################################################################################################
void ei_vPinMapping()
{
	EALLOW;
	ei_vInitEQep1Gpio();
	ei_vInitEQep2Gpio();
	ei_vInitI2CGpio();
	ei_vInitSpiaGpio();
 	ei_vInitEPwm2Gpio();
	ei_vInitEPwm3Gpio();
	ei_vInitEPwm4Gpio();
	ei_vInitEPwm5Gpio();
	ei_vInitEPwm8Gpio();
	ei_vInitSciaGpio();
	ei_vInitECanaGpio();
	ei_vInitGpioPins();
	ei_vInitAioPins();
	EDIS;
}

//#####################	Pin Mapping for this Project ##############################################################
//
//			PIN NO				GPIO NO				FUNCTION ASSIGNED
//
//			6					Gpio20				EQEP1A
//			7					Gpio21				EQEP1B
//			97					Gpio24				EQEP2A 
//			39					Gpio25				EQEP2B
//			99					Gpio32				SDAA
//			100					Gpio33				SCLA
//			69					Gpio54				SPISIMOA				
//			75					Gpio55				SPISOMIA
//			85					Gpio56				SPICLKA
//			74					Gpio10				Gpio Out	(CS_ADC)
//			88					Gpio15				Gpio Out	(CS_DAC)
//			84					Gpio2				EPWM2A 		(Fan Control)
//			9					Gpio4				EPWM3A 		(MCLK IN)
//			58					Gpio6				EPWM4A 		(UC2825_CLK_CH2)
//			54					Gpio8				EPWM5A		(UC2825_CLK_CH1)
//			1					Gpio42				EPWM8A		(Vovp Ref)
//			50					Gpio28				SCIRXDA
//			43					Gpio29				SCITXDA
//			41					Gpio30				CANRXA 
//			40					Gpio31				CANTXA 
//			2					Gpio23				Gpio in 	(PFC_DC_BUS_FAIL)
//			98					Gpio22				Gpio in 	(FAN_FAIL_1)
//			48					Gpio51				Gpio in		(OVP_FAULT)
//			89					Gpio57				Gpio in		(OTP_FAULT)
//			73					Gpio11				Gpio in		(AC_FAIL)
//			44					Gpio12				Gpio in		(IN_LOCK)
//			95					Gpio13				Gpio in		(SO_DSP)
//			55					Gpio16				Gpio in		(REM/LOC_STATUS_FP)
//			78					Gpio26				Gpio in		(COM_2_EN)
//			76					Gpio41				Gpio in		(CC/CV)
//			42					Gpio50				Gpio in		(DRDY)
//			66					Gpio39				Gpio out	(ON_OFF_DSP)
//			53					Gpio52				Gpio out	(O/P_FAST_DISCHARGE)
//			77					Gpio27				Gpio out	(Reset)
//			52					Gpio17				Gpio out	(TRANS_ENABLE)
//			65					Gpio53				Gpio out	(PS_GOOD)
//			96					Gpio14				Gpio out	(EXECUTION TIME)				
//			16										ADCINA7		(TEMPERATURE_SENSE)			SOC5, ADCINT2
//			18										ADCINA5		(V_MEASURE)
//			19					AIO4				ADCINA4		(OVP_REF)					SOC6, ADCINT1
//			20										ADCINA3		(I_MEASURE)
//			21					AIO2				ADCINA2		(DAC_REF)
//			22										ADCINA1		(I_PRI_CH2)
//			23										ADCINA0		(I_PRI_CH1)
//#################################################################################################################

//#####################	Initialise EQEP Pins ######################################################################
// Function Name: ei_vInitEQepXGpio
// Return Type: void
// Arguments:   void
// Description: Initialises EQep Gpios for interfacing rotary encoders.
//				Sampling performed 
//#################################################################################################################
static void ei_vInitEQep1Gpio(void)
{
   GpioCtrlRegs.GPAMUX2.bit.GPIO20 = PERIPHERAL_EQEP1; 		//EQep1A
   GpioCtrlRegs.GPAPUD.bit.GPIO20 = PULLUP_ENABLE;  		// enable
   GpioCtrlRegs.GPAQSEL2.bit.GPIO20 = SIX_SAMPLES_FOR_INPUT_QUAL; // Six Samples taken for input qualification
	
   GpioCtrlRegs.GPAMUX2.bit.GPIO21 = PERIPHERAL_EQEP1; 		//EQep1B
   GpioCtrlRegs.GPAPUD.bit.GPIO21 = PULLUP_ENABLE;  		// enable
   GpioCtrlRegs.GPAQSEL2.bit.GPIO21 = SIX_SAMPLES_FOR_INPUT_QUAL; // Six Samples taken for input qualifiaction
}
//-----------------------------------------------------------------------------------------------------------------
static void ei_vInitEQep2Gpio(void)
{
   GpioCtrlRegs.GPAMUX2.bit.GPIO24 = PERIPHERAL_EQEP2; 		//EQep2A
   GpioCtrlRegs.GPAPUD.bit.GPIO24 = PULLUP_ENABLE;  		// enable
   GpioCtrlRegs.GPAQSEL2.bit.GPIO24 = SIX_SAMPLES_FOR_INPUT_QUAL; // Six Samples taken for input qualification
	
   GpioCtrlRegs.GPAMUX2.bit.GPIO25 = PERIPHERAL_EQEP2; 		////EQep2B
   GpioCtrlRegs.GPAPUD.bit.GPIO25 = PULLUP_ENABLE;  		// enable
   GpioCtrlRegs.GPAQSEL2.bit.GPIO25 = SIX_SAMPLES_FOR_INPUT_QUAL; // Six Samples taken for input qualification
}

//#####################	Initialise I2C Pins #######################################################################
// Function Name: ei_vInitI2CGpio()
// Return Type: void
// Arguments:   void
// Description: Initialises I2C. No Input Qualification should be used on these pins.
//				Synchronised to SYSCLOUT by default
//#################################################################################################################
static void ei_vInitI2CGpio(void)
{
   GpioCtrlRegs.GPBMUX1.bit.GPIO32 = PERIPHERAL_I2C; 		//SDAA
   GpioCtrlRegs.GPBPUD.bit.GPIO32 = PULLUP_ENABLE;  		// enable
	
   GpioCtrlRegs.GPBMUX1.bit.GPIO33 = PERIPHERAL_I2C; 		//SCLA
   GpioCtrlRegs.GPBPUD.bit.GPIO33 = PULLUP_ENABLE;  		// enable
}

//#####################	Initialise SPI Pins #######################################################################
// Function Name: ei_vInitSpiaGpio()
// Return Type: void
// Arguments:   void
// Description: Initialises SPI pins. No Input Qualification should be used on these pins.
//				Synchronised to SYSCLOUT by default
//#################################################################################################################
static void ei_vInitSpiaGpio()
{
   GpioCtrlRegs.GPBMUX2.bit.GPIO54 = PERIPHERAL_SPIA; 		//SPISIMOA
   GpioCtrlRegs.GPBPUD.bit.GPIO54 = PULLUP_DISABLE;  		// enable
   
   GpioCtrlRegs.GPBMUX2.bit.GPIO55 = PERIPHERAL_SPIA; 		//SPISOMIA
   GpioCtrlRegs.GPBPUD.bit.GPIO55 = PULLUP_DISABLE;  		// enable
   
   GpioCtrlRegs.GPBMUX2.bit.GPIO56 = PERIPHERAL_SPIA; 		//SPICLKA
   GpioCtrlRegs.GPBPUD.bit.GPIO56 = PULLUP_DISABLE;  		// enable
   
/*   GpioCtrlRegs.GPBMUX2.bit.GPIO57 = GPIO_FUNCTION; 		//Chip select for AD7705
   GpioCtrlRegs.GPBPUD.bit.GPIO57 = PULLUP_ENABLE;  		// enable
   GpioDataRegs.GPBSET.bit.GPIO57 = 1;						// Should be high by default
   GpioCtrlRegs.GPBDIR.bit.GPIO57 = OUTPUT_PIN;				// Configured as output*/

   GpioCtrlRegs.GPAMUX1.bit.GPIO10 = GPIO_FUNCTION; 		//Chip select for AD7705
   GpioCtrlRegs.GPAPUD.bit.GPIO10 = PULLUP_ENABLE;  		// enable
   GpioDataRegs.GPASET.bit.GPIO10 = 1;						// Should be high by default
   GpioCtrlRegs.GPADIR.bit.GPIO10 = OUTPUT_PIN;				// Configured as output

   GpioCtrlRegs.GPAMUX1.bit.GPIO15 = GPIO_FUNCTION;			//Chip select for DAC8552
   GpioCtrlRegs.GPAPUD.bit.GPIO15 = PULLUP_ENABLE; 			// enable
   GpioDataRegs.GPASET.bit.GPIO15 = 1;						// Should be high by default
   GpioCtrlRegs.GPADIR.bit.GPIO15 = OUTPUT_PIN;				// Configured as output
}

//#####################	Initialise EPwm Pins ######################################################################
// Function Name: ei_vInitEPwmXGpio()
// Return Type: void
// Arguments:   void
// Description: Initialises EPwm pins. EPWMXB not used anywhere to get independent periods for each PWM.
//#################################################################################################################
//-----------------------------------------------------------------------------------------------------------------
// Fan Speed Control
static void ei_vInitEPwm2Gpio(void)
{ 
   GpioDataRegs.GPACLEAR.bit.GPIO2 = SET_1_TO_CLEAR;		// Initially low.
   GpioCtrlRegs.GPAMUX1.bit.GPIO2 = PERIPHERAL_PWM_PIN; 	// EPWM2A
   GpioCtrlRegs.GPAPUD.bit.GPIO2 = PULLUP_ENABLE;  			// enable
}
//-----------------------------------------------------------------------------------------------------------------
// MCLK IN
static void ei_vInitEPwm3Gpio(void)
{ 
   GpioDataRegs.GPACLEAR.bit.GPIO4 = SET_1_TO_CLEAR;		// Initially low.
   GpioCtrlRegs.GPAMUX1.bit.GPIO4 = PERIPHERAL_PWM_PIN; 	// EPWM3A
   GpioCtrlRegs.GPAPUD.bit.GPIO4 = PULLUP_ENABLE;  			// enable
}
//-----------------------------------------------------------------------------------------------------------------
// INTERLEAVE_2
static void ei_vInitEPwm4Gpio(void)
{ 
   GpioDataRegs.GPACLEAR.bit.GPIO6 = SET_1_TO_CLEAR;		// Initially low.
   GpioCtrlRegs.GPAMUX1.bit.GPIO6 = PERIPHERAL_PWM_PIN; 	// EPWM4A
   GpioCtrlRegs.GPAPUD.bit.GPIO6 = PULLUP_ENABLE;  			// enable
}
//-----------------------------------------------------------------------------------------------------------------
// INTERLEAVE_1
static void ei_vInitEPwm5Gpio(void)
{ 
   GpioDataRegs.GPACLEAR.bit.GPIO8 = SET_1_TO_CLEAR;		// Initially low.
   GpioCtrlRegs.GPAMUX1.bit.GPIO8 = PERIPHERAL_PWM_PIN; 	// EPWM5A
   GpioCtrlRegs.GPAPUD.bit.GPIO8 = PULLUP_ENABLE;  			// enable
}
//-----------------------------------------------------------------------------------------------------------------
// OVP_REF
static void ei_vInitEPwm8Gpio()
{ 
   GpioDataRegs.GPBCLEAR.bit.GPIO42 = SET_1_TO_CLEAR;		// Initially low. 
   GpioCtrlRegs.GPBMUX1.bit.GPIO42 = PERIPHERAL_PWM_PIN; 	// EPWM8A
   GpioCtrlRegs.GPBPUD.bit.GPIO42 = PULLUP_ENABLE;  		// enable
}

//#####################	Initialise SCI Pins #######################################################################
// Function Name: ei_vInitSciaGpio
// Return Type: void
// Arguments:   void
// Description: Initialises Scia Gpios for Rx and Tx operations. No Input Qualification should be used on these pins.
//				Synchronised to SYSCLOUT by default
//#################################################################################################################
static void ei_vInitSciaGpio()
{
	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = PERIPHERAL_SCIA; 	 //Scia Rx Pin
	GpioCtrlRegs.GPAPUD.bit.GPIO28 = PULLUP_ENABLE;  	 // enable

	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = PERIPHERAL_SCIA; 	 //Scia Tx Pin
	GpioCtrlRegs.GPAPUD.bit.GPIO29 = PULLUP_ENABLE;  	 // enable
}

//#####################	Initialise ECan Pins ######################################################################
// Function Name: ei_vInitECanaGpio()
// Return Type: void
// Arguments:   void
// Description: Initialises ECana Gpios for Rx and Tx operations. No Input Qualification should be used on these pins.
//				Synchronised to SYSCLOUT by default
//#################################################################################################################
static void ei_vInitECanaGpio()
{
   GpioCtrlRegs.GPAPUD.bit.GPIO30 = PULLUP_DISABLE;
   GpioCtrlRegs.GPAPUD.bit.GPIO31 = PULLUP_DISABLE;
   GpioCtrlRegs.GPAMUX2.bit.GPIO30 = PERIPHERAL_ECANA;   // Configure GPIO30 as CANRXA 
   GpioCtrlRegs.GPAMUX2.bit.GPIO31 = PERIPHERAL_ECANA;   // Configure GPIO21 as CANTXA
}

//#####################	Initialise Gpio Pins ######################################################################
// Function Name: ei_vInitGpioPins()
// Return Type: void
// Arguments:   void
// Description: Initialises General Purpose pins used for this project. Input Qualification Used on INPUT PINS
//#################################################################################################################
static void ei_vInitGpioPins()
{
		//	Input Gpios. 
   	
	// Set QUALPRD MAX (5.6661u Sampling period = (2 * QUALPRD * 11.11n))
	// No. of Samples taken = 6.
	// Total pin stable time required = (6 * 5.6661u) + (Sampling period + Sysclokout) = 39.674u
	// Thus input qualification is 39.674u.	

	GpioCtrlRegs.GPACTRL.bit.QUALPRD0 = INPUT_PIN_SAMPLE_PERIOD;
	GpioCtrlRegs.GPACTRL.bit.QUALPRD1 = INPUT_PIN_SAMPLE_PERIOD;
	GpioCtrlRegs.GPACTRL.bit.QUALPRD2 = INPUT_PIN_SAMPLE_PERIOD;
	GpioCtrlRegs.GPACTRL.bit.QUALPRD3 = INPUT_PIN_SAMPLE_PERIOD;
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD0 = INPUT_PIN_SAMPLE_PERIOD;
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD1 = INPUT_PIN_SAMPLE_PERIOD;
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD2 = INPUT_PIN_SAMPLE_PERIOD;
	GpioCtrlRegs.GPBCTRL.bit.QUALPRD3 = INPUT_PIN_SAMPLE_PERIOD;
	
	//	PFC_DC_BUS_FAIL 
	GpioCtrlRegs.GPAMUX2.bit.GPIO23 = GPIO_FUNCTION; 				
   	GpioCtrlRegs.GPADIR.bit.GPIO23 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPAPUD.bit.GPIO23 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPAQSEL2.bit.GPIO23 = SIX_SAMPLES_FOR_INPUT_QUAL;	
	
	//	FAN_FAIL 
	GpioCtrlRegs.GPAMUX2.bit.GPIO22 = GPIO_FUNCTION; 				
   	GpioCtrlRegs.GPADIR.bit.GPIO22 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPAPUD.bit.GPIO22 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPAQSEL2.bit.GPIO22 = SIX_SAMPLES_FOR_INPUT_QUAL;	
	
	//	OVP_FAULT
	GpioCtrlRegs.GPBMUX2.bit.GPIO51 = GPIO_FUNCTION; 				
   	GpioCtrlRegs.GPBDIR.bit.GPIO51 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPBPUD.bit.GPIO51 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPBQSEL2.bit.GPIO51 = SIX_SAMPLES_FOR_INPUT_QUAL;	
   	
   	//	OTP_FAULT
	GpioCtrlRegs.GPBMUX2.bit.GPIO57 = GPIO_FUNCTION;
   	GpioCtrlRegs.GPBDIR.bit.GPIO57 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPBPUD.bit.GPIO57 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPBQSEL2.bit.GPIO57 = SIX_SAMPLES_FOR_INPUT_QUAL;
   	
	//	AC_FAIL
	GpioCtrlRegs.GPAMUX1.bit.GPIO11 = GPIO_FUNCTION; 				
   	GpioCtrlRegs.GPADIR.bit.GPIO11 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPAPUD.bit.GPIO11 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPAQSEL1.bit.GPIO11 = SIX_SAMPLES_FOR_INPUT_QUAL;	

	//	IN_LOCK
	GpioCtrlRegs.GPAMUX1.bit.GPIO12 = GPIO_FUNCTION; 			
   	GpioCtrlRegs.GPADIR.bit.GPIO12 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPAPUD.bit.GPIO12 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPAQSEL1.bit.GPIO12 = SIX_SAMPLES_FOR_INPUT_QUAL;	

	//	SO_DSP
	GpioCtrlRegs.GPAMUX1.bit.GPIO13 = GPIO_FUNCTION; 				
   	GpioCtrlRegs.GPADIR.bit.GPIO13 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPAPUD.bit.GPIO13 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPAQSEL1.bit.GPIO13 = SIX_SAMPLES_FOR_INPUT_QUAL;	
   	
	//	REM/LOC_STATUS_FP
	GpioCtrlRegs.GPAMUX2.bit.GPIO16 = GPIO_FUNCTION; 				
   	GpioCtrlRegs.GPADIR.bit.GPIO16 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPAPUD.bit.GPIO16 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPAQSEL2.bit.GPIO16 = SIX_SAMPLES_FOR_INPUT_QUAL;	

	//	COM2# EN
	GpioCtrlRegs.GPAMUX2.bit.GPIO26 = GPIO_FUNCTION; 				
   	GpioCtrlRegs.GPADIR.bit.GPIO26 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPAPUD.bit.GPIO26 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPAQSEL2.bit.GPIO26 = SIX_SAMPLES_FOR_INPUT_QUAL;	
   	
   	//	CC/CV
	GpioCtrlRegs.GPBMUX1.bit.GPIO41 = GPIO_FUNCTION; 				
   	GpioCtrlRegs.GPBDIR.bit.GPIO41 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPBPUD.bit.GPIO41 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPBQSEL1.bit.GPIO41 = SIX_SAMPLES_FOR_INPUT_QUAL;	
	
	//	DRDY of AD7705 
	GpioCtrlRegs.GPBMUX2.bit.GPIO50 = GPIO_FUNCTION; 				
   	GpioCtrlRegs.GPBDIR.bit.GPIO50 = INPUT_PIN;  					// Configure pin as input
   	GpioCtrlRegs.GPBPUD.bit.GPIO50 = PULLUP_DISABLE;  				// disable
   	GpioCtrlRegs.GPBQSEL2.bit.GPIO50 = SIX_SAMPLES_FOR_INPUT_QUAL;	
   	
   		// Output Gpios
//   All of them must be set to a known state depending on the application. Write to data register before configuring 
//	 direction. This assures the state of the pin after being configured as output. 

	//	ON_OFF_DSP
	GpioCtrlRegs.GPBMUX1.bit.GPIO39 = GPIO_FUNCTION; 	
   	GpioDataRegs.GPBCLEAR.bit.GPIO39 = SET_1_TO_CLEAR;	//Set low. 
   	GpioCtrlRegs.GPBDIR.bit.GPIO39 = OUTPUT_PIN;  		// Configure pin as output
   	GpioCtrlRegs.GPBPUD.bit.GPIO39 = PULLUP_DISABLE;  	// disable 

	//	O/P FAST DISCHARGE
	GpioCtrlRegs.GPBMUX2.bit.GPIO52 = GPIO_FUNCTION; 	
   	GpioDataRegs.GPBCLEAR.bit.GPIO52 = SET_1_TO_CLEAR;	//Put Reset low initially to clear all faults.
   	GpioCtrlRegs.GPBDIR.bit.GPIO52 = OUTPUT_PIN;  		// Configure pin as output
   	GpioCtrlRegs.GPBPUD.bit.GPIO52 = PULLUP_DISABLE;  	// disable
   	
	//	DSP RESET and AD7705 reset
	GpioCtrlRegs.GPAMUX2.bit.GPIO27 = GPIO_FUNCTION; 	
   	GpioDataRegs.GPACLEAR.bit.GPIO27 = SET_1_TO_CLEAR;	//Put Reset low initially to clear all faults.
   	GpioCtrlRegs.GPADIR.bit.GPIO27 = OUTPUT_PIN;  		// Configure pin as output
   	GpioCtrlRegs.GPAPUD.bit.GPIO27 = PULLUP_DISABLE;  	// disable 
   	
	//	TRANS_ENABLE
	GpioCtrlRegs.GPAMUX2.bit.GPIO17 = GPIO_FUNCTION; 	
   	GpioDataRegs.GPASET.bit.GPIO17 = SET_1_TO_CLEAR;		//Set Low.
   	GpioCtrlRegs.GPADIR.bit.GPIO17 = OUTPUT_PIN;  			// Configure pin as output
   	GpioCtrlRegs.GPAPUD.bit.GPIO17 = PULLUP_DISABLE;  		// disable 
   	
	//	PS_GOOD 
	GpioCtrlRegs.GPBMUX2.bit.GPIO53 = GPIO_FUNCTION; 	
   	GpioDataRegs.GPBCLEAR.bit.GPIO53 = SET_1_TO_CLEAR;	//Set low. 
   	GpioCtrlRegs.GPBDIR.bit.GPIO53 = OUTPUT_PIN;  		// Configure pin as output
   	GpioCtrlRegs.GPBPUD.bit.GPIO53 = PULLUP_DISABLE;  	// disable 
   	
	//	EXECUTION_TIME 
	GpioCtrlRegs.GPAMUX1.bit.GPIO14 = GPIO_FUNCTION; 	
   	GpioDataRegs.GPACLEAR.bit.GPIO14 = SET_1_TO_CLEAR;	//Set low. 
   	GpioCtrlRegs.GPADIR.bit.GPIO14 = OUTPUT_PIN;  		// Configure pin as output
   	GpioCtrlRegs.GPAPUD.bit.GPIO14 = PULLUP_DISABLE;  	// disable 
}

//#####################	Initialise AIO Pins ######################################################################
// Function Name: ei_vInitAioPins()
// Return Type: void
// Arguments:   void
// Description: Initialises AIO pins used for this project.
//				AIO pins are configured by default as ADC/COMP inputs.
// We will use one of the Pins(AIO2) as ADC INPUT. They are already configured by default to this mode
// but we will still expicitly configure them to avoid confusion. 
//#################################################################################################################
static void ei_vInitAioPins()
{
	// AIO2 configured as ADC/COMP input.
	// The only option given in MUX register is AIO enable/disable where enable means AIO mode and disable means 
	// ADC or COMP input.
	// An assumption made here is that a pin can be used both as an input to ADC and COPMARATOR simultaneously.
	// We will need to verify this later. Anyways COMPARATOR is not used in the project as of now.
	GpioCtrlRegs.AIOMUX1.bit.AIO2 = AIO_DISABLE;
}
