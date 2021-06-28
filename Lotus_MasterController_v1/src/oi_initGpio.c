/*
 * oi_initGpio.c
 *
 *  Created on: Jan 15, 2021
 *      Author: enArka
 */

//
// Included Files
//
#include "F2837xS_device.h"
#include "F2837xS_Examples.h"
#include "oi_initGpio.h"
#include "oi_globalPrototypes.h"
#include "F28x_Project.h"


// This Function is to initialize GPIO's

void oi_initGpio()
{
    oi_initsystemGpio();                     // System GPIO Initialize
    oi_initEPwm1Gpio();                      // ADC Start of Conversion

}

// Initialize all system GPIO pins

void oi_initsystemGpio(void)
{
    EALLOW;
    // ISR Execution check LED
    GpioCtrlRegs.GPCGMUX2.bit.GPIO85 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO85 = 1;

    // ISR Execution check LED
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;

    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;

    GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;

    GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;

    GpioCtrlRegs.GPAGMUX1.bit.GPIO4 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;

     GpioCtrlRegs.GPCGMUX2.bit.GPIO86 = GPIO_FUNCTION;       // Configure as GPIO
     GpioCtrlRegs.GPCDIR.bit.GPIO86 = 1;

    EDIS;
}


//
// InitEPwm1Gpio - Initialize EPWM1 GPIOs
//
void oi_initEPwm1Gpio(void)
{
    EALLOW;

    //
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = DISABLE_PULLUP; // Disable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = DISABLE_PULLUP; // Disable pull-up on GPIO1 (EPWM1B)

    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = GPIO_FUNCTION; // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = GPIO_FUNCTION; // Configure GPIO1 as EPWM1B

    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = GPIO_FUNCTION; // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = GPIO_FUNCTION; // Configure GPIO1 as EPWM1B


    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = GPIO_FUNCTION; // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = GPIO_FUNCTION; // Configure GPIO1 as EPWM1B


    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = GPIO_FUNCTION; // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = GPIO_FUNCTION; // Configure GPIO1 as EPWM1B
    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = GPIO_FUNCTION; // Configure GPIO1 as EPWM1B


    //
    EDIS;
}






