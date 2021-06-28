/*****************************************************************************************************************
 *                             enArka India Private Limited proprietary                                           *
 * File: ei_InitPeripherals.c                                                                                     *
 * Copyright (c) 2019 by enArka Instruments Pvt. Ltd.                                                             *
 * All Rights Reserved                                                                                            *
 * Description: This file does the CAN communication operation                                                    *
 *****************************************************************************************************************/

/*****************************************************************************************************************
 * Include Files                                                                                                  *
 *****************************************************************************************************************/

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

Uint32 x = 0;
Uint16 i = 1;
Uint16 queryType = 0;
Uint16 canQueryCount = 0;
Uint16 statCheckCnt = 0;
Uint16 IOCANcounter = 0;
Uint16 CurrentCnt = 0;
Uint16 PurgeCnt = 0;
Uint16 StandByCnt = 0;
extern Uint32 cnt1, cnt2;
extern Uint16 trig1, trig2;

void oi_CANA_TX_Event_PSU();
void oi_CANA_RX_Event_PSU();
void CANA_TimeoutFaultsLogic();
void CANA_NodeCheckLogic();
void Turn_ON_Node_specific(int32 nodeID, int32 ON_command, int32 global);
void CANGlobalONOFFCommand(int16 ON_command);
void Save_Settings_Node_specific(int32 nodeID);
void Set_local_Node_specific(int32 nodeID);
void Set_Voltage_Node_specific(int32 nodeID, float32 voltage_Value,
                               Uint16 global);
void Set_Current_Node_specific(int32 nodeID, float32 current_Value,
                               Uint16 global);
void Set_Current_Global(float32 current_value);
void Query_ProgParams_Node_Specific(int32 nodeID);
void Query_MeasParams_Node_Specific(int32 nodeID);
void Query_Flts_Node_Specific(int32 nodeID);
void oi_CANA_InitRead_PSU();

