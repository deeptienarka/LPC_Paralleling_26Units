/*
 * ei_pll.h
 *
 *  Created on: Mar 13, 2019
 *      Author: enArka
 */

#ifndef SOURCE_EI_PLL_H_
#define SOURCE_EI_PLL_H_

#include "F2837xS_Device.h"
#include "oi_initperipherals.h"
/** Structures ***********************************************************************************/
/**
 * @brief structure to hold zero crossing information for one phase
 */
typedef struct {
  Uint32 freq_in_cap_counts;     /**< frequency of the last cycle in capture counts */
  Uint16 phase_shift_index;      /**< phase shift (in theta_index) of the module with the '+'0-Xing of the reference phase */
}zx_t;

/**
 * @brief structure to hold various bit flags used in pll algorithms
 */
typedef struct {
  Uint16 capture_int_ack :1;  /**< pll interrupt received*/
  Uint16 freq_low        :1;  /**< grid frequency below MIN_PERMISSIBLE_GRID_FREQ */
  Uint16 freq_high       :1;  /**< grid frequency above MIN_PERMISSIBLE_GRID_FREQ */
  Uint16 capture_missing :1;  /**< capture interrupt not detected in a while */
  Uint16 pll_in_sync     :1;  /**< module synced with grid */
  Uint16 reserved        :11;
}pll_stat_t;


typedef struct {
   Uint16 my_pll_in_sync          :1;  /**< module synced with grid */
   Uint16 my_pll_out_of_sync     :1;  /**< module synced with grid */
  Uint16 reserved             :14;
}my_pll_status;

extern my_pll_status my_pll;

/**
 * @brief structure to hold pll timers. All timers count in line cycles (i.e) 1 line cycle
 * corresponds to 1 count for the timer.
 *
 * The timers will increment once per cycle on the reaching the specific condition. They will
 * stop counting and freeze on reaching the @ref MAX_ABNORMAL_PLL_CYCLES. They will get reset
 * to 0 once the specific abnormal condition clears.
 */
typedef struct {
  Uint16 cap_int_missing;   /**< timer to measure cap_int_missing cycles */
  Uint16 freq_out_of_range; /**< timer to measure freq_out_of_range cycles */
  Uint16 pll_out_of_sync ;  /**< timer to measure cycles in which we inverter is out_of_sync with reference */
}pll_timers_t;
/**
 * @brief structure to hold complete pll vars
 */
typedef struct {
  volatile zx_t zx;
  volatile pll_stat_t pll_stat;
  pll_timers_t pll_timers;
  Uint16 sync_index;
  float32 freq_Measure;
}pll_t;


/**
 * @brief complete state of the converter
 */
typedef struct
{
  int16 adc_result_regs;            /**< copy of AdcResultRegs. 12 bit result stored */
  float32 norm_filtered_value;      /**< Filtered value after applying EMA */
} adc_result_buf_t;

/**
 * @brief structure to hold ac measurements
 */
typedef struct {
    float32 norm_acc;     /**< RMS accumulator */
    float32 norm_rms;     /**< normalised rms value */
   //float32 norm_offset;  /**< Offset for rms calibration */
  // float32 gain;         /**< Gain for rms calibration */
}ac_metrics_t;

typedef struct
{
  volatile adc_result_buf_t adc_buffer[NO_OF_ADC_CHANNELS_MEASURED]; /**< adc result registers */
  volatile ac_metrics_t ac_channels[NO_OF_AC_CHANNELS]; /**< ac channel measurements */
  Uint16 theta_index;
  Uint16 theta_index_R;
  Uint16 theta_index_Y;
  Uint16 theta_index_B;
  Uint16 healthStatus;
  pll_t pll;
} fec_state_t;



typedef struct {  float32  Ref;             // Input: reference set-point
                  float32  Fbk;             // Input: feedback
                  float32  Out;             // Output: controller output
                  float32  Kp;              // Parameter: proportional loop gain
                  float32  TsKi;            // Parameter: integral gain * sampling time
                  float32  Umax;            // Parameter: upper saturation limit
                  float32  Umin;            // Parameter: lower saturation limit
                  float32  up;              // Data: proportional term
                  float32  ui;              // Data: integral term
                  float32  v1;              // Data: pre-saturated controller output
                  float32  i1;              // Data: integrator storage: ui(k-1)
                  float32  w1;              // Data: saturation record: [u(k-1) - v(k-1)]
                } PI_CONTROLLER;



                #define RESET_PI(v)                 \
                    v.i1 = 0;     \
                    v.v1 = 0;         \
                    v.Out = 0;               \
                    v.ui = 0;               \



