/*
 * oi_globalPrototypes.h
 *
 *  Created on: Sep 29, 2020
 *      Author: enArka
 */

#ifndef INCLUDE_OI_GLOBALPROTOTYPES_H_
#define INCLUDE_OI_GLOBALPROTOTYPES_H_

#include <oi_fec_control.h>

// System Initialization

extern void oi_vSystemInit(void);
extern void oi_initPeripherals(void);
extern void oi_globalVar_init(void);
extern void oi_vInitState(void);
extern void StateInit(fec_state_t *conv_state);

// Temperature Sense

void oi_tempSenseEvent(void);

// Sine Generation Function Initialization

void Compute_Sine(float *inputVector, int16_t size);


// PWM functions

extern void oi_initEPwm1(void);
extern void oi_initEPwm2(void);
extern void oi_initEPwm6(void);
extern void oi_initEPwm7(void);
extern void oi_initEPwm8(void);

// GPIO Initialization
extern void oi_initGpio(void);
// System GPIO Initialization
extern void oi_initsystemGpio(void);
//GPIO initialization to the PWMs
extern void oi_initEPwm1Gpio(void);
extern void oi_initEPwm2Gpio(void);
extern void oi_initEPwm6Gpio(void);
extern void oi_initEPwm7Gpio(void);
extern void oi_initEPwm8Gpio(void);
extern void oi_sciInitGpio(void);


// DAC Initialization

extern void oi_runDAC(void);

// SCI Initialization and Event
extern void oi_sciEvent(void);
extern void oi_sciInit(void);

// Interrupt Service routine

extern interrupt void adca1_isr(void);
extern interrupt void epwm2_isr(void);
extern interrupt void ECap1FallingEdgeIsr(void);

// PLL
extern void P_LL(fec_state_t *conv_state);
extern void PllInit(pll_t *spll);
extern void Pll(pll_t *spll);

// Scheduler events

extern Uint16 oi_Event1();
extern Uint16 oi_Event2();
extern Uint16 oi_Event3();
extern Uint16 oi_Event4();
extern Uint16 oi_Event5();
extern Uint16 oi_Event6();

// ISR functions

extern void value_transfer(void);
extern void oi_runDAC(void);
extern void P_LL(fec_state_t *conv_state);
extern void oi_Protection_Module(void);


// SCI functions
extern void oi_sciEvent(void);
extern void oi_sciInit(void);
extern void oi_sciInitGpio(void);
extern int intToStr1(int x, unsigned char str[], int d);
extern void reverse(unsigned char *str, int len);


#endif /* INCLUDE_OI_GLOBALPROTOTYPES_H_ */
