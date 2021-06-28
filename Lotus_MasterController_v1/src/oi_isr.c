/*
 * oi_isr.c
 *
 *  Created on: Jan 15, 2021
 *      Author: enArka
 */
/*****************************************************************************************************************
 * Include Files                                                                                                  *
 *****************************************************************************************************************/

#include "F28x_Project.h"              // Standard libraries headers
#include "F2837xS_Device.h"
#include "math.h"
#include "stdlib.h"
#include "oi_externVariables.h"
#include "oi_genStructureDefs.h"
#include "oi_globalPrototypes.h"
Uint32 cnt1 = 0, cnt2 = 0;
Uint16 trig1 = 0, trig2 = 0;
Uint16 fsmCounter = 0;

interrupt void epwm1_isr(void)
{

    fsmCounter++;

    if(fsmCounter > 20)  // FSM occurs once evry 2ms (100us * 20 = 2ms)
    {
        oi_vState();
        fsmCounter = 0;
    }

    //Auto Bus on enabled

    if (CanaRegs.CAN_CTL.bit.Init == 1)
    {
        EALLOW;
        CanaRegs.CAN_CTL.bit.ABO = 1;
        DELAY_US(0.1);
        EDIS;
    }

    if (CanbRegs.CAN_CTL.bit.Init == 1)
    {
        EALLOW;
        CanbRegs.CAN_CTL.bit.ABO = 1;
        DELAY_US(0.1);
        EDIS;
    }

    cnt1++;

    if(cnt1 > 50000)
    {
        trig1 = 1;
        cnt1 = 51000;
    }
    else
    {
        trig1 = 0;
    }

    cnt2++;

    if(cnt2 > 60000)
    {
        trig2 = 1;
        cnt2 = 61000;
    }
    else
    {
        trig2 = 0;
    }

    ui_Scheduler_Index++;

    if (ui_Scheduler_Index >= 600) //60msec for the scheduler
    {
        ui_Scheduler_Index = 0;
    }

    // EVENT SCHEDULER
    // Check if the present index matches the ui_Scheduled_Occurrence of the next event. If it does then
    // this event_index is copied to ui_General_Event_Initiated.
    // Using ui_General_Event_Initiated the event is executed in main by calling the function pointer stored in the
    // General_Event_Array[ui_Event_Initiated].p_fn_EventDriver()
    if (ui_Scheduler_Index
            == Event_Array[ui_Next_Event_In_Line].ui_Scheduled_Occurrence)
    {
        ui_Event_Initiated = ui_Next_Event_In_Line;

        // Roll back ui_Next_Event_In_Line at the end of the array
        if (ui_Next_Event_In_Line < (TOTAL_NO_OF_EVENTS - 1))
        {
            ui_Next_Event_In_Line++;
        }
        else
        {
            ui_Next_Event_In_Line = 0;
        }

        Global_Flags.fg_Scheduled_General_Event_Flag = 1;
    }
    //
    // Clear INT flag for this timer
    //
    EPwm1Regs.ETCLR.bit.INT = 1;

    //
    // Acknowledge this interrupt to receive more interrupts from group 3
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

interrupt void canISR(void)
{
    uint32_t status;
    Uint16 k;

    //
    // Read the CAN interrupt status to find the cause of the interrupt
    //
    status = CAN_getInterruptCause(CANA_BASE);

    //
    // If the cause is a controller status interrupt, then get the status
    //
    if(status == CAN_INT_INT0ID_STATUS)
    {
        //
        // Read the controller status.  This will return a field of status
        // error bits that can indicate various errors.  Error processing
        // is not done in this example for simplicity.  Refer to the
        // API documentation for details about the error status bits.
        // The act of reading this status will clear the interrupt.
        //
        status = CAN_getStatus(CANA_BASE);

        //
        // Check to see if an error occurred.
        //
        if(((status  & ~(CAN_STATUS_TXOK | CAN_STATUS_RXOK)) != 7) &&
           ((status  & ~(CAN_STATUS_TXOK | CAN_STATUS_RXOK)) != 0))
        {
            //
            // Set a flag to indicate some errors may have occurred.
            //
            errorFlag = 1;
        }
    }


    //
    // Check if the cause is the receive message object 2
    //
    else if(status == MAILBOX_13)
    {
        //
        // Get the received message
        //
        CAN_readMessage(CANA_BASE, MAILBOX_13, RxMsgData_VSInt);

        CanaRead.MsgIdVSInt = CanaRegs.CAN_IF2ARB.bit.ID;
        CanaRead.MsgIdVSInt = CanaRead.MsgIdVSInt & CAN_NODEID_MASK;
        CanNode.VS_NodeInt = (Uint16) (CanaRead.MsgIdVSInt >> 8);

        CANRecdParams.Max_Cell_Num1_Int[CanNode.VS_Node] = (Uint16)(RxMsgData_VSInt[0]);

        CANRecdParams.Max_Cell_Volt1_Int[CanNode.VS_Node] = (((RxMsgData_VSInt[1] << 8)) + RxMsgData_VSInt[2]) * 0.0001;

        CANRecdParams.Max_Cell_Num2_Int[CanNode.VS_Node] = (Uint16)(RxMsgData_VSInt[3]);

        CANRecdParams.Max_Cell_Volt2_Int[CanNode.VS_Node] = (((RxMsgData_VSInt[4] << 8)) + RxMsgData_VSInt[5]) * 0.0001;

        for(k=0;k<8;k++)
        {
            RxMsgData_VSInt[k] = 0;
        }


        // Maximum Cell voltage1, Max Cell Voltage2, Corresponding Cell Number and Node is obtained here.

        CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[CanNode.VS_NodeInt] = CANRecdParams.Max_Cell_Volt1_Int[CanNode.VS_NodeInt]; // Highest Value from all nodes
        CANRecdParams.Max_Cell_Number_All_Nodes_Int[CanNode.VS_NodeInt] = CANRecdParams.Max_Cell_Num1_Int[CanNode.VS_NodeInt]; // Highest Value from all nodes

        CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[CanNode.VS_NodeInt+6] = CANRecdParams.Max_Cell_Volt2_Int[CanNode.VS_NodeInt]; // Second Highest Value from all nodes
        CANRecdParams.Max_Cell_Number_All_Nodes_Int[CanNode.VS_NodeInt+6] = CANRecdParams.Max_Cell_Num2_Int[CanNode.VS_NodeInt]; // Second Highest Value from all nodes

        CANRecdParams.Max_Cell_Voltage_Node_Num_Int[CanNode.VS_NodeInt] = CanNode.VS_NodeInt;
        CANRecdParams.Max_Cell_Voltage_Node_Num_Int[CanNode.VS_NodeInt + 6] = CanNode.VS_NodeInt;

//        // Minimum Cell voltage1, Minimum Cell Voltage2, Corresponding Cell Number and Node is obtained here.
//
//        CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[CanNode.VS_NodeInt] = CANRecdParams.Min_Cell_Volt1_Int[CanNode.VS_NodeInt]; // Lowest Value from all nodes
//        CANRecdParams.Min_Cell_Number_All_Nodes_Int[CanNode.VS_NodeInt] = CANRecdParams.Min_Cell_Num1_Int[CanNode.VS_NodeInt]; // Lowest Value from all nodes
//
//        CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[CanNode.VS_NodeInt+6] = CANRecdParams.Max_Cell_Volt2_Int[CanNode.VS_NodeInt]; // Second Lowest Value from all nodes
//        CANRecdParams.Min_Cell_Number_All_Nodes_Int[CanNode.VS_NodeInt+6] = CANRecdParams.Max_Cell_Num2_Int[CanNode.VS_NodeInt]; // Second Lowest Value from all nodes
//
//        CANRecdParams.Min_Cell_Voltage_Node_Num_Int[CanNode.VS_NodeInt] = CanNode.VS_NodeInt;
//        CANRecdParams.Min_Cell_Voltage_Node_Num_Int[CanNode.VS_NodeInt + 6] = CanNode.VS_NodeInt;

        oi_Cell_MaxVoltInt();
      //  oi_Cell_MinVoltInt();

        //
        // Getting to this point means that the RX interrupt occurred on
        // message object 2, and the message RX is complete.  Clear the
        // message object interrupt.
        //
        CAN_clearInterruptStatus(CANA_BASE, MAILBOX_13);

        //
        // Increment a counter to keep track of how many messages have been
        // received. In a real application this could be used to set flags to
        // indicate when a message is received.
        //
        rxMsgCount++;

        //
        // Since the message was received, clear any error flags.
        //
        errorFlag = 0;
    }



    //
    // Clear the global interrupt flag for the CAN interrupt line
    //
    CAN_clearGlobalInterruptStatus(CANA_BASE, CAN_GLOBAL_INT_CANINT0);

    //
    // Acknowledge this interrupt located in group 9
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}


void oi_Cell_MaxVoltInt(void)
{
    Uint16 j = 0;

    if(CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[1] > CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[2])
    {
        CANRecdParams.Highest_Cell_Volt_Int[0] = CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[1];
        CANRecdParams.Highest_Cell_Num_Int[0] = CANRecdParams.Max_Cell_Number_All_Nodes_Int[1];
        CANRecdParams.Highest_Cell_Volt_Node_Int[0] = CANRecdParams.Max_Cell_Voltage_Node_Num_Int[1];


        CANRecdParams.Highest_Cell_Volt_Int[1] = CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[2];
        CANRecdParams.Highest_Cell_Num_Int[1] = CANRecdParams.Max_Cell_Number_All_Nodes_Int[2];
        CANRecdParams.Highest_Cell_Volt_Node_Int[1] = CANRecdParams.Max_Cell_Voltage_Node_Num_Int[2];

    }
    else
    {
        CANRecdParams.Highest_Cell_Volt_Int[0] = CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[2];
        CANRecdParams.Highest_Cell_Num_Int[0] = CANRecdParams.Max_Cell_Number_All_Nodes_Int[2];
        CANRecdParams.Highest_Cell_Volt_Node_Int[0] = CANRecdParams.Max_Cell_Voltage_Node_Num_Int[2];

        CANRecdParams.Highest_Cell_Volt_Int[1] = CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[1];
        CANRecdParams.Highest_Cell_Num_Int[1] = CANRecdParams.Max_Cell_Number_All_Nodes_Int[1];
        CANRecdParams.Highest_Cell_Volt_Node_Int[1] = CANRecdParams.Max_Cell_Voltage_Node_Num_Int[1];


    }

    for(j=3;j<13;j++)
    {
        if(CANRecdParams.Highest_Cell_Volt_Int[0] < CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[j])
        {
            CANRecdParams.Highest_Cell_Volt_Int[1] = CANRecdParams.Highest_Cell_Volt_Int[0];
            CANRecdParams.Highest_Cell_Volt_Int[0] = CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[j];

            CANRecdParams.Highest_Cell_Num_Int[1] = CANRecdParams.Highest_Cell_Num_Int[0];
            CANRecdParams.Highest_Cell_Num_Int[0] = CANRecdParams.Max_Cell_Number_All_Nodes_Int[j];

            CANRecdParams.Highest_Cell_Volt_Node_Int[1] = CANRecdParams.Highest_Cell_Volt_Node_Int[0];
            CANRecdParams.Highest_Cell_Volt_Node_Int[0] = CANRecdParams.Max_Cell_Voltage_Node_Num_Int[j];

        }
        else if(CANRecdParams.Highest_Cell_Volt_Int[1] <= CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[j])
        {
            CANRecdParams.Highest_Cell_Volt_Int[1] = CANRecdParams.Max_Cell_Voltage_All_Nodes_Int[j];
            CANRecdParams.Highest_Cell_Num_Int[1] = CANRecdParams.Max_Cell_Number_All_Nodes_Int[j];
            CANRecdParams.Highest_Cell_Volt_Node_Int[1] = CANRecdParams.Max_Cell_Voltage_Node_Num_Int[j];
        }
    }
}


//void oi_Cell_MinVoltInt(void)
//{
//    Uint16 j = 0;
//
//    if(CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[1] > CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[2])
//    {
//        CANRecdParams.Lowest_Cell_Volt_Int[0] = CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[1];
//        CANRecdParams.Lowest_Cell_Num_Int[0] = CANRecdParams.Min_Cell_Number_All_Nodes_Int[1];
//        CANRecdParams.Lowest_Cell_Volt_Node_Int[0] = CANRecdParams.Max_Cell_Voltage_Node_Num_Int[1];
//
//
//        CANRecdParams.Lowest_Cell_Volt_Int[1] = CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[2];
//        CANRecdParams.Lowest_Cell_Num_Int[1] = CANRecdParams.Min_Cell_Number_All_Nodes_Int[2];
//        CANRecdParams.Lowest_Cell_Volt_Node_Int[1] = CANRecdParams.Min_Cell_Voltage_Node_Num_Int[2];
//
//    }
//    else
//    {
//        CANRecdParams.Lowest_Cell_Volt_Int[0] = CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[2];
//        CANRecdParams.Lowest_Cell_Num_Int[0] = CANRecdParams.Min_Cell_Number_All_Nodes_Int[2];
//        CANRecdParams.Lowest_Cell_Volt_Node_Int[0] = CANRecdParams.Min_Cell_Voltage_Node_Num_Int[2];
//
//        CANRecdParams.Lowest_Cell_Volt_Int[1] = CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[1];
//        CANRecdParams.Lowest_Cell_Num_Int[1] = CANRecdParams.Min_Cell_Number_All_Nodes_Int[1];
//        CANRecdParams.Lowest_Cell_Volt_Node_Int[1] = CANRecdParams.Min_Cell_Voltage_Node_Num_Int[1];
//
//
//    }
//
//    for(j=3;j<13;j++)
//    {
//        if(CANRecdParams.Lowest_Cell_Volt_Int[0] < CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[j])
//        {
//            CANRecdParams.Lowest_Cell_Volt_Int[1] = CANRecdParams.Lowest_Cell_Volt_Int[0];
//            CANRecdParams.Lowest_Cell_Volt_Int[0] = CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[j];
//
//            CANRecdParams.Lowest_Cell_Num_Int[1] = CANRecdParams.Lowest_Cell_Num_Int[0];
//            CANRecdParams.Lowest_Cell_Num_Int[0] = CANRecdParams.Min_Cell_Number_All_Nodes_Int[j];
//
//            CANRecdParams.Lowest_Cell_Volt_Node_Int[1] = CANRecdParams.Lowest_Cell_Volt_Node_Int[0];
//            CANRecdParams.Lowest_Cell_Volt_Node_Int[0] = CANRecdParams.Min_Cell_Voltage_Node_Num_Int[j];
//
//        }
//        else if(CANRecdParams.Lowest_Cell_Volt_Int[1] <= CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[j])
//        {
//            CANRecdParams.Lowest_Cell_Volt_Int[1] = CANRecdParams.Min_Cell_Voltage_All_Nodes_Int[j];
//            CANRecdParams.Lowest_Cell_Num_Int[1] = CANRecdParams.Min_Cell_Number_All_Nodes_Int[j];
//            CANRecdParams.Lowest_Cell_Volt_Node_Int[1] = CANRecdParams.Min_Cell_Voltage_Node_Num_Int[j];
//        }
//    }
//}
