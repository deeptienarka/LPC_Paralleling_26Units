//#################################################################################################################
//# 							enArka Instruments proprietary				  									  #
//# File: main.c													  									          #
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.					                                          #
//# All Rights Reserved												                                              #
//#                                                                                                               #
//#################################################################################################################

//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
#include "ExternalVariables.h"
//#################################################################################################################
							/*--------Global Prototypes---------*/
//void ei_vInitAdc();
//#################################################################################################################
							/*--------Local Prototypes---------*/
static void ei_vConfigureAdc();

// Local prototypes defined in this file
#define ADC_usDELAY  1000L

//#####################	Initialize ADC ############################################################################
// Function Name: ei_vInitAdc
// Return Type: void
// Arguments:   void
// Description: Initialises ADC circuits
//				PICCOLLO HAS TYPE3 ADC WHICH REPRESENTS A MAJOR CHANGE FROM 2808 TYPE1 ADC.
// Incase Offset setting of the ADC use the functions given in F2806x_Adc.c
//#################################################################################################################
void ei_vInitAdc()
{
    extern void DSP28x_usDelay(Uint32 Count);

    // Already done

    /*// *IMPORTANT*
    // The Device_cal function, which copies the ADC calibration values from TI reserved
    // OTP into the ADCREFSEL and ADCOFFTRIM registers, occurs automatically in the
    // Boot ROM. If the boot ROM code is bypassed during the debug process, the
    // following function MUST be called for the ADC to function according
    // to specification. The clocks to the ADC MUST be enabled before calling this
    // function.
    // See the device data manual and/or the ADC Reference
    // Manual for more information.

        EALLOW;
        SysCtrlRegs.PCLKCR0.bit.ADCENCLK = 1;
        (*Device_cal)();
        EDIS;*/

    // To powerup the ADC the ADCENCLK bit should be set first to enable
    // clocks, followed by powering up the bandgap, reference circuitry, and ADC core.
    // Before the first conversion is performed a 5ms delay must be observed
    // after power up to give all analog circuits time to power up and settle

    // Please note that for the delay function below to operate correctly the
    // CPU_RATE define statement in the F2806x_Examples.h file must
    // contain the correct CPU clock period in nanoseconds.
    EALLOW;
    AdcRegs.ADCCTL1.bit.ADCBGPWD  = 1;      // Power ADC BG
    AdcRegs.ADCCTL1.bit.ADCREFPWD = 1;      // Power reference
    AdcRegs.ADCCTL1.bit.ADCPWDN   = 1;      // Power ADC
    AdcRegs.ADCCTL1.bit.ADCENABLE = 1;      // Enable ADC
    AdcRegs.ADCCTL1.bit.ADCREFSEL = 0;      // Select interal BG
    EDIS;

    DELAY_US(ADC_usDELAY);         // Delay before converting ADC channels

    EALLOW;
    // Works at 45 MHz ADC CLOCK.
    // Note that the sampling window must be at least 116.66ns for efficient throughput
    
    // Clocking notes
//    CLKDIV2EN 	CLKDIV4EN 	ADCCLK
//		0 				0 		SYSCLK
//		0 				1 		SYSCLK
//		1 				0 		SYSCLK / 2
//		1 				1 		SYSCLK / 4
    AdcRegs.ADCCTL2.bit.CLKDIV2EN = ADC_CLOCK_DIVIDE_BY_2;
    EDIS;

    DELAY_US(ADC_usDELAY);         // Delay before converting ADC channels
    
    // Application specific configuration
    ei_vConfigureAdc();
}

