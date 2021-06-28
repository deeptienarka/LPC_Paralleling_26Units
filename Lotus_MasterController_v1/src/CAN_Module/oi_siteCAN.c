/*
 * oi_siteCAN.c
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
uint16_t test_flag=0;
uint16_t xxx=233;
void oi_CANB_Site_Event();
void oi_CANB_InitRead_Site();

void oi_CANB_InitRead_Site()
{
      CanbRead.MBox2 = CAN_readMessage(CANB_BASE, MAILBOX_2, RxMsgData_Site); //
      CanbRead.MBox3 = CAN_readMessage(CANB_BASE, MAILBOX_3, RxMsgData_Site); // Receive Mailbox of AI Block 2 Data
      CanbRead.MBox4 = CAN_readMessage(CANB_BASE, MAILBOX_4, RxMsgData_Site); // Receive Mailbox of Thermocouple Data
}


void oi_CANB_Site_Event()
{

   SiteCANCommands.MeasVoltParams_PSU[CanNode.PSU_Node] = CANRecdParams.Volt_Node[CanNode.PSU_Node] * 100;
   SiteCANCommands.MeasCurrParams_PSU[CanNode.PSU_Node] = CANRecdParams.Curr_Node[CanNode.PSU_Node] * 100;

    if(test_flag == 1)
    {

        txMsgData_Site[0] = (SiteCANCommands.MeasVoltParams_PSU[CanNode.PSU_Node] & 0xFF00) >> 8;
        txMsgData_Site[1] = (SiteCANCommands.MeasVoltParams_PSU[CanNode.PSU_Node] & 0x00FF);
        txMsgData_Site[2] = 0xAA;
        txMsgData_Site[3] = 0xAA;
        txMsgData_Site[4] = 0xAA;
        txMsgData_Site[5] = 0xAA;
        txMsgData_Site[6] = 0xAA;
        txMsgData_Site[7] = 0xAA;

      //  SiteCANCommands.MeasCurrParams_PSU[CanNode.PSU_Node] = ((txMsgData_Site[0] << 8) + txMsgData_Site[1]) * 0.01;

        CAN_sendMessage(CANB_BASE, MAILBOX_1, MSG_DATA_LENGTH_TX_CANB,
                        txMsgData_Site);
    }

    oi_CANB_InitRead_Site();

    if (CanbRead.MBox2 == 1)
    {
        CanbRead.MsgID1_Site = CanbRegs.CAN_IF2ARB.bit.ID;
        CanbRead.MsgID1_Site = CanbRead.MsgID1_Site & CAN_NODEID_MASK;

        SiteCANCommands.StateChange = RxMsgData_Site[0];
        SiteCANCommands.OFF_Command = RxMsgData_Site[1];

        CanbRead.MBox2 = 0;
    }


    if (CanbRead.MBox3 == 1)
    {
        txMsgData_Site[0] = SiteCANCommands.StateChange;
        txMsgData_Site[1] = SiteCANCommands.Max_Current_Reference_int >> 8;
        txMsgData_Site[2] = SiteCANCommands.Max_Current_Reference_int & 0xFF;
        txMsgData_Site[3] = SiteCANCommands.Voltage_Reference_int >> 8;
        txMsgData_Site[4] = SiteCANCommands.Voltage_Reference_int & 0xFF;
        txMsgData_Site[5] = NodePresent;

        CAN_sendMessage(CANB_BASE, MAILBOX_6, 8, txMsgData_Site);

        CanbRead.MBox3 = 0;
    }


    if (CanbRead.MBox4 == 1)
    {

        CanbRead.MsgID3_Site = CanbRegs.CAN_IF2ARB.bit.ID;
        CanbRead.MsgID3_Site = CanbRead.MsgID3_Site & CAN_NODEID_MASK;

            SiteCANCommands.Voltage_Reference_int = ((RxMsgData_Site[0] >> 4) * 1000)
                    + ((RxMsgData_Site[0] & 0X0F) * 100) + ((RxMsgData_Site[1] >> 4) * 10)
                    + ((RxMsgData_Site[1] & 0x0F) * 1);

            SiteCANCommands.Voltage_Reference_fract = ((RxMsgData_Site[2] >> 4) * 1000)
                    + ((RxMsgData_Site[2] & 0X0F) * 100) + ((RxMsgData_Site[3] >> 4) * 10)
                    + ((RxMsgData_Site[3] & 0x0F) * 1);
            SiteCANCommands.Voltage_Reference =
                    SiteCANCommands.Voltage_Reference_int
                            + (SiteCANCommands.Voltage_Reference_fract * 0.0001);

            SiteCANCommands.Max_Current_Reference_int = ((RxMsgData_Site[4] >> 4) * 1000)
                    + ((RxMsgData_Site[4] & 0X0F) * 100) + ((RxMsgData_Site[5] >> 4) * 10)
                    + ((RxMsgData_Site[5] & 0x0F) * 1);

            SiteCANCommands.Max_Current_Reference_fract = ((RxMsgData_Site[6] >> 4) * 1000)
                    + ((RxMsgData_Site[6] & 0X0F) * 100) + ((RxMsgData_Site[7] >> 4) * 10)
                    + ((RxMsgData_Site[7] & 0x0F) * 1);
            SiteCANCommands.Max_Current_Reference =
                    SiteCANCommands.Max_Current_Reference_int
                            + (SiteCANCommands.Max_Current_Reference_fract * 0.0001);

            CanbRead.MBox4 = 0;
    }


}
