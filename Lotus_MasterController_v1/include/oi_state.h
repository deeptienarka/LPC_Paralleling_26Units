/*
 * oi_state.h
 *
 *  Created on: Feb 3, 2021
 *      Author: enArka
 */

#ifndef INCLUDE_OI_STATE_H_
#define INCLUDE_OI_STATE_H_

#include "F2837xS_Device.h"

#define INITIAL_DELAY_TIMER_EXPIRED                             (State_timers.ui_Intial_Delay_Timer >=100)
#define INCREMENT_INITIAL_DELAY_TIMER                            State_timers.ui_Intial_Delay_Timer++
#define RESET_INITIAL_DELAY_TIMER                                State_timers.ui_Intial_Delay_Timer = 100

// ENUMS
typedef enum
{
    OI_ST_INIT,
    OI_STAND_BY,
    OI_PURGE,
    OI_IO_POWER,
    OI_ARMED_POWER,
    OI_STATUS_CHECK,
    OI_STACK_POWER,
    OI_FAULT,
    OI_SAFE_SHUT_DOWN
} STATE_VALUES;



typedef struct
{
    STATE_VALUES Next_St;
    STATE_VALUES Present_St;
    STATE_VALUES Previous_St;
    Uint16 ui_Cycle_Counter;                    // 5ms second counter that counts the no. of 5ms that have elasped since a particular state has been attained.
}SYSTEM_STATE;                                              // Every 4 counts corresponds to 1 cycle.


typedef struct
{
    Uint16 ui_Intial_Delay_Timer;                               // Counts the no. of consecutive mains cycles for which the mains frequency is valid.
}STATE_TIMERS_t;


#endif /* INCLUDE_OI_STATE_H_ */
