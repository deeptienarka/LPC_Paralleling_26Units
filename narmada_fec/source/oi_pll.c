/*****************************************************************************************************************
*                             enArka India Private Limited proprietary                                           *
* File: ei_pll.c                                                                                                 *
* Copyright (c) 2019 by enArka Instruments Pvt. Ltd.                                                             *
* All Rights Reserved                                                                                            *
* Description: This file has functions that synchronize internally generated sine with the input voltage         *
*****************************************************************************************************************/

/*****************************************************************************************************************
* Include Files                                                                                                  *
*****************************************************************************************************************/

#include <oi_fec_control.h>
#include "F2837xS_Device.h"
#include "stdlib.h"
#include "IQmathLib.h"
#include "oi_externVariables.h"

/**
 * @brief Initialize ZXing parameters
 * @param pointer to ZX structure
 */
void PllInit(pll_t *spll) {
  spll->zx.freq_in_cap_counts = 0;
  spll->zx.phase_shift_index = 0;

  spll->pll_stat.capture_int_ack = 0;
  spll->pll_stat.capture_missing = 0;
  spll->pll_stat.freq_high = 0;
  spll->pll_stat.freq_low = 0;
  spll->pll_stat.pll_in_sync = 0;
  spll->pll_stat.reserved = 0;

  spll->sync_index = 0;

  spll->pll_timers.cap_int_missing = 0;
  spll->pll_timers.freq_out_of_range = 0;
  spll->pll_timers.pll_out_of_sync = 0;
  spll->freq_Measure = 0;
}


