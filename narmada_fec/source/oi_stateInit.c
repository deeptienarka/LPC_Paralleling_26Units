/*
 * oi_stateInit.c
 *
 *  Created on: Oct 13, 2020
 *      Author: enArka
 */

#include <oi_fec_control.h>
#include "F2837xS_Device.h"
#include "oi_initperipherals.h"
#include "oi_genStructureDefs.h"
#include "oi_initperipherals.h"
#include "oi_externVariables.h"
#include "oi_globalPrototypes.h"
#include "oi_State.h"
fec_state_t fec_state;
my_pll_status my_pll;
//CONTROLLER Control_loop_param;
TIMERS timer;
fault_regs_t_l faults_l;
fault_regs_t_h faults_h;
AC_FEEDBACK AC_inst_Values;
DC_FEEDBACK DC_Avg_Values;
Temp_Meas_t Temp_Meas;
Phase_Seq Phase_Sequence;
ABC_TO_DQ_CONTROL current;
ABC_TO_DQ_CONTROL voltage;
CIRCUIT_GAIN CircuitGain;



GLOBAL_FLAGS Global_Flags;
EVENT Event_Array[TOTAL_NO_OF_EVENTS];
SYSTEM_STATE State;
STATE_TIMERS_t State_timers;

DQ_TO_ABC_CONTROL mod_Index, del_Mod_Index,mod_Index_ff;
PI_CONTROLLER voltage_Loop = {
                              .Fbk = 0, .Ref = 700.0, .Out = 0,
                              .Kp = 0.142065,
                              .TsKi = 17.8524e-6,
                              .i1 = 0, .ui = 0, .up = 0,
                              .v1 = 0, .w1 = 0,
                              .Umax = 5.0, .Umin = -5.0
};



/**
 * @brief Initialize Adc channels
 * @param pointer to adc_buffer
 */
static void AdcVarsInit(adc_result_buf_t *adc_buf)
{
  Uint16 i;
  for(i = 0; i < NO_OF_ADC_CHANNELS_MEASURED; i++)
  {
    adc_buf[i].adc_result_regs = 0;
  }
}

/**
 * @brief Initialise AC parameters
 * @param pointer to adc_buffer
 */


void StateInit(fec_state_t *conv_state)
{
  AdcVarsInit((adc_result_buf_t *)conv_state->adc_buffer);
  conv_state->theta_index = 0;
  conv_state->theta_index_R = 0;
  conv_state->theta_index_Y = 0;
  conv_state->theta_index_B = 0;
  PllInit((pll_t *)&conv_state->pll);

}

