/*
 * ei_State.c
 *
 *  Created on: Dec 3, 2020
 *      Author: enArka
 */

/*****************************************************************************************************************
*                             Ohmium India Private Limited proprietary                                           *
******************************************************************************************************************/

//######################################### INCLUDE FILES #########################################################
#include "F2837xS_Device.h"
#include "oi_genStructureDefs.h"
#include "oi_globalPrototypes.h"


//#################################################################################################################
                                    /*-------Global Prototypes----------*/
// void ei_vInitState();

//#################################################################################################################
                                    /*-------Local Prototypes----------*/
static void oi_vNextState();

//##################### State Initialization ######################################################################
// Function Name: ei_vInitState
// Return Type: void
// Arguments:   void
// Description: State code Initialization. Micro-controller boots with INIT State. All system registers and global flags also
//  initialized here.
//#################################################################################################################
void oi_vInitState()
{
    // Global Flags
    Global_Flags.fg_Scheduled_General_Event_Flag = 0;

    // State Variables Initialization
    State.Previous_St = EI_ST_INIT;
    State.Present_St = EI_ST_INIT;
    State.Next_St = EI_ST_INIT;
    State.ui_Cycle_Counter = 0;

}

//################################### State Code ##################################################################
// Function Name: ei_minv_og_vState
// Return Type: void
// Arguments:   void
// Description: This function is a scheduled event that is responsible for transition of states. Various flags that
//  determine the state changes are set/reset in different functions. Based on the information from these routines,
//  if the present state needs to be continued or a different state needs to be taken. Moreover,
//  this function determines changeover to the reset state during faults is also done in this routine.
//#################################################################################################################
//################################# Description of Priority Concept ###############################################
//
// WHY DESCRIBED HERE: TBD TBD TBD TBD
//
// MOTIVATION FOR PRIORITY CONCEPT: TBD TBD TBD TBD
//
// TYPES OF PRIORITY AND BASIC IMPLEMENTATION NOTES: TBD TBD TBD TBD
//
//
// GRID PRIORITY: TBD TBD TBD TBD
//#################################################################################################################
void ei_minv_og_vState()
{
    //  Determine the Next State depending on the flags set/reset in the rest of the code
    oi_vNextState();

    switch (State.Present_St)
    {
    case EI_ST_INIT:
    {
        StateInit(&fec_state);
        Global_Flags.state_define_flag = 1;
        break;
    }

    case EI_ST_FAULT:
    {
        Global_Flags.state_define_flag = 2;
        break;
    }

    case EI_ST_STANDBY:
    {
        Global_Flags.state_define_flag = 3;
        break;
    }

    case EI_ST_ACTIVE:
    {
        Global_Flags.state_define_flag = 4;
        break;
    }

    default:
        break;
    }

}

//################################ Next State #####################################################################
// Function Name: ei_minv_og_vNextState
// Return Type: void
// Arguments:   void
// Description: Determines the next state depending on the various flags set/reset in different locations.
                    // States Implemented
//          EI_minv_og_ST_INIT
//          EI_minv_og_ST_MAINS
//          EI_minv_og_ST_INVERTER
//          EI_minv_og_ST_FAULT
//          EI_minv_og_ST_BYPASS
//#################################################################################################################
static void oi_vNextState()
{
    switch (State.Present_St)
    {
    case EI_ST_INIT:
    {

        if (INITIAL_DELAY_TIMER_EXPIRED)
        {
            //Switch to Forward state
            State.Next_St = EI_ST_FAULT;

            RESET_INITIAL_DELAY_TIMER;
        }
        else
        {
            INCREMENT_INITIAL_DELAY_TIMER;

        }
        break;
    }
    case EI_ST_FAULT:
    {
        if (INITIAL_DELAY_TIMER_2SEC_EXPIRED)
        {
            RESET_INITIAL_DELAY_TIMER;
            if (faults_h.bit.system_fault == 0)
            {
                Global_Flags.St_fault_flag = 0;
                State.Next_St = EI_ST_STANDBY;
            }
            else
            {
                Global_Flags.St_fault_flag = 1;
                State.Next_St = EI_ST_FAULT;

            }
        }
        else
        {
            INCREMENT_INITIAL_DELAY_TIMER;
        }
        break;
    }
    case EI_ST_STANDBY:
    {
        if ((Global_Flags.St_fault_flag == 0)
                && (GpioDataRegs.GPADAT.bit.GPIO4 == 1))
        {
            State.Next_St = EI_ST_ACTIVE;
//                Global_Flags.fg_Next_State = 1;
        }
        else if (faults_h.bit.system_fault == 1)
        {
            State.Next_St = EI_ST_FAULT;
        }
        else
        {
            State.Next_St = EI_ST_STANDBY;
        }
        break;
    }
    case EI_ST_ACTIVE:
    {

        if (faults_h.bit.system_fault == 1)
        {
            State.Next_St = EI_ST_FAULT;
        }
        else if (GpioDataRegs.GPADAT.bit.GPIO4 == 0)
        {
            State.Next_St = EI_ST_STANDBY;
        }
        else
        {
            State.Next_St = EI_ST_ACTIVE;
        }

        break;
    }

    default:
        break;
    }

// If the State.Next remains the same as State.Current no action needs to be taken
// However if two above mentioned states are different, the System is disabled
// and the state change flags, which track the time after a state change are reset
// to their initial states.
    if (State.Next_St == State.Present_St)
    {
        // asm ("NOP");
    }
    else
    {
        //ei_minv_og_vStateCycleFlagsReset();
    }
    State.Previous_St = State.Present_St;
    State.Present_St = State.Next_St;
}










