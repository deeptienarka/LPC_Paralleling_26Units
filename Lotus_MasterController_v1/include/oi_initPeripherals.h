/*
 * oi_peripherals.h
 *
 *  Created on: Jan 15, 2021
 *      Author: enArka
 */

#ifndef INCLUDE_OI_INITPERIPHERALS_H_
#define INCLUDE_OI_INITPERIPHERALS_H_

// PWM defines

#define EPWM1_TIMER_TBPRD                               5000.0  // Period register


#define SOC_DISABLE                                     0     // ADC start of Conversion Disable
#define SOC_ENABLE                                      1     // ADC start of Conversion Enable
#define DIVIDE_BY_TWO                                   1     // PLLSYSCLK / 2
#define EPWM1_TRIGGER                                   5     // SOC triggered using ePWM1


#endif /* INCLUDE_OI_INITPERIPHERALS_H_ */
