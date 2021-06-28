/*
 * oi_peripherals.c
 *
 *  Created on: Oct 9, 2020
 *      Author: enArka
 */

#include "F2837xS_device.h"
#include "F2837xS_Examples.h"
#include "oi_initPeripherals.h"
#include "oi_globalPrototypes.h"
#include "oi_genStructureDefs.h"
#include "F2837xS_Examples.h"
#include "device.h"
#include "driverlib.h"

//
// Initialize EPWM1 configuration - ADC Start of Conversion
//
void oi_initEPwm()
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;             // This bit reset to 0 stops all PWM clocks.
    oi_initEPwm1();                                   // Initialization of EPWMs
    oi_initEPwm2();                                   // Initialization of EPWMs
    oi_initEPwm6();                                   // Initialization of EPWMs
    oi_initEPwm7();                                   // Initialization of EPWMs
    oi_initEPwm8();                                   // Initialization of EPWMs

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;             // This bit set to 1 turns ON all pwm clocks at the same time
    EDIS;
}

void oi_initEPwm1()
{    //
// Setup TBCLK
//
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;     // Count up/down and down
    EPwm1Regs.TBPRD = EPWM1_TIMER_TBPRD;               // Set timer period
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;            // Disable phase loading
    EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;                // Phase is 0
    EPwm1Regs.TBCTR = 0x0000;                          // Clear counter
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;           // Clock ratio to SYSCLKOUT
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;

    //
    // Setup shadow register load on ZERO
    //

    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    EPwm1Regs.CMPA.bit.CMPA = 0;

    //
    // Set Actions
    //

    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;                  // Set PWM1A on period
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;                    // Clear PWM1A on event B, down
                                                          // count
    EPwm1Regs.AQCTLB.bit.CBU = AQ_CLEAR;                  // Clear PWM1A on period
    EPwm1Regs.AQCTLB.bit.CBD = AQ_SET;                    // Set PWM1A on event A, up

    // Assumes ePWM clock is already enabled

    EPwm1Regs.ETSEL.bit.SOCAEN = SOC_DISABLE;             // Disable SOC on A group
    EPwm1Regs.ETSEL.bit.SOCASEL = ET_CTR_PRDZERO;         // Select SOC on period-match or zero
    EPwm1Regs.ETSEL.bit.SOCBSEL = ET_CTR_PRDZERO;         // Select SOC on period-match or zero
    EPwm1Regs.ETPS.bit.SOCAPRD = ET_1ST;                  // Generate pulse on 1st event
    EPwm1Regs.ETSEL.bit.SOCAEN = SOC_ENABLE;                      // enable SOCA

}

//
// InitEPwm2Example - Initialize EPWM2 configuration - ADC Start of Conversion
//
void oi_initEPwm2()
{
    //
    // Setup TBCLK
    //
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up/down and down
    EPwm2Regs.TBPRD = EPWM2_TIMER_TBPRD;           // Set timer period
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;        // Disable phase loading
    EPwm2Regs.TBPHS.bit.TBPHS = 0x0000;            // Phase is 0
    EPwm2Regs.TBCTR = 0x0000;                      // Clear counter
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;       // Clock ratio to SYSCLKOUT
    EPwm2Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    //
    // Setup shadow register load on ZERO
    //
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    //
    // Set Actions
    //
    EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;             // Set PWM2A on period
    EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;               // Clear PWM2A on event B, down
                                                     // count

    EPwm2Regs.AQCTLB.bit.CBU = AQ_CLEAR;             // Clear PWM2A on period
    EPwm2Regs.AQCTLB.bit.CBD = AQ_SET;               // Set PWM2A on event A, up

    EPwm2Regs.CMPA.bit.CMPA = 0;

    // Interrupt where we will change the Compare Values
    //
    EPwm2Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;         // Select INT on Zero event
    EPwm2Regs.ETSEL.bit.INTEN = 1;                   // Enable INT
    EPwm2Regs.ETPS.bit.INTPRD = ET_1ST;              // Generate INT on 3rd event

}

//************ R Phase PWM Switching pulses***************