#define PI_MACRO(v)                                           \
                                                                \
    /* proportional term */                                     \
    v.up = v.Ref - v.Fbk;                                       \
                                                                \
    /* integral term */                                         \
    v.ui = (v.Out == v.v1)?( (v.TsKi * v.up)+ v.i1) : ( (v.TsKi * ( v.up + ( 500.0*(v.Out-v.v1) )) ) + v.i1); \
/*    v.ui = (v.Out == v.v1)?( (v.TsKi * v.up)+ v.i1) : v.i1;*/     \
    v.i1 = v.ui;                                                \
                                                                \
    /* control output */                                        \
    v.v1 = (v.Kp * v.up) + v.ui;                           \
    /* Saturate PI Output */    \
    if(v.Out > v.Umax) \
        { \
            v.Out = v.Umax; \
        } \
    else if(v.Out < v.Umin) \
        { \
             v.Out = v.Umin; \
         } \
         else v.Out = v.v1;\
/**
 * @brief structure to store fault register
 */
typedef struct {
  Uint32 dc_bus_overcurrent:1;  // bit8
  Uint32 grid_current_violation:1; // bit9
  Uint32 inst_grid_rstcurrent_violation:1; // bit10
  Uint32 system_fault:1; // bit11
  Uint32 over_temp_voilation:1; // bit12
  Uint32 freq_out_of_Range:1; // bit13
  Uint32 inputphase_missing:1; // bit14
  Uint32 healthStatus:1;      // bit15
} FAULT_t_h;

typedef struct{
    Uint32 grid_ac_undervolt:1;    // bit0
    Uint32 hardware_fault:1;       // bit1
    Uint32 grid_ac_overvolt:1;     // bit2
    Uint32 grid_ac_rstovervolt:1;  // bit3
    Uint32 dc_bus_overvolt:1;      // bit4
    Uint32 dc_bus_rstovervolt:1;   // bit5
    Uint32 dc_bus_undervolt:1;     // bit6
    Uint32 dc_bus_undervolt1:1 ;   // bit7
}FAULT_t_l;

typedef struct  // FULL SCALE ADC VALUES
{
    float32 vdc_ui;
    float32 VREF;
    float32 vdc_Kp;
    float32 vdc_Ki;
    float32 vdc_up;
    float32 vdc_v1;
    float32 vdc_v1_disp;
    float32 voltage_loop_out;
    float32 Vbus_ref;
    float32 voltage_ref;
    float32 power_ref_per_ph;
    float32 UNIT_SINE_R_FINAL;
    float32 DAC_CMPVAL1;
    float32 DAC_CMPVAL2;
    float32 Vdc_bus1;
    float32 Iref_Rph;
    float32 Iref_Yph;
    float32 Iref_Bph;
    float32 Vrms_Rph;
    float32 Vrms_Yph;
    float32 Vrms_Bph;
    int16   DCDC_On;
}Control;

typedef struct
{
    float32 alpha;
    float32 beta;
    float32 d_axis;

    float32 q_axis;
    float32 I_q_ff;
    float32 I_d_ff;
    float32 I_filtered;
    float32 Vll_rms;
    float32 Vll_rms_filtered;
    float32 Kpi;
}ABC_TO_DQ_CONTROL;

typedef struct
{
    float32 d_axis;
    float32 q_axis;
    float32 alpha;
    float32 beta;
    float32 R_phase;
    float32 Y_phase;
    float32 B_phase;

}DQ_TO_ABC_CONTROL;


typedef struct
{
    float32 Vin_ac_ry;
    float32 Iin_ac_rn1;
    float32 Vin_ac_yb;
    float32 Iin_ac_yn1;
    float32 Vin_ac_br;
    float32 Iin_ac_bn1;
    float32 V_R_PHASE;
    float32 V_Y_PHASE;
    float32 V_B_PHASE;
}AC_FEEDBACK;

