/*
 * oi_pll.c
 *
 *  Created on: Oct 19, 2020
 *      Author: enArka
 */

//
// Included Files
//
#include "F2837xS_device.h"
#include "F2837xS_Examples.h"
#include "F28x_Project.h"
#include "oi_globalVariables.h"
#include "oi_sinGeneration.h"

// Sine Generation

   //
   // Compute_Sine - Generate the input vector array
   //
   void Compute_Sine(float *inputVector, int16_t size)
{
    int16_t i;
    float step = (MAX_SINE_LIMIT - MIN_SINE_LIMIT) / size;

        inputVector[0] = MIN_SINE_LIMIT;
        for (i = 1; i < size; i++)
        {
            inputVector[i] = (inputVector[i - 1] + step);
        }

}


