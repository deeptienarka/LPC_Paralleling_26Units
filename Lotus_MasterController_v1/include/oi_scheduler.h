/*
 * oi_scheduler.h
 *
 *  Created on: Dec 2, 2020
 *      Author: enArka
 */

#ifndef INCLUDE_OI_SCHEDULER_H_
#define INCLUDE_OI_SCHEDULER_H_

#include "F2837xS_Device.h"

#define TOTAL_NO_OF_EVENTS  22
// Structure to store Event related Info
typedef struct
{
    Uint16 ui_Scheduled_Occurrence;
    Uint16 (*p_fn_EventDriver)();
}EVENT;

// Global Flags

typedef struct
{
    Uint32 fg_Scheduled_General_Event_Flag:1;                   // Scheduled event activated?
    Uint32 St_fault_flag : 1;
    Uint16 state_define_flag;

}GLOBAL_FLAGS;



#endif /* INCLUDE_OI_SCHEDULER_H_ */