typedef struct
{
    float32 Vdcbus_neg;
    float32 Vdcbus_pos;
    float32 Vdc_tot_feedback;
    float32 Idc_out;
}DC_FEEDBACK;

/**
 * @brief bit and word access to Fault regs
 */
typedef union {
  FAULT_t_h bit;
  Uint32 all;
} fault_regs_t_h;

typedef union {
  FAULT_t_l bit;
  Uint32 all;
} fault_regs_t_l;


typedef struct  // FULL SCALE ADC VALUES
{
 Uint16 Seq_chk_flag;
 Uint16 PosIndFlag;
 Uint16 NegIndFlag;
 Uint16 theta_index_initflag;
 Uint16 phase_Seq_change_indflag;

}Phase_Seq;




typedef struct  // Various state timers implemented in the project
{
    Uint16 ui_ac_undervoltage_Timer;
    Uint16 ui_ac_overvoltage_Timer;
    Uint16 ui_Iin_Overload_Timer;                             // Timer for Iin_overload_limit_110.
    Uint16 ui_grid_reconnect_timer;
    Uint16 ui_dc_bus_disconnect_timer;
    Uint16 ui_dc_bus_overvolt_timer;
    Uint16 ui_dc_bus_undervolt_timer;
    Uint16 ui_dc_overcurrent_timer;
    Uint16 sys_out_relay_timer;
    Uint16 otp_sense_timer;
    Uint16 freq_out_of_range_timer;
    Uint16 inputphase_missing_timer;
    Uint16  isr_dc_ov_timer;
    Uint16  inst_ac_oc_timer;
    Uint16  latch_timer;
}TIMERS;


typedef struct  // Various state timers implemented in the project
{
    float32 VOLTAGE_NTC;
    float32 RES_NTC;
    float32 TEMP_MEASURED_IN_DEG;
    float32 BETA_NTC;
    float32 TEMP_25_DEG;
    float32 RES_25DEG;
    float32 temp_count;
}Temp_Meas_t;


typedef struct
{
    float32 ACVoltage;
    float32 ACCurrent;
    float32 DCVoltage;
    float32 DCCurrent;
}CIRCUIT_GAIN;

/** Global functions *****************************************************************************/
extern void PllInit(pll_t *spll);
extern void Pll(pll_t *spll);

/** Constants ************************************************************************************/
#define MAX_INCREMENTAL_CHANGE_IN_COUNTS                1000
#define MAX_CHANGE_FROM_LINE_FREQUENCY_ALLOWED_SYNC     20.0

#define LINE_NOMINAL_FREQUENCY_IN_HZ                    50
#define SYSCLK_FREQUENCY_IN_MHZ                         200
#define SYSCLK_FREQUENCY_IN_HZ                          (SYSCLK_FREQUENCY_IN_MHZ * 1e6)
#define NO_OF_MINOR_CYCLES_PER_LINE_CYCLE               1024


#define FREQ_UPPER_LIMIT  (LINE_NOMINAL_FREQUENCY_IN_HZ + MAX_CHANGE_FROM_LINE_FREQUENCY_ALLOWED_SYNC) /* eg: 60.0 + 4.0 */
#define FREQ_LOWER_LIMIT  (LINE_NOMINAL_FREQUENCY_IN_HZ - MAX_CHANGE_FROM_LINE_FREQUENCY_ALLOWED_SYNC) /* eg: 60.0 - 4.0 */


/**
 * eg. calculations for TBPRD RANGE
 * tbprd_upper_limit = (150M/(56.0*512)) - 1
 * tbprd_lower_limit = (150M/(64.0*512)) - 1
 */
#define TBPRD_UPPER_LIMIT ((SYSCLK_FREQUENCY_IN_HZ/(4*FREQ_LOWER_LIMIT*NO_OF_MINOR_CYCLES_PER_LINE_CYCLE)))
#define TBPRD_LOWER_LIMIT ((SYSCLK_FREQUENCY_IN_HZ/(4*FREQ_UPPER_LIMIT*NO_OF_MINOR_CYCLES_PER_LINE_CYCLE)))


