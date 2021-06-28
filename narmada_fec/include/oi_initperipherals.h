/*
 * oi_peripherals.h
 *
 *  Created on: Oct 9, 2020
 *      Author: enArka
 */

#ifndef INCLUDE_OI_INITPERIPHERALS_H_
#define INCLUDE_OI_INITPERIPHERALS_H_


// PWM defines

#define EPWM1_TIMER_TBPRD                               1953.0  // Period register
#define EPWM2_TIMER_TBPRD                               976.0   // Period register
#define EPWM6_TIMER_TBPRD                               1953.0  // Period register
#define EPWM7_TIMER_TBPRD                               1953.0   // Period register
#define EPWM8_TIMER_TBPRD                               1953.0   // Period register

#define SOC_DISABLE                                     0     // ADC start of Conversion Disable
#define SOC_ENABLE                                      1     // ADC start of Conversion Enable
#define DIVIDE_BY_TWO                                   1     // PLLSYSCLK / 2
#define EPWM1_TRIGGER                                   5     // SOC triggered using ePWM1
// ADC defines

//
// Defines ADC
//

#define ADCINA0                                         0
#define ADCINA1                                         1
#define ADCINA2                                         2
#define ADCINA3                                         3
#define ADCINA4                                         4
#define ADCINA5                                         5
#define ADCINA6                                         6
#define ADCINA7                                         7
#define ADCIN14                                        14
#define ADCIN15                                        15

#define ADCINB0                                         0
#define ADCINB1                                         1
#define ADCINB2                                         2
#define ADCINB3                                         3
#define ADCINB4                                         4
#define ADCINB5                                         5
#define ADCINB6                                         6
#define ADCINB7                                         7


/** ADC Channels *********************************************************************************/
#define NO_OF_ADC_CHANNELS_MEASURED                  16
#define NO_OF_AC_CHANNELS                            16



#define  HS2_TEMP_MEAS_DSP                            0   // temperature
#define  HS1_TEMP_MEAS_DSP                            1   // temperature
#define  POS_BUS_DSP                                  2   // DC
#define  NEG_BUS_DSP                                  3   // DC
#define  IR_SINE                                      4   // AC
#define  IY_SINE                                      5   // AC
#define  IB_SINE                                      6   // AC
#define  TEMP_MEAS                                    7   // temperature
#define  IOUT_DC_SENSE_DSP                            8   // DC
#define  VR_PHASE_SENSE_DSP                           9   // AC
#define  VY_PHASE_SENSE_DSP                           10  // AC
#define  VB_PHASE_SENSE_DSP                           11  // AC





#endif /* INCLUDE_OI_INITPERIPHERALS_H_ */
