//#################################################################################################################
//# 							enArka Instruments proprietary
//# File: ei_UserDefinedIsr.c
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.
//# All Rights Reserved
//#
//#################################################################################################################

//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
#include "ExternalVariables.h"
//#################################################################################################################
									/*-------Global Prototypes----------*/
// interrupt void ei_vEPWM6_underflow_ISR();
// interrupt void ei_vSciaFifoRX_ISR();
// interrupt void ei_vCpu_Timer0_ISR();

//#####################	Scheduler Interrupt #######################################################################
// Function Name: void EPWM6_underflow_ISR
// Return Type: void
// Arguments:   void
// Description: This is the timer interrupt that gives the time for scheduled events. Only interrupt event is KickDog.
//				Interrupt Frequency: 1 ms
//				Total time given by this interrupt is 500ms (Reinitialise Interrupt_No = 1 when Interrupt_No = 500)

					//Scheduler Table
// EventNo		EventName		InterruptNos				OccurrenceCycle
//		 1		CheckFault		30,130,230,330,430			100ms
//
//		 2 		DisplayEvent	500							250ms

//		 3		EncoderEvent	91,191,291,391,491			100ms

//		 4		KeyScanEvent	10,110,210,310,410			100ms

//		 5		OvpAdjust		60							60ms
//				Operation

//		 5		InternalAdc		60							60ms	// Not Used
//				Operation

// 		 6		ExternalAdc		42,82,122,162		 		40ms with 120ms gaps after 4th and 8th occurrence
//				Operation		282,322,362,402

//		 7		SciCommEvent	7,17,27,37,47,57,67, 		10ms event
//								77,87,97
//								107,117,127,137,147,157,167,
//								177,187,197
//								207,217,227,237,247,257,267,
//								277,287,297
//								307,317,327,337,347,357,367,
//								377,387,397
//								407,417,427,437,447,457,467,
//								477,487,497

//		 8		CanCommEvent	4,14,24,34,44,54,64, 		10ms event
//								74,84,94
//								104,114,124,134,144,154,164,
//								174,184,194
//								204,214,224,234,244,254,264,
//								274,284,294
//								304,314,324,334,344,354,364,
//								374,384,394
//								404,414,424,434,444,454,464,
//								474,484,494

//		9	   TemperatureSenseEvent		20				500ms event

//		10 	   WriteLastSettings			450 			500ms event	// TEMPORARY. ONLY DURING DEVELOPMENT