/**
 * @brief PLL event
 * @param pointer to pll structure
 * @return None
 *
 * # PLL Algorithm #
 * PLL Algorithm consists of 3 main parts-
 * 1. Zero crossing (Zx) interrupt
 * 2. Frequency sync
 * 3. Phase sync
 *
 * ## Zero Crossing (Zx) interrupt ##
 * A Zero crossing detector circuit generates the reference square wave from grid that our internally generated sine wave must
 * match in frequency as well as phase. The Zx circuit works in the following way:
 * - It generates a high to low for grid's positive zero crossing (0 degrees)
 * - It generates a low to high for grid's negative zero crossing (180 degrees)
 * We have set the capture module to trigger an interrupt on falling edge thus ensuring that we get an interrupt everytime
 * grid crosses 0 degrees.
 * In the interrupt we do the following 2 things:
 * - We get the period of previous cycle in capture counts. This is used for FREQUENCY_SYNC. Counts to frequency calculation
 * is given by the following formula:
 *  \f$ Grid_Frequency = (sysclk_freq_in_Hz) / (Capture_Counts) \f$
 * - We mark the phase difference of internally generated sine wrt reference wave. This is done by capturing the value of
 * theta_index at the ZX instant. Theta_index can vary from 0 to NO_OF_MINOR_CYCLES_PER_LINE_CYCLE-1 in one line cycle. Let us assume
 * a value of 512 for NO_OF_MINOR_CYCLES_PER_LINE_CYCLE. Thus theta_index varies from 0-511 in one line cycle. The following
 * table indicates phase releationship for different values of theta_index wrt to reference square wave.
 *
 * | Theta_Index @ | Phase      |
 * | ZX interrupt  |            |
 * |:-------------:|:----------:|
 * | 0             | Inphase    |
 * | 1-255         | Phase Lead |
 * | 257-511       | Phase Lag  |
 *
 * @ref phase_shift_index stores the value of theta_index at Zero Crossing
 *
 * ## PLL Event ##
 * PLL is a scheduled event that is executed once every line cycle. As mentioned earlier it includes -
 * FREQUENCY_SYNC and PHASE_SYNC. It also includes conditions to handle a few corner cases.
 *
 * ### FREQUENCY SYNC ###
 * Frequency syncing has only one step. Based on the capture_counts of the previous line cycle (determined from ZX interrupt),
 * we calculate what must be the next value written to Timer period so that 512 interrupts can be achieved in one line cycle. Let
 * us take an example with following values:
 * clk = 150MHz
 * line_freq_old = 60Hz
 * line_freq_new = 58Hz
 *
 * Let us say, we started with 60Hz  grid frequency. The cap_counts would be:
 *  \f$ cap_counts = clk/line_freq_old = 150e6/60 = 2500000 \f$
 *
 *  To get 512 interrupts in 60Hz, the 150Mhz up-counter timer must have a value of:
 *  \f$ tbprd = cap_counts/512 = ~4882 \f$
 *
 *  Now assume that the frquency changes from 60 to 58Hz. The new cap_counts would be:
 *  \f$ cap_counts = 150e6/58 = ~2586206 \f$
 *  To get 512 interrupts in 58Hz, we need to change the tbprd from 4882 to 5051 (=2586206/512).
 *
 *  That is all FREQUENCY_SYNC involves. To prevent abrupt frequency changes of our internal sine, we put a limit
 *  on maximum change in tbprd allowed in one cycle.
 *
 *  ### PHASE SYNC ###
 *  Phase syncing involves matching our internal sine wave to the reference signal in such a way that our theta_index
 *  is exactly 0 at ZX interrupt. As per the earlier table, we can determine whether we are leading or lagging wrt refer-
 *  ence wave. The phase correction action behaves according to following table:
 *
 *------------------------------------------------
 * | Theta_Index @ | Phase      | Corrective    |
 * | ZX interrupt  |            | Action        |
 * |:-------------:|:----------:|:-------------:|
 * | 0             | Inphase    | No action     |
 * | 1-255         | Phase Lead | Increase tbprd|
 * | 257-511       | Phase Lag  | Reduce tbprd  |
 *------------------------------------------------
 * As seen from the above table, if we are leading wrt reference signal, we sligtly reduce our frequency by increasing
 * timer period by tbprd_stepsize. If we are lagging, then we increase our frequency by reducing timer period by tbprd_stepsize.
 *
 * tbprd_stepsize is a variable that determines that by how many counts we need to go up or down to phase sync. A higher
 * value of stepsize allows faster syncing. We employ a higher stepsize when the phase shift between the reference signal
 * is greater than 5 (out of 512) counts. The table below determines tbprd_stepsize
 *
 *------------------------------------
 * | Phase shift   | tbprd_stepsize |
 * |               |                |
 * |:-------------:|:--------------:|
 * | 0             | 0              | In phase
 * | 1-5           | 1              | Mildly out of phase
 * | >5            | 4              | Greatly out of phase
 *------------------------------------
 *
 * ### Handling Corner cases ###
 *
 * In each of the below corner cases we move from whatever the current freqeuncy is, to nominal frequency (50 or 60Hz).
 * These cases describe various failure conditions for pll.
 *
 * #### Grid Missing ####
 * When the grid is missing, no capture interrupts will be generated. In such a case we will hold the previous timer period
 * for MAX_CAP_INT_MISSING_CYCLES_ALLOWED and then slew to nominal frequency
 *
 * #### Grid Frequency out of range ####
 * If the grid frequency is out of range (56 to 64 for 60Hz system and 46 to 54 for 50Hz system), then we will hold the
 * previous timer period for MAX_ABNORMAL_FREQ_CYCLES_ALLOWED and then slew to nominal frequency
 *
 */
int32 present_tprd, next_tprd, final_tprd, change, grid_bad_count=0;



