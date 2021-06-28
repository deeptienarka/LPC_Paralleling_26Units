/*****************************************************************************************************************
*                             enArka India Private Limited proprietary                                           *
* File: ei_pll.c                                                                                                 *
* Copyright (c) 2019 by enArka Instruments Pvt. Ltd.                                                             *
* All Rights Reserved                                                                                            *
* Description: This file checks all protection of Power factor controller                                        *                                                                                                                *
*****************************************************************************************************************/


/*****************************************************************************************************************
* Include Files                                                                                                  *
*****************************************************************************************************************/

#include "F28x_Project.h"              // Standard libraries headers
#include "F2837xS_Device.h"
#include "oi_InitPeripherals.h"        // Peripherals declaration include file
#include "math.h"                      // Standard library for math operations
#include "oi_genStructureDefs.h"                    // PLL module header file
#include "oi_externVariables.h"
// ************External Variables*********** //


void oi_Protection_Module(void)
{
    /********* Output DC under-voltage Check************/


//    if ((DC_Avg_Values.Vdc_tot_feedback < DC_UNDER_VOLTAGE_LIMIT)) // || (vdcbus_pos1 < DC_UNDER_VOLTAGE_LIMIT1) || (vdcbus_neg1 < DC_UNDER_VOLTAGE_LIMIT1))
//    {
//        ++timer.ui_dc_bus_undervolt_timer;
//        if (timer.ui_dc_bus_undervolt_timer >= 10)
//        {
//            timer.ui_dc_bus_undervolt_timer = 10;
//            faults.bit.dc_bus_undervolt = 1;
//            faults.bit.system_fault = 1;
//        }
//    }
//    else
//    {
//        timer.ui_dc_bus_undervolt_timer = 0;
//    }

    /********* Output DC over-voltage Check************/

    if ((DC_Avg_Values.Vdc_tot_feedback > DC_OVER_VOLTAGE_LIMIT)
            || (DC_Avg_Values.Vdcbus_pos > DC_OVER_VOLTAGE_LIMIT1)
            || (DC_Avg_Values.Vdcbus_neg > DC_OVER_VOLTAGE_LIMIT1))
    {
        ++timer.ui_dc_bus_overvolt_timer;
        if (timer.ui_dc_bus_overvolt_timer >= 10)
        {
            timer.ui_dc_bus_overvolt_timer = 10;
            faults_l.bit.dc_bus_overvolt = 1;
            faults_h.bit.system_fault = 1;
        }
    }
    else if ((DC_Avg_Values.Vdc_tot_feedback < (DC_OVER_VOLTAGE_LIMIT * 0.97))
            && (DC_Avg_Values.Vdcbus_pos < (DC_OVER_VOLTAGE_LIMIT1 * 0.97))
            && (DC_Avg_Values.Vdcbus_neg < (DC_OVER_VOLTAGE_LIMIT1 * 0.97)))
    {
        timer.ui_dc_bus_overvolt_timer = 0;
    }

    /********* Input AC under-voltage Check************/

    if (voltage.Vll_rms_filtered < AC_UNDER_VOLTAGE_LIMIT)
    {

        ++timer.ui_ac_undervoltage_Timer;
        if (timer.ui_ac_undervoltage_Timer >= 1024)
        {
            timer.ui_ac_undervoltage_Timer = 1024;
            faults_l.bit.grid_ac_undervolt = 1;
            faults_h.bit.system_fault = 1;
        }
    }
    else if (voltage.Vll_rms_filtered > (AC_UNDER_VOLTAGE_LIMIT * 1.03))
    {
        timer.ui_ac_undervoltage_Timer = 0;
        faults_l.bit.grid_ac_undervolt = 0;
    }


    /********* Input AC over-voltage Check************/

    if (voltage.Vll_rms_filtered > AC_OVER_VOLTAGE_LIMIT)
    {
        ++timer.ui_ac_overvoltage_Timer;
        if (timer.ui_ac_overvoltage_Timer >= 1024)
        {
            timer.ui_ac_overvoltage_Timer = 1024;
            faults_h.bit.system_fault = 1;
            faults_l.bit.grid_ac_overvolt = 1;
        }

    }
    else if (voltage.Vll_rms_filtered < (AC_OVER_VOLTAGE_LIMIT * 0.97))
    {
        timer.ui_ac_overvoltage_Timer = 0;
        faults_l.bit.grid_ac_overvolt = 0;
    }

    /********* Input AC Over-Current Check************/

    if ((fec_state.ac_channels[IR_SINE].norm_rms > AC_OVER_CUR_LIMIT)
            || (fec_state.ac_channels[IY_SINE].norm_rms > AC_OVER_CUR_LIMIT)
            || (fec_state.ac_channels[IB_SINE].norm_rms > AC_OVER_CUR_LIMIT))
    {
        ++timer.ui_Iin_Overload_Timer;
        if (timer.ui_Iin_Overload_Timer >= 1024)
        {
            timer.ui_Iin_Overload_Timer = 1024;
            faults_h.bit.grid_current_violation = 1;
            faults_h.bit.system_fault = 1;
        }
    }
    else
    {
        timer.ui_Iin_Overload_Timer = 0;
    }

    /********* Output DC Over-Current Check*********/

    if (DC_Avg_Values.Idc_out > DC_OVER_CURRENT_LIMIT)
    {
        ++timer.ui_dc_overcurrent_timer;
        if (timer.ui_dc_overcurrent_timer >= 500)
        {
            timer.ui_dc_overcurrent_timer = 500;
            faults_h.bit.system_fault = 1;
            faults_h.bit.dc_bus_overcurrent = 1;
        }
    }
    else
    {
        timer.ui_dc_overcurrent_timer = 0;
    }

    /********* Over Temperature Check ************/


//    if (Temp_Meas.TEMP_MEASURED_IN_DEG > OTP_TRIP_LIMIT)
//    {
//        ++timer.otp_sense_timer;
//        if (timer.otp_sense_timer >= 1000)
//        {
//            timer.otp_sense_timer = 1000;
//            faults.bit.over_temp_voilation = 1;
//            faults.bit.system_fault = 1;
//        }
//    }
//    else if (Temp_Meas.TEMP_MEASURED_IN_DEG < OTP_RECOVERY_LIMIT)
//    {
//        timer.otp_sense_timer = 0;
//        faults.bit.over_temp_voilation = 0;
//    }

    /********* Frequency Out of Range ************/

    if ((FrequencyInHz < 40) || (FrequencyInHz > 60))
    {
        ++timer.freq_out_of_range_timer;
        if (timer.freq_out_of_range_timer > 2048)
        {
            timer.freq_out_of_range_timer = 2048;
            faults_h.bit.freq_out_of_Range = 1;
            faults_h.bit.system_fault = 1;
        }
    }
    else if ((FrequencyInHz >= 45) && (FrequencyInHz <= 55))
    {
        faults_h.bit.freq_out_of_Range = 0;
        timer.freq_out_of_range_timer = 0;
    }

    /********* Phase missing Check ************/

    phase_R = fec_state.ac_channels[VR_PHASE_SENSE_DSP].norm_rms;
    phase_Y = fec_state.ac_channels[VY_PHASE_SENSE_DSP].norm_rms;
    phase_B = fec_state.ac_channels[VB_PHASE_SENSE_DSP].norm_rms;

    if (((abs)(phase_R - (phase_Y + phase_B)) <= 15)
            || ((abs)(phase_Y - (phase_B + phase_R)) <= 15)
            || ((abs)(phase_B - (phase_R + phase_Y)) <= 15))
    {
        ++timer.inputphase_missing_timer;
        if (timer.inputphase_missing_timer >= 1000)
        {
            timer.inputphase_missing_timer = 1000;
            faults_h.bit.inputphase_missing = 1;
            faults_h.bit.system_fault = 1;
        }
    }
    else if (((abs)(phase_R - (phase_Y + phase_B)) >= 25)
            && ((abs)(phase_Y - (phase_B + phase_R)) >= 25)
            && ((abs)(phase_B - (phase_R + phase_Y)) >= 25))
    {
        timer.inputphase_missing_timer = 0;
        faults_h.bit.inputphase_missing = 0;
    }


    if(GpioDataRegs.GPBDAT.bit.GPIO41 == 1)
    {
        faults_l.bit.hardware_fault = 1;
    }
    else
    {
        faults_l.bit.hardware_fault = 0;
    }
    /*---------------------------------PFC trip setting and resetting-----------------*/

    if ((faults_l.bit.dc_bus_overvolt == 0) && (faults_h.bit.freq_out_of_Range == 0)
            && (faults_h.bit.inputphase_missing == 0)
            && (faults_h.bit.over_temp_voilation == 0)
            && (faults_l.bit.dc_bus_undervolt == 0)
            && (faults_l.bit.grid_ac_overvolt == 0)
            && (faults_l.bit.grid_ac_undervolt == 0)
            && (faults_h.bit.dc_bus_overcurrent == 0)
            && (faults_h.bit.grid_current_violation == 0) && (my_pll.my_pll_in_sync ==1) && (CapInterruptAck == 1))

    {
        faults_h.bit.system_fault = 0;
    }
    else
    {
        faults_h.bit.system_fault = 1;
    }

}


