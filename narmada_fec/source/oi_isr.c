/*
 * oi_isr.c
 *
 *  Created on: Sep 29, 2020
 *      Author: enArka
 */
/*****************************************************************************************************************
 * Include Files                                                                                                  *
 *****************************************************************************************************************/

#include "F28x_Project.h"              // Standard libraries headers
#include "F2837xS_Device.h"
#include "oi_initperipherals.h"
#include "oi_genStructureDefs.h"
#include "oi_sinGeneration.h"
#include "oi_externVariables.h"
#include "oi_fec_control.h"
#include "math.h"
#include "stdlib.h"
#include "oi_globalPrototypes.h"


interrupt void epwm2_isr(void)
{

    P_LL(&fec_state);                             // PLL Algorithm Function call

    /************ADC results Acquisition to buffer **********************************/

    if (fec_state.theta_index > NO_OF_SAMPLES_IN_ONE_CYCLE)
    {
        fec_state.theta_index = 0;
    }
    fec_state.theta_index = fec_state.theta_index + 1;

    /************30 degree shifting of theta index for R phase************************/

    if (Phase_Sequence.Seq_chk_flag == 1)
    {
        fec_state.theta_index_R = fec_state.theta_index + PHASE_SHIFT_R; // Ph lead LL  by 30 deg

        if (fec_state.theta_index_R > NO_OF_SAMPLES_IN_ONE_CYCLE)
        {
            fec_state.theta_index_R = fec_state.theta_index_R
                    - NO_OF_SAMPLES_IN_ONE_CYCLE;
        }
    }
    else
    {
        fec_state.theta_index_R = NO_OF_SAMPLES_IN_ONE_CYCLE
                - (fec_state.theta_index - PHASE_SHIFT_R); // Ph lags LL by 30 deg; Reverse in Rotation

        if (fec_state.theta_index_R > NO_OF_SAMPLES_IN_ONE_CYCLE)
        {
            fec_state.theta_index_R = fec_state.theta_index_R
                    - NO_OF_SAMPLES_IN_ONE_CYCLE;
        }
    }

    /**
     * Half cycle RMS Calculations.
     * Rms accumulations are done every 4 interrupts.
     *
     * Rms calculations involves 2 main steps:
     * 1. square and accumulation
     * 2. mean and root
     *
     * Step1 (mean-square and accumulation) is performed every 4 interrupts.
     * Step2 is performed every half-cycle (i.e) once every NO_OF_MINOR_CYCLES_PER_LINE_CYCLE/2 interrupts.
     *
     * No. of samples taken for half-cycle rms calculation = NO_OF_MINOR_CYCLES_PER_LINE_CYCLE/(2*4)
     */
    if ((fec_state.theta_index % 4) == 0)
    {

        fec_state.ac_channels[VR_PHASE_SENSE_DSP].norm_acc +=
                square(((ac_inst((float32)(fec_state.adc_buffer[VR_PHASE_SENSE_DSP]))) - HW_OFFSET_VRY));
        fec_state.ac_channels[VY_PHASE_SENSE_DSP].norm_acc +=
                square(((ac_inst((float32)(fec_state.adc_buffer[VY_PHASE_SENSE_DSP]))) - HW_OFFSET_VYB));
        fec_state.ac_channels[VB_PHASE_SENSE_DSP].norm_acc +=
                square(((ac_inst((float32)(fec_state.adc_buffer[VB_PHASE_SENSE_DSP]))) - HW_OFFSET_VBR));
        fec_state.ac_channels[IR_SINE].norm_acc +=
                square(((ac_inst((float32)(fec_state.adc_buffer[IR_SINE]))) - HW_OFFSET_IR));
        fec_state.ac_channels[IY_SINE].norm_acc +=
                square(((ac_inst((float32)(fec_state.adc_buffer[IY_SINE]))) - HW_OFFSET_IY));
        fec_state.ac_channels[IB_SINE].norm_acc +=
                square(((ac_inst((float32)(fec_state.adc_buffer[IB_SINE]))) - HW_OFFSET_IB));

        //   ISR_TIMING_CLEAR();

        /* half cycle points (90 and 270 degrees) */

        if (fec_state.theta_index == (NO_OF_MINOR_CYCLES_PER_LINE_CYCLE)) //((rectifier_state.theta_index == NO_OF_MINOR_CYCLES_PER_LINE_CYCLE / 4) || (rectifier_state.theta_index == NO_OF_MINOR_CYCLES_PER_LINE_CYCLE * 3 / 4))
        {

            fec_state.ac_channels[VR_PHASE_SENSE_DSP].norm_rms =
                    mean_full_cycle(
                            fec_state.ac_channels[VR_PHASE_SENSE_DSP].norm_acc);
            fec_state.ac_channels[VY_PHASE_SENSE_DSP].norm_rms =
                    mean_full_cycle(
                            fec_state.ac_channels[VY_PHASE_SENSE_DSP].norm_acc);
            fec_state.ac_channels[VB_PHASE_SENSE_DSP].norm_rms =
                    mean_full_cycle(
                            fec_state.ac_channels[VB_PHASE_SENSE_DSP].norm_acc);
            fec_state.ac_channels[IR_SINE].norm_rms = mean_full_cycle(
                    fec_state.ac_channels[IR_SINE].norm_acc);
            fec_state.ac_channels[IY_SINE].norm_rms = mean_full_cycle(
                    fec_state.ac_channels[IY_SINE].norm_acc);
            fec_state.ac_channels[IB_SINE].norm_rms = mean_full_cycle(
                    fec_state.ac_channels[IB_SINE].norm_acc);

            /*square root */
            fec_state.ac_channels[VR_PHASE_SENSE_DSP].norm_rms =
                    (sqrt(fec_state.ac_channels[VR_PHASE_SENSE_DSP].norm_acc)
                            * CircuitGain.ACVoltage);
            fec_state.ac_channels[VY_PHASE_SENSE_DSP].norm_rms =
                    (sqrt(fec_state.ac_channels[VY_PHASE_SENSE_DSP].norm_acc)
                            * CircuitGain.ACVoltage); // op amp and calibration
            fec_state.ac_channels[VB_PHASE_SENSE_DSP].norm_rms =
                    (sqrt(fec_state.ac_channels[VB_PHASE_SENSE_DSP].norm_acc)
                            * CircuitGain.ACVoltage);
            fec_state.ac_channels[IR_SINE].norm_rms = (sqrt(
                    fec_state.ac_channels[IR_SINE].norm_acc) * 0.029642);
            fec_state.ac_channels[IY_SINE].norm_rms = (sqrt(
                    fec_state.ac_channels[IY_SINE].norm_acc) * 0.029642);
            fec_state.ac_channels[IB_SINE].norm_rms = (sqrt(
                    fec_state.ac_channels[IB_SINE].norm_acc) * 0.029642);

            fec_state.ac_channels[VR_PHASE_SENSE_DSP].norm_acc =
                    fec_state.ac_channels[VY_PHASE_SENSE_DSP].norm_acc =
                            fec_state.ac_channels[VB_PHASE_SENSE_DSP].norm_acc =
                                    0;
            fec_state.ac_channels[IR_SINE].norm_acc =
                    fec_state.ac_channels[IY_SINE].norm_acc =
                            fec_state.ac_channels[IB_SINE].norm_acc = 0;

        }

     }

    //  Allows a higher priority interrupt to interrupt this ISR
        DINT;                                   // Disable when changes have to be made to IER
        IER |= M_INT1;                          // Enable Group 1 interrupts at CPU level
        PieCtrlRegs.PIEACK.all = 0xFFFF;        // Allow all interrupts at the PIE level by acknowledging it
        EINT;

    //
    // Clear INT flag for this timer
    //
    EPwm2Regs.ETCLR.bit.INT = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

interrupt void adca1_isr(void)
{
    //GpioDataRegs.GPCSET.bit.GPIO85 = 1;

    /******** Latch Reset Pulse Starts here    ****/

    if ((timer.latch_timer > 20) && (timer.latch_timer <= 35))
    {
        GpioDataRegs.GPASET.bit.GPIO16 = 1;
    }

    timer.latch_timer++;

    if (timer.latch_timer > 36)
    {
        timer.latch_timer = 37;

        GpioDataRegs.GPACLEAR.bit.GPIO16 = 1;
    }
    /******** Latch Reset Pulse ends here    ****/

    /******** ADC value Acquisition  Starts here    ****/

    value_transfer();   // ADC result register value transferred here

    /******** ADC value Acquisition  ends here    ****/

    /*********  DC bus over voltage Check starts here************/

    if ((DC_Avg_Values.Vdcbus_neg > INST_DC_OVER_VOLTAGE_LIMIT)
            || (DC_Avg_Values.Vdcbus_pos > INST_DC_OVER_VOLTAGE_LIMIT))
    {
        timer.isr_dc_ov_timer++;
        if (timer.isr_dc_ov_timer >= OV_COUNTER_LIMIT)
        {
            faults_h.bit.system_fault = 1;
            timer.isr_dc_ov_timer = (OV_COUNTER_LIMIT + 1);
        }
    }
    /*********  DC bus over voltage Check ends here************/

    /*********  AC instantaneous over current protection starts here  ************/
    Inst_AC_Current_Envelop =
            abs(AC_inst_Values.Iin_ac_rn1) > abs(AC_inst_Values.Iin_ac_yn1) ?
                    abs(AC_inst_Values.Iin_ac_rn1) : abs(AC_inst_Values.Iin_ac_yn1);
    Inst_AC_Current_Envelop =
            (Inst_AC_Current_Envelop > abs(AC_inst_Values.Iin_ac_bn1)) ?
                    Inst_AC_Current_Envelop : abs(AC_inst_Values.Iin_ac_bn1);

    if (Inst_AC_Current_Envelop > Inst_AC_Over_Currlmt)
    {
        timer.inst_ac_oc_timer++;
        if (timer.inst_ac_oc_timer >= 10)
        {
            timer.inst_ac_oc_timer = 10;
            faults_h.bit.system_fault = 1;
            faults_h.bit.inst_grid_rstcurrent_violation = 1;
        }

    }
    /*********  AC instantaneous over current protection ends here  ************/

    /*********  DAC updation starts here  ************/

    oi_runDAC();

    /*********  DAC updation ends here  ************/

    /*********  Unit Vector generation starts here  ************/

    UNIT_COS_R = __cospuf32(inputVector[fec_state.theta_index_R]);  // Unit cosine theta vector - R-Phase

    UNIT_SINE_R = __sinpuf32(inputVector[fec_state.theta_index_R]); // Unit sine theta vector   - R-phase

    /*********  Unit Vector generation ends here  ***************/

    /************* Instantaneous acquisition AC voltage, AC current, AVERAGE DC BUS VALUES and Average DC Current starts here***********/

    AC_inst_Values.Vin_ac_ry = 0.5 * ( AC_inst_Values.Vin_ac_ry + (CircuitGain.ACVoltage)
            * ((ac_inst((float32)(fec_state.adc_buffer[VR_PHASE_SENSE_DSP])))
                    - HW_OFFSET_VRY) );  //offset of 1.65V to be removed

    AC_inst_Values.Vin_ac_yb = 0.5 * ( AC_inst_Values.Vin_ac_yb + (CircuitGain.ACVoltage)
            * ((ac_inst((float32)(fec_state.adc_buffer[VY_PHASE_SENSE_DSP])))
                    - HW_OFFSET_VYB) );  //offset of 1.65V to be removed

    AC_inst_Values.Vin_ac_br = 0.5 * ( AC_inst_Values.Vin_ac_br + (CircuitGain.ACVoltage)
            * ((ac_inst((float32)(fec_state.adc_buffer[VB_PHASE_SENSE_DSP])))
                    - HW_OFFSET_VBR) );  //offset of 1.65V to be removed

    AC_inst_Values.Iin_ac_rn1 = 0.5*(AC_inst_Values.Iin_ac_rn1 + (CircuitGain.ACCurrent)
                * ((ac_inst((float32)(fec_state.adc_buffer[IR_SINE])))
                        - HW_OFFSET_IR)); //offset of 1.65V to be removed

    AC_inst_Values.Iin_ac_yn1 = 0.5*(AC_inst_Values.Iin_ac_yn1 + (CircuitGain.ACCurrent)
            * ((ac_inst((float32)(fec_state.adc_buffer[IY_SINE])))
                    - HW_OFFSET_IY)); //offset of 1.65V to be removed

    AC_inst_Values.Iin_ac_bn1 = 0.5*(AC_inst_Values.Iin_ac_bn1 + (CircuitGain.ACCurrent)
                * ((ac_inst((float32)(fec_state.adc_buffer[IB_SINE])))
                        - HW_OFFSET_IB)); //offset of 1.65V to be removed

    EMA_FILTER(fec_state.adc_buffer[POS_BUS_DSP]);
    EMA_FILTER(fec_state.adc_buffer[NEG_BUS_DSP]);
    EMA_FILTER(fec_state.adc_buffer[IOUT_DC_SENSE_DSP]);

    DCBusVoltInst = (DCBusVoltInst + ((float32)(fec_state.adc_buffer[POS_BUS_DSP].adc_result_regs) + (float32)(fec_state.adc_buffer[NEG_BUS_DSP].adc_result_regs)) * 0.1116)/2.0;

    DC_Avg_Values.Vdcbus_neg =
            ((((float32) (fec_state.adc_buffer[POS_BUS_DSP].norm_filtered_value))
                    * CircuitGain.DCVoltage));
    DC_Avg_Values.Vdcbus_pos =
            ((((float32) (fec_state.adc_buffer[NEG_BUS_DSP].norm_filtered_value))
                    * CircuitGain.DCVoltage));  //ADC gain and calibration factor
    DC_Avg_Values.Vdc_tot_feedback = DC_Avg_Values.Vdcbus_pos
            + DC_Avg_Values.Vdcbus_neg;

    DC_Avg_Values.Idc_out =
            ((((float32) ((fec_state.adc_buffer[IOUT_DC_SENSE_DSP].norm_filtered_value)
                    - HW_OFFSET_IOUT)) * CircuitGain.DCCurrent)); //ADC gain and calibration factor

    /************* Instantaneous acquisition AC voltage, AC current, AVERAGE DC BUS VALUES and Average DC Current ends here***********/

    /* *******************Protection Module starts here***********************************/

    oi_Protection_Module();

    /* *******************Protection Module ends here***********************************/

    /* *******************Current Feedforward calculation and filtering starts here***********************************/

     current.I_q_ff =  (fec_state.adc_buffer[IOUT_DC_SENSE_DSP].adc_result_regs - HW_OFFSET_IOUT )*(-0.027643*1.732051/1.4142135)*(Vref/voltage.Vll_rms_filtered);

     current.I_filtered = current.I_q_ff * 0.1119 + current.I_filtered * 0.8881;

     /* *******************Current Feedforward calculation and filtering end here***********************************/


    /* *******************Line to phase transformations starts here***********************************/

    AC_inst_Values.V_R_PHASE = (AC_inst_Values.Vin_ac_ry
            - AC_inst_Values.Vin_ac_br) / 3.0;
    AC_inst_Values.V_Y_PHASE = (AC_inst_Values.Vin_ac_yb
            - AC_inst_Values.Vin_ac_ry) / 3.0;
    AC_inst_Values.V_B_PHASE = (AC_inst_Values.Vin_ac_br
            - AC_inst_Values.Vin_ac_yb) / 3.0;

    /* *******************Line to phase transformations ends here***************************************/

    /* *******************Line to line RMS calculation and filtering starts here***********************************/


    voltage.Vll_rms = abs((1.2247449 * AC_inst_Values.V_R_PHASE * UNIT_SINE_R) - (0.7071068 * AC_inst_Values.Vin_ac_yb * UNIT_COS_R));

    voltage.Vll_rms_filtered = voltage.Vll_rms * 0.001255 + voltage.Vll_rms_filtered * 0.998745; // Low pass filter of 10Hz


    /* *******************Line to line RMS calculation and filtering ends here**************************************/

    /* *********Relay on/off based on DCVoltage feedback and Input voltage starts here*************************/

    if((DC_Avg_Values.Vdc_tot_feedback > 460.0) && (DC_Avg_Values.Vdc_tot_feedback > (0.98 * 1.414 *voltage.Vll_rms_filtered)))
    {
        RELAY_ON;
    }
    else if((DC_Avg_Values.Vdc_tot_feedback < 380) || (faults_h.bit.system_fault == 1))
    {
       RELAY_OFF;
    }

    /* *************Relay on/off based on DCVoltage feedback and Input voltage ends here***********************/

    /* *******************  Current transformations starts here ***********************************/

    current.alpha = 1.5 * AC_inst_Values.Iin_ac_rn1;

    current.beta = (AC_inst_Values.Iin_ac_yn1 - AC_inst_Values.Iin_ac_bn1)* (1.7320508 / 2.0);


    current.d_axis = current.alpha * UNIT_COS_R + current.beta * UNIT_SINE_R;

    current.q_axis = -current.alpha * UNIT_SINE_R + current.beta * UNIT_COS_R;

    /* ******************* Current transformations ends here***********************************/

    /* *******************DC Voltage Soft Start and Instantaneous DC feedback update starts here***********************************/

    if ((faults_h.bit.system_fault == 0) && (onCommand == 1.0))
    {
        voltage_Loop.Ref += 0.0004;
        if (voltage_Loop.Ref >= Vref)
            voltage_Loop.Ref = Vref;
    }
    else
    {
        RESET_PI(voltage_Loop);
        current.I_filtered = 0;
        voltage_Loop.Ref = voltage_Loop.Fbk;
    }


    voltage_Loop.Fbk = DCBusVoltInst;

    /* *******************DC Voltage Soft Start and Instantaneous DC feedback update starts here***********************************/


    /* *******************Voltage loop starts here***********************************/

    PI_MACRO(voltage_Loop);

    /* ******************Pulse on/off at no load condition to avoid DC Bus voltage shoot-up starts here************************************/

    if ((DCBusVoltInst >= (Vref * 1.03)) && (VDCStatusChkFlg == 1))
    {
        VDCStatusChkFlg = 0;
    }
    else if ((DCBusVoltInst < (Vref * 0.96)) && (VDCStatusChkFlg == 0))
    {
        VDCStatusChkFlg = 1;
    }

    if (VDCStatusChkFlg == 0)
    {
        RESET_PI(voltage_Loop);
        PULSE_OFF;
    }
    else
    {
        if ((faults_h.bit.system_fault == 0) && (onCommand == 1.0))
        {
            PULSE_ON;
        }
        else
        {
            PULSE_OFF;
        }
    }
    /*******************Pulse on/off at no load condition to avoid DC Bus voltage shoot-up ends here************************************/

    /* *******************Voltage loop ends here***********************************/

    /* *******************Current loop starts here***********************************/

    del_Mod_Index.d_axis =   current.Kpi * (0 - current.d_axis);

    if(Phase_Sequence.Seq_chk_flag == 1)
    {
        del_Mod_Index.q_axis =   current.Kpi *  (-(voltage_Loop.Out + current.I_filtered) - current.q_axis);    //-ve Iq Reference
    }
    else
    {
        del_Mod_Index.q_axis =   current.Kpi *  (+(voltage_Loop.Out + current.I_filtered) - current.q_axis);    //+ve Iq Reference
    }

    /* *******************Current loop ends here***********************************/

    /***************modulation index Calculation starts here**********/

    del_Mod_Index.alpha = del_Mod_Index.d_axis * UNIT_COS_R
            - del_Mod_Index.q_axis * UNIT_SINE_R;
    del_Mod_Index.beta = del_Mod_Index.d_axis * UNIT_SINE_R
            + del_Mod_Index.q_axis * UNIT_COS_R;

    del_Mod_Index.R_phase = (2.0 / 3.0) * del_Mod_Index.alpha;
    del_Mod_Index.Y_phase = -(1.0 / 3.0) * del_Mod_Index.alpha
            + (0.577350269) * del_Mod_Index.beta;
    del_Mod_Index.B_phase = -(1.0 / 3.0) * del_Mod_Index.alpha
            - (0.577350269) * del_Mod_Index.beta;

    mod_Index_ff.R_phase = 1.998 * AC_inst_Values.V_R_PHASE
            / DCBusVoltInst;
    mod_Index_ff.Y_phase = 1.998 * AC_inst_Values.V_Y_PHASE
            / DCBusVoltInst;
    mod_Index_ff.B_phase = 1.998 * AC_inst_Values.V_B_PHASE
            / DCBusVoltInst;


    del_Mod_Com = (AC_inst_Values.Iin_ac_rn1 + AC_inst_Values.Iin_ac_yn1 + AC_inst_Values.Iin_ac_bn1) * k_com + (DC_Avg_Values.Vdcbus_pos - DC_Avg_Values.Vdcbus_neg) * k_diff;

    mod_Index.R_phase  =   mod_Index_ff.R_phase  -  del_Mod_Index.R_phase - del_Mod_Com;
    mod_Index.Y_phase  =   mod_Index_ff.Y_phase  -  del_Mod_Index.Y_phase - del_Mod_Com;
    mod_Index.B_phase  =   mod_Index_ff.B_phase  -  del_Mod_Index.B_phase - del_Mod_Com;


    /***************Common mode Calculation starts here****************/

    max_mod_index =
            (mod_Index.R_phase > mod_Index.Y_phase) ?
                    mod_Index.R_phase : mod_Index.Y_phase;
    max_mod_index =
            (max_mod_index > mod_Index.B_phase) ?
                    max_mod_index : mod_Index.B_phase;

    min_mod_index =
            (mod_Index.R_phase < mod_Index.Y_phase) ?
                    mod_Index.R_phase : mod_Index.Y_phase;
    min_mod_index =
            (min_mod_index < mod_Index.B_phase) ?
                    min_mod_index : mod_Index.B_phase;

    com_mod_index = 0.5 * (max_mod_index + min_mod_index);

    /***************Common mode Calculation ends here****************/

    mod_Index.R_phase = mod_Index.R_phase - com_mod_index;
    mod_Index.Y_phase = mod_Index.Y_phase - com_mod_index;
    mod_Index.B_phase = mod_Index.B_phase - com_mod_index;

    /***************modulation index Calculation ends here****************/


    /***************Compare update PWM Generation starts here****************/

    if ((faults_h.bit.system_fault == 0) && (onCommand == 1.0))
    {
        //Update PWM Compare Register
        fec_state.healthStatus = 1;
        EPwm6Regs.CMPA.bit.CMPA = abs(mod_Index.R_phase * EPWM6_TIMER_TBPRD);
        EPwm7Regs.CMPA.bit.CMPA = abs(mod_Index.Y_phase * EPWM7_TIMER_TBPRD);
        EPwm8Regs.CMPA.bit.CMPA = abs(mod_Index.B_phase * EPWM8_TIMER_TBPRD);
    }
    else
    {
        fec_state.healthStatus = 0;
        PULSE_OFF;
        RESET_PI(voltage_Loop);
        EPwm6Regs.CMPA.bit.CMPA = EPWM6_TIMER_TBPRD;
        EPwm7Regs.CMPA.bit.CMPA = EPWM7_TIMER_TBPRD;
        EPwm8Regs.CMPA.bit.CMPA = EPWM8_TIMER_TBPRD;
    }

    /***************Compare update PWM Generation ends here****************/

    /***************Scheduler count starts here****************/

    ui_Scheduler_Index++;

    if (ui_Scheduler_Index >= 5120) //100msec for the scheduler(19.53microsec * 5120 = 100msec)
    {
        ui_Scheduler_Index = 0;
    }

    /***************Scheduler count ends here****************/


// EVENT SCHEDULER
// Check if the present index matches the ui_Scheduled_Occurrence of the next event. If it does then
// this event_index is copied to ui_General_Event_Initiated.
// Using ui_General_Event_Initiated the event is executed in main by calling the function pointer stored in the
// General_Event_Array[ui_Event_Initiated].p_fn_EventDriver()
    if (ui_Scheduler_Index
            == Event_Array[ui_Next_Event_In_Line].ui_Scheduled_Occurrence)
    {
        ui_Event_Initiated = ui_Next_Event_In_Line;

        // Roll back ui_Next_Event_In_Line at the end of the array
        if (ui_Next_Event_In_Line < (TOTAL_NO_OF_EVENTS - 1))
        {
            ui_Next_Event_In_Line++;
        }
        else
        {
            ui_Next_Event_In_Line = 0;
        }

        // Setting this flag triggers the event being called in main
        Global_Flags.fg_Scheduled_General_Event_Flag = 1;
}
/***************Compare update PWM Generation ends here****************/

    DSP_BEAT_TOGGLE; // DSP Fault detection
  //  GpioDataRegs.GPCCLEAR.bit.GPIO85 = 1;

    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

//************** ECap1FallingEdgeIsr ****************************************************************************//
// Function Name: void ECap1FallingEdgeIsr                                                                       *
// Return Type: void                                                                                             *
// Arguments:   void                                                                                             *
// Description: This interrupt is generated at every zero crossing
//*****         of Grid Sine wave given to the capture module.        ********************************************//

interrupt void ECap1FallingEdgeIsr()
{
/******phase Sequence Checking starts here*******************************/

    if (AC_inst_Values.Vin_ac_yb < 0)
    {
        Phase_Sequence.Seq_chk_flag = 1;                    // Positive Sequence
    }
    else if (AC_inst_Values.Vin_ac_yb > 0)
    {
        Phase_Sequence.Seq_chk_flag = 0;                    // Negative Sequence
    }

    fec_state.pll.zx.freq_in_cap_counts = ECap1Regs.CAP1; // frequency of the last cycle in capture counts
    fec_state.pll.zx.phase_shift_index = fec_state.theta_index; // get present phase difference at 0Xing instant

    CapInterruptAck = 1;                                  // ack capture interrupt

    /******phase Sequence Checking ends here*******************************/

    ECap1Regs.ECCLR.bit.CEVT1 = 1;
    ECap1Regs.ECCLR.bit.INT = 1;
    ECap1Regs.ECCTL2.bit.REARM = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;

}

void value_transfer (void)
{
    fec_state.adc_buffer[HS2_TEMP_MEAS_DSP].adc_result_regs =
            AdcaResultRegs.ADCRESULT2;
    fec_state.adc_buffer[HS1_TEMP_MEAS_DSP].adc_result_regs =
            AdcaResultRegs.ADCRESULT3;
    fec_state.adc_buffer[POS_BUS_DSP].adc_result_regs =
            AdcaResultRegs.ADCRESULT4;
    fec_state.adc_buffer[NEG_BUS_DSP].adc_result_regs =
            AdcaResultRegs.ADCRESULT5;
    fec_state.adc_buffer[IR_SINE].adc_result_regs = AdcaResultRegs.ADCRESULT0;
    fec_state.adc_buffer[IY_SINE].adc_result_regs = AdcaResultRegs.ADCRESULT1;
    fec_state.adc_buffer[IB_SINE].adc_result_regs = AdcbResultRegs.ADCRESULT0;
    fec_state.adc_buffer[TEMP_MEAS].adc_result_regs = AdcbResultRegs.ADCRESULT1;
    fec_state.adc_buffer[IOUT_DC_SENSE_DSP].adc_result_regs =
            AdcbResultRegs.ADCRESULT2;
    fec_state.adc_buffer[VR_PHASE_SENSE_DSP].adc_result_regs =
            AdcbResultRegs.ADCRESULT3;
    fec_state.adc_buffer[VY_PHASE_SENSE_DSP].adc_result_regs =
            AdcbResultRegs.ADCRESULT5;
    fec_state.adc_buffer[VB_PHASE_SENSE_DSP].adc_result_regs =
            AdcbResultRegs.ADCRESULT4;
}

void P_LL(fec_state_t *conv_state)
{
    Pll((pll_t *) &conv_state->pll);
}
