/*
 * oi_tempSense.c
 *
 *  Created on: 28-Jan-2021
 *      Author: niles
 */

#include "F2837xs_device.h"
#include "F2837xS_Examples.h"
#include "F28x_Project.h"
#include "stdio.h"
#include "oi_externVariables.h"
#include "oi_globalPrototypes.h"
#include "oi_genStructureDefs.h"
#include "oi_ntc.h"

void oi_tempSenseEvent(void);
void oi_ntcSense1(void);
void oi_ntcSense2(void);

temperature Temperature;

float X1,X2,X3,X4,X5,X6;

void oi_tempSenseEvent()
{
    Temperature.counter++;

    if(Temperature.counter == 1)
    {
        oi_ntcSense1();
    }
    else if(Temperature.counter == 2)
    {
        oi_ntcSense2();
        Temperature.counter = 0;
    }
}

void oi_ntcSense1()
{
    X1 = (fec_state.adc_buffer[TEMP_MEAS].adc_result_regs)
            *(0.000244)*(0.909);
    X2 = X1*X1;
    X3 = X2*X1;
    X4 = X3*X1;
    X5 = X4*X1;
    X6 = X5*X1;

    Temperature.ntcA = (NTC_COEFF_X6 * X6) - (NTC_COEFF_X5 * X5)
                        + (NTC_COEFF_X4 * X4) - (NTC_COEFF_X3 * X3)
                        + (NTC_COEFF_X2 * X2) - (NTC_COEFF_X1 * X1)
                        + (NTC_COEFF_X0);

    Temperature.ntcA = Temperature.ntcA * 1024.0;
}

void oi_ntcSense2()
{
    X1 = (fec_state.adc_buffer[HS2_TEMP_MEAS_DSP].adc_result_regs)
            *(0.000244)*(0.909);
    X2 = X1*X1;
    X3 = X2*X1;
    X4 = X3*X1;
    X5 = X4*X1;
    X6 = X5*X1;

    Temperature.ntcB = (NTC_COEFF_X6 * X6) - (NTC_COEFF_X5 * X5)
                        + (NTC_COEFF_X4 * X4) - (NTC_COEFF_X3 * X3)
                        + (NTC_COEFF_X2 * X2) - (NTC_COEFF_X1 * X1)
                        + (NTC_COEFF_X0);

    Temperature.ntcB = Temperature.ntcB * 1024.0;
}
