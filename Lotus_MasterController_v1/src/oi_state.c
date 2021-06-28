/*
 * oi_state.c
 *
 *  Created on: Feb 3, 2021
 *      Author: enArka
 */


//######################################### INCLUDE FILES #########################################################

#include "F2837xS_Device.h"
#include "oi_State.h"
#include"oi_genStructureDefs.h"
#include "oi_externVariables.h"

//#################################################################################################################
/*-------Local Prototypes----------*/
static void oi_vNextState();

//##################### State Initialization ######################################################################
// Function Name: oi_vInitState
// Return Type: void
// Arguments:   void
// Description: State code Initialization. Micro-controller boots with INIT State. All system registers and global flags also
//  initialized here.

Uint16 PurgeToIO1 = 0;
//#################################################################################################################
void oi_vInitState()
{
    // Global Flags
    Global_Flags.fg_Scheduled_General_Event_Flag = 0;

    // State Variables Initialization
    State.Previous_St = OI_STAND_BY;
    State.Present_St = OI_STAND_BY;
    State.Next_St = OI_STAND_BY;
    State.ui_Cycle_Counter = 0;
    CANA_Ui_Regs.Voltage_Set = 0;
    CANA_Ui_Regs.Current_Set = 0;
    QueryType.PSU = SET_VOLT;
    QueryType.VS = IDLE_VS;
    QueryType.IO = IDLE_IO;

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
//#################################################################################################################
void oi_vState()
{
    //  Determine the Next State depending on the flags set/reset in the rest of the code
    oi_vNextState();

    switch (State.Present_St)
    {
    case OI_ST_INIT:
    {
        oi_vInitState();
        Global_Flags.state_define_flag = 1;
        break;
    }
    case OI_STAND_BY:
    {
        Global_Flags.state_define_flag = 2;
        GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;

        break;
    }

    case OI_PURGE:
    {
        Global_Flags.state_define_flag = 3;


        break;
    }

    case OI_IO_POWER:
     {
        Global_Flags.state_define_flag = 4;
        GpioDataRegs.GPASET.bit.GPIO2 = 1;  // i/o POWER on
        GpioDataRegs.GPASET.bit.GPIO3 = 1;  // VFD control on
        break;
     }

    case OI_ARMED_POWER:
     {
         Global_Flags.state_define_flag = 5;
         GpioDataRegs.GPASET.bit.GPIO5 = 1;  // PLC turn on
         break;
     }

    case OI_STATUS_CHECK:
    {
        Global_Flags.state_define_flag = 6;
       // QueryType.PSU = SET_VOLT;

        QueryType.VS = MEAS_CELL_VOLT;

        break;
    }

    case OI_STACK_POWER:
    {
        Global_Flags.state_define_flag = 7;

        break;
    }

    case OI_FAULT:
    {
        Global_Flags.state_define_flag = 8;
        break;
    }

    case OI_SAFE_SHUT_DOWN:
    {
        Global_Flags.state_define_flag = 9;
        break;
    }

    default:
    break;
    }
}

//################################ Next State #####################################################################
// Function Name: oi_vNextState
// Return Type: void
// Arguments:   void
// Description: Determines the next state depending on the various flags set/reset in different locations.
                    // States Implemented

//#################################################################################################################
static void oi_vNextState()
{
    switch(State.Present_St)
    {

    case OI_ST_INIT:
        {

            if (INITIAL_DELAY_TIMER_EXPIRED)
            {
                //Switch to Forward state
                State.Next_St = OI_STAND_BY;

                RESET_INITIAL_DELAY_TIMER;
            }
            else
            {
                INCREMENT_INITIAL_DELAY_TIMER;

            }
            break;
        }
    case OI_STAND_BY:
    {
       if(SiteCANCommands.StateChange == 1)
       {
           //Switch to Forward state
           State.Next_St = OI_PURGE;

           SiteCANCommands.StateChangeStandBy = 1;
       }

        break;
    }
    case OI_PURGE:
    {

        if(SiteCANCommands.StateChange == 2)
        {
            State.Next_St = OI_IO_POWER;
        }

        if(SiteCANCommands.StateChange == 7)
        {
            State.Next_St = OI_STAND_BY;

            SiteCANCommands.StateChangeStandBy = 1;
        }
        break;
    }
    case OI_IO_POWER:
    {

        if(SiteCANCommands.StateChange == 3)
        {
            State.Next_St = OI_ARMED_POWER;
        }


        if(SiteCANCommands.StateChange == 7)
        {
            State.Next_St = OI_STAND_BY;

            SiteCANCommands.StateChangeStandBy = 1;
        }
        break;
    }
    case OI_ARMED_POWER:
    {
        if(SiteCANCommands.StateChange == 4)
        {
            State.Next_St = OI_STATUS_CHECK;
        }


        if(SiteCANCommands.StateChange == 7)
        {
            State.Next_St = OI_STAND_BY;

            SiteCANCommands.StateChangeStandBy = 1;
        }
        break;
    }
    case OI_STATUS_CHECK:
    {
        if(SiteCANCommands.StateChange == 5)
        {
            State.Next_St = OI_STACK_POWER;
        }

        if(SiteCANCommands.StateChange == 7)
        {
            State.Next_St = OI_STAND_BY;

            SiteCANCommands.StateChangeStandBy = 1;
        }
        break;
    }

    case OI_STACK_POWER:
    {

        if(SiteCANCommands.StateChange == 7)
        {
            State.Next_St = OI_STAND_BY;

            SiteCANCommands.StateChangeStandBy = 1;
        }
        break;
    }

    case OI_FAULT:
    {

        break;
    }

    case OI_SAFE_SHUT_DOWN:
    {

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