void Pll(pll_t *spll)
{

    //int32 present_tprd, next_tprd, final_tprd, change, grid_bad_count=0;
    int32 new_capture;
    Uint16 tbprd_stepsize;
    Uint16 shift_index;
    pll_timers_t *tim;
    tim = &(spll->pll_timers);
    /* Both next_tprd and present_tprd start with the present Timer register value */
    next_tprd = present_tprd = GET_PRESENT_TIMER_PRD();

    shift_index = spll->zx.phase_shift_index;

//  if((rectifier_state.ac_channels[VR_PH_SENSE].norm_rms < 15) || (rectifier_state.ac_channels[VY_PH_SENSE].norm_rms < 15) || (rectifier_state.ac_channels[VB_PH_SENSE].norm_rms < 15))
//        {
//            INTERRUPT_ACK = 0;
//            Phase_Sequence.theta_index_initflag = 0;
//            Phase_Sequence.phase_Seq_change_indflag = 0;
//
//        }

    if (CapInterruptAck == 0)
    {
        if (tim->cap_int_missing >= MAX_CAP_INT_MISSING_CYCLES_ALLOWED)
            spll->pll_stat.capture_missing = 1;
        else
            tim->cap_int_missing++;
    }
    else
    {
        RESET_CAP_INT_ACK(spll->pll_stat);
        spll->pll_stat.capture_missing = 0; /* Reset Cap missing flag */
        tim->cap_int_missing = 0; /* Reset cap missing timer */

        new_capture = spll->zx.freq_in_cap_counts;

        FrequencyInHz = (float32)(SYSCLK_FREQUENCY_IN_HZ/new_capture);

        spll->freq_Measure = (float32) (SYSCLK_FREQUENCY_IN_HZ / new_capture);

        /* Check if Grid frequency within limits */

        if (GRID_FREQ_OVER())
        {
            if (tim->freq_out_of_range >= MAX_ABNORMAL_FREQ_CYCLES_ALLOWED)
            {
                spll->pll_stat.freq_high = 1;
            }
            else
            {
                tim->freq_out_of_range++;
            }
        }
        else if (GRID_FREQ_UNDER())
        {
            if (tim->freq_out_of_range >= MAX_ABNORMAL_FREQ_CYCLES_ALLOWED)
            {
                spll->pll_stat.freq_low = 1;
            }
            else
            {
                tim->freq_out_of_range++;
            }
        }
        else
        {
            /* Grid within frequency range. Watch for a period of time before declaring grid frequency as good */
            if (tim->freq_out_of_range == 0)
            {
                spll->pll_stat.freq_low = 0;
                spll->pll_stat.freq_high = 0;
            }
            else
            {
                tim->freq_out_of_range--;
            }
        }

        if (GRID_FREQ_GOOD(spll->pll_stat))
        { /* Execute pll algorithm */
            final_tprd = new_capture / (4 * NO_OF_MINOR_CYCLES_PER_LINE_CYCLE);

            /**
             * FREQUENCY SYNC
             * The change in tbprd allowed is restricted to MAX_INCREMENTAL_CHANGE_IN_COUNTS to
             * ensure a smooth transition to new frequency
             */

            change = _IQsat((final_tprd - present_tprd),
                            MAX_INCREMENTAL_CHANGE_IN_COUNTS,
                            -MAX_INCREMENTAL_CHANGE_IN_COUNTS);
            next_tprd = present_tprd + change;

            /** PHASE SYNCING **/
            /* Phase sync speed control */

            if (spll->sync_index <= 5)
            {
                tbprd_stepsize = 1; /* Slightly out of sync. stepsize = 1 */
                spll->pll_stat.pll_in_sync = IN_SYNC;
                my_pll.my_pll_in_sync = 1;
                my_pll.my_pll_out_of_sync = 0;
            }
            else
            {
                tbprd_stepsize = 4; /* Completely out of sync. stepsize = 4 */
                spll->pll_stat.pll_in_sync = OUT_OF_SYNC;

                if (spll->sync_index >= 50) //15
                {
                    my_pll.my_pll_in_sync = 0;
                    my_pll.my_pll_out_of_sync = 1;
                }
            }

            //check = tbprd_stepsize;
            /* phase control */
            if (PHASE_LEAD(shift_index))
            {
                next_tprd += tbprd_stepsize; /* Phase Lead. Reduce frequency */
                spll->sync_index = shift_index;
            }
            else if (PHASE_LAG(shift_index))
            {
                next_tprd -= tbprd_stepsize; /* Phase lag. Increase frequency */
                spll->sync_index = NO_OF_MINOR_CYCLES_PER_LINE_CYCLE
                        - shift_index;
            }
            else
            {
                spll->sync_index = 0; /* No change */
            }
            //    balance = new_capture - next_tprd * NO_OF_MINOR_CYCLES_PER_LINE_CYCLE;

            /* limit the frequency with permissible range on operation */
            next_tprd = _IQsat(next_tprd, TBPRD_UPPER_LIMIT, TBPRD_LOWER_LIMIT);
        }
    }

    /* Slew to nominal frequency on PLL faults */
    if ((GRID_NOT_PRESENT(spll->pll_stat)) || (GRID_FREQ_BAD(spll->pll_stat)))
    {
        spll->pll_stat.pll_in_sync = OUT_OF_SYNC;
        my_pll.my_pll_in_sync = 0;
        my_pll.my_pll_out_of_sync = 1;

        next_tprd = present_tprd;
    }

    EPwm2Regs.TBPRD = (next_tprd);  // -1 for up counter

}