void oi_initEPwm6()
{    //
// Setup TBCLK
//
    EPwm6Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      // Count up/down and down
    EPwm6Regs.TBPRD = EPWM6_TIMER_TBPRD;                // Set switching period
    EPwm6Regs.TBCTL.bit.PHSEN = TB_DISABLE;             // Disable phase loading
    EPwm6Regs.TBPHS.bit.TBPHS = 0x0000;                 // Phase is 0
    EPwm6Regs.TBCTR = 0x0000;                           // Clear counter
    EPwm6Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;            // Clock ratio to SYSCLKOUT
    EPwm6Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm6Regs.TBCTL.bit.PRDLD = TB_SHADOW;

    //
    // Setup shadow register load on ZERO
    //

    EPwm6Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm6Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm6Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm6Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    EPwm6Regs.CMPA.bit.CMPA = EPWM6_TIMER_TBPRD;
    EPwm6Regs.CMPB.bit.CMPB = EPWM6_TIMER_TBPRD;
    //
    // Set Actions
    //

    EPwm6Regs.AQCTLA.bit.CAU = AQ_SET;              // Set R phase PWM on period
    EPwm6Regs.AQCTLA.bit.CAD = AQ_CLEAR;            // Clear PWM1A on event B, down
}

//************ Y Phase PWM Switching pulses***************

void oi_initEPwm7()
{    //
// Setup TBCLK
//
    EPwm7Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      // Count up/down and down
    EPwm7Regs.TBPRD = EPWM7_TIMER_TBPRD;                // Set timer period
    EPwm7Regs.TBCTL.bit.PHSEN = TB_DISABLE;             // Disable phase loading
    EPwm7Regs.TBPHS.bit.TBPHS = 0x0000;                         // Phase is 0
    EPwm7Regs.TBCTR = 0x0000;                           // Clear counter
    EPwm7Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;            // Clock ratio to SYSCLKOUT
    EPwm7Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm7Regs.TBCTL.bit.PRDLD = TB_SHADOW;

    //
    // Setup shadow register load on ZERO
    //

    EPwm7Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm7Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm7Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm7Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    EPwm7Regs.CMPA.bit.CMPA = EPWM7_TIMER_TBPRD;
    EPwm7Regs.CMPB.bit.CMPB = EPWM7_TIMER_TBPRD;
    //
    // Set Actions
    //

    EPwm7Regs.AQCTLA.bit.CAU = AQ_SET;                    // Set PWM1A on period
    EPwm7Regs.AQCTLA.bit.CAD = AQ_CLEAR;                  // Clear PWM1A on event B, down


}

void oi_initEPwm8()
{    //
// Setup TBCLK
//
    EPwm8Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;     // Count up/down and down
    EPwm8Regs.TBPRD = EPWM8_TIMER_TBPRD;               // Set timer period
    EPwm8Regs.TBCTL.bit.PHSEN = TB_DISABLE;            // Disable phase loading
    EPwm8Regs.TBPHS.bit.TBPHS = 0x0000;                // Phase is 0
    EPwm8Regs.TBCTR = 0x0000;                          // Clear counter
    EPwm8Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;           // Clock ratio to SYSCLKOUT
    EPwm8Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm8Regs.TBCTL.bit.PRDLD = TB_SHADOW;

    //
    // Setup shadow register load on ZERO
    //

    EPwm8Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm8Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm8Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm8Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    EPwm8Regs.CMPA.bit.CMPA = EPWM8_TIMER_TBPRD;

    //
    // Set Actions
    //

    EPwm8Regs.AQCTLA.bit.CAU = AQ_SET;                    // Set PWM1A on period
    EPwm8Regs.AQCTLA.bit.CAD = AQ_CLEAR;                  // Clear PWM1A on event B, down
    // count
    EPwm8Regs.AQCTLB.bit.CBU = AQ_CLEAR;                  // Clear PWM1A on period
    EPwm8Regs.AQCTLB.bit.CBD = AQ_SET;                    // Set PWM1A on event A, up

}

//Sequence of Channel conversion should be given in a proper order for both A and B module
//ACQPS is 12 bit - 75ns, Trigger source for ADC conversion is EPWM2