void oi_CANA_TX_Event_PSU()
{

    switch (State.Present_St)
    {

    case OI_STAND_BY:

        if(((QueryType.PSU == SET_VOLT)
                ||(QueryType.PSU == SET_CURR)
                ||(QueryType.PSU == QUERY_PROGPARAM)
                ||(QueryType.PSU == QUERY_OP_PARAM)
                ||(QueryType.PSU == TURN_ONFEC)
                ||(QueryType.PSU == TURN_ON)
                ||(QueryType.PSU == QUERY_FLTS))
                && (SiteCANCommands.StateChangeStandBy == 1))
        {
            QueryType.PSU = TURN_OFF;

            SiteCANCommands.StateChangeStandBy = 0;

            CANA_timers.TX_count = 0;
        }

        if(QueryType.PSU == TURN_OFF)
        {
            CANA_timers.TX_count++;

            Turn_ON_Node_specific(0, TURN_OFF_PSU, 1);

            if (CANA_timers.TX_count > 10)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = IDLE_PSU;
            }
        }

        else if (QueryType.PSU == IDLE_PSU)
        {
            QueryType.PSU = SET_VOLT;
        }

        else if (QueryType.PSU == SET_VOLT)
        {
            CANA_Ui_Regs.Voltage_Set = 0;

            CANA_timers.TX_count++;

            Set_Voltage_Node_specific(0, CANA_Ui_Regs.Voltage_Set, 1);

            if(CANA_timers.TX_count > 10)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = SET_CURR;

            }

        }
        else if (QueryType.PSU == SET_CURR)
        {
            CANA_Ui_Regs.Current_Set = 0;

            CANA_timers.TX_count++;

            Set_Current_Node_specific(0, CANA_Ui_Regs.Current_Set, 1);

            if(CANA_timers.TX_count > 10)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = QueryType.PSU = QUERY_PROGPARAM;
            }
        }
        break;

    case OI_PURGE:


        if (QueryType.PSU == QUERY_PROGPARAM)
        {
            CANA_timers.TX_count++;

            Query_ProgParams_Node_Specific(CANA_timers.TX_count);

            if (CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = QUERY_PROGPARAM;
            }
        }
        break;

    case OI_IO_POWER:

        if(QueryType.PSU == QUERY_PROGPARAM)
        {
            QueryType.PSU = QUERY_OP_PARAM;
        }

        else if (QueryType.PSU == QUERY_OP_PARAM)
        {
            CANA_timers.TX_count++;

            Query_MeasParams_Node_Specific(CANA_timers.TX_count);

            if (CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = SET_VOLT;
            }
        }
        else if (QueryType.PSU == SET_VOLT)
        {
            CANA_timers.TX_count++;

            Set_Voltage_Node_specific(0,SiteCANCommands.Voltage_Reference,1);

            if(CANA_timers.TX_count > 10)
            {
                QueryType.PSU = TURN_ONFEC;

                CANA_timers.TX_count = 0;
            }
        }
        else if(QueryType.PSU == TURN_ONFEC)
        {
            CANA_timers.TX_count++;

            Turn_ON_Node_specific(0, TURN_ON_FEC, 1);

            if(CANA_timers.TX_count > 10)
            {
                QueryType.PSU = QUERY_OP_PARAM;

                CANA_timers.TX_count = 0;
            }
        }
        break;

    case OI_ARMED_POWER:

        if((QueryType.PSU == TURN_ONFEC)
                ||(QueryType.PSU == SET_VOLT))
        {
            QueryType.PSU = TURN_ON;
        }

        else if (QueryType.PSU == TURN_ON)
        {
            Turn_ON_Node_specific(0, TURN_ON_PSU, 1);

            CANA_timers.TX_count++;

            if(CANA_timers.TX_count > 10)
            {
                QueryType.PSU = QUERY_FLTS;

                CANA_timers.TX_count = 0;
            }
        }
        else if (QueryType.PSU == QUERY_FLTS)
        {
            CANA_timers.TX_count++;

            Query_Flts_Node_Specific(CANA_timers.TX_count);

            if (CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = QUERY_OP_PARAM;
            }
        }

        else if (QueryType.PSU == QUERY_OP_PARAM)
        {
            CANA_timers.TX_count++;

            Query_MeasParams_Node_Specific(CANA_timers.TX_count);

            if (CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = SET_CURR;
            }
        }
        else if (QueryType.PSU == SET_CURR)
        {

            Set_Current_Node_specific(0, CANA_Ui_Regs.Current_Set, 1);

            CANA_timers.TX_count++;

            if(CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
            {
                QueryType.PSU = TURN_ON;

                CANA_timers.TX_count = 0;
            }
        }

        break;

    case OI_STATUS_CHECK:

        if(QueryType.PSU == TURN_ON)
        {
            QueryType.PSU = QUERY_OP_PARAM;
        }

        if(QueryType.PSU == QUERY_OP_PARAM)
        {
            CANA_timers.TX_count++;

            Query_MeasParams_Node_Specific(CANA_timers.TX_count);

            if (CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = QUERY_FLTS;
            }
        }
        else if (QueryType.PSU == QUERY_FLTS)
        {
            CANA_timers.TX_count++;

            Query_Flts_Node_Specific(CANA_timers.TX_count);

            if (CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = SET_CURR;
            }
        }
        else if (QueryType.PSU == SET_CURR)
        {
            CANA_timers.TX_count++;

            Set_Current_Node_specific(0, CANA_Ui_Regs.Current_Set, 1);

            if(CANA_timers.TX_count > 10)
            {
                QueryType.PSU = QUERY_OP_PARAM;

                CANA_timers.TX_count = 0;
            }
        }
        break;

    case OI_STACK_POWER:

        if(QueryType.PSU == QUERY_OP_PARAM)
        {
            CANA_timers.TX_count++;

            Query_MeasParams_Node_Specific(CANA_timers.TX_count);

            if (CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = SET_VOLT;
            }
        }
        else if (QueryType.PSU == SET_VOLT)
        {
            CANA_timers.TX_count++;

            Set_Voltage_Node_specific(0,SiteCANCommands.Voltage_Reference,1);

            if(CANA_timers.TX_count > 10)
            {
                QueryType.PSU = SET_CURR;

                CANA_timers.TX_count = 0;
            }
        }
        else  if (QueryType.PSU == SET_CURR)
        {
            CANA_timers.TX_count++;

            Set_Current_Node_specific(0,CANA_Ui_Regs.Current_Set, 1);

            if(CANA_timers.TX_count > 10)
            {
                QueryType.PSU = TURN_ONFEC;

                CANA_timers.TX_count = 0;
            }
        }
        else if(QueryType.PSU == TURN_ONFEC)
        {
            CANA_timers.TX_count++;

            Turn_ON_Node_specific(0, TURN_ON_FEC, 1);

            if(CANA_timers.TX_count > 10)
            {
                QueryType.PSU = TURN_ON;

                CANA_timers.TX_count = 0;
            }
        }
        else if (QueryType.PSU == TURN_ON)
        {
            Turn_ON_Node_specific(0, TURN_ON_PSU, 1);

            CANA_timers.TX_count++;

            if(CANA_timers.TX_count > 10)
            {
                QueryType.PSU = QUERY_FLTS;

                CANA_timers.TX_count = 0;
            }
        }
        else if (QueryType.PSU == QUERY_FLTS)
        {
            CANA_timers.TX_count++;

            Query_Flts_Node_Specific(CANA_timers.TX_count);

            if (CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
            {
                CANA_timers.TX_count = 0;

                QueryType.PSU = QUERY_OP_PARAM;
            }
        }


        break;

    default:
        break;
    }

    if(SiteCANCommands.OFF_Command == 0x55)
    {
        QueryType.PSU = TURN_OFF;
    }
    else if(SiteCANCommands.OFF_Command == 0xAA)
    {
        QueryType.PSU = IDLE_PSU;
    }

    if (QueryType.PSU == TURN_OFF)
    {
        CANA_timers.TX_count++;

        Turn_ON_Node_specific(0, TURN_OFF_PSU, 1);

        if (CANA_timers.TX_count > 10)
        {
            CANA_timers.TX_count = 0;

            QueryType.PSU = IDLE_PSU;
        }

    }

    if (QueryType.PSU == SAVE_SETTINGS)
    {
        CANA_timers.TX_count++;
        Save_Settings_Node_specific(CANA_timers.TX_count);

        if (CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
        {
            CANA_timers.TX_count = 0;
        }
    }

    else if (QueryType.PSU == LOCAL_REMOTE)
    {
        CANA_timers.TX_count++;
        Set_local_Node_specific(CANA_timers.TX_count);

        if (CANA_timers.TX_count > TOTAL_PSU_NODE_NUMBER)
        {
            CANA_timers.TX_count = 0;
        }
    }
}

void oi_CANA_RX_Event_PSU()
{
    if(trig2 == 1)
    {
      ActiveNodeRegs.all = 0;
      ActiveNodeRegs1.all = 0;
      CanaRead.MsgId1_PSU = 0x00;
      CanaRead.MsgId2_PSU = 0x00;
      CanaRead.MsgId3_PSU = 0x00;
      CanNode.PSU_Node = 0;
      CanNode.Hi_PSU_Node = 0;
    }

    oi_CANA_InitRead_PSU();


    if (CanaRead.MBox5 == 1)  // Measured Output Parameters
    {
        CAN_readMessage(CANA_BASE, MAILBOX_5, RxMsgData);

        cnt2 = 0;

        CanaRead.MsgId1_PSU = CanaRegs.CAN_IF2ARB.bit.ID;
        CanaRead.MsgId1_PSU = CanaRead.MsgId1_PSU & CAN_NODEID_MASK;
        CanNode.PSU_Node = (Uint16) (CanaRead.MsgId1_PSU >> 8);

        CANRecdParams.Volt_int[CanNode.PSU_Node] = ((RxMsgData[0] >> 4) * 1000)
                + ((RxMsgData[0] & 0X0F) * 100) + ((RxMsgData[1] >> 4) * 10)
                + ((RxMsgData[1] & 0x0F) * 1);

        CANRecdParams.Volt_frac[CanNode.PSU_Node] = ((RxMsgData[2] >> 4) * 1000)
                + ((RxMsgData[2] & 0X0F) * 100) + ((RxMsgData[3] >> 4) * 10)
                + ((RxMsgData[3] & 0x0F) * 1);
        CANRecdParams.Volt_Node[CanNode.PSU_Node] =
                CANRecdParams.Volt_int[CanNode.PSU_Node]
                        + (CANRecdParams.Volt_frac[CanNode.PSU_Node] * 0.0001);

        CANRecdParams.Curr_int[CanNode.PSU_Node] = ((RxMsgData[4] >> 4) * 1000)
                + ((RxMsgData[4] & 0X0F) * 100) + ((RxMsgData[5] >> 4) * 10)
                + ((RxMsgData[5] & 0x0F) * 1);

        CANRecdParams.Curr_frac[CanNode.PSU_Node] = ((RxMsgData[6] >> 4) * 1000)
                + ((RxMsgData[6] & 0X0F) * 100) + ((RxMsgData[7] >> 4) * 10)
                + ((RxMsgData[7] & 0x0F) * 1);
        CANRecdParams.Curr_Node[CanNode.PSU_Node] =
                CANRecdParams.Curr_int[CanNode.PSU_Node]
                        + (CANRecdParams.Curr_frac[CanNode.PSU_Node] * 0.0001);

        ActiveNodeRegs.all = ActiveNodeRegs.all | (1 << CanNode.PSU_Node);

        if(CanNode.PSU_Node < 16)
        {
            ActiveNodeRegs.all = ActiveNodeRegs.all | (1 << CanNode.PSU_Node);
        }
        else
        {
            CanNode.Hi_PSU_Node = (CanNode.PSU_Node) - 15;

            ActiveNodeRegs1.all = ActiveNodeRegs1.all | (1<<CanNode.Hi_PSU_Node);
        }

        CANA_NodeCheckLogic();

        CanaRead.MBox5 = 0;
    }

    else if (CanaRead.MBox6 == 1)  // Faults
    {
        CAN_readMessage(CANA_BASE, MAILBOX_6, RxMsgData);

        cnt2 = 0;

        CanaRead.MsgId2_PSU = CanaRegs.CAN_IF2ARB.bit.ID;
        CanaRead.MsgId2_PSU = CanaRead.MsgId2_PSU & CAN_NODEID_MASK;
        CanNode.PSU_Node = (Uint16) (CanaRead.MsgId2_PSU >> 8);

        FaultRegs[CanNode.PSU_Node].all =
                ((RxMsgData[0] << 8) | (RxMsgData[1]));

        FaultRegs[CanNode.PSU_Node].all =
                ((RxMsgData[4] << 8) | (RxMsgData[5]));

        if (FaultRegs[CanNode.PSU_Node].bit.bt_PFC_DC_Bus_Fault == 1)
        {
            DcFaultRegs.all = DcFaultRegs.all | (1 << CanNode.PSU_Node);
        }

        ActiveNodeRegs.all = ActiveNodeRegs.all | (1 << CanNode.PSU_Node);

        if(CanNode.PSU_Node < 16)
        {
            ActiveNodeRegs.all = ActiveNodeRegs.all | (1 << CanNode.PSU_Node);
        }
        else
        {
            CanNode.Hi_PSU_Node = (CanNode.PSU_Node) - 15;

            ActiveNodeRegs1.all = ActiveNodeRegs1.all | (1<<CanNode.Hi_PSU_Node);
        }

        CANA_NodeCheckLogic();

        CanaRead.MBox6 = 0;

    }

    else if (CanaRead.MBox7 == 1)  // Programmed Parameters
    {

        CAN_readMessage(CANA_BASE, MAILBOX_7, RxMsgData);

        cnt2 = 0;

        CanaRead.MsgId3_PSU = CanaRegs.CAN_IF2ARB.bit.ID;
        CanaRead.MsgId3_PSU = CanaRead.MsgId3_PSU & CAN_NODEID_MASK;
        CanNode.PSU_Node = (Uint16) (CanaRead.MsgId3_PSU >> 8);

        CANRecdParams.Prog_Volt_int[CanNode.PSU_Node] = ((RxMsgData[0] >> 4)
                * 1000) + ((RxMsgData[0] & 0X0F) * 100)
                + ((RxMsgData[1] >> 4) * 10) + ((RxMsgData[1] & 0x0F) * 1);

        CANRecdParams.Prog_Volt_frac[CanNode.PSU_Node] = ((RxMsgData[2] >> 4)
                * 1000) + ((RxMsgData[2] & 0X0F) * 100)
                + ((RxMsgData[3] >> 4) * 10) + ((RxMsgData[3] & 0x0F) * 1);
        CANRecdParams.Prog_Volt_Node[CanNode.PSU_Node] =
                CANRecdParams.Prog_Volt_int[CanNode.PSU_Node]
                        + (CANRecdParams.Prog_Volt_frac[CanNode.PSU_Node]
                                * 0.0001);

        CANRecdParams.Prog_Curr_int[CanNode.PSU_Node] = ((RxMsgData[4] >> 4)
                * 1000) + ((RxMsgData[4] & 0X0F) * 100)
                + ((RxMsgData[5] >> 4) * 10) + ((RxMsgData[5] & 0x0F) * 1);

        CANRecdParams.Prog_Curr_frac[CanNode.PSU_Node] = ((RxMsgData[6] >> 4)
                * 1000) + ((RxMsgData[6] & 0X0F) * 100)
                + ((RxMsgData[7] >> 4) * 10) + ((RxMsgData[7] & 0x0F) * 1);
        CANRecdParams.Prog_Curr_Node[CanNode.PSU_Node] =
                CANRecdParams.Prog_Curr_int[CanNode.PSU_Node]
                        + (CANRecdParams.Prog_Curr_frac[CanNode.PSU_Node]
                                * 0.0001);

        if(CanNode.PSU_Node < 16)
        {
            ActiveNodeRegs.all = ActiveNodeRegs.all | (1 << CanNode.PSU_Node);
        }
        else
        {
            CanNode.Hi_PSU_Node = (CanNode.PSU_Node) - 15;

            ActiveNodeRegs1.all = ActiveNodeRegs1.all | (1<<CanNode.Hi_PSU_Node);
        }

        CANA_NodeCheckLogic();

        CanaRead.MBox7 = 0;
    }

    CANA_TimeoutFaultsLogic();

}

void CANA_NodeCheckLogic()
{
    if (CanNode.PSU_Node == 1)
    {
        CANA_timers.CAN_failCountPSU_Node1 = 0;
    }
    else if(CanNode.PSU_Node == 2)
    {
        CANA_timers.CAN_failCountPSU_Node2 = 0;
    }
    else if(CanNode.PSU_Node == 3)
    {
        CANA_timers.CAN_failCountPSU_Node3 = 0;
    }
    else if(CanNode.PSU_Node == 4)
    {
        CANA_timers.CAN_failCountPSU_Node4 = 0;
    }
    else if(CanNode.PSU_Node == 5)
    {
        CANA_timers.CAN_failCountPSU_Node5 = 0;
    }
    else if (CanNode.PSU_Node == 6)
    {
        CANA_timers.CAN_failCountPSU_Node6 = 0;
    }
    else if(CanNode.PSU_Node == 7)
    {
        CANA_timers.CAN_failCountPSU_Node7 = 0;
    }
    else if(CanNode.PSU_Node == 8)
    {
        CANA_timers.CAN_failCountPSU_Node8 = 0;
    }
    else if(CanNode.PSU_Node == 9)
    {
        CANA_timers.CAN_failCountPSU_Node9 = 0;
    }
    else if(CanNode.PSU_Node == 10)
    {
        CANA_timers.CAN_failCountPSU_Node10 = 0;
    }
    else if (CanNode.PSU_Node == 11)
    {
        CANA_timers.CAN_failCountPSU_Node11 = 0;
    }
    else if(CanNode.PSU_Node == 12)
    {
        CANA_timers.CAN_failCountPSU_Node12 = 0;
    }
    else if(CanNode.PSU_Node == 13)
    {
        CANA_timers.CAN_failCountPSU_Node13 = 0;
    }
    else if(CanNode.PSU_Node == 14)
    {
        CANA_timers.CAN_failCountPSU_Node14 = 0;
    }
    else if(CanNode.PSU_Node == 15)
    {
        CANA_timers.CAN_failCountPSU_Node15 = 0;
    }
    else if (CanNode.PSU_Node == 16)
    {
        CANA_timers.CAN_failCountPSU_Node16 = 0;
    }
    else if(CanNode.PSU_Node == 17)
    {
        CANA_timers.CAN_failCountPSU_Node17 = 0;
    }
    else if(CanNode.PSU_Node == 18)
    {
        CANA_timers.CAN_failCountPSU_Node18 = 0;
    }
    else if(CanNode.PSU_Node == 19)
    {
        CANA_timers.CAN_failCountPSU_Node19 = 0;
    }
    else if(CanNode.PSU_Node == 20)
    {
        CANA_timers.CAN_failCountPSU_Node20 = 0;
    }
    else if(CanNode.PSU_Node == 21)
    {
        CANA_timers.CAN_failCountPSU_Node21 = 0;
    }
    else if(CanNode.PSU_Node == 22)
    {
        CANA_timers.CAN_failCountPSU_Node22 = 0;
    }
    else if(CanNode.PSU_Node == 23)
    {
        CANA_timers.CAN_failCountPSU_Node23 = 0;
    }
    else if(CanNode.PSU_Node == 24)
    {
        CANA_timers.CAN_failCountPSU_Node24 = 0;
    }
    else if(CanNode.PSU_Node == 25)
    {
        CANA_timers.CAN_failCountPSU_Node25 = 0;
    }
    else if(CanNode.PSU_Node == 26)
    {
        CANA_timers.CAN_failCountPSU_Node26 = 0;
    }
    else if(CanNode.PSU_Node == 27)
       {
           CANA_timers.CAN_failCountPSU_Node27 = 0;
       }

}
void CANA_TimeoutFaultsLogic()
{
    if (CanNode.PSU_Node != 1)
    {
        CANA_timers.CAN_failCountPSU_Node1++;
        if (CANA_timers.CAN_failCountPSU_Node1 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node1 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs.bit.bt_node1 = 0;
        }
    }

    if (CanNode.PSU_Node != 2)
    {
        CANA_timers.CAN_failCountPSU_Node2++;
        if (CANA_timers.CAN_failCountPSU_Node2 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node2 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs.bit.bt_node2 = 0;
        }
    }

    if (CanNode.PSU_Node != 3)
    {
        CANA_timers.CAN_failCountPSU_Node3++;
        if (CANA_timers.CAN_failCountPSU_Node3 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node3 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs.bit.bt_node3 = 0;
        }
    }

    if (CanNode.PSU_Node != 4 )
    {
        CANA_timers.CAN_failCountPSU_Node4++;
        if (CANA_timers.CAN_failCountPSU_Node4 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node4 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs.bit.bt_node4 = 0;
        }
    }

    if (CanNode.PSU_Node != 5 )
    {
        CANA_timers.CAN_failCountPSU_Node5++;
        if (CANA_timers.CAN_failCountPSU_Node5 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node5 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs.bit.bt_node5 = 0;
        }
    }

    if (CanNode.PSU_Node != 6)
    {
        CANA_timers.CAN_failCountPSU_Node6++;
        if (CANA_timers.CAN_failCountPSU_Node6 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node6 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs.bit.bt_node6 = 0;
        }
    }

    if (CanNode.PSU_Node != 7)
    {
        CANA_timers.CAN_failCountPSU_Node7++;
        if (CANA_timers.CAN_failCountPSU_Node7 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node7 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs.bit.bt_node7 = 0;
        }
    }

    if (CanNode.PSU_Node != 8)
    {
        CANA_timers.CAN_failCountPSU_Node8++;
        if (CANA_timers.CAN_failCountPSU_Node8 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node8 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs.bit.bt_node8 = 0;
        }
    }

    if (CanNode.PSU_Node != 9)
    {
        CANA_timers.CAN_failCountPSU_Node9++;
        if (CANA_timers.CAN_failCountPSU_Node9 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node9 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs.bit.bt_node9 = 0;
        }
    }

    if (CanNode.PSU_Node != 10 )
    {
        CANA_timers.CAN_failCountPSU_Node10++;
        if (CANA_timers.CAN_failCountPSU_Node10 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node10 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs.bit.bt_node10 = 0;
        }
    }

    if (CanNode.PSU_Node != 11 )
       {
           CANA_timers.CAN_failCountPSU_Node11++;
           if (CANA_timers.CAN_failCountPSU_Node11 >= CAN_PSU_TIMEOUT)
           {
               CANA_timers.CAN_failCountPSU_Node11 = CAN_PSU_TIMEOUT;
               ActiveNodeRegs.bit.bt_node11 = 0;
           }
       }

    if (CanNode.PSU_Node != 12 )
          {
              CANA_timers.CAN_failCountPSU_Node12++;
              if (CANA_timers.CAN_failCountPSU_Node12 >= CAN_PSU_TIMEOUT)
              {
                  CANA_timers.CAN_failCountPSU_Node12 = CAN_PSU_TIMEOUT;
                  ActiveNodeRegs.bit.bt_node12 = 0;
              }
          }

    if (CanNode.PSU_Node != 13 )
          {
              CANA_timers.CAN_failCountPSU_Node13++;
              if (CANA_timers.CAN_failCountPSU_Node13 >= CAN_PSU_TIMEOUT)
              {
                  CANA_timers.CAN_failCountPSU_Node13 = CAN_PSU_TIMEOUT;
                  ActiveNodeRegs.bit.bt_node13 = 0;
              }
          }

    if (CanNode.PSU_Node != 14 )
          {
              CANA_timers.CAN_failCountPSU_Node14++;
              if (CANA_timers.CAN_failCountPSU_Node14 >= CAN_PSU_TIMEOUT)
              {
                  CANA_timers.CAN_failCountPSU_Node14 = CAN_PSU_TIMEOUT;
                  ActiveNodeRegs.bit.bt_node14 = 0;
              }
          }

    if (CanNode.PSU_Node != 15 )
          {
              CANA_timers.CAN_failCountPSU_Node15++;
              if (CANA_timers.CAN_failCountPSU_Node15 >= CAN_PSU_TIMEOUT)
              {
                  CANA_timers.CAN_failCountPSU_Node15 = CAN_PSU_TIMEOUT;
                  ActiveNodeRegs.bit.bt_node15 = 0;
              }
          }

    if (CanNode.PSU_Node != 16)
    {
        CANA_timers.CAN_failCountPSU_Node16++;
        if (CANA_timers.CAN_failCountPSU_Node16 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node16 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node16 = 0;
        }
    }

    if (CanNode.PSU_Node != 17)
    {
        CANA_timers.CAN_failCountPSU_Node17++;
        if (CANA_timers.CAN_failCountPSU_Node17 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node17 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node17 = 0;
        }
    }

    if (CanNode.PSU_Node != 18)
    {
        CANA_timers.CAN_failCountPSU_Node18++;
        if (CANA_timers.CAN_failCountPSU_Node18 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node18 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node18 = 0;
        }
    }

    if (CanNode.PSU_Node != 19)
    {
        CANA_timers.CAN_failCountPSU_Node19++;
        if (CANA_timers.CAN_failCountPSU_Node19 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node19 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node19 = 0;
        }
    }

    if (CanNode.PSU_Node != 20 )
    {
        CANA_timers.CAN_failCountPSU_Node20++;
        if (CANA_timers.CAN_failCountPSU_Node20 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node20 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node20 = 0;
        }
    }

    if (CanNode.PSU_Node != 21 )
    {
        CANA_timers.CAN_failCountPSU_Node21++;
        if (CANA_timers.CAN_failCountPSU_Node21 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node21 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node21 = 0;
        }
    }

    if (CanNode.PSU_Node != 22 )
    {
        CANA_timers.CAN_failCountPSU_Node22++;
        if (CANA_timers.CAN_failCountPSU_Node22 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node22 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node22 = 0;
        }
    }

    if (CanNode.PSU_Node != 23 )
    {
        CANA_timers.CAN_failCountPSU_Node23++;
        if (CANA_timers.CAN_failCountPSU_Node23 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node23 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node23 = 0;
        }
    }

    if (CanNode.PSU_Node != 24 )
    {
        CANA_timers.CAN_failCountPSU_Node24++;
        if (CANA_timers.CAN_failCountPSU_Node24 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node24 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node24 = 0;
        }
    }

    if (CanNode.PSU_Node != 25 )
    {
        CANA_timers.CAN_failCountPSU_Node25++;
        if (CANA_timers.CAN_failCountPSU_Node25 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node25 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node25 = 0;
        }
    }

    if (CanNode.PSU_Node != 26 )
    {
        CANA_timers.CAN_failCountPSU_Node26++;
        if (CANA_timers.CAN_failCountPSU_Node26 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node26 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node26 = 0;
        }
    }
    if (CanNode.PSU_Node != 27 )
    {
        CANA_timers.CAN_failCountPSU_Node27++;
        if (CANA_timers.CAN_failCountPSU_Node27 >= CAN_PSU_TIMEOUT)
        {
            CANA_timers.CAN_failCountPSU_Node27 = CAN_PSU_TIMEOUT;
            ActiveNodeRegs1.bit.bt_node27 = 0;
        }
    }

}

void Turn_ON_Node_specific(int32 nodeID, int32 ON_command, int32 global)
{
    if (global == 0)
    {
        CAN_setupMessageObject(
                CANA_BASE, MAILBOX_2,
                (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_TO_TURNON_PSU,
                CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                CAN_MSG_OBJ_NO_FLAGS,
                2);

        if (ON_command == TURN_ON_PSU)
        {
            txMsgData[0] = 0xAA;
            txMsgData[1] = 0xAA;
        }
        else if (ON_command == TURN_OFF_PSU)
        {
            txMsgData[0] = 0x00;
            txMsgData[1] = 0x00;
        }
        else if (ON_command == TURN_ON_FEC)
        {
            txMsgData[0] = 0xBB;
            txMsgData[1] = 0xBB;
        }

        CAN_sendMessage(CANA_BASE, MAILBOX_2, 2, txMsgData);
    }
    else if (global == 1)
    {
        CAN_setupMessageObject(CANA_BASE, MAILBOX_2,
        MSG_ID_GLOBAL_COMMAND_TO_TURNON_PSU,
                               CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX,
                               0x1FFFFFFF,
                               CAN_MSG_OBJ_NO_FLAGS,
                               2);

        if (ON_command == TURN_ON_PSU)
        {
            txMsgData[0] = 0xAA;
            txMsgData[1] = 0xAA;
        }
        else if (ON_command == TURN_OFF_PSU)
        {
            txMsgData[0] = 0x00;
            txMsgData[1] = 0x00;
        }
        else if (ON_command == TURN_ON_FEC)
        {
            txMsgData[0] = 0xBB;
            txMsgData[1] = 0xBB;
        }

        CAN_sendMessage(CANA_BASE, MAILBOX_2, 2, txMsgData);
    }

}

void CANGlobalONOFFCommand(int16 ON_command)
{
    if (ON_command == TURN_ON_PSU)
    {
        txMsgData[0] = 0xAA;
        txMsgData[1] = 0xAA;
    }
    else if (ON_command == TURN_OFF_PSU)
    {
        txMsgData[0] = 0x00;
        txMsgData[1] = 0x00;
    }

    CAN_sendMessage(CANA_BASE, MAILBOX_1, 2, txMsgData);
}
void Save_Settings_Node_specific(int32 nodeID)
{
    CAN_setupMessageObject(
            CANA_BASE, MAILBOX_2,
            (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_TO_TURNON_PSU,
            CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
            CAN_MSG_OBJ_NO_FLAGS,
            3);

    txMsgData[0] = 0xA1;
    txMsgData[1] = 0xA1;
    txMsgData[2] = 0x01;

    CAN_sendMessage(CANA_BASE, MAILBOX_2, 3, txMsgData);
}

void Set_local_Node_specific(int32 nodeID)
{
    CAN_setupMessageObject(
            CANA_BASE, MAILBOX_2,
            (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_TO_TURNON_PSU,
            CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
            CAN_MSG_OBJ_NO_FLAGS,
            3);

    txMsgData[0] = 0xAE;
    txMsgData[1] = 0xAE;
    txMsgData[2] = 0x00;

    CAN_sendMessage(CANA_BASE, MAILBOX_2, 3, txMsgData);
}

void Set_Voltage_Node_specific(int32 nodeID, float32 voltage_Value,
                               Uint16 global)
{

    if (global == 0)
    {
        CAN_setupMessageObject(
                CANA_BASE, MAILBOX_2,
                (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_TO_SETVOLTAGE_PSU,
                CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                CAN_MSG_OBJ_NO_FLAGS,
                4);
    }
    else if (global == 1)
    {
        CAN_setupMessageObject(CANA_BASE, MAILBOX_2,
        MSG_ID_GLOBAL_COMMAND_TO_SETVOLTAGE_PSU,
                               CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX,
                               0x1FFFFFFF,
                               CAN_MSG_OBJ_NO_FLAGS,
                               4);
    }

    temp5[0] = (int32) (voltage_Value);
    frac[0] = (int32) ((voltage_Value - temp5[0]) * 10000);

    txMsgData[0] = (((temp5[0] / 1000) & 0xF) << 4)
            | (((temp5[0] % 1000) / 100) & 0xF);
    txMsgData[1] = ((((temp5[0] % 100) / 10) & 0xF) << 4)
            | (((temp5[0] % 10) / 1) & 0xF);

    txMsgData[2] = (((frac[0] / 1000) & 0xF) << 4)
            | (((frac[0] % 1000) / 100) & 0xF);
    txMsgData[3] = ((((frac[0] % 100) / 10) & 0xF) << 4)
            | (((frac[0] % 10) / 1) & 0xF);

    CAN_sendMessage(CANA_BASE, MAILBOX_2, 4, txMsgData);
}

void Set_Current_Node_specific(int32 nodeID, float32 current_Value,
                               Uint16 global)
{
    if (global == 0)
    {
        CAN_setupMessageObject(
                CANA_BASE, MAILBOX_1,
                (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_TO_SETCURRENT_PSU,
                CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                CAN_MSG_OBJ_NO_FLAGS,
                4);
    }
    else if (global == 1)
    {
        CAN_setupMessageObject(CANA_BASE, MAILBOX_1,
        MSG_ID_GLOBAL_COMMAND_TO_SETCURRENT_PSU,
                               CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX,
                               0x1FFFFFFF,
                               CAN_MSG_OBJ_NO_FLAGS,
                               4);
    }

    temp5[0] = (int32) (current_Value);
    frac[0] = (int32) ((current_Value - temp5[0]) * 10000);

    txMsgData[0] = (((temp5[0] / 1000) & 0xF) << 4)
            | (((temp5[0] % 1000) / 100) & 0xF);
    txMsgData[1] = ((((temp5[0] % 100) / 10) & 0xF) << 4)
            | (((temp5[0] % 10) / 1) & 0xF);

    txMsgData[2] = (((frac[0] / 1000) & 0xF) << 4)
            | (((frac[0] % 1000) / 100) & 0xF);
    txMsgData[3] = ((((frac[0] % 100) / 10) & 0xF) << 4)
            | (((frac[0] % 10) / 1) & 0xF);

    CAN_sendMessage(CANA_BASE, MAILBOX_1, 4, txMsgData);
}

void Set_Current_Global(float32 current_value)
{
    CAN_setupMessageObject(CANA_BASE, MAILBOX_1, 0x11100006, CAN_MSG_FRAME_EXT,
                           CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                           CAN_MSG_OBJ_NO_FLAGS,
                           4);

    temp5[0] = (int32) (current_value);
    frac[0] = (int32) ((current_value - temp5[0]) * 10000);

    txMsgData[0] = (((temp5[0] / 1000) & 0xF) << 4)
            | (((temp5[0] % 1000) / 100) & 0xF);
    txMsgData[1] = ((((temp5[0] % 100) / 10) & 0xF) << 4)
            | (((temp5[0] % 10) / 1) & 0xF);

    txMsgData[2] = (((frac[0] / 1000) & 0xF) << 4)
            | (((frac[0] % 1000) / 100) & 0xF);
    txMsgData[3] = ((((frac[0] % 100) / 10) & 0xF) << 4)
            | (((frac[0] % 10) / 1) & 0xF);

    CAN_sendMessage(CANA_BASE, MAILBOX_1, 4, txMsgData);
}

void Query_ProgParams_Node_Specific(int32 nodeID)
{
    CAN_setupMessageObject(
            CANA_BASE, MAILBOX_2,
            (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_TO_PROG_PARAMS_PSU,
            CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
            CAN_MSG_OBJ_NO_FLAGS,
            2);

    txMsgData[0] = 0x61;
    txMsgData[1] = 0x61;

    CAN_sendMessage(CANA_BASE, MAILBOX_2, 2, txMsgData);

}

void Query_MeasParams_Node_Specific(int32 nodeID)
{
    CAN_setupMessageObject(
            CANA_BASE, MAILBOX_2,
            (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_TO_PROG_PARAMS_PSU,
            CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
            CAN_MSG_OBJ_NO_FLAGS,
            2);

    txMsgData[0] = 0x62;
    txMsgData[1] = 0x62;

    CAN_sendMessage(CANA_BASE, MAILBOX_2, 2, txMsgData);

}

void Query_Flts_Node_Specific(int32 nodeID)
{
    CAN_setupMessageObject(
            CANA_BASE, MAILBOX_2,
            (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_TO_FAULTS_PSU,
            CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
            CAN_MSG_OBJ_NO_FLAGS,
            2);

    txMsgData[0] = 0x6C;
    txMsgData[1] = 0x6C;

    CAN_sendMessage(CANA_BASE, MAILBOX_2, 2, txMsgData);

}

void oi_CANA_InitRead_PSU()
{

    CanaRead.MBox5 = CAN_readMessage(CANA_BASE, MAILBOX_5, RxMsgData); // Measured Parameters
    CanaRead.MBox6 = CAN_readMessage(CANA_BASE, MAILBOX_6, RxMsgData); // Faults
    CanaRead.MBox7 = CAN_readMessage(CANA_BASE, MAILBOX_7, RxMsgData); // Programmed Parameters
}
