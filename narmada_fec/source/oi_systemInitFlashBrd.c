/*
 * oi_systemInitFlashBrd.c
 *
 *
 */


//
// Included Files
//
#include "F2837xS_device.h"
#include "F2837xS_Examples.h"
#include "F28x_Project.h"
#include "stdio.h"
#include "oi_externVariables.h"
#include "oi_globalPrototypes.h"

void oi_vSystemInit(void)
{

    //
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the F2837xS_SysCtrl.c file.
    //
    InitSysCtrl();

    //
    // Step 2. Initialize GPIO:
    // This example function is found in the F2837xS_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    // InitGpio();

    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    //
    DINT;

    //
    // Initialize the PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    // This function is found in the F2837xS_PieCtrl.c file.
    //
    InitPieCtrl();

    //
    // Disable CPU interrupts and clear all CPU interrupt flags:
    //
    IER = 0x0000;
    IFR = 0x0000;

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in F2837xS_DefaultIsr.c.
    // This function is found in F2837xS_PieVect.c.
    //
    InitPieVectTable();

    // Step 6: Initialize Peripherals

    oi_initPeripherals();

    // Step 7: Initialize global variables

    //InitGlobVars();

    //
    // Map ISR functions
    //
    EALLOW;
    PieVectTable.ADCA1_INT = &adca1_isr; //function for ADCA interrupt 1
    PieVectTable.EPWM2_INT = &epwm2_isr; //function of ePWM interrupt
    PieVectTable.ECAP1_INT = &ECap1FallingEdgeIsr; //Capture Interrupt

    EDIS;

    //
    // Enable global Interrupts and higher priority real-time debug events:
    //
    IER |= M_INT1; //Enable group-1 interrupts
    IER |= M_INT3; //Enable group 3 interrupts
    IER |= M_INT4; //Enable group 4 interrupts



    //
    // Enable global Interrupts and higher priority real-time debug events:
    //
    EINT;
    // Enable Global interrupt INTM
    ERTM;


    // Enable Global real-time interrupt DBGM

    //
    // enable PIE interrupt
    //
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;     // ADC Interrupt
    PieCtrlRegs.PIEIER3.bit.INTx2 = 1;     // ePWM Interrupt
    PieCtrlRegs.PIEIER4.bit.INTx1 = 1;     // eCAPTURE Interrupt

    // Initialization of Global variables

    oi_globalVar_init();

    // Sine Vector Generation

    Compute_Sine(inputVector, NO_OF_SAMPLES_IN_ONE_CYCLE);


}