/**
 * eg. calculations for CAPTURE RANGE
 * capture_upper_limit = (150M/56.0)
 * capture_lower_limit = (150M/64.0)
 */
#define CAPTURE_UPPER_LIMIT (long)(SYSCLK_FREQUENCY_IN_HZ/FREQ_LOWER_LIMIT)
#define CAPTURE_LOWER_LIMIT (long)(SYSCLK_FREQUENCY_IN_HZ/FREQ_UPPER_LIMIT)

#define GRID_FREQ_OVER()  (new_capture < CAPTURE_LOWER_LIMIT)
#define GRID_FREQ_UNDER() (new_capture > CAPTURE_UPPER_LIMIT)

#define GRID_FREQ_GOOD(A) ((A.freq_low == 0) && (A.freq_high == 0))
#define GRID_FREQ_BAD(A)  ((A.freq_low == 1) || (A.freq_high == 1))

#define IN_SYNC     1
#define OUT_OF_SYNC 0

/** PLL Timers ***********************************************************************************/
#define MAX_CAP_INT_MISSING_CYCLES_ALLOWED  3  //10
#define MAX_ABNORMAL_FREQ_CYCLES_ALLOWED    3  //10

/** function macros ******************************************************************************/
#define ACK_CAP_INT(A)          (A.capture_int_ack = 1)
#define NO_CAPTURE_INT(A)       (A.capture_int_ack == 0)
#define RESET_CAP_INT_ACK(A)    (A.capture_int_ack = 0)
#define GET_PRESENT_TIMER_PRD() (EPWM2_TIMER_TBPRD)  // +1 for up counter

#define GRID_NOT_PRESENT(A)    (A.capture_missing == 1)

/*
 * PHASE CHECK
 * 1. Phase lead: converter in positive half cycle at capture interrupt
 * 2. Phase lag:  converter in negative half cycle at capture interrupt
 */
#define PHASE_LEAD(A)                           ((A>0) && (A<=NO_OF_MINOR_CYCLES_PER_LINE_CYCLE/2))
#define PHASE_LAG(A)                            ((A>NO_OF_MINOR_CYCLES_PER_LINE_CYCLE/2) && (A < (NO_OF_MINOR_CYCLES_PER_LINE_CYCLE)))

#define ac_inst(A)                              (A.adc_result_regs)
#define EMA_FILTER(A)                           (A.norm_filtered_value = A.norm_filtered_value + ((((A.adc_result_regs))-A.norm_filtered_value)/FILTER_VALUE))


#define square(A)                               (A*A)
#define mean_full_cycle(A)                      (A = (A / 256))

#define HW_OFFSET_VRY                           2046
#define HW_OFFSET_VYB                           2042
#define HW_OFFSET_VBR                           2041

#define HW_OFFSET_IR                           2057
#define HW_OFFSET_IY                           2056
#define HW_OFFSET_IB                           2051
#define HW_OFFSET_IOUT                         2027
#define FILTER_VALUE                            16

#define AC_UNDER_VOLTAGE_LIMIT                  350
#define AC_OVER_VOLTAGE_LIMIT                   500

#define DC_OVER_VOLTAGE_LIMIT                   840
#define DC_OVER_VOLTAGE_LIMIT1                  425
#define AC_OVER_CUR_LIMIT                       42.0   //testing purpose
#define OTP_TRIP_LIMIT                          75.0    //testing purpose
#define OTP_RECOVERY_LIMIT                      60.0    //testing purpose

#define DC_OVER_CURRENT_LIMIT                   23.0


#define INST_DC_OVER_VOLTAGE_LIMIT              425
#define OV_COUNTER_LIMIT                        3

#define RELAY_ON                (GPIO_WritePin(4,1))
#define RELAY_OFF               (GPIO_WritePin(4,0))
//
#define PULSE_ON                (GPIO_WritePin(17,0))
#define PULSE_OFF               (GPIO_WritePin(17,1))

#define DSP_BEAT_TOGGLE               (GpioDataRegs.GPBTOGGLE.bit.GPIO59 = 1)


#endif /* SOURCE_EI_PLL_H_ */