void ConfigureADC(void)
{
    EALLOW;

    //
    //write configurations
    //
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4

    AdcbRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4

    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

    //
    //Set pulse positions to late
    //

    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    //
    //power up the ADC
    //
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;

    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;

    //
    //delay for 1ms to allow ADC time to power up
    //
    DELAY_US(1000);

    EDIS;
}

//
// SetupADCEpwm - Setup ADC EPWM acquisition window
//
void InitAdc()
{
    Uint16 acqps;

    //
    // Determine minimum acquisition window (in SYSCLKS) based on resolution
    //
    if ((ADC_RESOLUTION_12BIT == AdcaRegs.ADCCTL2.bit.RESOLUTION)
            || (ADC_RESOLUTION_12BIT == AdcbRegs.ADCCTL2.bit.RESOLUTION))
    {
        acqps = 14; //75ns
    }
    else //resolution is 16-bit
    {
        acqps = 63; //320ns
    }

    //
    //Select the channels to convert and end of conversion flag
    //
    EALLOW;

    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0; //end of SOC0 of Channel A will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;        //enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      //make sure INT1 flag is cleared

    AdcaRegs.ADCSOC0CTL.bit.CHSEL = ADCIN14;  //SOC0 will convert pin A0
    AdcaRegs.ADCSOC1CTL.bit.CHSEL = ADCIN15;  //SOC1 will convert pin A1
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = ADCINA2;  //SOC2 will convert pin A2
    AdcaRegs.ADCSOC3CTL.bit.CHSEL = ADCINA3;  //SOC3 will convert pin A3
    AdcaRegs.ADCSOC4CTL.bit.CHSEL = ADCINA4;  //SOC4 will convert pin A4
    AdcaRegs.ADCSOC5CTL.bit.CHSEL = ADCINA5;  //SOC5 will convert pin A5

    AdcbRegs.ADCSOC0CTL.bit.CHSEL = ADCINB1;  //SOC8 will convert pin B0
    AdcbRegs.ADCSOC1CTL.bit.CHSEL = ADCINB0;  //SOC9 will convert pin B1
    AdcbRegs.ADCSOC2CTL.bit.CHSEL = ADCINB2;  //SOC10 will convert pin B2
    AdcbRegs.ADCSOC3CTL.bit.CHSEL = ADCINB3;  //SOC11 will convert pin B3
    AdcbRegs.ADCSOC4CTL.bit.CHSEL = ADCINB4;  //SOC12 will convert pin B4
    AdcbRegs.ADCSOC5CTL.bit.CHSEL = ADCINB5;  //SOC13 will convert pin B5

    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcaRegs.ADCSOC5CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcaRegs.ADCSOC14CTL.bit.TRIGSEL = EPWM1_TRIGGER;  //trigger on ePWM1 SOCA/C
    AdcaRegs.ADCSOC15CTL.bit.TRIGSEL = EPWM1_TRIGGER;  //trigger on ePWM1 SOCA/C

    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcbRegs.ADCSOC3CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcbRegs.ADCSOC4CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C
    AdcbRegs.ADCSOC5CTL.bit.TRIGSEL = EPWM1_TRIGGER;   //trigger on ePWM1 SOCA/C

    AdcaRegs.ADCSOC0CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcaRegs.ADCSOC3CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcaRegs.ADCSOC4CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcaRegs.ADCSOC5CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcaRegs.ADCSOC14CTL.bit.ACQPS = acqps; //sample window is 100 SYSCLK cycles
    AdcaRegs.ADCSOC15CTL.bit.ACQPS = acqps; //sample window is 100 SYSCLK cycles

    AdcbRegs.ADCSOC0CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcbRegs.ADCSOC1CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcbRegs.ADCSOC2CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcbRegs.ADCSOC3CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcbRegs.ADCSOC4CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles
    AdcbRegs.ADCSOC5CTL.bit.ACQPS = acqps;  //sample window is 100 SYSCLK cycles

    EDIS;

}

