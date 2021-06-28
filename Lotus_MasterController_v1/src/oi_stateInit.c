/*
 * oi_stateInit.c
 *
 *  Created on: Oct 13, 2020
 *      Author: enArka
 */



#include <oi_can.h>
#include "F2837xS_Device.h"
#include "oi_state.h"
#include "oi_externVariables.h"
#include "oi_scheduler.h"
#include "oi_globalPrototypes.h"

//
EVENT Event_Array[TOTAL_NO_OF_EVENTS];
GLOBAL_FLAGS Global_Flags;
CANA_timers_t CANA_timers;
CANA_Ui_Regs_R CANA_Ui_Regs;
CAN_Read CanaRead,CanbRead;
Site_CANCommands SiteCANCommands;
CAN_Node CanNode;
Query_Type QueryType;
SystemParams SystemParam;
CAN_Recd_Params CANRecdParams;
union FAULT_REGS FaultRegs[29];
union FAULT_REGS FaultRegs[29];
union DC_FAULT_REGS DcFaultRegs;
union NODE_FAULT_REGS NodeFaultRegs;
union ACTIVE_NODE_REGS ActiveNodeRegs;
union ACTIVE_NODE_REGS1 ActiveNodeRegs1;
union IO_DO_REG IO_DigitalOutputRegs[TOTAL_IO_NODE_NUMBER], IO_SetDigitalOutputRegs[TOTAL_IO_NODE_NUMBER];
union IO_DI_REG IO_DigitalInputRegs[TOTAL_IO_NODE_NUMBER];
union AI_ENABLE_REG IO_AnalogInputEnableRegs[TOTAL_IO_NODE_NUMBER], IO_SetAnalogInputEnableRegs[TOTAL_IO_NODE_NUMBER];
AOx_Data AO_Data[TOTAL_IO_NODE_NUMBER], Set_AO_Data[TOTAL_IO_NODE_NUMBER];
AIx_Data AI_Data[TOTAL_IO_NODE_NUMBER];
AI_SensorData1 AI_SensorData;
THERMODATA Thermo_Data[TOTAL_IO_NODE_NUMBER];
IO_FLAG IO_Flags;
SYSTEM_STATE State;
STATE_TIMERS_t State_timers;
void oi_globalVar_init(void)
{
 Uint16 i,j;
    // CAN variables Initialization

    for(i=0;i<=8;i++)
    {
        txMsgData[i] = 0;
        RxMsgData[i] = 0;
        frac[i] = 0;
        temp5[i] = 0;

    }

    for(i=0;i<8;i++)
    {
        for (j = 0; j < 32; j++)
        {
            CANRecdParams.Meas_Cell_Volt[i][j] = 0;
        }
    }
    for(i = 1;i<12; i++)
    {
        CANRecdParams.Max_Cell_Voltage_All_Nodes[i] = 0;
        CANRecdParams.Max_Cell_Number_All_Nodes[i] = 0;
        CANRecdParams.Max_Cell_Voltage_Node_Num[i] = 0;
    }

    for(i = 1;i<8; i++)
    {
        CANRecdParams.Max_Cell_Num1[i] = 0;
        CANRecdParams.Max_Cell_Num2[i] = 0;

        CANRecdParams.Max_Cell_Volt1[i] = 0;
        CANRecdParams.Max_Cell_Volt2[i] = 0;

        CANRecdParams.Min_Cell_Volt1[i] = 0;
        CANRecdParams.Min_Cell_Volt2[i] = 0;

        CANRecdParams.Min_Cell_Num1[i] = 0;
        CANRecdParams.Min_Cell_Num2[i] = 0;


    }

    for(i = 1;i<2; i++)
    {
        CANRecdParams.Highest_Cell_Num[i] = 0;
        CANRecdParams.Highest_Cell_Volt[i] = 0;
        CANRecdParams.Highest_Cell_Volt_Node[i] = 0;
    }

    CanNode.VS_Node = 0;
    CANA_Ui_Regs.Node = 0;
    CANA_Ui_Regs.Voltage_Set = 0.0;
    CANA_Ui_Regs.Current_Set = 0.0;
    QueryType.PSU = IDLE_PSU;
    State.Present_St = OI_ST_INIT;


    // Initialize pointer to initial location in memory
    ui_Scheduler_Index = 0;
    ui_Event_Initiated = 0;
    ui_Next_Event_In_Line = 0;

//    // Event initialization.
//    // Event sequence is stored in a structure object that contains 2 pieces of information:
//    //  1. A function pointer that points to the particular Eventn() function.
//    //  2. The ISR number in which the particular event must occur.
//
//    //  The information for all events is stored in this way in an array of structures, the array is
//    //  ordered in the sequence that the events occur. The number of array locations is equal to the number
//    // of events occurring in one cycle.
//
//    // Event 1: - CAN-A module TX PSU Event.
//    // Event 2: - CAN-A module RX PSU Event + CAN-A module TX VS Event.
      // Event 3: - CAN-A module RX VS Event + CAN-A module TX IO Event.
      // Event 4: - CAN-A module RX IO Event.
      // Event 5: - CAN-B module TX/RX Site Controller Event.
//
    Event_Array[0].p_fn_EventDriver = oi_Event1;
    Event_Array[0].ui_Scheduled_Occurrence = 5;             // 5-25 2ms         // CAN TX event PSU
//
    Event_Array[1].p_fn_EventDriver = oi_control_Event1;
    Event_Array[1].ui_Scheduled_Occurrence = 25;            // 26-44 2ms        // FSM ??

    Event_Array[2].p_fn_EventDriver = oi_control_Event2;
    Event_Array[2].ui_Scheduled_Occurrence = 45;            // 45-64 2ms        // Heart Beat??

    Event_Array[3].p_fn_EventDriver = oi_control_Event3;
    Event_Array[3].ui_Scheduled_Occurrence = 65;            // 65-84 2ms        // faults check??

    Event_Array[4].p_fn_EventDriver = oi_control_Event4;
    Event_Array[4].ui_Scheduled_Occurrence =  85;           // 85-104 2ms       // PLC drive ??

    Event_Array[5].p_fn_EventDriver = oi_control_Event5;
    Event_Array[5].ui_Scheduled_Occurrence =  105;          // 105-124 2ms      // VFD drive ??

    Event_Array[6].p_fn_EventDriver = oi_Event2;
    Event_Array[6].ui_Scheduled_Occurrence = 125;           // 125-164 4ms      // CAN RX event PSU + CAN TX event VS


    Event_Array[7].p_fn_EventDriver = oi_control_Event2;
    Event_Array[7].ui_Scheduled_Occurrence = 165;           // 165-184 2ms      // FSM ??

    Event_Array[8].p_fn_EventDriver = oi_control_Event2;
    Event_Array[8].ui_Scheduled_Occurrence = 185;           // 185-204 2ms      // Heart Beat??

    Event_Array[9].p_fn_EventDriver = oi_control_Event3;
    Event_Array[9].ui_Scheduled_Occurrence = 205;           // 205-224 2ms      // faults detection ??

    Event_Array[10].p_fn_EventDriver = oi_control_Event4;
    Event_Array[10].ui_Scheduled_Occurrence = 225;           // 225-244 2ms     // PLC??

    Event_Array[11].p_fn_EventDriver = oi_control_Event5;
    Event_Array[11].ui_Scheduled_Occurrence = 245;           // 245-364 12ms     // VFD Drive ??

    Event_Array[12].p_fn_EventDriver = oi_Event3;
    Event_Array[12].ui_Scheduled_Occurrence = 365;           // 365-404 4ms     // CAN RX event VS + CAN TX Event IO

    Event_Array[13].p_fn_EventDriver = oi_control_Event2;
    Event_Array[13].ui_Scheduled_Occurrence = 405;           // 405-425 2ms     // FSM ??

    Event_Array[14].p_fn_EventDriver = oi_control_Event2;
    Event_Array[14].ui_Scheduled_Occurrence = 425;           // 425-444 2ms     // Heart Beat ??

    Event_Array[15].p_fn_EventDriver = oi_control_Event3;
    Event_Array[15].ui_Scheduled_Occurrence = 445;           // 445-464 2ms     // Fault Detection ??

    Event_Array[16].p_fn_EventDriver = oi_control_Event4;
    Event_Array[16].ui_Scheduled_Occurrence = 465;           // 465-484 2ms     // PLC ??

    Event_Array[17].p_fn_EventDriver = oi_control_Event5;
    Event_Array[17].ui_Scheduled_Occurrence = 485;           // 485-504 2ms     // VFD Drive ??

    Event_Array[18].p_fn_EventDriver = oi_Event4;
    Event_Array[18].ui_Scheduled_Occurrence = 505;           // 405-524 2ms     // CAN RX IO

    Event_Array[19].p_fn_EventDriver = oi_Event5;
    Event_Array[19].ui_Scheduled_Occurrence = 525;           // 525-564 4ms     // CAN Site Controller TX/RX

    Event_Array[20].p_fn_EventDriver = oi_control_Event2;
    Event_Array[20].ui_Scheduled_Occurrence = 565;           // 565-585 2ms

    Event_Array[21].p_fn_EventDriver = oi_control_Event2;
    Event_Array[21].ui_Scheduled_Occurrence = 585;           // 585-005 2ms

}

