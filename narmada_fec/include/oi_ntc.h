/*
 * oi_ntc.h
 *
 *  Created on: 28-Jan-2021
 *      Author: niles
 */

#ifndef INCLUDE_OI_NTC_H_
#define INCLUDE_OI_NTC_H_

#define NTC_COEFF_X6   0.646777
#define NTC_COEFF_X5   3.44922
#define NTC_COEFF_X4   6.262695
#define NTC_COEFF_X3   5.5078125
#define NTC_COEFF_X2   2.598633
#define NTC_COEFF_X1   0.745996
#define NTC_COEFF_X0   0.16006

typedef struct
{
    float ntcA;
    float ntcB;
    uint16_t counter;
} temperature;


#endif /* INCLUDE_OI_NTC_H_ */