void oi_globalVar_init(void)
{
    //PLL
    StateInit(&fec_state);
    fec_state.theta_index = 0;
    fec_state.theta_index_R = 0;
    fec_state.theta_index_Y = 0;
    fec_state.theta_index_B = 0;
    fec_state.healthStatus = 1;
    pll_t pll;
    FrequencyInHz = 0;
    DC_UNDER_VOLTAGE_LIMIT = 0;
    phase_R = 0;
    phase_Y = 0;
    phase_B = 0;
    //Flags
    Phase_Sequence.Seq_chk_flag = 0;
    Phase_Sequence.theta_index_initflag = 0;
    Phase_Sequence.phase_Seq_change_indflag = 0;
    faults_l.bit.dc_bus_undervolt = 0;
    faults_h.bit.grid_current_violation = 0;
    faults_l.bit.dc_bus_overvolt = 0;
    faults_l.bit.grid_ac_overvolt = 0;
    faults_l.bit.grid_ac_undervolt = 0;
    faults_h.bit.inst_grid_rstcurrent_violation = 0;
    faults_h.bit.dc_bus_overcurrent = 0;

    //timers
    timer.ui_ac_undervoltage_Timer = 0;
    timer.freq_out_of_range_timer = 0;
    timer.isr_dc_ov_timer = 0;
    timer.inputphase_missing_timer = 0;
    timer.otp_sense_timer = 0;
    timer.ui_Iin_Overload_Timer = 0;
    timer.ui_ac_overvoltage_Timer = 0;
    timer.latch_timer = 0;
    current.I_filtered=0;
    current.I_q_ff = 0;
    current.Kpi = 0.0167175;
    voltage.Vll_rms = 0;
    voltage.Vll_rms_filtered = 0;

    CircuitGain.ACVoltage = 0.4047;
    CircuitGain.ACCurrent = -0.029642;
    CircuitGain.DCVoltage = 0.1116;
    CircuitGain.DCCurrent = -0.027643;

    Inst_AC_Over_Currlmt = 42.0;
    Inst_AC_Current_Envelop = 0.0;
    min_mod_index = 0.0;
    max_mod_index = 0.0;
    com_mod_index = 0.0;

    Vref = 800.0;
    DCBusVoltInst = 0;
    del_Mod_Com = 0;
    k_com = 0.0;
    k_diff = 0.0;
    VDCStatusChkFlg = 1;

    onCommand = 0;
    rxChar = 0;
    rxStatus = 0;
    sci_cnt = 0;
    sci_msg1 = 0;
    sci_msg2 = 0;
    onStatus = 0;
    receivedChar = 0;


    // Initialize pointer to initial location in memory
    ui_Scheduler_Index = 0;
    ui_Event_Initiated = 0;
    ui_Next_Event_In_Line = 0;

    // Event initialization.
    // Event sequence is stored in a structure object that contains 2 pieces of information:
    //  1. A function pointer that points to the particular Eventn() function.
    //  2. The ISR number in which the particular event must occur.

    //  The information for all events is stored in this way in an array of structures, the array is
    //  ordered in the sequence that the events occur. The number of array locations is equal to the number
    // of events occurring in one cycle.

    // Event 1: - SCI Event.
    // Event 2: - CAN Event.
    // Event 3: - State Machine and Check low priority Faults.

    Event_Array[0].p_fn_EventDriver = oi_Event1;
    Event_Array[0].ui_Scheduled_Occurrence = 5;             // 5-56 1ms
    Event_Array[1].p_fn_EventDriver = oi_Event2;
    Event_Array[1].ui_Scheduled_Occurrence = 58;            //58-160 2ms
    Event_Array[2].p_fn_EventDriver = oi_Event3;
    Event_Array[2].ui_Scheduled_Occurrence = 162;           //162-213 1ms
    Event_Array[3].p_fn_EventDriver = oi_Event5;
    Event_Array[3].ui_Scheduled_Occurrence = 215;           //215 - 515 TBD
    Event_Array[4].p_fn_EventDriver = oi_Event1;
    Event_Array[4].ui_Scheduled_Occurrence = 517;           //517-568 1ms
    Event_Array[5].p_fn_EventDriver = oi_Event2;
    Event_Array[5].ui_Scheduled_Occurrence = 570;           //570-672 2ms
    Event_Array[6].p_fn_EventDriver = oi_Event4;
    Event_Array[6].ui_Scheduled_Occurrence = 674;           //674-878 4ms

    Event_Array[7].p_fn_EventDriver = oi_Event1;
    Event_Array[7].ui_Scheduled_Occurrence = 1029;          //1029-1080 1msec

    Event_Array[8].p_fn_EventDriver = oi_Event2;
    Event_Array[8].ui_Scheduled_Occurrence = 1082;           //1082 - 1184 2ms
    Event_Array[9].p_fn_EventDriver = oi_Event1;
    Event_Array[9].ui_Scheduled_Occurrence = 1541;           //1541 - 1592 1ms
    Event_Array[10].p_fn_EventDriver = oi_Event2;
    Event_Array[10].ui_Scheduled_Occurrence = 1594;          //1594 - 1696 2ms

    Event_Array[11].p_fn_EventDriver = oi_Event1;
    Event_Array[11].ui_Scheduled_Occurrence = 2053;           //2053 - 2104 1ms

    Event_Array[12].p_fn_EventDriver = oi_Event2;
    Event_Array[12].ui_Scheduled_Occurrence = 2106;           //2106 - 2208 2ms
    Event_Array[13].p_fn_EventDriver = oi_Event1;
    Event_Array[13].ui_Scheduled_Occurrence = 2565;          //2565 - 2616 1ms

    Event_Array[14].p_fn_EventDriver = oi_Event2;
    Event_Array[14].ui_Scheduled_Occurrence = 2618;           //2618-2720 2ms

    Event_Array[15].p_fn_EventDriver = oi_Event3;
    Event_Array[15].ui_Scheduled_Occurrence = 2722;         //2722 - 2773 1ms
    Event_Array[16].p_fn_EventDriver = oi_Event1;
    Event_Array[16].ui_Scheduled_Occurrence = 3077;         //3077-3128 1ms
    Event_Array[17].p_fn_EventDriver = oi_Event2;
    Event_Array[17].ui_Scheduled_Occurrence = 3130;         //3130-3232 2ms

    Event_Array[18].p_fn_EventDriver = oi_Event1;
    Event_Array[18].ui_Scheduled_Occurrence = 3589;         //3589-3640 1ms
    Event_Array[19].p_fn_EventDriver = oi_Event2;
    Event_Array[19].ui_Scheduled_Occurrence = 3642;         //3642-3744 2ms

    Event_Array[20].p_fn_EventDriver = oi_Event1;
    Event_Array[20].ui_Scheduled_Occurrence = 4101;        //4101-4152 1ms

    Event_Array[21].p_fn_EventDriver = oi_Event2;
    Event_Array[21].ui_Scheduled_Occurrence = 4154;         //4154-4256 2ms
    Event_Array[22].p_fn_EventDriver = oi_Event1;
    Event_Array[22].ui_Scheduled_Occurrence = 4613;         //4613-4664 1ms

    Event_Array[23].p_fn_EventDriver = oi_Event2;
    Event_Array[23].ui_Scheduled_Occurrence = 4666;         //4666-4768 2ms
}

//-----------------------------------Event Drivers------------------------------------------------//

//------------------------------------------------------------------------------------------------//
Uint16 oi_Event1()      // Occurrence: 4,260 - SCI event
{
    oi_sciEvent();
    return SUCCESS;
}
//------------------------------------------------------------------------------------------------//
Uint16 oi_Event2()      // Occurrence: 125,253,381,509 - STATE event
{
    GpioDataRegs.GPCTOGGLE.bit.GPIO85 = 1;
    return SUCCESS;
}
//------------------------------------------------------------------------------------------------//
Uint16 oi_Event3() // Occurrence: 132,388 - Protection event
{
    return SUCCESS;
}
//------------------------------------------------------------------------------------------------//
Uint16 oi_Event4() // Occurrence: 132,388 - Protection event
{
    GpioDataRegs.GPCTOGGLE.bit.GPIO86 = 1;
    return SUCCESS;
}
//------------------------------------------------------------------------------------------------//
Uint16 oi_Event5()  // Occurrence: 132
{
    oi_tempSenseEvent();
    return SUCCESS;
}