//-----------------------------------Event Drivers------------------------------------------------//

////------------------------------------------------------------------------------------------------//
Uint16 oi_Event1()      // Occurrence:  - CAN-A module TX event PSU
{
    oi_CANA_TX_Event_PSU();
    return SUCCESS;
}

////------------------------------------------------------------------------------------------------//
Uint16 oi_Event2()      // Occurrence:  - CAN-A module RX event PSU + CAN-A module TX event VS
{
    oi_CANA_RX_Event_PSU();
//    oi_CANA_TX_Event_VS();
    return SUCCESS;
}
////------------------------------------------------------------------------------------------------//
Uint16 oi_Event3()      // Occurrence:  - CAN-A module RX event VS + CAN-A module TX event I/O
{
//    oi_CANA_RX_Event_VS();
    oi_CANA_TX_Event_IO();
    return SUCCESS;
}
////------------------------------------------------------------------------------------------------//
Uint16 oi_Event4()      // Occurrence:  - CAN-A module RX event IO
{
    oi_CANA_RX_Event_IO();
    return SUCCESS;
}
////------------------------------------------------------------------------------------------------//
Uint16 oi_Event5()      // Occurrence:  - CANB module Site Controller
{
    oi_CANB_Site_Event();
    return SUCCESS;
}
////------------------------------------------------------------------------------------------------//
Uint16 oi_control_Event1()
{
    CurrentSetLogic();

    return SUCCESS;
}
////------------------------------------------------------------------------------------------------//
Uint16 oi_control_Event2()
{
    return SUCCESS;
}

////------------------------------------------------------------------------------------------------//
Uint16 oi_control_Event3()
{
    return SUCCESS;
}

////------------------------------------------------------------------------------------------------//
Uint16 oi_control_Event4()
{
    return SUCCESS;
}

////------------------------------------------------------------------------------------------------//
Uint16 oi_control_Event5()
{
    return SUCCESS;
}