//#################################################################################################################
Int8 sciQR = 0;
Int8 FaultChar[4];
Int8 i_sci;
Int8 msgStart;
//#####################	Scheduler Interrupt #######################################################################
// Function Name: EPWM6_underflow_ISR
// Return Type: void
// Arguments:   void
// Description: This is the timer interrupt that gives the time for scheduled events.
//				Interrupt Frequency: 1 ms
//#################################################################################################################
interrupt void ei_vEPWM6_underflow_ISR()
{
//    CANFailCnt++;
    if(CANFailCnt >= CAN_FAIL_SET_CNT)
    {
        CANFailCnt = CAN_FAIL_SET_CNT;
        CANFailTrig = 1;
    }

	// Check for the Event to executed
	if(ui_Interrupt_Number == Event_Array[ui_Next_Event_In_Line].ui_Scheduled_Occurrence)
	{
		ui_Event_Initiated = ui_Next_Event_In_Line;
		if(ui_Next_Event_In_Line < (TOTAL_NO_OF_EVENTS - 1))
		{
			ui_Next_Event_In_Line++;

			// AD7705 channel configurations (Active channel being converted and Next channel to be converted) are done here
			if(ui_Interrupt_Number == 42)
			{
				Global_Flags.fg_Active_AD7705_Channel = CH2;
			}
			else if(ui_Interrupt_Number == 162)
			{
				Global_Flags.fg_Next_AD7705_Channel = CH1;
			}
			else if(ui_Interrupt_Number == 282)
			{
				Global_Flags.fg_Active_AD7705_Channel = CH1;
			}
			else if(ui_Interrupt_Number == 402)
			{
				Global_Flags.fg_Next_AD7705_Channel = CH2;
			}
			else
			{
				asm("	nop");
			}
		}
		else
		{
			ui_Next_Event_In_Line = 0;
		}
		Global_Flags.fg_Scheduled_Event_Flag = TRUE;
	}

	// Increment current rise timer when output is ON
	if((OUTPUT_ON) && (Timers.ui_Current_Rise_Timer != Product_Info.ui_Current_Rise_Time))
	{
		INCREMENT_CURRENT_RISE_TIMER;
	}

	// Increment EEPROM write cycle timer just after a write has been completed
	if(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED)
	{
		INCREMENT_EEPROM_WRITE_CYCLE_TIMER;
	}

	ui_Interrupt_Number++;
	if(ui_Interrupt_Number == (NO_OF_INTERRUPTS_IN_ONE_CYCLE + 1))
	{
		// 500ms time elapsed. Next time interrupt occurs we should be at 1.
		ui_Interrupt_Number = 1;
	}

	// SRQ TRANMISSION LOGIC
	// The SRQ concept is simple for both SCPI and CAN

	// SCPI: If the STATUS_BYTE_REGISTER changes from all 0's to 1 and SRQ is enabled for that change in enable register, SRQ is raised and Service Request bit is set.
	// The Service Request bit is cleared when the particular STATUS_BYTE_REGISTER gets cleared

	// CAN: If any of the bits in Fault Event or Operation Condition gets set SRQ is raised.

	// For both modes (SCPI and CAN) SRQ is raised for change from all 0's to a single bit set only once.

	// If an SRQ condition has occurred and its notification has been enabled.
	// Note: MSS_RQS_bit always masked.
	if(((Service_Request_Regs.STATUS_BYTE_REGISTER.all & ~(MSS_RQS_BIT)) & Service_Request_Regs.SERVICE_REQUEST_ENABLE.all)  != NO_EVENT)
	{
		// RQS bit will be set in case one of the other bits is 0.
		Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_Service_Request = SET;

		// Both Present and Previous Status must be 0 for SRQ to be generated
		// MSS/RQS bit always masked.
		if(Service_Request_Regs.STATUS_BYTE_PREVIOUS_REGISTER.all == NO_EVENT)
		{
			Global_Flags.fg_Uart_SRQ_Tranmsit = TRUE;

			/*// Transition from no event to event. SRQ needs to be raised.
			int8 s_Temp[6];

			s_Temp[0] = '!';
			s_Temp[1] = (Reference.ui_Set_Address/10) + ASCII_FOR_0;
			s_Temp[2] = (Reference.ui_Set_Address%10) + ASCII_FOR_0;
			s_Temp[3] = '\r';
			s_Temp[4] = '\n';

//			NO RTL INSIDE INTERRUPTS. DIRECTLY WRITING THE VALUES TO THE TRANMSIT ARRAY IS FASTER AND SAFER.
//			sprintf(s_Temp,"!%02u\r\n",Reference.ui_Set_Address);

			ei_vEnqueueOutputMessageInTransmitBuffer(s_Temp);*/

			Service_Request_Regs.STATUS_BYTE_PREVIOUS_REGISTER.all = Service_Request_Regs.STATUS_BYTE_REGISTER.all;	// Copy to previous*/*/
		}
		else	// Previous Event register non zero. SRQ already issued. Copy enabled and occurred events to Previous register
		{
			Service_Request_Regs.STATUS_BYTE_PREVIOUS_REGISTER.all = (Service_Request_Regs.STATUS_BYTE_REGISTER.all & Service_Request_Regs.STATUS_BYTE_PREVIOUS_REGISTER.all);	// Copy to previous*/*/
		}
	}
	else
	{
		// Clear the RQS Service_Request bit.
		Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_Service_Request = CLEARED;
		Service_Request_Regs.STATUS_BYTE_PREVIOUS_REGISTER.all = CLEARED;
	}

	// CAN SRQs
	if((Global_Flags.fg_CAN_Srq_Sent == FALSE) && ((Fault_Regs.FAULT_EVENT.all != NO_EVENT) || (Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all != NO_EVENT)))
	{
		// Transmit SRQ
		ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
		ECanaShadow.CANTRS.bit.TRS31 = 1;
		ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;

		// Make flag true
		Global_Flags.fg_CAN_Srq_Sent = TRUE;
	}
	else if((Fault_Regs.FAULT_EVENT.all == NO_EVENT) && (Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all == NO_EVENT))
	{
		// Clear the SRQ Flag
		Global_Flags.fg_CAN_Srq_Sent = FALSE;
	}
	else
	{
		// No Operation for some Event and CAN SRQ already sent
		asm("	nop");
	}

//	This setting of certain Event registers is given after SRQ transmission because it would enable retransmission of SRQ when these Event registers are cleared on reading.

	// Form the Event register for Operation.
	Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all |= (Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.all & Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all);

	// Set the Operation_Summary in the Status Byte register when some condition detected and notification enabled
	if(Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all != NO_EVENT)
	{
		// Set the appropriate status byte bit
		Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_Operation_Summary = SET;
	}

	// Set the Standard_Event_Summary bit in the Status Byte register when an event detected and notification enabled
	if((Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.all & Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_ENABLE.all)  != NO_EVENT)
	{
		// Set the appropriate status byte bit
		Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_Standard_Event_Summary = SET;
	}

	EPwm6Regs.ETCLR.bit.INT = CLEAR_INTERRUPT;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

//#####################	UART Interrupt ############################################################################
// Function Name: ei_vSciaFifoRX_ISR
// Return Type: void
// Arguments:   void
// Description: This interrupt is called whenever the Uart receives a character. The complete Uart communication
//				is interrupt based. All charcters are received in interrupt only. But taking action on the received
//				data is a scheduled event.

// 				Enable scheduler interrupt inside this interrupt. It has higher priority.

//				Important charcters to be taken care of in the interrupt
//
//	Charcter		Name				Ascii		Hex			Action
//	Backspace		Backspace			8			0x08		Last character cleared. Do not increment Next
//	'\r'			Carriage return		13			0x0D		Signals end of message. Data ready to be serviced
//	'$'				Dollar				36			0x24		User has included Checksum in the communication
//
//#################################################################################################################
interrupt void ei_vSciaFifoRX_ISR()
{
//	Allows a higher priority interrupt to interrupt this ISR
	DINT; 									// Disable when changes have to be made to IER
	IER |= M_INT3; 							// Enable Group 3 interrupts at CPU level
	PieCtrlRegs.PIEACK.all = 0xFFFF; 		// Allow all interrupts at the PIE level by acknowledging it
	EINT;
	Int8 c_ReceivedCharcter;

	//sciQR = 1;
//	c_ReceivedCharcter = 0xFF;

	c_ReceivedCharcter = SciaRegs.SCIRXBUF.all;
//
	    if(c_ReceivedCharcter == 0xFF)
	    {
	        msgStart = 1;
	        i_sci = 0;
	    }

	    if(msgStart == 1)
	    {
	        FaultChar[i_sci] = c_ReceivedCharcter;
	        i_sci++;
	    }

	    if(i_sci > 3)
	    {
	        msgStart = 0;
	    }


    if(c_ReceivedCharcter == LINE_FEED_CHARACTER)
    {
        // do nothing. Line feed is not put into the buffer. This helps to detect command and queries with line feed and
        // line feed
        asm("   nop");
    }
    else    // Any other character. Increment the Rear Subject to:
    {
        INCREMENT_RECEIVED_BUFFER(Receive_Circular_Buffer.i_Rear)

        if(RECEIVE_BUFFER_FULL)
        {
            // Do not overwrite any data. The newest data will be lost and error flag set.
            ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E341);

            // Take rear a step back to its previous location
            DECREMENT_RECEIVED_BUFFER(Receive_Circular_Buffer.i_Rear);
        }
        else    // BUFFER NOT FULL
        {
            Receive_Circular_Buffer.a_c_ReceiveBuffer[Receive_Circular_Buffer.i_Rear] = c_ReceivedCharcter;
        }

	}



	// Clear all interrupt flags
	SciaRegs.SCIFFRX.bit.RXFFOVRCLR=1;   // Clear Overflow flag
	SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag
}

//#####################	TIMER 0 Interrupt #########################################################################
// Function Name: ei_vCpu_Timer0_ISR
// Return Type: void
// Arguments:   void
// Description: This is the timer 0 interrupt that measures the Power On time from when the PSU was first switched ON.
//				Gives an Interrupt every minute.
//#################################################################################################################
interrupt void ei_vCpu_Timer0_ISR()
{
	Product_Info.ul_Product_Power_On_Time_In_Minutes++;
	// Acknowledge this interrupt to receive more interrupts from group 1
   PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
