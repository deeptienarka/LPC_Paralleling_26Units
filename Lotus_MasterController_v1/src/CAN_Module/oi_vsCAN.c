/*
 * oi_vsCAN.c
 *
 *  Created on: 16-Mar-2021
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

void oi_CANA_TX_Event_VS();
void oi_CANA_RX_Event_VS();
void oi_CANA_InitRead_VS();
void Query_MeasCell_VoltParam_Node_Specific(int32 nodeID);
void oi_Cell_MaxVolt(void);
void oi_Cell_MinVolt(void);


void oi_CANA_TX_Event_VS()
{
    switch (State.Present_St)
    {

    case OI_STATUS_CHECK:

        if (QueryType.VS == MEAS_CELL_VOLT)
        {
            CANA_timers.TX_count_VS++;

            Query_MeasCell_VoltParam_Node_Specific(CANA_timers.TX_count_VS);

            if (CANA_timers.TX_count_VS >= TOTAL_VS_NODE_NUMBER)
            {
                CANA_timers.TX_count_VS = 0;
            }
        }
        break;

    default:
        break;
    }

}

void oi_CANA_RX_Event_VS()
{
    Uint16 k;
    oi_CANA_InitRead_VS();

    if (trig1 == 1)
    {
        CanaRead.MsgId4_VS = 0;
        CanNode.VS_Node = 0;
    }

    if (CanaRead.MBox12 == 1)
    {

        cnt1 = 0;

        CanaRead.MsgId4_VS = CanaRegs.CAN_IF2ARB.bit.ID;
        CanaRead.MsgId4_VS = CanaRead.MsgId4_VS & CAN_NODEID_MASK;

        CanNode.VS_Node = (Uint16) (CanaRead.MsgId4_VS >> 8);

        // CAN failure Detection

        CANRecdParams.Channel_ID_VS = RxMsgData1[0];

        switch (CANRecdParams.Channel_ID_VS)
        {
        case 1:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][1] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][2] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][3] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 2:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][4] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][5] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][6] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 3:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][7] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][8] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][9] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 4:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][10] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][11] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][12] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 5:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][13] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][14] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][15] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 6:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][16] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][17] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][18] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 7:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][19] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][20] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][21] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 8:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][22] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][23] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][24] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 9:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][25] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][26] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][27] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 10:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][28] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][29] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][30] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 11:
            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][31] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Meas_Cell_Volt[CanNode.VS_Node][32] = (((RxMsgData1[4]
                    << 8)) + RxMsgData1[5]) * 0.0001;

            CANRecdParams.Max_Cell_Volt1[CanNode.VS_Node] = (((RxMsgData1[6]
                    << 8)) + RxMsgData1[7]) * 0.0001;

            CANRecdParams.Max_Cell_Num1[CanNode.VS_Node] =
                    (Uint16) (RxMsgData1[1]);

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        case 12:
            CANRecdParams.Max_Cell_Num2[CanNode.VS_Node] =
                    (Uint16) (RxMsgData1[1]);

            CANRecdParams.Max_Cell_Volt2[CanNode.VS_Node] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            CANRecdParams.Min_Cell_Num1[CanNode.VS_Node] =
                    (Uint16) (RxMsgData1[4]);

            CANRecdParams.Min_Cell_Volt1[CanNode.VS_Node] = (((RxMsgData1[5]
                    << 8)) + RxMsgData1[6]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

        case 13:
            CANRecdParams.Min_Cell_Num2[CanNode.VS_Node] =
                    (Uint16) (RxMsgData1[1]);

            CANRecdParams.Min_Cell_Volt2[CanNode.VS_Node] = (((RxMsgData1[2]
                    << 8)) + RxMsgData1[3]) * 0.0001;

            for (k = 0; k < 8; k++)
            {
                RxMsgData1[k] = 0;
            }

            break;

        default:
            break;

        }

        // Maximum Cell voltage1, Max Cell Voltage2, Corresponding Cell Number and Node is obtained here.

        CANRecdParams.Max_Cell_Voltage_All_Nodes[CanNode.VS_Node] =
                CANRecdParams.Max_Cell_Volt1[CanNode.VS_Node]; // Highest Value from all nodes
        CANRecdParams.Max_Cell_Number_All_Nodes[CanNode.VS_Node] =
                CANRecdParams.Max_Cell_Num1[CanNode.VS_Node]; // Highest Value from all nodes

        CANRecdParams.Max_Cell_Voltage_All_Nodes[CanNode.VS_Node + 6] =
                CANRecdParams.Max_Cell_Volt2[CanNode.VS_Node]; // Second Highest Value from all nodes
        CANRecdParams.Max_Cell_Number_All_Nodes[CanNode.VS_Node + 6] =
                CANRecdParams.Max_Cell_Num2[CanNode.VS_Node]; // Second Highest Value from all nodes

        CANRecdParams.Max_Cell_Voltage_Node_Num[CanNode.VS_Node] =
                CanNode.VS_Node;
        CANRecdParams.Max_Cell_Voltage_Node_Num[CanNode.VS_Node + 6] =
                CanNode.VS_Node;

        // Minimum Cell voltage1, Minimum Cell Voltage2, Corresponding Cell Number and Node is obtained here.

        CANRecdParams.Min_Cell_Voltage_All_Nodes[CanNode.VS_Node] =
                CANRecdParams.Min_Cell_Volt1[CanNode.VS_Node]; // Lowest Value from all nodes
        CANRecdParams.Min_Cell_Number_All_Nodes[CanNode.VS_Node] =
                CANRecdParams.Min_Cell_Num1[CanNode.VS_Node]; // Lowest Value from all nodes

        CANRecdParams.Min_Cell_Voltage_All_Nodes[CanNode.VS_Node + 6] =
                CANRecdParams.Max_Cell_Volt2[CanNode.VS_Node]; // Second Lowest Value from all nodes
        CANRecdParams.Min_Cell_Number_All_Nodes[CanNode.VS_Node + 6] =
                CANRecdParams.Max_Cell_Num2[CanNode.VS_Node]; // Second Lowest Value from all nodes

        CANRecdParams.Min_Cell_Voltage_Node_Num[CanNode.VS_Node] =
                CanNode.VS_Node;
        CANRecdParams.Min_Cell_Voltage_Node_Num[CanNode.VS_Node + 6] =
                CanNode.VS_Node;

        oi_Cell_MaxVolt();
        oi_Cell_MinVolt();

    }

    if (CanNode.VS_Node != 1)
    {
        CANA_timers.CAN_Fail_count_Node1++;
        if (CANA_timers.CAN_Fail_count_Node1 >= 200)
        {
            CANA_timers.CAN_Fail_count_Node1 = 200;
            for (k = 0; k <= 31; k++)
            {
                CANRecdParams.Meas_Cell_Volt[1][k] = 0;
            }
        }

    }
    else
    {
        CANA_timers.CAN_Fail_count_Node1 = 0;
    }

    if (CanNode.VS_Node != 2)
    {
        CANA_timers.CAN_Fail_count_Node2++;
        if (CANA_timers.CAN_Fail_count_Node2 >= 200)
        {
            CANA_timers.CAN_Fail_count_Node2 = 200;
            for (k = 0; k <= 31; k++)
            {
                CANRecdParams.Meas_Cell_Volt[2][k] = 0;
            }
        }

    }
    else
    {
        CANA_timers.CAN_Fail_count_Node2 = 0;
    }

    if (CanNode.VS_Node != 3)
    {
        CANA_timers.CAN_Fail_count_Node3++;
        if (CANA_timers.CAN_Fail_count_Node3 >= 200)
        {
            CANA_timers.CAN_Fail_count_Node3 = 200;
            for (k = 0; k <= 31; k++)
            {
                CANRecdParams.Meas_Cell_Volt[3][k] = 0;
            }
        }

    }
    else
    {
        CANA_timers.CAN_Fail_count_Node3 = 0;
    }

    if (CanNode.VS_Node != 4)
    {
        CANA_timers.CAN_Fail_count_Node4++;
        if (CANA_timers.CAN_Fail_count_Node4 >= 200)
        {
            CANA_timers.CAN_Fail_count_Node4 = 200;
            for (k = 0; k <= 31; k++)
            {
                CANRecdParams.Meas_Cell_Volt[4][k] = 0;
            }
        }

    }
    else
    {
        CANA_timers.CAN_Fail_count_Node4 = 0;
    }

    if (CanNode.VS_Node != 5)
    {
        CANA_timers.CAN_Fail_count_Node5++;
        if (CANA_timers.CAN_Fail_count_Node5 >= 200)
        {
            CANA_timers.CAN_Fail_count_Node5 = 200;
            for (k = 0; k <= 31; k++)
            {
                CANRecdParams.Meas_Cell_Volt[5][k] = 0;
            }
        }

    }
    else
    {
        CANA_timers.CAN_Fail_count_Node5 = 0;
    }

    if (CanNode.VS_Node != 6)
    {
        CANA_timers.CAN_Fail_count_Node6++;
        if (CANA_timers.CAN_Fail_count_Node6 >= 200)
        {
            CANA_timers.CAN_Fail_count_Node6 = 200;
            for (k = 0; k <= 31; k++)
            {
                CANRecdParams.Meas_Cell_Volt[6][k] = 0;
            }
        }

    }
    else
    {
        CANA_timers.CAN_Fail_count_Node6 = 0;
    }
    CanaRead.MBox12 = 0;
}

void oi_CANA_InitRead_VS()
{

    CanaRead.MBox12 = CAN_readMessage(CANA_BASE, MAILBOX_12, RxMsgData1);

}

void Query_MeasCell_VoltParam_Node_Specific(int32 nodeID)
{
    CAN_setupMessageObject(
            CANA_BASE, MAILBOX_11,
            (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_TO_MEAS_CELL_VS,
            CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
            CAN_MSG_OBJ_NO_FLAGS,
            1);

    txMsgData[0] = 0x01;

    CAN_sendMessage(CANA_BASE, MAILBOX_11, 1, txMsgData);

}

void oi_Cell_MaxVolt(void)
{
    Uint16 j = 0;

    if (CANRecdParams.Max_Cell_Voltage_All_Nodes[1]
            > CANRecdParams.Max_Cell_Voltage_All_Nodes[2])
    {
        CANRecdParams.Highest_Cell_Volt[0] =
                CANRecdParams.Max_Cell_Voltage_All_Nodes[1];
        CANRecdParams.Highest_Cell_Num[0] =
                CANRecdParams.Max_Cell_Number_All_Nodes[1];
        CANRecdParams.Highest_Cell_Volt_Node[0] =
                CANRecdParams.Max_Cell_Voltage_Node_Num[1];

        CANRecdParams.Highest_Cell_Volt[1] =
                CANRecdParams.Max_Cell_Voltage_All_Nodes[2];
        CANRecdParams.Highest_Cell_Num[1] =
                CANRecdParams.Max_Cell_Number_All_Nodes[2];
        CANRecdParams.Highest_Cell_Volt_Node[1] =
                CANRecdParams.Max_Cell_Voltage_Node_Num[2];

    }
    else
    {
        CANRecdParams.Highest_Cell_Volt[0] =
                CANRecdParams.Max_Cell_Voltage_All_Nodes[2];
        CANRecdParams.Highest_Cell_Num[0] =
                CANRecdParams.Max_Cell_Number_All_Nodes[2];
        CANRecdParams.Highest_Cell_Volt_Node[0] =
                CANRecdParams.Max_Cell_Voltage_Node_Num[2];

        CANRecdParams.Highest_Cell_Volt[1] =
                CANRecdParams.Max_Cell_Voltage_All_Nodes[1];
        CANRecdParams.Highest_Cell_Num[1] =
                CANRecdParams.Max_Cell_Number_All_Nodes[1];
        CANRecdParams.Highest_Cell_Volt_Node[1] =
                CANRecdParams.Max_Cell_Voltage_Node_Num[1];

    }

    for (j = 3; j < 13; j++)
    {
        if (CANRecdParams.Highest_Cell_Volt[0]
                < CANRecdParams.Max_Cell_Voltage_All_Nodes[j])
        {
            CANRecdParams.Highest_Cell_Volt[1] =
                    CANRecdParams.Highest_Cell_Volt[0];
            CANRecdParams.Highest_Cell_Volt[0] =
                    CANRecdParams.Max_Cell_Voltage_All_Nodes[j];

            CANRecdParams.Highest_Cell_Num[1] =
                    CANRecdParams.Highest_Cell_Num[0];
            CANRecdParams.Highest_Cell_Num[0] =
                    CANRecdParams.Max_Cell_Number_All_Nodes[j];

            CANRecdParams.Highest_Cell_Volt_Node[1] =
                    CANRecdParams.Highest_Cell_Volt_Node[0];
            CANRecdParams.Highest_Cell_Volt_Node[0] =
                    CANRecdParams.Max_Cell_Voltage_Node_Num[j];

        }
        else if (CANRecdParams.Highest_Cell_Volt[1]
                <= CANRecdParams.Max_Cell_Voltage_All_Nodes[j])
        {
            CANRecdParams.Highest_Cell_Volt[1] =
                    CANRecdParams.Max_Cell_Voltage_All_Nodes[j];
            CANRecdParams.Highest_Cell_Num[1] =
                    CANRecdParams.Max_Cell_Number_All_Nodes[j];
            CANRecdParams.Highest_Cell_Volt_Node[1] =
                    CANRecdParams.Max_Cell_Voltage_Node_Num[j];
        }
    }
}

void oi_Cell_MinVolt(void)
{
    Uint16 j = 0;

    if (CANRecdParams.Min_Cell_Voltage_All_Nodes[1]
            > CANRecdParams.Min_Cell_Voltage_All_Nodes[2])
    {
        CANRecdParams.Lowest_Cell_Volt[0] =
                CANRecdParams.Min_Cell_Voltage_All_Nodes[1];
        CANRecdParams.Lowest_Cell_Num[0] =
                CANRecdParams.Min_Cell_Number_All_Nodes[1];
        CANRecdParams.Lowest_Cell_Volt_Node[0] =
                CANRecdParams.Max_Cell_Voltage_Node_Num[1];

        CANRecdParams.Lowest_Cell_Volt[1] =
                CANRecdParams.Min_Cell_Voltage_All_Nodes[2];
        CANRecdParams.Lowest_Cell_Num[1] =
                CANRecdParams.Min_Cell_Number_All_Nodes[2];
        CANRecdParams.Lowest_Cell_Volt_Node[1] =
                CANRecdParams.Min_Cell_Voltage_Node_Num[2];

    }
    else
    {
        CANRecdParams.Lowest_Cell_Volt[0] =
                CANRecdParams.Min_Cell_Voltage_All_Nodes[2];
        CANRecdParams.Lowest_Cell_Num[0] =
                CANRecdParams.Min_Cell_Number_All_Nodes[2];
        CANRecdParams.Lowest_Cell_Volt_Node[0] =
                CANRecdParams.Min_Cell_Voltage_Node_Num[2];

        CANRecdParams.Lowest_Cell_Volt[1] =
                CANRecdParams.Min_Cell_Voltage_All_Nodes[1];
        CANRecdParams.Lowest_Cell_Num[1] =
                CANRecdParams.Min_Cell_Number_All_Nodes[1];
        CANRecdParams.Lowest_Cell_Volt_Node[1] =
                CANRecdParams.Min_Cell_Voltage_Node_Num[1];

    }

    for (j = 3; j < 13; j++)
    {
        if (CANRecdParams.Lowest_Cell_Volt[0]
                < CANRecdParams.Min_Cell_Voltage_All_Nodes[j])
        {
            CANRecdParams.Lowest_Cell_Volt[1] =
                    CANRecdParams.Lowest_Cell_Volt[0];
            CANRecdParams.Lowest_Cell_Volt[0] =
                    CANRecdParams.Min_Cell_Voltage_All_Nodes[j];

            CANRecdParams.Lowest_Cell_Num[1] = CANRecdParams.Lowest_Cell_Num[0];
            CANRecdParams.Lowest_Cell_Num[0] =
                    CANRecdParams.Min_Cell_Number_All_Nodes[j];

            CANRecdParams.Lowest_Cell_Volt_Node[1] =
                    CANRecdParams.Lowest_Cell_Volt_Node[0];
            CANRecdParams.Lowest_Cell_Volt_Node[0] =
                    CANRecdParams.Min_Cell_Voltage_Node_Num[j];

        }
        else if (CANRecdParams.Lowest_Cell_Volt[1]
                <= CANRecdParams.Min_Cell_Voltage_All_Nodes[j])
        {
            CANRecdParams.Lowest_Cell_Volt[1] =
                    CANRecdParams.Min_Cell_Voltage_All_Nodes[j];
            CANRecdParams.Lowest_Cell_Num[1] =
                    CANRecdParams.Min_Cell_Number_All_Nodes[j];
            CANRecdParams.Lowest_Cell_Volt_Node[1] =
                    CANRecdParams.Min_Cell_Voltage_Node_Num[j];
        }
    }
}
