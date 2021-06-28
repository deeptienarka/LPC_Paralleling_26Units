/*
 * oi_State.h
 *
 *  Created on: Dec 3, 2020
 *      Author: enArka
 */

#ifndef INCLUDE_OI_STATE_H_
#define INCLUDE_OI_STATE_H_


#define INITIAL_DELAY_TIMER_EXPIRED (State_timers.ui_Intial_Delay_Timer >=100)
#define INITIAL_DELAY_TIMER_2SEC_EXPIRED (State_timers.ui_Intial_Delay_Timer >=200)
#define INCREMENT_INITIAL_DELAY_TIMER State_timers.ui_Intial_Delay_Timer++
#define RESET_INITIAL_DELAY_TIMER State_timers.ui_Intial_Delay_Timer = 200


// ENUMS
typedef enum
{
    EI_ST_INIT,
    EI_ST_FAULT,
    EI_ST_STANDBY,
    EI_ST_ACTIVE,
} STATE_VALUES;



typedef struct
{
   // struct SYSTEM_MODE Mode;
    STATE_VALUES Next_St;
    STATE_VALUES Present_St;
    STATE_VALUES Previous_St;
    Uint16 ui_Cycle_Counter;                    // 5ms second counter that counts the no. of 5ms that have elasped since a particular state has been attained.
}SYSTEM_STATE;


typedef struct
{
    Uint16 ui_Intial_Delay_Timer;                               // Counts the no. of consecutive mains cycles for which the mains frequency is valid.
    uint16_t timer1;
}STATE_TIMERS_t;

#endif /* INCLUDE_OI_STATE_H_ */
