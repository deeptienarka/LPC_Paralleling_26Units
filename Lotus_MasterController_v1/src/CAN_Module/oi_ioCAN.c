/*
 * oi_ioCAN.c
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



void oi_CANA_TX_Event_IO();
void oi_CANA_RX_Event_IO();
void oi_CANA_InitRead_IO();
void Command_IO_Node_Specific_default(int32 nodeID);
void Command_IO_Node_Specific_Purge(int32 nodeID, Uint16 Stage);
void Query_IO_Node_Specific(int32 nodeID);
void Command_IO_Node_Specific_IOPower(int32 nodeID, Uint16 Stage);

void oi_CANA_InitRead_IO()
{
    CanaRead.MBox16 = CAN_readMessage(CANA_BASE, MAILBOX_16, RxMsgData_IO); // Receive Mailbox of AI Block 1 Data
    CanaRead.MBox17 = CAN_readMessage(CANA_BASE, MAILBOX_17, RxMsgData_IO); // Receive Mailbox of AI Block 2 Data
    CanaRead.MBox18 = CAN_readMessage(CANA_BASE, MAILBOX_18, RxMsgData_IO); // Receive Mailbox of Thermocouple Data
    CanaRead.MBox19 = CAN_readMessage(CANA_BASE, MAILBOX_19, RxMsgData_IO); // Receive Mailbox of Generic I/O Data
}

void oi_CANA_TX_Event_IO()
{

    switch (State.Present_St)
    {
    case OI_STAND_BY:

        if(QueryType.IO == IDLE_IO)
        {
            QueryType.IO = DEFAULT;
        }

        else if(QueryType.IO == DEFAULT)
        {
            CANA_timers.TX_count_IO++;

            Command_IO_Node_Specific_default(CAN_LPC_100_NODE); // Command to Set default values to LPC I/O Card

            if(CANA_timers.TX_count_IO > 5)
            {
                CANA_timers.TX_count_IO = 0;

                if(IO_Flags.LPC100_DefaultACK == 1)
                {
                    QueryType.IO = PURGE_FAN1_ON;
                }
                else
                {
                    QueryType.IO = DEFAULT;
                }
            }
        }
        break;

    case OI_PURGE:

        CANA_timers.Time_count_IO_Purge++;

        if (CANA_timers.Time_count_IO_Purge >= 17)
        {
            CANA_timers.Time_count_IO_Purge = 0;

            CANA_timers.Seconds_count_IO_Purge++;
        }

        if (CANA_timers.Seconds_count_IO_Purge >= 60)
        {
            CANA_timers.Minutes_count_IO_Purge++;

            CANA_timers.Seconds_count_IO_Purge = 0;

            CANA_timers.Minutes_flag_IO_Purge = 1;
        }

        if (QueryType.IO == PURGE_FAN1_ON)
        {
            CANA_timers.TX_count_IO++;

            Command_IO_Node_Specific_Purge(CAN_LPC_100_NODE, 1); // Command to turn on purge fans 1 to LPC I/O card

            IO_Flags.PurgeFansON = 1;

            if(CANA_timers.TX_count_IO > 5)
            {
                CANA_timers.TX_count_IO = 0;

                if(IO_Flags.PurgeFan1ON_ACK == 1)
                {
                    QueryType.IO = PURGE_FAN2_ON;
                }
                else
                {
                    QueryType.IO = PURGE_FAN1_ON;
                }
            }
        }

        else if ((CANA_timers.Minutes_count_IO_Purge == 1)
                && (QueryType.IO == PURGE_FAN2_ON))

        {
            CANA_timers.TX_count_IO++;

            Command_IO_Node_Specific_Purge(CAN_LPC_100_NODE, 2); // Command to turn on purge fans 2 to LPC I/O card

            IO_Flags.PurgeFansON = 2;

            if(CANA_timers.TX_count_IO > 5)
            {
                CANA_timers.TX_count_IO = 0;

                if(IO_Flags.PurgeFan2ON_ACK == 1)
                {
                    QueryType.IO = PURGE_FAN3_ON;
                }
                else
                {
                    QueryType.IO = PURGE_FAN2_ON;
                }
            }
        }
        else if ((CANA_timers.Minutes_count_IO_Purge == 2)
                && (QueryType.IO == PURGE_FAN3_ON))
        {
            CANA_timers.TX_count_IO++;

            Command_IO_Node_Specific_Purge(CAN_LPC_100_NODE, 3); // Command to turn on purge fans 3 to LPC I/O card

            IO_Flags.PurgeFansON = 3;

            if(CANA_timers.TX_count_IO > 5)
            {
                CANA_timers.TX_count_IO = 0;

                if(IO_Flags.PurgeFan3ON_ACK == 1)
                {
                    QueryType.IO = AI_BLK1_QUERY;
                }
                else
                {
                    QueryType.IO = PURGE_FAN3_ON;
                }
            }
        }
        else if ((CANA_timers.Minutes_count_IO_Purge == 3)
                && (QueryType.IO == AI_BLK1_QUERY))
        {
            CANA_timers.TX_count_IO++;

            Query_IO_Node_Specific(CAN_LPC_100_NODE); // Query Hydrogen residue with LPC I/O Card

            if(CANA_timers.TX_count_IO > 5)
            {
                CANA_timers.TX_count_IO = 0;

                if((IO_Flags.hydrogenValuesOK == 1)
                        &&(CANA_timers.Seconds_count_IO_Purge >= 40))
                {
                    QueryType.IO = LHC_POWER_ON;
                }
                else
                {
                    QueryType.IO = AI_BLK1_QUERY;
                }
            }
        }

        break;

    case OI_IO_POWER:

        if(QueryType.IO == LHC_POWER_ON)
        {
            CANA_timers.TX_count_IO++;

            Command_IO_Node_Specific_IOPower(CAN_LPC_100_NODE, 1); // Turn ON LHC IO & VS Cards from LPC IO Card

            if(CANA_timers.TX_count_IO > 5)
            {
                CANA_timers.TX_count_IO = 0;

                if(IO_Flags.LHC_POwerON == 1)
                {
                    QueryType.IO = LHC101_DEFAULT;
                }
                else
                {
                    QueryType.IO = LHC_POWER_ON;
                }
            }

        }
        else if (QueryType.IO == LHC101_DEFAULT)
        {
            CANA_timers.TX_count_IO++;

            Command_IO_Node_Specific_default(CAN_LHC_101_NODE); // Default Set to LHC_101 and Enable level Sensors LVL101 and LVL102

            if(CANA_timers.TX_count_IO > 5)
            {
                CANA_timers.TX_count_IO = 0;

                if(IO_Flags.LHC101_DefaultACK == 1)
                {
                    QueryType.IO = LHC102_DEFAULT;
                }
                else
                {
                    QueryType.IO = LHC101_DEFAULT;
                }

            }
        }
        else if (QueryType.IO == LHC102_DEFAULT)
        {
            CANA_timers.TX_count_IO++;

            Command_IO_Node_Specific_default(CAN_LHC_102_NODE); // Default Set to LHC_102 and Enable LVL401

            if(CANA_timers.TX_count_IO > 5)
            {
                CANA_timers.TX_count_IO = 0;

                if(IO_Flags.LHC102_DefaultACK == 1)
                {
                    QueryType.IO = AI_BLK1_QUERY;
                }
                else
                {
                    QueryType.IO = LHC102_DEFAULT;
                }
            }
        }
        else if (QueryType.IO == AI_BLK1_QUERY)
        {
            CANA_timers.TX_count_IO++;

            if(CANA_timers.TX_count_IO_Query == 0)
            {
                Query_IO_Node_Specific(CAN_LPC_100_NODE); // Query Hydrogen residue with LPC I/O Card
            }
            else if(CANA_timers.TX_count_IO_Query == 1)
            {
                Query_IO_Node_Specific(CAN_LHC_101_NODE); // Query level sensor LVL-101,102 values with LHC_101

            }
            else if(CANA_timers.TX_count_IO_Query == 2)
            {
                Query_IO_Node_Specific(CAN_LHC_102_NODE); // Query level sensor LVL-401 values with LHC_102
            }

            if(CANA_timers.TX_count_IO > 3)
            {
                CANA_timers.TX_count_IO = 0;

                CANA_timers.TX_count_IO_Query++;

                if(CANA_timers.TX_count_IO_Query > 2)
                {
                    CANA_timers.TX_count_IO_Query = 0;
                }

                if(IO_Flags.WaterLevelOk == 1)
                {
                    QueryType.IO = PRT_READ_VFD_ON;

                    CANA_timers.TX_count_IO_Query = 0;
                }
                else
                {
                    QueryType.IO = AI_BLK1_QUERY;
                }
            }
        }
        else if(QueryType.IO == PRT_READ_VFD_ON)
        {
            CANA_timers.TX_count_IO++;

            if(CANA_timers.TX_count_IO_Query == 0)
            {
                Command_IO_Node_Specific_IOPower(CAN_LPC_100_NODE, 2); // Turn ON VFD from LPC card
            }
            else if (CANA_timers.TX_count_IO_Query == 1)
            {
                if(CANA_timers.TX_count_IO_Query2 == 0)
                {
                    Command_IO_Node_Specific_IOPower(CAN_LPC_100_NODE, 3); // Query for LVL-101, LVL-102, PRT-101, PRT-103 values

                    CANA_timers.TX_count_IO_Query2 = 1;
                }
                else if(CANA_timers.TX_count_IO_Query2 == 1)
                {
                    Command_IO_Node_Specific_IOPower(CAN_LHC_101_NODE, 4); // Query for PRT-301 values

                    CANA_timers.TX_count_IO_Query2 = 2;
                }
                else if(CANA_timers.TX_count_IO_Query2 == 2)
                {
                    Command_IO_Node_Specific_IOPower(CAN_LHC_102_NODE, 1); // Query for LVL-401 values

                    CANA_timers.TX_count_IO_Query2 = 0;
                }
            }

            if(CANA_timers.TX_count_IO > 3)
            {
                CANA_timers.TX_count_IO = 0;

                if(IO_Flags.VFD101_ONAck == 1)
                {
                    CANA_timers.TX_count_IO_Query = 1;
                }
            }
        }
    }
}

void oi_CANA_RX_Event_IO()
{
    oi_CANA_InitRead_IO();

    if (CanaRead.MBox16 == 1)
    {
        CAN_readMessage(CANA_BASE, MAILBOX_16, RxMsgData_IO);

        CanaRead.IO_AIBLK1_MSGID = CanaRegs.CAN_IF2ARB.bit.ID;
        CanaRead.IO_AIBLK1_MSGID = CanaRead.IO_AIBLK1_MSGID & CAN_NODEID_MASK;

        CanNode.IOCard_Node = (Uint16) (CanaRead.IO_AIBLK1_MSGID >> 8);

        if (CanNode.IOCard_Node == 1)
        {
            CanNode.IOCard_Type = LPC_100;
        }
        else if (CanNode.IOCard_Node == 2)
        {
            CanNode.IOCard_Type = LHC_101;
        }
        else if (CanNode.IOCard_Node == 3)
        {
            CanNode.IOCard_Type = LHC_102;
        }

        if (CanNode.IOCard_Type == LPC_100)
        {
            AI_Data[LPC_100_NODE].AI0_Data = ((RxMsgData_IO[0] << 8)
                    | (RxMsgData_IO[1])) * 0.001;
            AI_Data[LPC_100_NODE].AI1_Data = ((RxMsgData_IO[2] << 8)
                    | (RxMsgData_IO[3])) * 0.001;
            AI_Data[LPC_100_NODE].AI2_Data = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            AI_Data[LPC_100_NODE].AI3_Data = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;

            AI_SensorData.HYS_101 = AI_Data[LPC_100_NODE].AI0_Data;
            AI_SensorData.HYS_401 = AI_Data[LPC_100_NODE].AI1_Data;
            AI_SensorData.HYS_501 = AI_Data[LPC_100_NODE].AI2_Data;
        }
        else if (CanNode.IOCard_Type == LHC_101)
        {
            AI_Data[LHC_101_NODE].AI0_Data = ((RxMsgData_IO[0] << 8)
                    | (RxMsgData_IO[1])) * 0.001;
            AI_Data[LHC_101_NODE].AI1_Data = ((RxMsgData_IO[2] << 8)
                    | (RxMsgData_IO[3])) * 0.001;
            AI_Data[LHC_101_NODE].AI2_Data = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            AI_Data[LHC_101_NODE].AI3_Data = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;

            AI_SensorData.LVL_101 = AI_Data[LHC_101_NODE].AI0_Data;
            AI_SensorData.LVL_102 = AI_Data[LHC_101_NODE].AI1_Data;
            AI_SensorData.PRT_101 = AI_Data[LHC_101_NODE].AI2_Data;
            AI_SensorData.PRT_103 = AI_Data[LHC_101_NODE].AI3_Data;
        }
        else if (CanNode.IOCard_Type == LHC_102)
        {
            AI_Data[LHC_102_NODE].AI0_Data = ((RxMsgData_IO[0] << 8)
                    | (RxMsgData_IO[1])) * 0.001;
            AI_Data[LHC_102_NODE].AI1_Data = ((RxMsgData_IO[2] << 8)
                    | (RxMsgData_IO[3])) * 0.001;
            AI_Data[LHC_102_NODE].AI2_Data = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            AI_Data[LHC_102_NODE].AI3_Data = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;

            AI_SensorData.LVL_401 = AI_Data[LHC_102_NODE].AI0_Data;
            AI_SensorData.COS_101 = AI_Data[LHC_102_NODE].AI1_Data;
            AI_SensorData.PRT_102 = AI_Data[LHC_102_NODE].AI2_Data;
            AI_SensorData.PRT_103 = AI_Data[LHC_102_NODE].AI3_Data;
        }

        switch (State.Present_St)
        {

        case OI_PURGE:

            if ((AI_SensorData.HYS_101 <= 7) && (AI_SensorData.HYS_401 <= 7)
                    && (AI_SensorData.HYS_501 <= 7))
            {
                IO_Flags.hydrogenValuesOK = 1;
            }
            else if ((AI_SensorData.HYS_101 >= 7.5)
                    && (AI_SensorData.HYS_401 >= 7.5)
                    && (AI_SensorData.HYS_501 >= 7.5))
            {
                IO_Flags.hydrogenValuesOK = 0;
            }
            break;

        case OI_IO_POWER:

            if ((AI_SensorData.HYS_101 <= 7) && (AI_SensorData.HYS_401 <= 7)
                    && (AI_SensorData.HYS_501 <= 7))
            {
                IO_Flags.hydrogenValuesOK2 = 1;
            }
            else if ((AI_SensorData.HYS_101 >= 7.5)
                    || (AI_SensorData.HYS_401 >= 7.5)
                    || (AI_SensorData.HYS_501 >= 7.5))
            {
                IO_Flags.hydrogenValuesOK2 = 0;
            }

            if ((AI_SensorData.LVL_101 <= 8)
                    || (AI_SensorData.LVL_102 <= 8))
            {
                IO_Flags.WaterLevelOk = 0;
            }
            else if ((AI_SensorData.LVL_101 >= 10.0)
                    && (AI_SensorData.LVL_102 >= 10.0))
            {
                IO_Flags.WaterLevelOk = 1;
            }

            if ((AI_SensorData.PRT_102 >= 12.0)
                    ||(AI_SensorData.PRT_103 >= 12.0)
                    ||(AI_SensorData.PRT_301 >= 5.6))
            {
                IO_Flags.PressureNotOK = 1;
            }

            break;
        }

        CanaRead.MBox16 = 0;
    }
    else if (CanaRead.MBox17 == 1)
    {
        CAN_readMessage(CANA_BASE, MAILBOX_17, RxMsgData_IO);

        CanaRead.IO_AIBLK2_MSGID = CanaRegs.CAN_IF2ARB.bit.ID;
        CanaRead.IO_AIBLK2_MSGID = CanaRead.IO_AIBLK2_MSGID & CAN_NODEID_MASK;

        CanNode.IOCard_Node = (Uint16) (CanaRead.IO_AIBLK2_MSGID >> 8);

        if (CanNode.IOCard_Node == 1)
        {
            CanNode.IOCard_Type = LPC_100;
        }
        else if (CanNode.IOCard_Node == 2)
        {
            CanNode.IOCard_Type = LHC_101;
        }
        else if (CanNode.IOCard_Node == 3)
        {
            CanNode.IOCard_Type = LHC_102;
        }

        if (CanNode.IOCard_Type == LPC_100)
        {
            AI_Data[LPC_100_NODE].AI4_Data = ((RxMsgData_IO[0] << 8)
                    | (RxMsgData_IO[1])) * 0.001;
            AI_Data[LPC_100_NODE].AI5_Data = ((RxMsgData_IO[2] << 8)
                    | (RxMsgData_IO[3])) * 0.001;
            AI_Data[LPC_100_NODE].AI6_Data = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            AI_Data[LPC_100_NODE].AI7_Data = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;
        }
        else if (CanNode.IOCard_Type == LHC_101)
        {
            AI_Data[LHC_101_NODE].AI4_Data = ((RxMsgData_IO[0] << 8)
                    | (RxMsgData_IO[1])) * 0.001;
            AI_Data[LHC_101_NODE].AI5_Data = ((RxMsgData_IO[2] << 8)
                    | (RxMsgData_IO[3])) * 0.001;
            AI_Data[LHC_101_NODE].AI6_Data = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            AI_Data[LHC_101_NODE].AI7_Data = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;
        }
        else if (CanNode.IOCard_Type == LHC_102)
        {
            AI_Data[LHC_102_NODE].AI4_Data = ((RxMsgData_IO[0] << 8)
                    | (RxMsgData_IO[1])) * 0.001;
            AI_Data[LHC_102_NODE].AI5_Data = ((RxMsgData_IO[2] << 8)
                    | (RxMsgData_IO[3])) * 0.001;
            AI_Data[LHC_102_NODE].AI6_Data = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            AI_Data[LHC_102_NODE].AI7_Data = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;
        }

        CanaRead.MBox17 = 0;
    }

    else if (CanaRead.MBox18 == 1)
    {
        CAN_readMessage(CANA_BASE, MAILBOX_18, RxMsgData_IO);

        CanaRead.IO_THERMO_MSGID = CanaRegs.CAN_IF2ARB.bit.ID;
        CanaRead.IO_THERMO_MSGID = CanaRead.IO_THERMO_MSGID & CAN_NODEID_MASK;

        CanNode.IOCard_Node = (Uint16) (CanaRead.IO_THERMO_MSGID >> 8);

        if (CanNode.IOCard_Node == 1)
        {
            CanNode.IOCard_Type = LPC_100;
        }
        else if (CanNode.IOCard_Node == 2)
        {
            CanNode.IOCard_Type = LHC_101;
        }
        else if (CanNode.IOCard_Node == 3)
        {
            CanNode.IOCard_Type = LHC_102;
        }

        if (CanNode.IOCard_Type == LPC_100)
        {
            Thermo_Data[LPC_100_NODE].T0_Data = ((RxMsgData_IO[0] << 8)
                    | (RxMsgData_IO[1])) * 0.001;
            Thermo_Data[LPC_100_NODE].T1_Data = ((RxMsgData_IO[2] << 8)
                    | (RxMsgData_IO[3])) * 0.001;
            Thermo_Data[LPC_100_NODE].T2_Data = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            Thermo_Data[LPC_100_NODE].T3_Data = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;
        }
        else if (CanNode.IOCard_Type == LHC_101)
        {
            Thermo_Data[LHC_101_NODE].T0_Data = ((RxMsgData_IO[0] << 8)
                    | (RxMsgData_IO[1])) * 0.001;
            Thermo_Data[LHC_101_NODE].T1_Data = ((RxMsgData_IO[2] << 8)
                    | (RxMsgData_IO[3])) * 0.001;
            Thermo_Data[LHC_101_NODE].T2_Data = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            Thermo_Data[LHC_101_NODE].T3_Data = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;
        }
        else if (CanNode.IOCard_Type == LHC_102)
        {
            Thermo_Data[LHC_102_NODE].T0_Data = ((RxMsgData_IO[0] << 8)
                    | (RxMsgData_IO[1])) * 0.001;
            Thermo_Data[LHC_102_NODE].T1_Data = ((RxMsgData_IO[2] << 8)
                    | (RxMsgData_IO[3])) * 0.001;
            Thermo_Data[LHC_102_NODE].T2_Data = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            Thermo_Data[LHC_102_NODE].T3_Data = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;
        }

        CanaRead.MBox18 = 0;
    }

    else if (CanaRead.MBox19 == 1)
    {
        CAN_readMessage(CANA_BASE, MAILBOX_19, RxMsgData_IO);

        CanaRead.IO_GENERIC_MSGID = CanaRegs.CAN_IF2ARB.bit.ID;
        CanaRead.IO_GENERIC_MSGID = CanaRead.IO_GENERIC_MSGID & CAN_NODEID_MASK;

        CanNode.IOCard_Node = (Uint16) (CanaRead.IO_GENERIC_MSGID >> 8);

        if (CanNode.IOCard_Node == 1)
        {
            CanNode.IOCard_Type = LPC_100;
        }
        else if (CanNode.IOCard_Node == 2)
        {
            CanNode.IOCard_Type = LHC_101;
        }
        else if (CanNode.IOCard_Node == 3)
        {
            CanNode.IOCard_Type = LHC_102;
        }

        if (CanNode.IOCard_Type == LPC_100)
        {
            IO_DigitalInputRegs[LPC_100_NODE].all = RxMsgData_IO[0];
            IO_DigitalOutputRegs[LPC_100_NODE].all = RxMsgData_IO[1];
            AO_Data[LPC_100_NODE].AO_1 = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            AO_Data[LPC_100_NODE].AO_2 = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;
        }
        else if (CanNode.IOCard_Type == LHC_101)
        {
            IO_DigitalInputRegs[LHC_101_NODE].all = RxMsgData_IO[0];
            IO_DigitalOutputRegs[LHC_101_NODE].all = RxMsgData_IO[1];
            AO_Data[LHC_101_NODE].AO_1 = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            AO_Data[LHC_101_NODE].AO_2 = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;
        }
        else if (CanNode.IOCard_Type == LHC_102)
        {
            IO_DigitalInputRegs[LHC_102_NODE].all = RxMsgData_IO[0];
            IO_DigitalOutputRegs[LHC_102_NODE].all = RxMsgData_IO[1];
            AO_Data[LHC_102_NODE].AO_1 = ((RxMsgData_IO[4] << 8)
                    | (RxMsgData_IO[5])) * 0.001;
            AO_Data[LHC_102_NODE].AO_2 = ((RxMsgData_IO[6] << 8)
                    | (RxMsgData_IO[7])) * 0.001;
        }

        switch (State.Present_St)
        {

        case OI_STAND_BY:

            if((IO_DigitalInputRegs[LPC_100_NODE].all == 0x00)
                    &&(IO_DigitalOutputRegs[LPC_100_NODE].all == 0x00)
                    &&(AO_Data[LPC_100_NODE].AO_1 == 0x00)
                    &&(AO_Data[LPC_100_NODE].AO_2 == 0x00))
            {
                IO_Flags.LPC100_DefaultACK = 1;
            }
            else
            {
                IO_Flags.LPC100_DefaultACK = 0;
            }


        case OI_PURGE:

            if (IO_Flags.PurgeFansON == 1)
            {
                if (IO_DigitalOutputRegs[LPC_100_NODE].bit.DO_bit0 == 1)
                {
                    IO_Flags.PurgeFan1ON_ACK = 1;
                }
                else
                {
                    IO_Flags.PurgeFan1ON_ACK = 0;
                }
            }
            else if (IO_Flags.PurgeFansON == 2)
            {
                if (IO_DigitalOutputRegs[LPC_100_NODE].bit.DO_bit1 == 1)
                {
                    IO_Flags.PurgeFan2ON_ACK = 1;
                }
                else
                {
                    IO_Flags.PurgeFan2ON_ACK = 0;
                }
            }
            else if (IO_Flags.PurgeFansON == 3)
            {
                if (IO_DigitalOutputRegs[LPC_100_NODE].bit.DO_bit2 == 1)
                {
                    IO_Flags.PurgeFan3ON_ACK = 1;
                }
                else
                {
                    IO_Flags.PurgeFan3ON_ACK = 0;
                }
            }
            break;

        case OI_IO_POWER:

            if (IO_DigitalOutputRegs[LPC_100_NODE].bit.DO_bit6 == 1)
            {
                IO_Flags.LHC_POwerON = 1;
            }
            else
            {
                IO_Flags.LHC_POwerON = 0;
            }

            if ((IO_DigitalOutputRegs[LHC_101_NODE].all == 0)
                    && (IO_DigitalInputRegs[LHC_101_NODE].all == 0)
                    && (IO_SetAnalogInputEnableRegs[LHC_101_NODE].all == 0x03))
            {
                IO_Flags.LHC101_DefaultACK = 1;
            }
            else
            {
                IO_Flags.LHC101_DefaultACK = 0;
            }

            if ((IO_DigitalOutputRegs[LHC_102_NODE].all == 0)
                    && (IO_DigitalInputRegs[LHC_102_NODE].all == 0)
                    && (IO_SetAnalogInputEnableRegs[LHC_102_NODE].all == 0x01))
            {
                IO_Flags.LHC102_DefaultACK = 1;
            }
            else
            {
                IO_Flags.LHC102_DefaultACK = 0;
            }

            if((IO_DigitalOutputRegs[LPC_100_NODE].bit.DO_bit7 == 1)
                    &&(AO_Data[LPC_100_NODE].AO_1 == 1.0))
            {
                IO_Flags.VFD101_ONAck = 1;
            }
            else
            {
                IO_Flags.VFD101_ONAck = 0;
            }

            break;
        }
        CanaRead.MBox19 = 0;
    }
}
void Command_IO_Node_Specific_default(int32 nodeID)
{
    CAN_setupMessageObject(CANA_BASE, MAILBOX_20,
                           (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_IO,
                           CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                           CAN_MSG_OBJ_NO_FLAGS,
                           8);

    if (nodeID == CAN_LPC_100_NODE) // LPC I/O Card Node ID
    {
        Set_AO_Data[LPC_100_NODE].AO_1 = 0;
        Set_AO_Data[LPC_100_NODE].AO_2 = 0;

        IO_SetDigitalOutputRegs[LPC_100_NODE].all = DEFAULT_SETDO_LPC_100;
        IO_SetAnalogInputEnableRegs[LPC_100_NODE].all = DEFAULT_SETAI_LPC_100;

        IO_SetAnalogInputEnableRegs[LPC_100_NODE].all = 0x07; // Enable Analog Inputs

        txMsgData[0] = IO_COMMAND_QUERY_GENERIC;
        txMsgData[1] = IO_DO_SET;
        txMsgData[2] = IO_SetDigitalOutputRegs[LPC_100_NODE].all;  // Default
        txMsgData[3] = IO_SetAnalogInputEnableRegs[LPC_100_NODE].all;
        txMsgData[4] = (Set_AO_Data[LPC_100_NODE].AO_1 << 8);
        txMsgData[5] = (Set_AO_Data[LPC_100_NODE].AO_1 & 0xFF);
        txMsgData[6] = (Set_AO_Data[LPC_100_NODE].AO_2 << 8);
        txMsgData[7] = (Set_AO_Data[LPC_100_NODE].AO_2 & 0xFF);
    }
    else if (nodeID == CAN_LHC_101_NODE)
    {
        Set_AO_Data[LHC_101_NODE].AO_1 = 0;
        Set_AO_Data[LHC_101_NODE].AO_2 = 0;

        IO_SetDigitalOutputRegs[LHC_101_NODE].all = DEFAULT_SETDO_LHC_101;
        IO_SetAnalogInputEnableRegs[LHC_101_NODE].all = DEFAULT_SETAI_LHC_101;

        IO_SetAnalogInputEnableRegs[LHC_101_NODE].bit.AI_EN_bit0 = 1;
        IO_SetAnalogInputEnableRegs[LHC_101_NODE].bit.AI_EN_bit1 = 1; // Enable Level Sensors LVL101 and LVL102

        txMsgData[0] = IO_COMMAND_QUERY_GENERIC;
        txMsgData[1] = IO_DO_SET;
        txMsgData[2] = IO_SetDigitalOutputRegs[LHC_101_NODE].all;
        txMsgData[3] = IO_SetAnalogInputEnableRegs[LHC_101_NODE].all;
        txMsgData[4] = (Set_AO_Data[LHC_101_NODE].AO_1 << 8);
        txMsgData[5] = (Set_AO_Data[LHC_101_NODE].AO_1 & 0xFF);
        txMsgData[6] = (Set_AO_Data[LHC_101_NODE].AO_2 << 8);
        txMsgData[7] = (Set_AO_Data[LHC_101_NODE].AO_2 & 0xFF);
    }
    else if (nodeID == CAN_LHC_102_NODE)
    {
        Set_AO_Data[LHC_102_NODE].AO_1 = 0;
        Set_AO_Data[LHC_102_NODE].AO_2 = 0;

        IO_SetDigitalOutputRegs[LHC_102_NODE].all = DEFAULT_SETDO_LHC_102;
        IO_SetAnalogInputEnableRegs[LHC_102_NODE].all = DEFAULT_SETAI_LHC_102;

        IO_SetAnalogInputEnableRegs[LHC_102_NODE].bit.AI_EN_bit0 = 1; // Enable Level Sensors LVL401

        txMsgData[0] = IO_COMMAND_QUERY_GENERIC;
        txMsgData[1] = IO_DO_SET;
        txMsgData[2] = IO_SetDigitalOutputRegs[LHC_102_NODE].all;
        txMsgData[3] = IO_SetAnalogInputEnableRegs[LHC_102_NODE].all;
        txMsgData[4] = (Set_AO_Data[LHC_102_NODE].AO_1 << 8);
        txMsgData[5] = (Set_AO_Data[LHC_102_NODE].AO_1 & 0xFF);
        txMsgData[6] = (Set_AO_Data[LHC_102_NODE].AO_2 << 8);
        txMsgData[7] = (Set_AO_Data[LHC_102_NODE].AO_2 & 0xFF);
    }

    CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);
}

void Command_IO_Node_Specific_Purge(int32 nodeID, Uint16 Stage)
{

    CAN_setupMessageObject(CANA_BASE, MAILBOX_20,
                           (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_IO,
                           CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                           CAN_MSG_OBJ_NO_FLAGS,
                           8);

    IO_SetDigitalOutputRegs[LPC_100_NODE].all = 0;

    if (nodeID == CAN_LPC_100_NODE) // LPC I/O Card Node ID
    {
        if (Stage == 1)
        {
            Set_AO_Data[LPC_100_NODE].AO_1 = 0;
            Set_AO_Data[LPC_100_NODE].AO_2 = 0;

            IO_SetDigitalOutputRegs[LPC_100_NODE].bit.DO_bit0 = 1; // Turn ON Purge Fans 1

            txMsgData[0] = IO_COMMAND_QUERY_GENERIC;
            txMsgData[1] = IO_DO_SET;
            txMsgData[2] = IO_SetDigitalOutputRegs[LPC_100_NODE].all; // LPC Purge Fans ON DO_0 and DO_1 high
            txMsgData[3] = IO_SetAnalogInputEnableRegs[LPC_100_NODE].all;
            txMsgData[4] = (Set_AO_Data[LPC_100_NODE].AO_1 << 8);
            txMsgData[5] = (Set_AO_Data[LPC_100_NODE].AO_1 & 0xFF);
            txMsgData[6] = (Set_AO_Data[LPC_100_NODE].AO_2 << 8);
            txMsgData[7] = (Set_AO_Data[LPC_100_NODE].AO_2 & 0xFF);
        }
        else if (Stage == 2)
        {
            IO_SetDigitalOutputRegs[LPC_100_NODE].bit.DO_bit1 = 1; // Turn ON Purge Fans 2

            txMsgData[0] = IO_COMMAND_QUERY_GENERIC;
            txMsgData[1] = IO_DO_SET;
            txMsgData[2] = IO_SetDigitalOutputRegs[LPC_100_NODE].all; // LPC Purge Fans ON DO_0 and DO_1 high
            txMsgData[3] = IO_SetAnalogInputEnableRegs[LPC_100_NODE].all;
            txMsgData[4] = (Set_AO_Data[LPC_100_NODE].AO_1 << 8);
            txMsgData[5] = (Set_AO_Data[LPC_100_NODE].AO_1 & 0xFF);
            txMsgData[6] = (Set_AO_Data[LPC_100_NODE].AO_2 << 8);
            txMsgData[7] = (Set_AO_Data[LPC_100_NODE].AO_2 & 0xFF);
        }
        else if (Stage == 3)
        {
            IO_SetDigitalOutputRegs[LPC_100_NODE].bit.DO_bit2 = 1; // Turn ON Purge Fans 3

            txMsgData[0] = IO_COMMAND_QUERY_GENERIC;
            txMsgData[1] = IO_DO_SET;
            txMsgData[2] = IO_SetDigitalOutputRegs[LPC_100_NODE].all; // LPC Purge Fans ON DO_0 and DO_1 high
            txMsgData[3] = IO_SetAnalogInputEnableRegs[LPC_100_NODE].all;
            txMsgData[4] = (Set_AO_Data[LPC_100_NODE].AO_1 << 8);
            txMsgData[5] = (Set_AO_Data[LPC_100_NODE].AO_1 & 0xFF);
            txMsgData[6] = (Set_AO_Data[LPC_100_NODE].AO_2 << 8);
            txMsgData[7] = (Set_AO_Data[LPC_100_NODE].AO_2 & 0xFF);
        }
    }

    CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);
}

void Query_IO_Node_Specific(int32 nodeID)
{
    Uint16 Query_No;

    CAN_setupMessageObject(CANA_BASE, MAILBOX_20,
                           (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_IO,
                           CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                           CAN_MSG_OBJ_NO_FLAGS,
                           8);

    if (QueryType.IO == AI_BLK1_QUERY)
    {
        Query_No = IO_QUERY_AIBLK1;
    }
    else if (QueryType.IO == AI_BLK2_QUERY)
    {
        Query_No = IO_QUERY_AIBLK2;
    }
    else if (QueryType.IO == THERMO_QUERY)
    {
        Query_No = IO_QUERY_THERMO;
    }
    else if (QueryType.IO == GENERIC_QUERY)
    {
        Query_No = IO_QUERY_GENERIC;
    }

    txMsgData[0] = IO_GENERAL_QUERY;
    txMsgData[1] = Query_No;
    txMsgData[2] = 0x0;
    txMsgData[3] = 0x0;
    txMsgData[4] = 0x0;
    txMsgData[5] = 0x0;
    txMsgData[6] = 0x0;
    txMsgData[7] = 0x0;

    CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);

}

void Command_IO_Node_Specific_IOPower(int32 nodeID, Uint16 Stage)
{

    CAN_setupMessageObject(CANA_BASE, MAILBOX_20,
                           (nodeID << 8) | MSG_ID_MODEL_SPECIFIC_QUERY_IO,
                           CAN_MSG_FRAME_EXT, CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                           CAN_MSG_OBJ_NO_FLAGS,
                           8);

    if (nodeID == CAN_LPC_100_NODE) // LPC I/O Card Node ID
    {
        if (Stage == 1)
        {
            Set_AO_Data[LPC_100_NODE].AO_1 = 0;
            Set_AO_Data[LPC_100_NODE].AO_2 = 0;

            IO_SetDigitalOutputRegs[LPC_100_NODE].all = 0;
            IO_SetAnalogInputEnableRegs[LPC_100_NODE].all = 0;

            IO_SetDigitalOutputRegs[LPC_100_NODE].bit.DO_bit6 = 1; // Turn On Relay to power LHC I/O & VS Cards

            txMsgData[0] = IO_COMMAND_QUERY_GENERIC;
            txMsgData[1] = IO_DO_SET;
            txMsgData[2] = IO_SetDigitalOutputRegs[LPC_100_NODE].all;
            txMsgData[3] = IO_SetAnalogInputEnableRegs[LPC_100_NODE].all;
            txMsgData[4] = (Set_AO_Data[LPC_100_NODE].AO_1 << 8);
            txMsgData[5] = (Set_AO_Data[LPC_100_NODE].AO_1 & 0xFF);
            txMsgData[6] = (Set_AO_Data[LPC_100_NODE].AO_2 << 8);
            txMsgData[7] = (Set_AO_Data[LPC_100_NODE].AO_2 & 0xFF);

            CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);
        }
        else if (Stage == 2)
        {

            IO_SetDigitalOutputRegs[LPC_100_NODE].all = 0;
            IO_SetAnalogInputEnableRegs[LPC_100_NODE].all = 0;

            IO_SetDigitalOutputRegs[LPC_100_NODE].bit.DO_bit7 = 1; // Turn ON VFD Relay

            Set_AO_Data[LPC_100_NODE].AO_1 = 1.0; // 1V corresponds to 10Hz in VFD
            Set_AO_Data[LPC_100_NODE].AO_2 = 0;

            txMsgData[0] = IO_COMMAND_QUERY_GENERIC;
            txMsgData[1] = IO_DO_SET;
            txMsgData[2] = IO_SetDigitalOutputRegs[LPC_100_NODE].all; // Turn On power to LHC IO Card Do_5 High
            txMsgData[3] = IO_SetAnalogInputEnableRegs[LPC_100_NODE].all;
            txMsgData[4] = (Set_AO_Data[LPC_100_NODE].AO_1 << 8);
            txMsgData[5] = (Set_AO_Data[LPC_100_NODE].AO_1 & 0xFF);
            txMsgData[6] = (Set_AO_Data[LPC_100_NODE].AO_2 << 8);
            txMsgData[7] = (Set_AO_Data[LPC_100_NODE].AO_2 & 0xFF);

            CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);
        }
        else if(Stage == 3)
        {
            txMsgData[0] = IO_GENERAL_QUERY;
            txMsgData[1] = IO_COMMAND_QUERY_GENERIC;
            txMsgData[2] = 0x00;
            txMsgData[3] = 0x00;
            txMsgData[4] = 0x00;
            txMsgData[5] = 0x00;
            txMsgData[6] = 0x00;
            txMsgData[7] = 0x00;

            CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);
        }

    }
    else if (nodeID == CAN_LHC_101_NODE)
    {
        if (Stage == 1)
        {
            Set_AO_Data[LHC_101_NODE].AO_1 = 0;
            Set_AO_Data[LHC_101_NODE].AO_2 = 0;

            IO_SetAnalogInputEnableRegs[LHC_101_NODE].all = 0;
            IO_SetDigitalOutputRegs[LHC_101_NODE].all = 0;

            IO_SetAnalogInputEnableRegs[LHC_101_NODE].bit.AI_EN_bit0 = 1; // Enable LVL 101
            IO_SetAnalogInputEnableRegs[LHC_101_NODE].bit.AI_EN_bit1 = 1; // Enable LVL 102


            txMsgData[0] = IO_COMMAND_QUERY_GENERIC;
            txMsgData[1] = IO_DO_SET;
            txMsgData[2] = IO_SetDigitalOutputRegs[LPC_100_NODE].all;
            txMsgData[3] = IO_SetAnalogInputEnableRegs[LPC_100_NODE].all;
            txMsgData[4] = (Set_AO_Data[LHC_101_NODE].AO_1 << 8);
            txMsgData[5] = (Set_AO_Data[LHC_101_NODE].AO_1 & 0xFF);
            txMsgData[6] = (Set_AO_Data[LHC_101_NODE].AO_2 << 8);
            txMsgData[7] = (Set_AO_Data[LHC_101_NODE].AO_2 & 0xFF);

            CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);
        }
        else if (Stage == 2)
        {
            Set_AO_Data[LHC_101_NODE].AO_1 = 0;
            Set_AO_Data[LHC_101_NODE].AO_2 = 0;

            IO_SetAnalogInputEnableRegs[LHC_101_NODE].all = 0;
            IO_SetDigitalOutputRegs[LHC_101_NODE].all = 0;

            IO_SetAnalogInputEnableRegs[LHC_101_NODE].bit.AI_EN_bit2 = 1;  // Enable PRT 101
            IO_SetAnalogInputEnableRegs[LHC_101_NODE].bit.AI_EN_bit3 = 1;  // Enable PRT 103
            IO_SetAnalogInputEnableRegs[LHC_101_NODE].bit.AI_EN_bit4 = 1;  // Enable PRT 301

            txMsgData[0] = IO_COMMAND_QUERY_AIBLK1;
            txMsgData[1] = IO_DONTCARE;
            txMsgData[2] = IO_SetDigitalOutputRegs[LPC_100_NODE].all;
            txMsgData[3] = IO_SetAnalogInputEnableRegs[LPC_100_NODE].all;
            txMsgData[4] = (Set_AO_Data[LHC_101_NODE].AO_1 << 8);
            txMsgData[5] = (Set_AO_Data[LHC_101_NODE].AO_1 & 0xFF);
            txMsgData[6] = (Set_AO_Data[LHC_101_NODE].AO_2 << 8);
            txMsgData[7] = (Set_AO_Data[LHC_101_NODE].AO_2 & 0xFF);

            CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);
        }
        else if (Stage == 3)
        {
            txMsgData[0] = IO_GENERAL_QUERY;
            txMsgData[1] = IO_COMMAND_QUERY_AIBLK1;
            txMsgData[2] = 0x00;
            txMsgData[3] = 0x00;
            txMsgData[4] = 0x00;
            txMsgData[5] = 0x00;
            txMsgData[6] = 0x00;
            txMsgData[7] = 0x00;

            CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);
        }
        else if (Stage == 4)
        {
            txMsgData[0] = IO_GENERAL_QUERY;
            txMsgData[1] = IO_COMMAND_QUERY_AIBLK2;
            txMsgData[2] = 0x00;
            txMsgData[3] = 0x00;
            txMsgData[4] = 0x00;
            txMsgData[5] = 0x00;
            txMsgData[6] = 0x00;
            txMsgData[7] = 0x00;

            CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);
        }
    }
    else if (nodeID == CAN_LHC_102_NODE)
    {
        if (Stage == 1)
        {
            txMsgData[0] = IO_GENERAL_QUERY;
            txMsgData[1] = IO_COMMAND_QUERY_AIBLK1;
            txMsgData[2] = 0x00;
            txMsgData[3] = 0x00;
            txMsgData[4] = 0x00;
            txMsgData[5] = 0x00;
            txMsgData[6] = 0x00;
            txMsgData[7] = 0x00;

            CAN_sendMessage(CANA_BASE, MAILBOX_20, 8, txMsgData);
        }
    }
}
