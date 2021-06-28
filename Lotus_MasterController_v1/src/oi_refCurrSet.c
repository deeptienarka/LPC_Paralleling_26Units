/*
 * oi_RefCurrSet.c
 *
 *  Created on: 10-Mar-2021
 *      Author: OHMLAP0037
 */


#include <oi_can.h>                  // CAN parameters include file
#include "F28x_Project.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_can.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/can.h"
#include "driverlib/debug.h"
#include "driverlib/interrupt.h"
#include "math.h"
#include "oi_externVariables.h"
#include "oi_genStructureDefs.h"
#include "oi_globalPrototypes.h"
#include "oi_state.h"

void CurrentSetLogic();

Uint16 NodePresent, PrevNodePresent;
Uint16 StatCnt = 0, statusCheckCurrentCounter = 0;

void CurrentSetLogic()
{
    PrevNodePresent = NodePresent;

    NodePresent = ActiveNodeRegs.bit.bt_node1 + ActiveNodeRegs.bit.bt_node2 + ActiveNodeRegs.bit.bt_node3 + ActiveNodeRegs.bit.bt_node4
            + ActiveNodeRegs.bit.bt_node5 + ActiveNodeRegs.bit.bt_node6 + ActiveNodeRegs.bit.bt_node7 + ActiveNodeRegs.bit.bt_node8 + ActiveNodeRegs.bit.bt_node9
            + ActiveNodeRegs.bit.bt_node10 + ActiveNodeRegs.bit.bt_node11 + ActiveNodeRegs.bit.bt_node12 + ActiveNodeRegs.bit.bt_node13 + ActiveNodeRegs.bit.bt_node14 + ActiveNodeRegs.bit.bt_node15
            + ActiveNodeRegs1.bit.bt_node16 + ActiveNodeRegs1.bit.bt_node17 + ActiveNodeRegs1.bit.bt_node18 + ActiveNodeRegs1.bit.bt_node19 + ActiveNodeRegs1.bit.bt_node20
            + ActiveNodeRegs1.bit.bt_node21 + ActiveNodeRegs1.bit.bt_node22 + ActiveNodeRegs1.bit.bt_node23 + ActiveNodeRegs1.bit.bt_node24 + ActiveNodeRegs1.bit.bt_node25
            + ActiveNodeRegs1.bit.bt_node26+ ActiveNodeRegs1.bit.bt_node27;





    if(NodePresent == 0)
    {
        CANA_Ui_Regs.Current_Set = 0;
    }

    CANA_Ui_Regs.Prev_Current_Set = CANA_Ui_Regs.Current_Set;

    if(State.Present_St == OI_STAND_BY)
    {
        CANA_Ui_Regs.Current_Set = 0;
    }
    else if(State.Present_St == OI_ARMED_POWER)
    {
        CANA_Ui_Regs.Total_Curr_Set = 0;

        CANA_Ui_Regs.Current_Set = CANA_Ui_Regs.Total_Curr_Set;
    }

    else if(State.Present_St == OI_STATUS_CHECK)
    {
        statusCheckCurrentCounter++;

        if (statusCheckCurrentCounter >= 100)
        {
            CANA_Ui_Regs.Total_Curr_Set += 1;

            statusCheckCurrentCounter = 0;
        }

        if(CANA_Ui_Regs.Total_Curr_Set >= SiteCANCommands.Max_Current_Reference)
        {
            CANA_Ui_Regs.Total_Curr_Set = SiteCANCommands.Max_Current_Reference;
        }

        CANA_Ui_Regs.Current_Set = CANA_Ui_Regs.Total_Curr_Set / NodePresent;
    }

    else if(State.Present_St == OI_STACK_POWER)
    {
        CANA_Ui_Regs.Total_Curr_Set = SiteCANCommands.Max_Current_Reference;

        CANA_Ui_Regs.Current_Set = CANA_Ui_Regs.Total_Curr_Set / NodePresent;
    }

}