//#####################	ADC Initialisation ########################################################################
// Function Name: ei_vConfigureAdc
// Return Type: void
// Arguments:   void
// Description: Application specific configuration of ADC peripheral
//
// IMPORTANT NOTE: THE 12 BIT ADC RESULTS ARE RIGHT JUSTIFIED UNLIKE 2808 which was left justified.
//#################################################################################################################
void ei_vConfigureAdc()
{
	// Some ADC registers are eallow protected.
	EALLOW;
	
		//ADCCTL1
	//The Powerup and Reference selection bits are already set in the proevious function.
	
	// The INT pulse will be generated 1 cycle before the results are latched to the result register.
	AdcRegs.ADCCTL1.bit.INTPULSEPOS = INTERRUPT_AT_END_OF_CONVERSION;
	AdcRegs.ADCCTL1.bit.VREFLOCONV = ENABLE_ADCINB5;
	AdcRegs.ADCCTL1.bit.TEMPCONV = ENABLE_ADCINA5;

		//ADCCTL2
	//Clock settings done in the previous function
	AdcRegs.ADCCTL2.bit.ADCNONOVERLAP = DISABLE_SAMPLE_AND_CONVERSION_OVERLAP;

		//ADCINTFLGCLR and ADCINTOVFCLR
	// Clear all interrupt flags as a safety measure. Write 1 to clear register.
	AdcRegs.ADCINTFLGCLR.all = ADC_FLAGS_CLEAR;
	AdcRegs.ADCINTOVFCLR.all = ADC_FLAGS_CLEAR;
	
		//ADCINTSEL1N2
	// ADCINT1 Interrupt enabled and it is bound to EOC of the last conversion in the set.
	// But no actual interrupts are serviced because the interrupt for this peripheral is notr enabled in PIE.
	// The ADCINTFLG for this particular is polled to denote EOC.
	AdcRegs.INTSEL1N2.bit.INT1CONT = NON_CONTINUOUS_INTERRUPT_MODE;		// No further pulses till ADCINTFLG cleared
	AdcRegs.INTSEL1N2.bit.INT1SEL = INT_SRC_EOC_6;						// EOC 6 selected as the source of interrupt
	AdcRegs.INTSEL1N2.bit.INT1E = ENABLE_ADCINT;

	AdcRegs.INTSEL1N2.bit.INT2CONT = NON_CONTINUOUS_INTERRUPT_MODE;		// No further pulses till ADCINTFLG cleared
	AdcRegs.INTSEL1N2.bit.INT2SEL = INT_SRC_EOC_5;						// EOC 5 selected as the source of interrupt
	AdcRegs.INTSEL1N2.bit.INT2E = ENABLE_ADCINT;

		// ADCSOC Registers.
	// We operate in Sequential sampling. ADCSAMPLEMODE left at default configuration for all SOCs
	// All SOCs generated by software only by writing 1 to appropriate bits in ADCSOCFRC1
	// Sample Window Size Calculation
	// Sample Window Size = (ACQPS+1)*(ADCCLK)
//						  = 7 * (1/45MHz)
//						  = 155.55ns
	// Conevrsion time for each channel = Sample window size + 13 ADCCLKs
//										= 155.55ns + 288.88ns
//										= 444.44ns
	// Total Approximate conversion time for 5 channels = 444.44ns*5
	//													= 2.222us
	
	// SOC0 settings.					  
	AdcRegs.ADCSOC0CTL.bit.TRIGSEL = SOFTWARE_TRIGGER;
	AdcRegs.ADCSOC0CTL.bit.CHSEL = ADCINA0;					// SOC0 is set to convert ADCINA0
	AdcRegs.ADCSOC0CTL.bit.ACQPS = SAMPLE_WINDOW_SIZE;
	
	// SOC1 settings.
	AdcRegs.ADCSOC1CTL.bit.TRIGSEL = SOFTWARE_TRIGGER;
	AdcRegs.ADCSOC1CTL.bit.CHSEL = ADCINA1;					// SOC1 is set to convert ADCINA1
	AdcRegs.ADCSOC1CTL.bit.ACQPS = SAMPLE_WINDOW_SIZE;
	
	// SOC2 settings.
	AdcRegs.ADCSOC2CTL.bit.TRIGSEL = SOFTWARE_TRIGGER;
	AdcRegs.ADCSOC2CTL.bit.CHSEL = ADCINA2;					// SOC2 is set to convert ADCINA2
	AdcRegs.ADCSOC2CTL.bit.ACQPS = SAMPLE_WINDOW_SIZE;
	
	// SOC3 settings.
	AdcRegs.ADCSOC3CTL.bit.TRIGSEL = SOFTWARE_TRIGGER;
	AdcRegs.ADCSOC3CTL.bit.CHSEL = ADCINA3;					// SOC3 is set to convert ADCINA3
	AdcRegs.ADCSOC3CTL.bit.ACQPS = SAMPLE_WINDOW_SIZE;
	
	// SOC4 settings.
	AdcRegs.ADCSOC4CTL.bit.TRIGSEL = SOFTWARE_TRIGGER;
	AdcRegs.ADCSOC4CTL.bit.CHSEL = ADCINA5;					// SOC4 is set to convert ADCINA5
	AdcRegs.ADCSOC4CTL.bit.ACQPS = SAMPLE_WINDOW_SIZE;
	
	// SOC5 settings.
	AdcRegs.ADCSOC5CTL.bit.TRIGSEL = SOFTWARE_TRIGGER;
	AdcRegs.ADCSOC5CTL.bit.CHSEL = ADCINA7;					// SOC5 is set to convert ADCINA7
	AdcRegs.ADCSOC5CTL.bit.ACQPS = SAMPLE_WINDOW_SIZE;
	
	// SOC6 settings.
	AdcRegs.ADCSOC6CTL.bit.TRIGSEL = SOFTWARE_TRIGGER;
	AdcRegs.ADCSOC6CTL.bit.CHSEL = ADCINA4;					// SOC6 is set to convert ADCINA4
	AdcRegs.ADCSOC6CTL.bit.ACQPS = SAMPLE_WINDOW_SIZE;
	
	// Unused registers
	// Remaining ADCINTSELxNy registers.
	// SOCPRICTL
	// ADCSAMPLEMODE
	// ADCINTSOCSEL1
	// ADCINTSOCSEL2
	// ADCSOC5-15CTL
	EDIS;
}