void ConfigureDAC()
{
    EALLOW;
    DacaRegs.DACCTL.bit.DACREFSEL = 1;          // Using Internal reference
    DacaRegs.DACCTL.bit.LOADMODE = 0;           // Sync with ePWM Module.
    DacaRegs.DACOUTEN.bit.DACOUTEN = 1;         // Enable DACA Output
    DacaRegs.DACVALS.bit.DACVALS = 0;           // Set Shadow Output to 0.
    DELAY_US(15);

    DacbRegs.DACCTL.bit.DACREFSEL = 1;          // Using Internal reference
    DacbRegs.DACCTL.bit.LOADMODE = 0;           // Sync with ePWM Module.
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;         // Enable DACB Output
    DacbRegs.DACVALS.bit.DACVALS = 0;           // Set Shadow Output to 0.
    DELAY_US(15);
    EDIS;
}

/**
 * @brief Initialise ECAP1 to measure Capture Counts to get line frequency
 */
void InitECap1(void)
{

    ECap1Regs.ECEINT.all = 0x0000;          // Disable all capture __interrupts
    ECap1Regs.ECCLR.all = 0xFFFF;           // Clear all CAP __interrupt flags
    ECap1Regs.ECCTL1.bit.CAPLDEN = 0;       // Disable CAP1-CAP4 register loads
    ECap1Regs.ECCTL2.bit.TSCTRSTOP = 0;     // Make sure the counter is stopped

    //
    // Configure peripheral registers
    //
    ECap1Regs.ECCTL2.bit.CONT_ONESHT = 0;   // One-shot
    ECap1Regs.ECCTL2.bit.STOP_WRAP = 1;     // Stop at 2 events
    ECap1Regs.ECCTL1.bit.CAP1POL = 0;       // Rising edge
    ECap1Regs.ECCTL1.bit.CAP2POL = 1;       // Falling edge
    ECap1Regs.ECCTL1.bit.CTRRST1 = 1;       // Reset counter after latch
    ECap1Regs.ECCTL1.bit.CTRRST2 = 0;       // Do not reset counter after
    ECap1Regs.ECCTL2.bit.SYNCI_EN = 0;      // Disable sync in
    ECap1Regs.ECCTL2.bit.SYNCO_SEL = 0;     // Pass through
    ECap1Regs.ECCTL1.bit.CAPLDEN = 1;       // Enable capture units

    ECap1Regs.ECCTL2.bit.TSCTRSTOP = 1;     // Start Counter
    ECap1Regs.ECCTL2.bit.REARM = 1;         // arm one-shot
    ECap1Regs.ECCTL1.bit.CAPLDEN = 1;       // Enable CAP1-CAP4 register loads
    ECap1Regs.ECEINT.bit.CEVT1 = 1;         // 4 events = __interrupt

}

void oi_sciInit(void)
{
    EALLOW;

    SCI_performSoftwareReset(SCIB_BASE);

    SCI_setConfig(SCIB_BASE, DEVICE_LSPCLK_FREQ, 9600, (SCI_CONFIG_WLEN_8 |
    SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE));
    SCI_disableLoopback(SCIB_BASE);

    SCI_resetChannels(SCIB_BASE);
    SCI_resetRxFIFO(SCIB_BASE);
    SCI_resetTxFIFO(SCIB_BASE);
    SCI_clearInterruptStatus(SCIB_BASE, SCI_INT_TXFF | SCI_INT_RXFF);
    SCI_enableFIFO(SCIB_BASE);
    SCI_enableModule(SCIB_BASE);
    SCI_performSoftwareReset(SCIB_BASE);

    EDIS;
}

void oi_initPeripherals()
{
    EALLOW;
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = DIVIDE_BY_TWO; // Divide by 2 of PLLSYSCLK
    EDIS;
    oi_initGpio();                                          // GPIO initialization to the PWMs
    oi_initEPwm();                                          // Initialization of EPWMs
    ConfigureADC();                                         // ADC configuration
    InitAdc();
    ConfigureDAC();                                         // DAC configuration
    InitECap1Gpio(58);

    EALLOW;
    GpioCtrlRegs.GPBCTRL.bit.QUALPRD3 = 0xFF;
    GpioCtrlRegs.GPBCSEL4.bit.GPIO58 = 6;
    EDIS;
    InitECap1();
    oi_sciInit();

}
