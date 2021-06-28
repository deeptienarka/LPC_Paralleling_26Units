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
    PieVectTable.EPWM1_INT = &epwm1_isr; //function of ePWM interrupt
    EDIS;

    Interrupt_register(INT_CANA0, &canISR);

    //
    // Enable global Interrupts and higher priority real-time debug events:
    //
    IER |= M_INT3; //Enable group 3 interrupts


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
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;     // ePWM Interrupt

    //
    // Enable the CAN interrupt signal
    //
    Interrupt_enable(INT_CANA0);
    CAN_enableGlobalInterrupt(CANA_BASE, CAN_GLOBAL_INT_CANINT0);

    oi_CAN_init();


    // Initialization of Global variables

    oi_globalVar_init();

}


