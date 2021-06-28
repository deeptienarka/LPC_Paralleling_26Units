/*
 * oi_initGPIO.c
 *
 *  Created on: Oct 9, 2020
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
    oi_initEPwm2Gpio();                      // ePWM interrupt generation
    oi_initEPwm6Gpio();                      // ePWM6 R-Phase
    oi_initEPwm7Gpio();                      // ePWM7 Y-Phase
    oi_initEPwm8Gpio();                      // ePWM8 B-Phase
    oi_sciInitGpio();                        // SCI GPIO Initialization

}

// Initialize all system GPIO pins

void oi_initsystemGpio(void)
{
    EALLOW;

    //Latch Reset GPIO
    GpioCtrlRegs.GPAMUX2.bit.GPIO16 = GPIO_FUNCTION;    // Configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO16 = 1;                 // GPIO direction as output
    GpioDataRegs.GPACLEAR.bit.GPIO16 = 1;               // Initialise to '0'

    // Relay Control GPIO   HIGH - RELAY-ON , LOW - RELAY - OFF
    GpioCtrlRegs.GPAGMUX1.bit.GPIO4 = GPIO_FUNCTION;    // Configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1;                  // GPIO direction as output
    GpioDataRegs.GPACLEAR.bit.GPIO4 = 1;                // Initialise to '0'

    // Gate Driver control GPIO     HIGH - PULSE-OFF , LOW - PULSE - ON
    GpioCtrlRegs.GPAGMUX2.bit.GPIO17 = GPIO_FUNCTION;    // Configure as GPIO
    GpioCtrlRegs.GPADIR.bit.GPIO17 = 1;                  // GPIO direction as output
    GpioDataRegs.GPASET.bit.GPIO17 = 1;                  // Initialise to '1'

    // DSP Beat
    GpioCtrlRegs.GPBGMUX2.bit.GPIO59 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPBDIR.bit.GPIO59 = 1;                     // GPIO direction as output
//    GpioDataRegs.GPBCLEAR.bit.GPIO59 = 1;                   // Initialise to '0'

    // ISR Execution check LED
    GpioCtrlRegs.GPCGMUX2.bit.GPIO85 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO85 = 1;                     // GPIO direction as output
//    GpioDataRegs.GPBCLEAR.bit.GPIO59 = 1;                   // Initialise to '0'

    // ISR Execution check LED
    GpioCtrlRegs.GPCGMUX2.bit.GPIO86 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO86 = 1;                     // GPIO direction as output
//    GpioDataRegs.GPBCLEAR.bit.GPIO59 = 1;                   // Initialize to '0'

    // SCI ENABLE LED
    GpioCtrlRegs.GPCGMUX1.bit.GPIO69 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO69 = 1;                     // GPIO direction as output
    GpioDataRegs.GPCSET.bit.GPIO69 = 1;                   // Initialize to '1'

    // SCI ENABLE LED
    GpioCtrlRegs.GPCGMUX1.bit.GPIO69 = GPIO_FUNCTION;       // Configure as GPIO
    GpioCtrlRegs.GPCDIR.bit.GPIO69 = 1;                     // GPIO direction as output
    GpioDataRegs.GPCSET.bit.GPIO69 = 1;                   // Initialize to '1'


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
    EDIS;
}

//
// InitEPwm1Gpio - Initialize EPWM1 GPIOs
//
void oi_initEPwm2Gpio(void)
{
    EALLOW;

    //
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = DISABLE_PULLUP; // Disable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO3 = DISABLE_PULLUP; // Disable pull-up on GPIO1 (EPWM1B)

    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = GPIO_FUNCTION; // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = GPIO_FUNCTION; // Configure GPIO1 as EPWM1B
    //
    EDIS;
}

//
// InitEPwm6Gpio - Initialize EPWM6 GPIOs
//
void oi_initEPwm6Gpio(void)
{
    EALLOW;
    //
    // Disable internal pull-up for the selected output pins
    // for reduced power consumption
    // Pull-ups can be enabled or disabled by the user.
    // This will enable the pullups for the specified pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAPUD.bit.GPIO10 = 1;    // Disable pull-up on GPIO10 (EPWM6A)
    GpioCtrlRegs.GPAPUD.bit.GPIO11 = 1;    // Disable pull-up on GPIO11 (EPWM6B)
    // GpioCtrlRegs.GPEPUD.bit.GPIO155 = 1;    // Disable pull-up on GPIO155 (EPWM6A)
    // GpioCtrlRegs.GPEPUD.bit.GPIO156 = 1;    // Disable pull-up on GPIO156 (EPWM6B)

    //
    // Configure EPWM-6 pins using GPIO regs
    // This specifies which of the possible GPIO pins will be EPWM6 functional
    // pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 1;   // Configure GPIO10 as EPWM6A
    GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 1;   // Configure GPIO11 as EPWM6B
    // GpioCtrlRegs.GPEMUX2.bit.GPIO155 = 1;   // Configure GPIO155 as EPWM6A
    // GpioCtrlRegs.GPEMUX2.bit.GPIO156 = 1;   // Configure GPIO156 as EPWM6B

    EDIS;
}

//
// InitEPwm7Gpio - Initialize EPWM7 GPIOs
//
void oi_initEPwm7Gpio(void)
{
    EALLOW;

    //
    // Disable internal pull-up for the selected output pins
    // for reduced power consumption
    // Pull-ups can be enabled or disabled by the user.
    // This will enable the pullups for the specified pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAPUD.bit.GPIO12 = 1;    // Disable pull-up on GPIO12 (EPWM7A)
    GpioCtrlRegs.GPAPUD.bit.GPIO13 = 1;    // Disable pull-up on GPIO13 (EPWM7B)
    // GpioCtrlRegs.GPEPUD.bit.GPIO157 = 1;    // Disable pull-up on GPIO157 (EPWM7A)
    // GpioCtrlRegs.GPEPUD.bit.GPIO158 = 1;    // Disable pull-up on GPIO158 (EPWM7B)

    //
    // Configure EPWM-6 pins using GPIO regs
    // This specifies which of the possible GPIO pins will be EPWM6 functional
    // pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 1;   // Configure GPIO12 as EPWM7A
    GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 1;   // Configure GPIO13 as EPWM7B
    // GpioCtrlRegs.GPEMUX2.bit.GPIO157 = 1;   // Configure GPIO157 as EPWM7A
    // GpioCtrlRegs.GPEMUX2.bit.GPIO158 = 1;   // Configure GPIO158 as EPWM7B

    EDIS;
}

//
// InitEPwm8Gpio - Initialize EPWM8 GPIOs
//
void oi_initEPwm8Gpio(void)
{
    EALLOW;
    //
    // Disable internal pull-up for the selected output pins
    // for reduced power consumption
    // Pull-ups can be enabled or disabled by the user.
    // This will enable the pullups for the specified pins.
    // Comment out other unwanted lines.
    //
    GpioCtrlRegs.GPAPUD.bit.GPIO14 = 1;    // Disable pull-up on GPIO14 (EPWM8A)
    GpioCtrlRegs.GPAPUD.bit.GPIO15 = 1;    // Disable pull-up on GPIO15 (EPWM8B)
//  GpioCtrlRegs.GPEPUD.bit.GPIO159 = 1;    // Disable pull-up on GPIO159 (EPWM8A)
//  GpioCtrlRegs.GPFPUD.bit.GPIO160 = 1;    // Disable pull-up on GPIO160 (EPWM8B)

     //
     // Configure EPWM-6 pins using GPIO regs
     // This specifies which of the possible GPIO pins will be EPWM6 functional
     // pins.
     // Comment out other unwanted lines.
     //
    GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 1;   // Configure GPIO14 as EPWM8A
    GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 1;   // Configure GPIO15 as EPWM8B
    // GpioCtrlRegs.GPEMUX2.bit.GPIO159 = 1;   // Configure GPIO159 as EPWM8A
    // GpioCtrlRegs.GPFMUX1.bit.GPIO160 = 1;   // Configure GPIO160 as EPWM8B

    EDIS;
}



