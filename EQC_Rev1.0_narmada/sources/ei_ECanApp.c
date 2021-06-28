//#################################################################################################################
//# 							enArka Instruments proprietary				  									  #
//# File: ECanApp.c													  									          #
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.					                                          #
//# All Rights Reserved												                                              #
//#                                                                                                               #
//#################################################################################################################

//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
#include "ExternalVariables.h"
//#################################################################################################################
// Specific definesf
volatile struct MBOX MboxShadow;		// To modify mailbox in some cases.
//#################################################################################################################
									/*-------Global Prototypes----------*/
//void ei_vCanCommEvent();
//#################################################################################################################
									/*-------Local Prototypes----------*/
static void ei_vProcessEnarkaGlobalMailbox();
static void ei_vProcessPsuGlobalMailbox();
static void ei_vProcess1UGlobalMailbox();
static void ei_vModelSpecificGlobalMailbox();
static void ei_vOuptutControlMailbox();
static void ei_vStateControlMailbox();
static void ei_vResponseToCanQueries();
static void ei_vPsuCalibrationFromCan();
static void ei_vCanByteCommandProcessing(Uint16 ResponseKey);
static void ei_vCanProgramOutputVoltage(Uint16 DataKey);
static void ei_vCanProgramOutputCurrent(Uint16 DataKey);
static void ei_vCanProgramOvp(Uint16 DataKey);
static void ei_vCanProgramUvl(Uint16 DataKey);
static void ei_vCanSystemCal(Uint16 CommandType);
static void ei_vSetPreCANCalibrationParameters();
int32 watch_expression= 0;
Int8 sci_fecOnCommand = 0;
//#################################################################################################################

//###################################	eCAN Baud Rate Settings  ##################################################
//	We operate at 100Mhz Sysclkout. 
//		
//	Important Formulae used for calculation
//		1. BitRate = SYSCLKOUT/(2*(Bit Time * BRP))
//		2. BitTime = (TSEG1 + TSEG2) + 1
//
//		Note: All registers value are enhanced by 1. So after calcultating above Values we write Value-1 in the register.
// 
//	The following bit timing rules must be fulfilled when determining the bit segment values:
//		� TSEG1(min) >= TSEG2
//		� IPT <= TSEG1 <= 16 TQ
//		� IPT <= TSEG2 <= 8 TQ
//		� IPT = 3/BRP (the resulting IPT has to be rounded up to the next integer value)
//		� 1 TQ <= SJW <= min[4 TQ, TSEG2] (SJW = Synchronization jump width)
//		� To utilize three-time sampling mode, BRP > 5 has to be selected
//
//#################################################################################################################
//							NOTES ON CAN ARCHITECTURE.
//		OBJECTIVE: Since SCPI is so difficult and complex, the motto behind CAN is Keep It Simple Boss.
//
//		1. CAN cannot implement or share the same architecture as SCPI.
//		2. Every User-CAN message will check for Data range, Data length and integrity mismatch and generate only error
//		3. Errors will be signalled only through 2 CAN mailboxes. (SRQ and Error mailbox).
//		   responses instantaneously.
//		4. For SRQ implementation CAN will take into account only Fault registers. In case additional events need
//		   to be includede in SRQ separate registers for CAN alone can be created.
//		5. SRQ will have a global enable/disable switch and also the Fault Enable register for selective implenetation of SRQ.
//		   Any new registers created for SRQ reporting will also have Enable switches for individual bit. (like fault enable)
//		6. CAN will not respond for successful execution of a command.
//		7. Parameter Query can be implemented for all paramters and Fault Register and Operation condition register only.
//		8. Global commands addressed to all units in the bus will evoke no response.
//		9. Calibration commands not supported is a valid error signal that will be reported when user tries to 
//		   change calibration data.
//		10. No queue of any sort implemeted in CAN so the user grouping commands and sending at a time may 
//			cause Data loss.
//		11. Operation condition registers bits will be modified here at appropriate locations to maintain
//			 CAN and RS485 integrity but no other SCPI registers will be touched by CAN. (Fault registers of course
//			 belong to both SCPI and CAN).
//		12. The CAN_Srq_Enable_Switch clears everytime on power up. It is not stored in EEPROM as of now.
//			Therefor the user has to enable them everytime on power up. But CAN State_Control message set has a 
//			byte commmand that can modify PSC. Thus the user can choose to retain all individual enable registers after a power off
//			but he still has to enable CAN_Srq_Enable_Switch everytime during powerup for the PSU to generate SRQs on CAN.

//#####################	Communication Event #######################################################################
// Function Name: CanCommEvent
// Return Type: void
// Arguments:   void
// Description: Does all the CAN communication related operations
//#################################################################################################################
void ei_vCanCommEvent()
{
	/*// Before anything else check if any transmit frames have caused an error. We need to cleasr and cancel them first.
	ECanaShadow.CANES.all = ECanaRegs.CANES.all;
	if(ECanaShadow.CANES.bit.ACKE == CAN_ACK_ERROR)
	{
		// Cancel all pending transmissions.
		ECanaShadow.CANTRR.all = ECanaRegs.CANTRR.all;
		ECanaShadow.CANTRR.bit.TRR31 = RESET_TRANSMISSION_REQUEST;
		ECanaShadow.CANTRR.bit.TRR17 = RESET_TRANSMISSION_REQUEST;
		ECanaShadow.CANTRR.bit.TRR16 = RESET_TRANSMISSION_REQUEST;
		
		ECanaRegs.CANTRR.all = ECanaShadow.CANTRR.all;
		
				// Acknowledge the ACKE bit.
		
		// CANES register is Write 1 to Clear bit. Therefore we need to write 1 to only ACKE bit. We will write 
		// 0 to other bits because writes of 0 have no effect on "Write 1 to clear" bits.
		
		ECanaShadow.CANES.all = 0;		//First make all the bits of CANES shdow register to 0.
		ECanaShadow.CANES.bit.ACKE = ACKNOWLEDGE_CAN_ERROR_FLAG;	// Write 1 to acknowledge this bit
		ECanaRegs.CANES.all = ECanaShadow.CANES.all;		// Copy back
	}*/
	
	ECanaShadow.CANRMP.all = ECanaRegs.CANRMP.all;
	if(ECanaShadow.CANRMP.all != EMPTY)	// Data Received
	{
	    CANFailCnt = 0;

	    CANFailTrig = 0;

	    CANFailEnable = 1;

		// Clear the temp mailbox to clear previously stored data. This data from the actual mailbox is  
		// moved to this temporary storage for ease of calculations.
		MboxShadow.MSGCTRL.all = CLEARED;
		MboxShadow.MDL.all = CLEARED;				
		MboxShadow.MDH.all = CLEARED;
		
		// Always clear the error signaling mailbox initially before message checking.
		ECanaMboxes.MBOX16.MDH.all = 0x00000000;
		ECanaMboxes.MBOX16.MDL.all = 0x00000000;
		
		// The following types of messages are checked each time  a message is received. Each type of messages will be
		// in a different mailbox.
		
		// The following mailboxes are active even before Factory calibration.
		// But for now we will process them only if the PSU is calibrated to avoid any violations of uncalibrated PSU
		
//		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
//		{
			if(ENARKA_GLOBAL_MESSAGES_RECEIVED)
			{
				ei_vProcessEnarkaGlobalMailbox();
			}
			
			if(PSU_GLOBAL_MESSAGES_RECEIVED)
			{
				ei_vProcessPsuGlobalMailbox();
			}
			
			if(GLOBAL_1U_MESSAGES_RECEIVED)
			{
				ei_vProcess1UGlobalMailbox();
			}
//		}
		
		// The following mailboxes will be created only Post factory calibration through RS485
		
		// Check for these mailboxes only post calibration to avoid violations.
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			if(MODEL_SPECIFIC_GLOBAL_MESSAGES_RECEIVED)
			{
				ei_vModelSpecificGlobalMailbox();
			}
			
			if(OUTPUT_CONTROL_MESSAGES_RECEIVED)
			{
				ei_vOuptutControlMailbox();
			}
			
			if(STATE_CONTROL_MESSAGES_RECEIVED)
			{
				ei_vStateControlMailbox();
			}
		}
		
		// This mailbox is created only post RS485 calibration. But once RS485 calibration done, it
		// is always active because this mailbox responds to queries of calibration paramters also.
		if(MESSAGE_RECEIVED_IN_THE_QUERY_MAILBOX)
		{
			ei_vResponseToCanQueries();
		}
		
		// This mailbox is created only post RS485 calibration. But once RS485 calibration done, it
		// is always active. But note that not all commands inside the function are always active.
		// Refer to the comments above the function header for a more detailed note.
		if(MESSAGE_RECEIVED_IN_THE_CALIBRATION_MAILBOX)
		{
			ei_vPsuCalibrationFromCan();
		}
		
		// Individual bits of Shadow RMP are set in respective mailboxes.
		ECanaRegs.CANRMP.all = ECanaShadow.CANRMP.all;		// Copy Back
	}

}

//#####################	CAN Global enArka Messages ################################################################
// Function Name: ei_vProcessEnarkaGlobalMailbox
// Return Type: void
// Arguments:   void
// Description: Checks for enArka Global messages on CAN.
//				These messages are supported by all the CAN based products in enArka.
//				The commands are checked for errors but do not respond with any error signalling.

// 	Commands implemented as of now:
//	ON/OFF
//#################################################################################################################
static void ei_vProcessEnarkaGlobalMailbox()
{
	Uint16 ui_Extension;
	
	// Check the message id's last two nibbles to determine the action to be taken
	ui_Extension = ECanaMboxes.MBOX15.MSGID.all & MBOX15_MSG_ID_MASK;
	
	// Temp mailbox data for ease of calculations in case MBOX data is modified.
	MboxShadow.MSGCTRL.all = ECanaMboxes.MBOX15.MSGCTRL.all;
	MboxShadow.MDL.all = ECanaMboxes.MBOX15.MDL.all;				
	MboxShadow.MDH.all = ECanaMboxes.MBOX15.MDH.all;
	
	switch(ui_Extension)
	{
		case SINGLE_BYTE_EXTENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == ENARKA_GLOBAL_BYTE_COMMANDS_DATA_LENGTH)		
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(ENARKA_GLOBAL_BYTE_COMMANDS);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
					
					// Error deteceted but no error response generated
				}
			}
			else	// Data length violation
			{
				//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
				
				// Error deteceted but no error response generated
			}
			break;
		}
		
		default:
		{
			// Unsupported extension.
			
			// No error signalling for lobal commands
			break;
		}
	}
	
	// Clear RMP
	ECanaShadow.CANRMP.bit.RMP15 = CLEAR_RMP;	// Clear
}

//#####################	CAN PSU enArka Messages ###################################################################
// Function Name: ei_vProcessPsuGlobalMailbox
// Return Type: void
// Arguments:   void
// Description: Checks for Global messages for all enArka PSUs connected on CAN.
//				These messages are supported by all the CAN based PSU in enArka.
//				The commands are checked for errors but do not respond with any error signalling.
//#################################################################################################################
static void ei_vProcessPsuGlobalMailbox()
{
	Uint16 ui_Extension;
	
	// Check the message id's last two nibbles to determine the action to be taken
	ui_Extension = ECanaMboxes.MBOX14.MSGID.all & MBOX14_MSG_ID_MASK;
	
	// Temp mailbox data for ease of calculations in case MBOX data is modified.
	MboxShadow.MSGCTRL.all = ECanaMboxes.MBOX14.MSGCTRL.all;
	MboxShadow.MDL.all = ECanaMboxes.MBOX14.MDL.all;				
	MboxShadow.MDH.all = ECanaMboxes.MBOX14.MDH.all;
	
	switch(ui_Extension)
	{
		case SINGLE_BYTE_EXTENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == PSU_GLOBAL_BYTE_COMMANDS_DATA_LENGTH)		
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(PSU_GLOBAL_BYTE_COMMANDS);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
					
					// Error deteceted but no error response generated
				}
			}
			else if(MboxShadow.MSGCTRL.bit.DLC == PSU_GLOBAL_BYTE_COMMANDS_DATA_LENGTH+1)	// Single Byte commands with one additional parameter
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(PSU_GLOBAL_BYTE_COMMANDS_WITH_ARG);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
					
					// Error deteceted but no error response generated
				}
			}
			else	// Data length violation
			{
				//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
				
				// Error deteceted but no error response generated
			}
			break;
		}
		
		default:
		{
			// Unsupported extension.
			
			// No error signalling for global commands
			break;
		}
	}
	
	// Clear RMP
	ECanaShadow.CANRMP.bit.RMP14 = CLEAR_RMP;	// Clear
}

//#####################	CAN 1U enArka Messages ####################################################################
// Function Name: ei_vProcess1UGlobalMailbox
// Return Type: void
// Arguments:   void
// Description: Checks for Global messages for all enArka 1U PSUs connected on CAN.
//				These messages are supported by all the CAN based 1U PSUs in enArka.
//				The commands are checked for errors but do not respond with any error signalling.

// Right now the CAN messages for 1U_Global (MBOX13) is same as PSU_Global (MBOX14). So the below function passes
// the ResponseKey to ei_vCanByteCommandProcessing(ResponseKey) as the above function. In case we decide to change the commands
// for either PSU_Global and 1U_Global different ResponseKey will have to be passed and new cases created in 
// ei_vCanByteCommandProcessing(ResponseKey).
//#################################################################################################################
static void ei_vProcess1UGlobalMailbox()
{
	Uint16 ui_Extension;
	
	// Check the message id's last two nibbles to determine the action to be taken
	ui_Extension = ECanaMboxes.MBOX13.MSGID.all & MBOX13_MSG_ID_MASK;
	
	// Temp mailbox data for ease of calculations in case MBOX data is modified.
	MboxShadow.MSGCTRL.all = ECanaMboxes.MBOX13.MSGCTRL.all;
	MboxShadow.MDL.all = ECanaMboxes.MBOX13.MDL.all;				
	MboxShadow.MDH.all = ECanaMboxes.MBOX13.MDH.all;
	
	switch(ui_Extension)
	{
		case SINGLE_BYTE_EXTENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == PSU_GLOBAL_BYTE_COMMANDS_DATA_LENGTH)		
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(PSU_GLOBAL_BYTE_COMMANDS);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
					
					// Error deteceted but no error response generated
				}
			}
			else if(MboxShadow.MSGCTRL.bit.DLC == PSU_GLOBAL_BYTE_COMMANDS_DATA_LENGTH+1)	// Single Byte commands with one additional parameter
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(PSU_GLOBAL_BYTE_COMMANDS_WITH_ARG);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
					
					// Error deteceted but no error response generated
				}
			}
			else	// Data length violation
			{
				//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
				
				// Error deteceted but no error response generated
			}
			break;
		}
		

        case PROGRAM_VOLTAGE_EXENSION:
        {
            if(MboxShadow.MSGCTRL.bit.DLC == PROGRAM_VOLT_DATA_LENGTH)
            {
                ei_vCanProgramOutputVoltage(MBOX_VALUE);
            }
            else    // Data length violation
            {
                //ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;

                // Error deteceted but no error response generated
            }

            break;
        }

        case PROGRAM_CURRENT_EXENSION:
        {
            if(MboxShadow.MSGCTRL.bit.DLC == PROGRAM_CURR_DATA_LENGTH)
            {
                ei_vCanProgramOutputCurrent(MBOX_VALUE);
            }
            else    // Data length violation
            {
                //ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;

                // Error deteceted but no error response generated
            }
            break;
        }

		default:
		{
			// Unsupported extension.
			
			// No error signalling for global commands
			break;
		}
	}
	
	// Clear RMP
	ECanaShadow.CANRMP.bit.RMP13 = CLEAR_RMP;	// Clear
}

//#####################	CAN MODEL Global enArka Messages ########################################################
// Function Name: ei_vModelSpecificGlobalMessages
// Return Type: void
// Arguments:   void
// Description: Checks for Global messages for all enArka 1U PSUs of the same model connected on CAN.
//				These messages are supported by all the CAN based 1U PSUs of the same model in enArka.
//				The commands are checked for errors but do not respond with any error signalling.

//	THIS MAILBOX IS CREATED POST FACTORY CALIBRATION FROM RS485

// Right now the single byte messages in yhis model are same as MBOX14 (PSU_GLOBAL).
// This in addition to SINGLE_BYTE_EXTENSION also includes VOLTAGEand CURRENT EXTENSION
//#################################################################################################################
static void ei_vModelSpecificGlobalMailbox()
{
	Uint16 ui_Extension;
	
	// Check the message id's last two nibbles to determine the action to be taken
	ui_Extension = ECanaMboxes.MBOX12.MSGID.all & MBOX12_MSG_ID_MASK;
	
	// Temp mailbox data for ease of calculations in case MBOX data is modified.
	MboxShadow.MSGCTRL.all = ECanaMboxes.MBOX12.MSGCTRL.all;
	MboxShadow.MDL.all = ECanaMboxes.MBOX12.MDL.all;				
	MboxShadow.MDH.all = ECanaMboxes.MBOX12.MDH.all;
	
	switch(ui_Extension)
	{
		case SINGLE_BYTE_EXTENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == PSU_GLOBAL_BYTE_COMMANDS_DATA_LENGTH)		
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(PSU_GLOBAL_BYTE_COMMANDS);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
					
					// Error deteceted but no error response generated
				}
			}
			else if(MboxShadow.MSGCTRL.bit.DLC == PSU_GLOBAL_BYTE_COMMANDS_DATA_LENGTH+1)	// Single Byte commands with one additional parameter
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(PSU_GLOBAL_BYTE_COMMANDS_WITH_ARG);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
					
					// Error deteceted but no error response generated
				}
			}
			else	// Data length violation
			{
				//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
				
				// Error deteceted but no error response generated
			}
			break;
		}
		
		case PROGRAM_VOLTAGE_EXENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == PROGRAM_VOLT_DATA_LENGTH)		
			{
				ei_vCanProgramOutputVoltage(MBOX_VALUE);
			}
			else	// Data length violation
			{
				//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
				
				// Error deteceted but no error response generated
			}	
				
			break;
		}
		
		case PROGRAM_CURRENT_EXENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == PROGRAM_CURR_DATA_LENGTH)		
			{
				ei_vCanProgramOutputCurrent(MBOX_VALUE);
			}
			else	// Data length violation
			{
				//ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
				
				// Error deteceted but no error response generated
			}
			break;
		}
		
        case FAULT_STATUS:
        {
            if(MboxShadow.MSGCTRL.bit.DLC == FAULT_STATUS_QUERY_LENGTH)
            {
               // ei_vCanFaultStatus(MBOX_VALUE);
            }
            else    // Data length violation
            {
                //ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;

                // Error deteceted but no error response generated
            }
            break;
        }

		default:
		{
			// Unsupported extension.
			
			// No error signalling for global commands
			break;
		}
	}
	
	// Clear RMP
	ECanaShadow.CANRMP.bit.RMP12 = CLEAR_RMP;	// Clear
}

//#####################	Ouptut Messages ###########################################################################
// Function Name: ei_vOuptutControlMessages
// Return Type: void
// Arguments:   void
// Description: Checks for Output control messages on CAN.

//	THIS MAILBOX IS CREATED POST FACTORY CALIBRATION FROM RS485
//	THE MESSAGE ID INCLUDES PSU ADDRESS SET FROM THE FRONTPANEL IN THE NODE ID FIELD.

// This Mailbox responds with error signalling on mailbox on MBOX16
//#################################################################################################################
static void ei_vOuptutControlMailbox()
{
	Uint16 ui_Extension;
	
	// Check the message id's last two nibbles to determine the action to be taken
	ui_Extension = ECanaMboxes.MBOX10.MSGID.all & MBOX10_MSG_ID_MASK;
	
	// Temp mailbox data for ease of calculations in case MBOX data is modified.
	MboxShadow.MSGCTRL.all = ECanaMboxes.MBOX10.MSGCTRL.all;
	MboxShadow.MDL.all = ECanaMboxes.MBOX10.MDL.all;				
	MboxShadow.MDH.all = ECanaMboxes.MBOX10.MDH.all;
	
	switch(ui_Extension)
	{
		case SINGLE_BYTE_EXTENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == OUTPUT_CONTROL_BYTE_COMMANDS_DATA_LENGTH)		
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(OUTPUT_CONTROL_BYTE_COMMANDS);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
				}
			}
			else if(MboxShadow.MSGCTRL.bit.DLC == OUTPUT_CONTROL_BYTE_COMMANDS_DATA_LENGTH+1)	// Single Byte commands with one additional parameter
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(OUTPUT_CONTROL_BYTE_COMMANDS_WITH_ARG);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
				}
			}
			else	// Data length violation
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
			}
			break;
		}
		
		case PROGRAM_VOLTAGE_EXENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == PROGRAM_VOLT_DATA_LENGTH)		
			{
				ei_vCanProgramOutputVoltage(MBOX_VALUE);
			}
			else	// Data length violation
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
			}	
				
			break;
		}
		
		case PROGRAM_CURRENT_EXENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == PROGRAM_CURR_DATA_LENGTH)		
			{
				ei_vCanProgramOutputCurrent(MBOX_VALUE);
			}
			else	// Data length violation
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
			}
			break;
		}
		
		case PROGRAM_OVP_EXTENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == PROGRAM_VOLT_DATA_LENGTH)		
			{
				ei_vCanProgramOvp(MBOX_VALUE);
			}
			else	// Data length violation
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
			}	
				
			break;
		}
		
		case PROGRAM_UVL_EXTENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == PROGRAM_CURR_DATA_LENGTH)		
			{
				ei_vCanProgramUvl(MBOX_VALUE);
			}
			else	// Data length violation
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
			}
			break;
		}
		
		default:
		{
			ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_UNSUPPORTED_MSG_ID;
			break;
		}
	}
	
	// Transmit if there is some general response to be transmited
	if(MBOX16_TX_BUFFER_NOT_EMPTY)
	{
		// Transmit response
		ECanaMboxes.MBOX16.MSGCTRL.bit.DLC = 2;
		ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
		ECanaShadow.CANTRS.bit.TRS16 = 1;
		ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
	}
	
	// Clear RMP
	ECanaShadow.CANRMP.bit.RMP10 = CLEAR_RMP;	// Clear
}

//#####################	State Control Messages #####################################################################
// Function Name: ei_vStateControlMessages
// Return Type: void
// Arguments:   void
// Description: Checks for State control messages on CAN.

//	THIS MAILBOX IS CREATED POST FACTORY CALIBRATION FROM RS485
//	THE MESSAGE ID INCLUDES PSU ADDRESS SET FROM THE FRONTPANEL IN THE NODE ID FIELD.

// This Mailbox responds with error signalling on mailbox on MBOX16
//#################################################################################################################
static void ei_vStateControlMailbox()
{
	Uint16 ui_Extension;
	
	// Check the message id's last two nibbles to determine the action to be taken
	ui_Extension = ECanaMboxes.MBOX9.MSGID.all & MBOX9_MSG_ID_MASK;
	
	// Temp mailbox data for ease of calculations in case MBOX data is modified.
	MboxShadow.MSGCTRL.all = ECanaMboxes.MBOX9.MSGCTRL.all;
	MboxShadow.MDL.all = ECanaMboxes.MBOX9.MDL.all;				
	MboxShadow.MDH.all = ECanaMboxes.MBOX9.MDH.all;
	
	switch(ui_Extension)
	{
		case SINGLE_BYTE_EXTENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == STATE_CONTROL_BYTE_COMMANDS_DATA_LENGTH)		
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(STATE_CONTROL_BYTE_COMMANDS);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
				}
			}
			else if(MboxShadow.MSGCTRL.bit.DLC == STATE_CONTROL_BYTE_COMMANDS_DATA_LENGTH+2)	// Single Byte commands with 2 additional parameters
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(STATE_CONTROL_BYTE_COMMANDS_WITH_ARG);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
				}
			}
			else	// Data length violation
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
			}
			break;
		}
		
		default:
		{
			ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_UNSUPPORTED_MSG_ID;
			break;
		}
	}
	
	// Transmit if there is some general response to be transmited
	if(MBOX16_TX_BUFFER_NOT_EMPTY)
	{
		// Transmit response
		ECanaMboxes.MBOX16.MSGCTRL.bit.DLC = 2;
		ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
		ECanaShadow.CANTRS.bit.TRS16 = 1;
		ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
	}
	
	// Clear RMP
	ECanaShadow.CANRMP.bit.RMP9 = CLEAR_RMP;	// Clear
}

//#####################	Query Processing ##########################################################################
// Function Name: ei_vResponseToCanQueries
// Return Type: void
// Arguments:   void
// Description: Checks for QueryType Messages. All Queries are received in this mailbox. And byte0 and byte1 in the data
//	field correspond to the query of a particular parameter. Once a query is received the response for the query is
//	generated in MBOX17.

//	THIS MAILBOX IS CREATED POST FACTORY CALIBRATION FROM RS485
//	THE MESSAGE ID INCLUDES PSU ADDRESS SET FROM THE FRONTPANEL IN THE NODE ID FIELD.

// This Mailbox responds with error signalling on mailbox on MBOX16

// As of now only single Byte extension supported here but this mailbox can accept upto 4 message IDs. (Mask is 0x03)

// The Data field for the Query Bytes must contain the (Command ID + Extension of the particular Response) of MBOX17 
//#################################################################################################################
static void ei_vResponseToCanQueries()
{
	Uint16 ui_Extension;
	
	// Check the message id's last two nibbles to determine the action to be taken
	ui_Extension = ECanaMboxes.MBOX8.MSGID.all & MBOX8_MSG_ID_MASK;
	
	// Temp mailbox data for ease of calculations in case MBOX data is modified.
	MboxShadow.MSGCTRL.all = ECanaMboxes.MBOX8.MSGCTRL.all;
	MboxShadow.MDL.all = ECanaMboxes.MBOX8.MDL.all;				
	MboxShadow.MDH.all = ECanaMboxes.MBOX8.MDH.all;
	
	switch(ui_Extension)
	{
		case SINGLE_BYTE_EXTENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == QUERY_BYTE_DATA_LENGTH)		
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					// Disable the MBOX17 because the message ID field will be changed depending on the query.
					ECanaShadow.CANME.all = ECanaRegs.CANME.all;
					ECanaShadow.CANME.bit.ME17 = 0; // Disable mailbox. We need to change msg id depending on query received.
					ECanaRegs.CANME.all = ECanaShadow.CANME.all;
					
					ei_vCanByteCommandProcessing(QUERY_BYTES);	// Further checking of the data required to determine response
					
					// Appropriate message Id, DLC and Data copied to MBOX17. Re-Enable MBOX17 for transmit operation
					ECanaShadow.CANME.all = ECanaRegs.CANME.all;
					ECanaShadow.CANME.bit.ME17 = 1; // Re-enable mailbox. We need to change msg id depending on query received.
					ECanaRegs.CANME.all = ECanaShadow.CANME.all;
				}
				else	// Data integrity failed
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
				}
			}
			else	// Data length violation
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
			}
			break;
		}
		
		default:
		{
			ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_UNSUPPORTED_MSG_ID;
			break;
		}
	}
	
	// Transmit if there is some general response to be transmited
	if(MBOX16_TX_BUFFER_NOT_EMPTY)
	{
		// Transmit response
		ECanaMboxes.MBOX16.MSGCTRL.bit.DLC = 2;
		ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
		ECanaShadow.CANTRS.bit.TRS16 = 1;
		ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
	}
	else	// No Errors. Appropriate Response generated
	{
		// Transmit the CAN data
		ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
		ECanaShadow.CANTRS.bit.TRS17 = 1;
		ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
	}
	
	// Clear RMP
	ECanaShadow.CANRMP.bit.RMP8 = CLEAR_RMP;	// Clear
}

//#####################	Calibration from CAN #######################################################################
// Function Name: ei_vPsuCalibrationFromCan
// Return Type: void
// Arguments:   void
// Description: Checks for Calibration Messages. 

//	THIS MAILBOX IS CREATED POST FACTORY CALIBRATION FROM RS485. This means that atleast once the user must have calibrated the
//	PSU from RS485 for this mailbox to be available.
//	
// 	NOTE ON OPERATION:
// 	Only the Reset Calibration message is supported here during normal operation (Post calibration). No other commands are supported.
// 	But once the present calibration is reset (Note that resetting the calibration never destroys the CAN model no. from EEPROM), all
// 	the messages in this mailbox becomes active.
//
//	NOTE ON NODE ID FIELD OF THIS MAILBOX:
//	When resetting the Calibration of the PSU during normal operation the NODE ID field of this mailbox will hold the presently set value.
//  But once reset and AC power to PSU is recycled, the PSU will go to default NODE ID of 7 which will also be reflected in the mailbox.
//  All PSUs will exhibit this behaviour. That is why if the AC power is recycled then the PSU must be calibrated in isolation.
//	
//	Alternate Scenario: The present calibration of the PSU is reset and AC power is not recycled. Then the node ID field will
//	remain intact and it should be possible (means needs to be tested) to calibrate PSU from CAN with the same Node ID and
//  also when an uncalibrated PSU is connected to other PSUs on the Bus. 

//	It is important that the AC power is not recycled in between or else the PSU will go to default settings of NODE ID:7
//	
//	This Mailbox responds with error signalling on mailbox on MBOX16
//#################################################################################################################
static void ei_vPsuCalibrationFromCan()
{
	// CAN Calibration is completely differnt to RS485. Here since the PSU always knows the model specs, setting
	// of rated and specified parameters are not a part of CAN Calibration. Only the following parameters need
	// to be entered.
	
	// 1. DATE
	// 2. Voltage Zero and Full
	// 3. Measured Voltage Values
	// 4. Current Zero and Full
	// 5. Measrured Current Parameters.
	
	Uint16 ui_Extension;
	
	// Check the message id's last two nibbles to determine the action to be taken
	ui_Extension = ECanaMboxes.MBOX11.MSGID.all & MBOX11_MSG_ID_MASK;
	
	// Temp mailbox data for ease of calculations in case MBOX data is modified.
	MboxShadow.MSGCTRL.all = ECanaMboxes.MBOX11.MSGCTRL.all;
	MboxShadow.MDL.all = ECanaMboxes.MBOX11.MDL.all;				
	MboxShadow.MDH.all = ECanaMboxes.MBOX11.MDH.all;
	
	switch(ui_Extension)
	{
		case SINGLE_BYTE_EXTENSION:
		{
			if(MboxShadow.MSGCTRL.bit.DLC == CAL_BYTE_COMMAND_DATA_LENGTH)		
			{
				if(MboxShadow.MDL.byte.BYTE0 == MboxShadow.MDL.byte.BYTE1)	// Check for data integrity
				{
					ei_vCanByteCommandProcessing(CALIBRATION_BYTE_COMMANDS);	// Further checking of the data required to determine response
				}
				else	// Data integrity failed
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SINGLE_BYTE_MATCH_ERROR;
				}
			}
			else	// Data length violation
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
			}
			break;
		}
		
		case CALIBRATION_DATE_EXTENSION:
		{
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				// Processed Only If Calibration Done
				if(MboxShadow.MSGCTRL.bit.DLC == CAL_DATE_PARAMETER_DATA_LENGTH)		
				{
					ei_vCanSystemCal(SET_CALIBRATION_DATE);
				}
				else	// Data length violation
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
				}
			}
			else	// Psu Already calibrated. Calibration commands not supported.
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_UNSUPPORTED_CALIBRATION_COMMANDS;
			}
			break;
		}
		
		case MEASURED_VOLTAGE_PARAMS_EXTENSION:
		{
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				// Processed Only If Calibration Done
				if(MboxShadow.MSGCTRL.bit.DLC == CAL_MEASURED_PARAMTERS_DATA_LENGTH)		
				{
					ei_vCanSystemCal(CALIBRATE_VOLTAGE);
				}
				else	// Data length violation
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
				}
			}
			else	// Psu Already calibrated. Calibration commands not supported.
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_UNSUPPORTED_CALIBRATION_COMMANDS;
			}
			break;
		}
		
		case MEASURED_CURRENT_PARAMS_EXTENSION:
		{
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				// Processed Only If Calibration Done
				if(MboxShadow.MSGCTRL.bit.DLC == CAL_MEASURED_PARAMTERS_DATA_LENGTH)		
				{
					ei_vCanSystemCal(CALIBRATE_CURRENT);
				}
				else	// Data length violation
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_DATA_LENGTH_MISMATCH;
				}
			}
			else	// Psu Already calibrated. Calibration commands not supported.
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_UNSUPPORTED_CALIBRATION_COMMANDS;
			}
			break;
		}
		
		default:
		{
			ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_UNSUPPORTED_MSG_ID;
			break;
		}
	}
	
	// Transmit if there is some general response to be transmited
	if(MBOX16_TX_BUFFER_NOT_EMPTY)
	{
		// Transmit response
		ECanaMboxes.MBOX16.MSGCTRL.bit.DLC = 2;
		ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
		ECanaShadow.CANTRS.bit.TRS16 = 1;
		ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
	}
	
	// Clear RMP
	ECanaShadow.CANRMP.bit.RMP11 = CLEAR_RMP;	// Clear
}

//#####################	CAN Byte Commands #########################################################################
// Function Name: ei_vCanByteCommandProcessing
// Return Type: void
// Arguments:   Uint16 ResponseKey
// Description: All Byte commands for the Single Byte extension and Query Commands checking is done here.
//				Each Mailbox with Single byte commands or the like for which the Data field does not specify
//	the parameter value straightaway is processed here. Different Mailbox drivers (functions checking individual
//	mailboxes for data) call this function for processing the single byte commands. 
//#################################################################################################################
static void ei_vCanByteCommandProcessing(Uint16 ResponseKey)
{
	switch(ResponseKey)
	{
		///////////////////// ENARKA GLOBAL BYTE COMMANDS ///////////////////////////////////////////////////////////////////////
		case ENARKA_GLOBAL_BYTE_COMMANDS:
		{
			// check for DataByte 
			if(MboxShadow.MDL.byte.BYTE0 == CAN_OUTPUT_ON_BYTE)	
			{
				// This is a Global Ouptut ON message for all enArka Products. In case the unit is not in the 
				// condition to turn output ON, then this command will have no effect. 
				// No error signalling possible.
				
				State.Mode.bt_Output_Status = ON;						// Turn the state on
				State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;		// Put the PSU in Remote
				Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = CLEARED;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_FEC_ON_BYTE)
			{
			    sci_fecOnCommand = 1;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_OUTPUT_OFF_BYTE)
			{
				State.Mode.bt_Output_Status = OFF;
				sci_fecOnCommand = 0;
				//SWITCH_OUTPUT_OFF;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_RESET_PSU_TO_SAFE_STATE)
			{
				ei_vResetPsuToSafeState();
			}
			else
			{
				// Illegal Command that is not supported. No error signalling possible
				asm("	nop");
			}
			
			break;
		}
		
		///////////////////// PSU GLOBAL BYTE COMMANDS //////////////////////////////////////////////////////////////////////////
		case PSU_GLOBAL_BYTE_COMMANDS:
		{
			// check for DataByte 
			if(MboxShadow.MDL.byte.BYTE0 == CAN_OUTPUT_ON_BYTE)	
			{
				// This is a Global Ouptut ON message for all enArka Products. In case the unit is not in the 
				// condition to turn output ON, then this command will have no effect. 
				// No error signalling possible.
				State.Mode.bt_Output_Status = ON;						// Turn the state on
				State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;		// Put the PSU in Remote
				Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = CLEARED;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_FEC_ON_BYTE)
			{
                sci_fecOnCommand = 1;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_OUTPUT_OFF_BYTE)
			{
			    sci_fecOnCommand = 0;
				State.Mode.bt_Output_Status = OFF;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_RESET_PSU_TO_SAFE_STATE)
			{
				ei_vResetPsuToSafeState();
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_VOLTAGE_MINIMUM)
			{
				ei_vCanProgramOutputVoltage(MIN_VALUE);
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_VOLTAGE_MAXIMUM)
			{
				ei_vCanProgramOutputVoltage(MAX_VALUE);
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_CURRENT_MINIMUM)
			{
				ei_vCanProgramOutputCurrent(MIN_VALUE);
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_CURRENT_MAXIMUM)
			{
				ei_vCanProgramOutputCurrent(MAX_VALUE);
			}
			else
			{
				// Illegal Command that is not supported. No error signalling possible
				asm("	nop");
			}
			
			break;
		}
		
		//------------------- PSU_GLOBAL_BYTE_COMMANDS_WITH_ARG -----------------------------------------------------------------
		case PSU_GLOBAL_BYTE_COMMANDS_WITH_ARG:
		{
			if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SAVE_SETTINGS)
			{
				// Check the mode to be saved in. Right now only 1 mode (last settings will be updated.
				// But this is for future expansion.
				if(MboxShadow.MDL.byte.BYTE2 == MODE_1)
				{
					ei_vWriteLastSettingsEEPROM(Product_Info.ui_Calibration_Status);
				}
				else
				{
					// Illegal parameter.
					// No error signalling for global Messages)
				}
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_RECALL_SETTINGS)
			{
				// Check the mode to be saved in. Right now only 1 mode (last settings will be updated.
				// But this is for future expansion.
				if(MboxShadow.MDL.byte.BYTE2 == MODE_1)
				{
					ei_vRecallLastPSUSettings();
				}
				else
				{
					// Illegal parameter.
					// No error signalling for global Messages)
				}
			}
			else
			{
				// Illegal Command that is not supported. No error signalling possible
				asm("	nop");
			}
			
			break;
		}
		
		///////////////////// OUTPUT CONTROL BYTE COMMANDS ///////////////////////////////////////////////////////////////////////
		case OUTPUT_CONTROL_BYTE_COMMANDS:
		{
			// check for DataByte 
			if(MboxShadow.MDL.byte.BYTE0 == CAN_OUTPUT_ON_BYTE)	
			{
				State.Mode.bt_Output_Status = ON;						// Turn the state on
				State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;		// Put the PSU in Remote
				Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = CLEARED;
				Global_Flags.fg_Output_Switched_On_From_Can = TRUE;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_FEC_ON_BYTE)
			{
                sci_fecOnCommand = 1;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_OUTPUT_OFF_BYTE)
			{
				State.Mode.bt_Output_Status = OFF;
			    sci_fecOnCommand = 0;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_RESET_PSU_TO_SAFE_STATE)
			{
				ei_vResetPsuToSafeState();
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_VOLTAGE_MINIMUM)
			{
				ei_vCanProgramOutputVoltage(MIN_VALUE);
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_VOLTAGE_MAXIMUM)
			{
				ei_vCanProgramOutputVoltage(MAX_VALUE);
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_CURRENT_MINIMUM)
			{
				ei_vCanProgramOutputCurrent(MIN_VALUE);
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_CURRENT_MAXIMUM)
			{
				ei_vCanProgramOutputCurrent(MAX_VALUE);
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_OVP_MINIMUM)
			{
				ei_vCanProgramOvp(MIN_VALUE);
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_OVP_MAXIMUM)
			{
				ei_vCanProgramOvp(MAX_VALUE);
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_UVL_MINIMUM)
			{
				ei_vCanProgramUvl(MIN_VALUE);
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_UVL_MAXIMUM)
			{
				ei_vCanProgramUvl(MAX_VALUE);
			}
			else
			{
				// Illegal Command that is not supported.
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
			}
			break;
		}
		
		//------------------- OUTPUT_CONTROL_BYTE_COMMANDS_WITH_ARG -----------------------------------------------------------------
		case OUTPUT_CONTROL_BYTE_COMMANDS_WITH_ARG:
		{
			if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SAVE_SETTINGS)
			{
				// Check the mode to be saved in. Right now only 1 mode (last settings will be updated.
				// But this is for future expansion.
				if(MboxShadow.MDL.byte.BYTE2 == MODE_1)
				{
					ei_vWriteLastSettingsEEPROM(Product_Info.ui_Calibration_Status);
				}
				else
				{
					// Illegal parameter.
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
				}
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_RECALL_SETTINGS)
			{
				// Check the mode to be saved in. Right now only 1 mode (last settings will be updated.
				// But this is for future expansion.
				if(MboxShadow.MDL.byte.BYTE2 == MODE_1)
				{
					ei_vRecallLastPSUSettings();
				}
				else
				{
					// Illegal parameter.
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
				}
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_FOLDBACK_MODE)
			{
				if(MboxShadow.MDL.byte.BYTE2 == CAN_BYTE_TO_DISABLE_FOLDBACK)
				{
					State.Mode.bt_FoldBack_Mode = FOLDBACK_DISARMED;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Fold_Enabled = CLEARED;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Current_Foldback_Mode = CLEARED;
				}
				else if(MboxShadow.MDL.byte.BYTE2 == CAN_BYTE_TO_ENABLE_FOLDBACK_FROM_CC)
				{
					State.Mode.bt_FoldBack_Mode = FOLDBACK_ARMED_TO_PROTECT_FROM_CC;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Fold_Enabled = SET;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Current_Foldback_Mode = SET;
				}
				else
				{
					// Illegal parameter.
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
				}
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_PROTECTION_DELAY)
			{
				// Byte 2 will receive a value from 0 to 255. This denotes the protection delay in no. of 100ms
				// A value of 0 is not allowed. Minimum is 1 (100ms). Incase 0 received, the value set will be MIN_PROTECTION_DELAY_IN_COUNTS only.
				
				Timers.ui_MaxFoldBackDelayInCounts = MboxShadow.MDL.byte.BYTE2; 
				
				// Check if the value is 0.
				if(Timers.ui_MaxFoldBackDelayInCounts == 0)
				{
					// if it is then copy MIN_PROTECTION_DELAY_IN_COUNTS to both
					Timers.ui_MaxFoldBackDelayInCounts = MIN_PROTECTION_DELAY_IN_COUNTS; 
				}
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_PSU_ACCESS_MODE)
			{
				if(MboxShadow.MDL.byte.BYTE2 == CAN_BYTE_TO_BRING_PSU_TO_LOCAL)
				{	
					State.Mode.bt_PSU_Control = LOCAL;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 1;
				}
				else if(MboxShadow.MDL.byte.BYTE2 == CAN_BYTE_TO_BRING_PSU_TO_REMOTE)
				{	
					State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
				}
				else if(MboxShadow.MDL.byte.BYTE2 == CAN_BYTE_TO_LOCKOUT_LOCAL)
				{	
					State.Mode.bt_PSU_Control = LOCAL_LOCKOUT;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
				}
				else
				{
					// Illegal parameter.
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
				}
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_START_MODE)
			{
				if(MboxShadow.MDL.byte.BYTE2 == CAN_AUTO_RESTART_BYTE)
				{	
					State.Mode.bt_Start_Mode = AUTO_RESTART;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Auto_start_Enabled = SET;
				}
				else if(MboxShadow.MDL.byte.BYTE2 == CAN_SAFE_START_BYTE)
				{	
					State.Mode.bt_Start_Mode = SAFE_START;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Auto_start_Enabled = CLEARED;
				}
				else
				{
					// Illegal parameter.
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
				}
			}
			else
			{
				// Illegal Command that is not supported.
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
			}
			
			break;
		}
		
		///////////////////// STATE CONTROL BYTE COMMANDS////////////////////////////////////////////////////////////////////////////
		case STATE_CONTROL_BYTE_COMMANDS:
		{
			// check for DataByte 
			if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_CLEAR_EVENT_REGISTERS)	
			{
				// Clear the entire Status structure.
				ei_vClearPsuStatusStructure();
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_ENABLE_POWER_ON_STATUS_CLEAR)	
			{
				State.Mode.bt_Power_On_Status_Clear = ON;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_DISABLE_POWER_ON_STATUS_CLEAR)	
			{
				State.Mode.bt_Power_On_Status_Clear = OFF;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_ENABLE_CAN_SRQ)	
			{
				State.Mode.bt_Can_SRQ_Switch = ON;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_DISABLE_CAN_SRQ)	
			{
				State.Mode.bt_Can_SRQ_Switch = OFF;
			}
			else
			{
				// Illegal Command that is not supported.
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
			}
			break;
		}
		
		//------------------- STATE_CONTROL_BYTE_COMMANDS_WITH_ARG -----------------------------------------------------------------
		case STATE_CONTROL_BYTE_COMMANDS_WITH_ARG:
		{
			if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_FAULT_ENABLE)
			{
				// Byte 2 stores MSB of the Fault enable.
				Fault_Regs.FAULT_ENABLE.all = ((Uint16)MboxShadow.MDL.byte.BYTE2) << 8;
				
				// Byte 3 stores LSB of fault enable
				Fault_Regs.FAULT_ENABLE.all |= MboxShadow.MDL.byte.BYTE3;
			}
			else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_OPERATION_CONDITION_ENABLE)
			{
				// Byte 2 stores MSB of Operation Condition enable.
				Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all = ((Uint16)MboxShadow.MDL.byte.BYTE2) << 8;
				
				// Byte 3 stores LSB of Operation Condition enable.
				Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all |= MboxShadow.MDL.byte.BYTE3;
			}
			else
			{
				// Illegal Command that is not supported.
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
			}
			break;
		}
		
		///////////////////// CALIBRATION BYTE COMMANDS//////////////////////////////////////////////////////////////////////////////
		case CALIBRATION_BYTE_COMMANDS:
		{
			if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_RESET_CALIBRATION)
			{
				Uint16 I2C_Tx_Data_Array[1];
		
				// Write Default Value to model number to reset calibration
				I2C_Tx_Data_Array[0] = 0xFF;
				
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				ei_uiI2CWrite(EEPROM_24LC256_ADDRESS,MODEL_NO_ADDR,1,I2C_Tx_Data_Array);
				RESET_EEPROM_WRITE_CYCLE_TIMER;
				
				// Reset PSU to safe state and put it in uncalibrated mode.
				ei_vResetPsuToSafeState();
				Product_Info.ui_Calibration_Status = FACTORY_DEFAULT;
				Product_Info.ui_Model_Number = DEFAULT_1V_1A_MODEL;
			}
			else	// Other commands than the above.
			{
				// Only if the product is in Factory Default mode. Calibrated power supplies will not accept the commands
				if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
				{
					if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_VOLTAGE_ZERO)
					{
						ei_vSetVoltageCurrentForCalib(VOLTAGE_LOW);
						ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SUCCESSFULL_COMMAND;
					}
					else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_VOLTAGE_FULL)
					{
						ei_vSetVoltageCurrentForCalib(VOLTAGE_FULL);
						ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SUCCESSFULL_COMMAND;
					}
					else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_CURRENT_ZERO)
					{
						ei_vSetVoltageCurrentForCalib(CURRENT_LOW);
						ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SUCCESSFULL_COMMAND;
					}
					else if(MboxShadow.MDL.byte.BYTE0 == CAN_BYTE_TO_SET_CURRENT_FULL)
					{
						ei_vSetVoltageCurrentForCalib(CURRENT_FULL);
						ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SUCCESSFULL_COMMAND;
					}
					else	// Illegal command
						ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
				}
				else	// Psu Already calibrated. Calibration commands not supported.
				{
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_UNSUPPORTED_CALIBRATION_COMMANDS;
				}
			}
			break;
		}
		
		///////////////////// CAN QUERIES //////////////////////////////////////////////////////////////////////////////
		case QUERY_BYTES:
		{
			Uint32 ul_Temp;

			if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_PROGRAMMED_PARAMETERS)
			{
				// Transmit Programmed values on Mailbox17
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_MBOX_PROGRAMMED_PARAMS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				// Copy programmed voltage in Byte0-3
				ul_Temp = _IQ15rmpy(Reference.iq_Voltage_Reference,10000);		
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
				
				// Copy programmed current in Byte4-7 
				ul_Temp = _IQ15rmpy(Reference.iq_Current_Reference,10000);
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE5 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDH.byte.BYTE6 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE7 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_OUTPUT_PARAMETERS)
			{
				// Transmit Output values on Mailbox17
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_MBOX_OUTPUT_PARAMS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				// Copy Measured voltage in Byte0-3
				ul_Temp = _IQ15rmpy(External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Calibrated_Value,10000);		
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
				
				// Copy measured current in Byte4-7 
				ul_Temp = _IQ15rmpy(External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Calibrated_Value,10000);
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE5 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE6 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE7 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
			}				
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_RATED_PARAMETERS)
			{
				// Transmit Rated parameters on Mailbox17
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_RATED_PARAMS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				// Copy rated voltage in Byte0-3
				ul_Temp = _IQ15rmpy(Product_Info.iq_Rated_Voltage,10000);		
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
				
				// Copy rated current in Byte4-7
				ul_Temp = _IQ15rmpy(Product_Info.iq_Rated_Current,10000);
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE5 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDH.byte.BYTE6 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE7 = ((ul_Temp/TENTH_PLACE)%10)|((ul_Temp)%10);										// 3rd and 4th digit after decimal point
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_SPECIFIED_PARAMETERS)
			{
				// Transmit Specified parameters on Mailbox17
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_SPECIFIED_PARAMS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				// Copy specified voltage in Byte0-3
				ul_Temp = _IQ15rmpy(Product_Info.iq_Specified_FS_Voltage,10000);		
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
				
				// Copy Specified current in Byte4-7 
				ul_Temp = _IQ15rmpy(Product_Info.iq_Specified_FS_Current,10000);
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE5 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDH.byte.BYTE6 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE7 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_SET_LIMITS)
			{
				// Transmit OVP/UVL values on Mailbox17
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_MBOX_SET_LIMITS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				// Copy OVP Reference in Byte0-3
				ul_Temp = _IQ15rmpy(Reference.iq_OVP_Reference,10000);		
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
				
				// Copy UVL Reference in Byte4-7 
				ul_Temp = _IQ15rmpy(Reference.iq_UVL_Reference,10000);
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE5 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDH.byte.BYTE6 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE7 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
			}			
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_V_CAL_PARAMETERS)
			{
				// Transmit voltage gain and offset values on Mailbox17
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_MBOX_VOLTAGE_CAL_PARAMS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				// Copy Voltage offset in Byte0-3
//				ul_Temp = _IQ15rmpy(Product_Info.iq_Voltage_Ref_Offset,10000);	
					
				// Offset can be negative also and hence for Offset alone we will transmit the IQ15 value (32bit Hexadecimal)
				// To Calculate Offset from the response, convert to decimal and divide by 2^15.
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = (Product_Info.iq_Voltage_Ref_Offset >> 24)&0xFF;	// MSW:MSB		
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = (Product_Info.iq_Voltage_Ref_Offset >> 16)&0xFF;	// MSW:LSB
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = (Product_Info.iq_Voltage_Ref_Offset >> 8)&0xFF;	// LSW:MSB
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = Product_Info.iq_Voltage_Ref_Offset&0xFF;			// LSW:LSB		
				
				// Copy Voltage gain in Byte4-7 
				ul_Temp = _IQ15rmpy(Product_Info.iq_Voltage_Ref_Gain,10000);
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE5 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDH.byte.BYTE6 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE7 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
			}				
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_I_CAL_PARAMETERS)
			{
				// Transmit current gain and offset values on Mailbox17
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_MBOX_CURRENT_CAL_PARAMS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				// Copy Current offset in Byte0-3
//				ul_Temp = _IQ15rmpy(Product_Info.iq_Current_Ref_Offset,10000);		
				
				// Offset can be negative also and hence for Offset alone we will transmit the IQ15 value (32bit Hexadecimal)
				// To Calculate Offset from the response, convert to decimal and divide by 2^15.
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = (Product_Info.iq_Current_Ref_Offset >> 24)&0xFF;	// MSW:MSB		
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = (Product_Info.iq_Current_Ref_Offset >> 16)&0xFF;	// MSW:LSB
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = (Product_Info.iq_Current_Ref_Offset >> 8)&0xFF;	// LSW:MSB
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = Product_Info.iq_Current_Ref_Offset&0xFF;			// LSW:LSB
				
				// Copy current gain in Byte4-7 
				ul_Temp = _IQ15rmpy(Product_Info.iq_Current_Ref_Gain,10000);
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE5 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDH.byte.BYTE6 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE7 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_VDISPLAY_CAL_PARAMETERS)
			{
				// Transmit Voltage Display gain and offset values on Mailbox17
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_MBOX_VOLTAGE_DISPLAY_CAL_PARAMS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				// Copy Voltage Display offset in Byte0-3
//				ul_Temp = _IQ15rmpy(Product_Info.iq_Voltage_Display_Offset,10000);		
				
				// Offset can be negative also and hence for Offset alone we will transmit the IQ15 value (32bit Hexadecimal)
				// To Calculate Offset from the response, convert to decimal and divide by 2^15.
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = (Product_Info.iq_Voltage_Display_Offset >> 24)&0xFF;	// MSW:MSB		
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = (Product_Info.iq_Voltage_Display_Offset >> 16)&0xFF;	// MSW:LSB
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = (Product_Info.iq_Voltage_Display_Offset >> 8)&0xFF;	// LSW:MSB
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = Product_Info.iq_Voltage_Display_Offset&0xFF;			// LSW:LSB
				
				// Copy Voltage Display gain in Byte4-7 
				ul_Temp = _IQ15rmpy(Product_Info.iq_Voltage_Display_Gain,10000);
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE5 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDH.byte.BYTE6 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE7 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_IDISPLAY_CAL_PARAMETERS)
			{
				// Transmit Current Display gain and offset values on Mailbox17
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_MBOX_CURRENT_DISPLAY_CAL_PARAMS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				// Copy Current Display offset in Byte0-3
//				ul_Temp = _IQ15rmpy(Product_Info.iq_Current_Display_Offset,10000);		
				
				// Offset can be negative also and hence for Offset alone we will transmit the IQ15 value (32bit Hexadecimal)
				// To Calculate Offset from the response, convert to decimal and divide by 2^15.
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = (Product_Info.iq_Current_Display_Offset >> 24)&0xFF;	// MSW:MSB		
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = (Product_Info.iq_Current_Display_Offset >> 16)&0xFF;	// MSW:LSB
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = (Product_Info.iq_Current_Display_Offset >> 8)&0xFF;	// LSW:MSB
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = Product_Info.iq_Current_Display_Offset&0xFF;			// LSW:LSB
				
				// Copy Current Display gain in Byte4-7 
				ul_Temp = _IQ15rmpy(Product_Info.iq_Current_Display_Gain,10000);
				
				// Temp has 8 digits. 4 before decimal. 4 after decimal.
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = (((ul_Temp/ONE_CRORTH_PLACE)%10) << 4)|((ul_Temp/TEN_LAKTH_PLACE)%10);			// 1st and 2nd digit before decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE5 = (((ul_Temp/ONE_LAKTH_PLACE)%10) << 4)|((ul_Temp/TEN_THOUSANDTH_PLACE)%10);		// 3rd and 4th digit before decimal point 
				ECanaMboxes.MBOX17.MDH.byte.BYTE6 = (((ul_Temp/ONE_THOUSANDTH_PLACE)%10) << 4)|((ul_Temp/ONE_HUNDREDTH_PLACE)%10);	// 1st and 2nd digit after decimal point
				ECanaMboxes.MBOX17.MDH.byte.BYTE7 = (((ul_Temp/TENTH_PLACE)%10) << 4)|((ul_Temp)%10);								// 3rd and 4th digit after decimal point
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_ONBOARD_AMBIENT_TEMP)
			{
				// Transmit Onboard Ambient Temperature
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_MBOX_PSU_ONBOARD_TEMPERATURE_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 2;
				
				// Byte 0 Stores the MSB of the temperature
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = iq8_Temperature >> 8;
				
				// Byte 1 stores the LSB of the temperature. Only the first 4 bits have importance. The seconds 4 bits
				// will always be 0.
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = (iq8_Temperature & 0x00FF);
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_STATE_SET_1)
			{
				// Transmit PSU State Set 1
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_PSU_STATE_SET_1_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				// Byte0 stores remote status
				if(State.Mode.bt_PSU_Control == LOCAL)
				{
					ECanaMboxes.MBOX17.MDL.byte.BYTE0 = CAN_PSU_IN_LOCAL_BYTE;
				}
				else if(State.Mode.bt_PSU_Control == NON_LATCHED_REMOTE)
				{
					ECanaMboxes.MBOX17.MDL.byte.BYTE0 = CAN_PSU_IN_REMOTE_BYTE;
				}
				else	// Local lockout
				{
					ECanaMboxes.MBOX17.MDL.byte.BYTE0 = CAN_PSU_IN_LOCKOUT_BYTE;
				}
				
				// Byte1 stores 0x00
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = 0;
				
				// Byte2 stores Foldback status.
				if(State.Mode.bt_FoldBack_Mode == FOLDBACK_DISARMED)
				{
					ECanaMboxes.MBOX17.MDL.byte.BYTE2 = CAN_FOLDBACK_OFF_BYTE;
				}
				else if(State.Mode.bt_FoldBack_Mode == FOLDBACK_ARMED_TO_PROTECT_FROM_CC)	// Output off
				{
					ECanaMboxes.MBOX17.MDL.byte.BYTE2 = CAN_FOLDBACK_CC_BYTE;
				}
				else	// Foldback to protect from CV
				{
					ECanaMboxes.MBOX17.MDL.byte.BYTE2 = CAN_FOLDBACK_CV_BYTE;
				}
				
				// Byte3 stores start mode
				if(State.Mode.bt_Start_Mode == AUTO_RESTART)
				{
					ECanaMboxes.MBOX17.MDL.byte.BYTE3 = CAN_AUTO_RESTART_BYTE;
				}
				else	// Safe start
				{	
					ECanaMboxes.MBOX17.MDL.byte.BYTE3 = CAN_SAFE_START_BYTE;
				}
				
				// Byte4 stores Foldback delay timer. Value returned is from 0 to 255.
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = Timers.ui_MaxFoldBackDelayInCounts;
				
				// Byte5 stores PSU Mode
				if(OUTPUT_ON)
				{
					if(CV_MODE)
					{
						ECanaMboxes.MBOX17.MDH.byte.BYTE5 = CAN_CV_MODE_BYTE;
					}
					else	// CC Mode
					{
						ECanaMboxes.MBOX17.MDH.byte.BYTE5 = CAN_CC_MODE_BYTE;
					}	
				}
				else // OUTPUT OFF
				{
					ECanaMboxes.MBOX17.MDH.byte.BYTE5 = CAN_OUTPUT_OFF_BYTE;
				}
				
				// Byte6 stores Power On Status Clear Setting
				if(State.Mode.bt_Power_On_Status_Clear == ON)
				{
					ECanaMboxes.MBOX17.MDH.byte.BYTE6 = CAN_PSC_ON_BYTE;
				}
				else	// PSC is OFF
				{
					ECanaMboxes.MBOX17.MDH.byte.BYTE6 = CAN_PSC_OFF_BYTE;
				}
				
				// Byte7 stores CAN_SRQ_SWITCH status
				if(State.Mode.bt_Can_SRQ_Switch == ON)
				{
					ECanaMboxes.MBOX17.MDH.byte.BYTE7 = CAN_SRQ_ENABLED_BYTE;
				}
				else	// Can Srq is OFF
				{
					ECanaMboxes.MBOX17.MDH.byte.BYTE7 = CAN_SRQ_DISABLED_BYTE;
				} 
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_STATE_SET_2)
			{
				// Transmit PSU State Set 2
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_PSU_STATE_SET_2_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;
				
				Uint16 I2C_Rx_Data_Array[1];
				
				// Byte0 stores Master/Slave Mode
				if(State.Mode.bt_Master_Slave_Setting == PSU_MASTER)
				{
					ECanaMboxes.MBOX17.MDL.byte.BYTE0 = CAN_PSU_MASTER_BYTE;
				}
				else // Slave Power Supply
				{
					ECanaMboxes.MBOX17.MDL.byte.BYTE0 = CAN_PSU_SLAVE_BYTE;
				}
				
				// Byte1 PSU Model No.
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = Product_Info.ui_Model_Number;
				
				// Byte2 stores Calibration Count. To be read from I2C
				// Read the calibration count.
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, CALIBRATION_COUNT_ADDRESS, 1,I2C_Rx_Data_Array)) == FAILURE)
				{
					return;
				}
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = I2C_Rx_Data_Array[0];
				
				// Byte3 stores MSW:MSB of PON time
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = (Product_Info.ul_Product_Power_On_Time_In_Minutes) >> 24;
				// Byte4 stores MSW:LSB of PON time
				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = (Product_Info.ul_Product_Power_On_Time_In_Minutes) >> 16;
				// Byte5 stores LSW:MSB of PON time
				ECanaMboxes.MBOX17.MDH.byte.BYTE5 = (Product_Info.ul_Product_Power_On_Time_In_Minutes) >> 8;
				// Byte6 stores LSW:LSB of PON time
				ECanaMboxes.MBOX17.MDH.byte.BYTE6 = (Product_Info.ul_Product_Power_On_Time_In_Minutes) & 0x000000FF;
				
				// Byte 7 Unused. Always 0
				ECanaMboxes.MBOX17.MDH.byte.BYTE7 = 0;
				
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_INTERNAL_PSU_REGISTERS)
			{
				// Transmit Fault and Operation Condition Regs.
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_INTERNAL_PSU_REGISTERS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 7;
				
				// 	Fault Regs with MSB in Byte0 and LSB in Byte1
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = Fault_Regs.FAULT_REGISTER.all >> 8;
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = Fault_Regs.FAULT_REGISTER.all & 0xFF;
				
				// 	Operation Condition Regs with MSB in Byte2 and LSB in Byte3
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.all >> 8;
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.all & 0xFF;

				ECanaMboxes.MBOX17.MDH.byte.BYTE4 = FecFaultHReg.all;
                ECanaMboxes.MBOX17.MDH.byte.BYTE5 = FecFaultLReg.all;

                ECanaMboxes.MBOX17.MDH.byte.BYTE6 = fecOnStatus;
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_ENABLE_REGISTERS)
			{
				// Transmit Fault and Operation Condition Regs.
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_ENABLE_REGISTERS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 4;
				
				// 	Fault Regs with MSB in Byte0 and LSB in Byte1
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = Fault_Regs.FAULT_ENABLE.all >> 8;
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = Fault_Regs.FAULT_ENABLE.all & 0xFF;
				
				// 	Operation Condition Regs with MSB in Byte2 and LSB in Byte3
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all >> 8;
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all & 0xFF;
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_EVENT_REGISTERS)
			{
				// Transmit Fault and Operation Condition Regs.
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_EVENT_REGISTERS_EXTENSION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 4;
				
				// 	Fault Regs with MSB in Byte0 and LSB in Byte1
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = Fault_Regs.FAULT_EVENT.all >> 8;
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = Fault_Regs.FAULT_EVENT.all & 0xFF;
				
				// 	Operation Condition Regs with MSB in Byte2 and LSB in Byte3
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all >> 8;
				ECanaMboxes.MBOX17.MDL.byte.BYTE3 = Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all & 0xFF;
				
				// Clear Event Registers after reading
				CLEAR_ON_READING(Fault_Regs.FAULT_EVENT.all);
				CLEAR_ON_READING(Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all);
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_DATE_OF_CALIBRATION)
			{
				// Date of PSU Calibration Over CAN
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | RESPONSE_DATE_OF_CALIBRATION;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 3;
				
				Uint16 a_uiI2cRxDataArray[3];
				
				// Read Date of Psu Calibration over CAN from EEPROM 
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, CAN_CALIBRATION_DATE_ADDRESS,3,a_uiI2cRxDataArray)) == FAILURE)
				{
					return;
				}
				
				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = a_uiI2cRxDataArray[0];	// YY
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = a_uiI2cRxDataArray[1];	// MM
				ECanaMboxes.MBOX17.MDL.byte.BYTE2 = a_uiI2cRxDataArray[2];	// DD
			}
			else if(MboxShadow.MDL.byte.BYTE1 == CAN_BYTE_TO_QUERY_FIRMWARE_REVISON)
			{
				// Firmware Revision Query
				ECanaMboxes.MBOX17.MSGID.all = ((ECanaMboxes.MBOX17.MSGID.all) & CLEAR_RESPONSE_EXTENSION_NIBBLE) | CAN_BYTE_TO_QUERY_FIRMWARE_REVISON;	// Copy the correct msg Id
				ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 2;

				ECanaMboxes.MBOX17.MDL.byte.BYTE0 = SOFTWARE_VERSION_IN_CAN_FORMAT >> 8;					// Main Revision control
				ECanaMboxes.MBOX17.MDL.byte.BYTE1 = SOFTWARE_VERSION_IN_CAN_FORMAT & LAST_EIGHT_BITS_ONLY;	// Auxiliary Revision control
			}
			else	// Illegal query. Transmit error message in general response mailbox.
			{
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_COMMAND;
				// Transmit response
				ECanaMboxes.MBOX16.MSGCTRL.bit.DLC = 2;
				ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
				ECanaShadow.CANTRS.bit.TRS16 = 1;
				ECanaRegs.CANTRS.all = ECanaShadow.CANTRS.all;
			}
			break;
		}
			
		default:
			break;
	}
}

//#####################	CAN voltage programming ###################################################################
// Function Name: ei_vCanProgramOutputVoltage
// Return Type: void
// Arguments:   Uint16 DataKey
// Description: Program the output voltage of PSU through CAN. This function can either set the parameter straight to
//				MIN/MAX value or read the data from Mbox and change the parameter to user specified value.
//			1.	This function will be called with MIN/MAX argument from SINGLE_BYTE_EXTENSION
//			2.	This function will be called with MBOX_VALUE in PROGRAMMED_VOLTAGE_EXTENSION

// The type of processing is specified by the argument DataKey
//	DataKey: MIN_VALUE --> Sets paramter to minimum possible value.
//	DataKey: MAX_VALUE --> Sets paramter to maximum possible value.
//	DataKey: MBOX_VALUE --> Sets paramter from the value specified by the user in MBOX.
//#################################################################################################################
static void ei_vCanProgramOutputVoltage(Uint16 DataKey)
{
	int32 iq_Voltage,iq_ProgramVoltageInQepCounts;
	
	if(DataKey == MAX_VALUE)
	{
		// Present MAX Possible Value
		iq_Voltage = _IQ15mpy(Reference.iq_OVP_Reference,_IQ15(MIN_DISTANCE_BELOW_OVP));
		if(iq_Voltage > Product_Info.iq_Specified_FS_Voltage)
		{
			iq_Voltage = Product_Info.iq_Specified_FS_Voltage;
		}
	}
	else if(DataKey == MIN_VALUE)
	{
		// Present MIN Possible Value
		iq_Voltage = _IQ15mpy(Reference.iq_UVL_Reference,_IQ15(MIN_DISTANCE_ABOVE_UVL));
	}
	else //if(DataKey == MBOX_VALUE)
	{
		int32 iq_ProgramVoltageBeforeDecimal, iq_ProgramVoltageAfterDecimal;
		int32 iq_VoltageRange, iq_UpperLimit, iq_LowerLimit;
		
		//Extract data from the CAN Data Field
	
		// Before decimal value is stored in Byte 0 and Byte 1
		iq_ProgramVoltageBeforeDecimal = MboxShadow.MDL.word.HI_WORD;	
		iq_ProgramVoltageBeforeDecimal = 	(iq_ProgramVoltageBeforeDecimal & 0x000F)*1
										+	((iq_ProgramVoltageBeforeDecimal >> 4) & 0x000F)*10
										+	((iq_ProgramVoltageBeforeDecimal >> 8) & 0x000F)*100
										+	((iq_ProgramVoltageBeforeDecimal >> 12) & 0x000F)*1000;
		
		// Calculate after decimal. Byte 3 and Byte 4
		iq_ProgramVoltageAfterDecimal = MboxShadow.MDL.word.LOW_WORD;
		iq_ProgramVoltageAfterDecimal = 	_IQ15mpy(_IQ15(iq_ProgramVoltageAfterDecimal & 0x000F), _IQ15(0.0001))
							  			  + _IQ15mpy(_IQ15((iq_ProgramVoltageAfterDecimal >> 4) & 0x000F), _IQ15(0.001))
							  			  + _IQ15mpy(_IQ15((iq_ProgramVoltageAfterDecimal >> 8) & 0x000F), _IQ15(0.01))
							  			  + _IQ15mpy(_IQ15((iq_ProgramVoltageAfterDecimal >> 12) & 0x000F), _IQ15(0.1));
							  
		// Actual Voltage in IQ
		 iq_Voltage = _IQ15(iq_ProgramVoltageBeforeDecimal) | iq_ProgramVoltageAfterDecimal;
		
		iq_VoltageRange = Product_Info.iq_Specified_FS_Voltage;	
		
		// Check if value outside programmable range (negative or greater than Full_Scale)
		// Avoiding direct IQ comparisons.
		if((_IQ15rmpy(iq_Voltage,10000) > _IQ15rmpy(iq_VoltageRange,10000)) || (iq_Voltage < 0))
		{
			// Outside range. Reply with Error.
			ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SETTING_OUTSIDE_RANGE;
			return;
		}
		else	// Within allowed Ranges. OVP/UVL limits still to be checked
		{
			iq_UpperLimit = _IQ15mpy(Reference.iq_OVP_Reference,_IQ15(MIN_DISTANCE_BELOW_OVP)); 
			iq_LowerLimit = _IQ15mpy(Reference.iq_UVL_Reference,_IQ15(MIN_DISTANCE_ABOVE_UVL)); 
			
			// Above OVP? 
			if(iq_Voltage > iq_UpperLimit)	// the RHS is Counts in IQ
			{
				// Above OVP. Return after signalling error
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SETTING_ABOVE_OVP;
				return;
			}
			
			// Below UVL?
			if(iq_Voltage < iq_LowerLimit) // the RHS is Counts in IQ
			{
				// Below UVL. Return after signalling error
				ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SETTING_BELOW_UVL;
				return;
			}
		}
	}
	
	// The Data is within limits. Value can be set.
	// Convert Voltage to QEP Counts
	iq_ProgramVoltageInQepCounts = _IQ15rmpy(_IQ15div(iq_Voltage,Product_Info.iq_Specified_FS_Voltage),_IQ15(QEP_FULL_ROTATIONS_COUNT));

	// Voltage in remote control
	Global_Flags.fg_Voltage_Remote_control = TRUE;
	
	// Update Reference
	Reference.iq_Voltage_Reference = iq_Voltage;
	
	// Copy values to encoder vars
	Encoder.iq_Qep_Voltage_Count = iq_ProgramVoltageInQepCounts;
	
	// Coarse takes the integer value. Got by multiplying with 1 rather than IQint because round up required.
	Encoder.i_VoltageKnob_Coarse_Count = _IQ15rmpy(iq_ProgramVoltageInQepCounts,1);
	
	Encoder.i_VoltageKnob_Fine_Count = QEP_FULL_ROTATIONS_COUNT/2;
	
	// Known state
	State.Mode.bt_Display_Mode = DISPLAY_OUTPUT;			// Change display to Display Output
	State.Mode.bt_Encoder_Operation = SET_VOLTAGE_CURRENT;	// Change operation to set_voltage_current
	State.Mode.bt_Encoder_Rotation_Mode = COARSE_ROTATION;	// Change rotation to coarse
	if(State.Mode.bt_PSU_Control == LOCAL)
	{
		State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
	}
	Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
	
	// Reset encoders
	Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count; 
	EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count; 
	EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
	
	// SET THE VOLTAGE HERE
	ei_vSwitchSpiClkPolarity(DAC_8552);
	ei_vSetVoltageReference();
	
	// Switch the polarity back to AD7705
	ei_vSwitchSpiClkPolarity(AD7705);
}

//#####################	CAN Current programming ###################################################################
// Function Name: ei_vCanProgramOutputCurrent
// Return Type: void
// Arguments:   Uint16 DataKey
// Description: Program the output Current of PSU through CAN. This function can either set the parameter straight to
//				MIN/MAX value or read the data from Mbox and change the parameter to user specified value.
//			1.	This function will be called with MIN/MAX argument from SINGLE_BYTE_EXTENSION
//			2.	This function will be called with MBOX_VALUE in PROGRAMMED_VOLTAGE_EXTENSION

// The type of processing is specified by the argument DataKey
//	DataKey: MIN_VALUE --> Sets paramter to minimum possible value.
//	DataKey: MAX_VALUE --> Sets paramter to maximum possible value.
//	DataKey: MBOX_VALUE --> Sets paramter from the value specified by the user in MBOX.
//#################################################################################################################
static void ei_vCanProgramOutputCurrent(Uint16 DataKey)
{
	int32 iq_Current,iq_ProgramCurrentInQepCounts;
	
	if(DataKey == MAX_VALUE)
	{
		// Present MAX Possible Value
		iq_Current = Product_Info.iq_Specified_FS_Current;
	}
	else if(DataKey == MIN_VALUE)
	{
		// Present MIN Possible Value
		iq_Current = 0;
	}
	else //if(DataKey == MAX_VALUE)
	{
		int32 iq_ProgramCurrentBeforeDecimal, iq_ProgramCurrentAfterDecimal;
		
		//Extract data from the CAN Data Field
		
		// Before decimal value is stored in Byte 0 and Byte 1
		iq_ProgramCurrentBeforeDecimal = MboxShadow.MDL.word.HI_WORD;	
		iq_ProgramCurrentBeforeDecimal = 	(iq_ProgramCurrentBeforeDecimal & 0x000F)*1
										+	((iq_ProgramCurrentBeforeDecimal >> 4) & 0x000F)*10
										+	((iq_ProgramCurrentBeforeDecimal >> 8) & 0x000F)*100
										+	((iq_ProgramCurrentBeforeDecimal >> 12) & 0x000F)*1000;
		
		// Calculate after decimal. Byte 3 and Byte 4
		iq_ProgramCurrentAfterDecimal = MboxShadow.MDL.word.LOW_WORD;
		iq_ProgramCurrentAfterDecimal = 	_IQ15mpy(_IQ15(iq_ProgramCurrentAfterDecimal & 0x000F), _IQ15(0.0001))
						  				+ _IQ15mpy(_IQ15((iq_ProgramCurrentAfterDecimal >> 4) & 0x000F), _IQ15(0.001))
						  				+ _IQ15mpy(_IQ15((iq_ProgramCurrentAfterDecimal >> 8) & 0x000F), _IQ15(0.01))
						  				+ _IQ15mpy(_IQ15((iq_ProgramCurrentAfterDecimal >> 12) & 0x000F), _IQ15(0.1));
		
		// Actual Current in IQ
		iq_Current  = _IQ15(iq_ProgramCurrentBeforeDecimal) | iq_ProgramCurrentAfterDecimal;
		
		// Check if value outside programmable range (negative or greater than Full_Scale)
		// Avoiding direct IQ comparisons.
		if((_IQ15rmpy(iq_Current,10000) > _IQ15rmpy(Product_Info.iq_Specified_FS_Current,10000)) || (iq_Current < 0)) 	
		{
			// Outside range. Reply with Error.
			ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SETTING_OUTSIDE_RANGE;
			return;
		}
	}
	
	// Convert Current to QEP Counts
	iq_ProgramCurrentInQepCounts = _IQ15rmpy(_IQ15div(iq_Current,Product_Info.iq_Specified_FS_Current),_IQ15(QEP_FULL_ROTATIONS_COUNT));
	
	// Current in remote control
	Global_Flags.fg_Current_Remote_control = TRUE;
	
	// Copy values to encoder vars
	Encoder.iq_Qep_Current_Count = iq_ProgramCurrentInQepCounts;
	// Coarse takes the integer value. Got by multiplying with 1 rather than IQint because round up required.
	Encoder.i_CurrentKnob_Coarse_Count = _IQ15rmpy(iq_ProgramCurrentInQepCounts,1);
	Reference.iq_Current_Reference = iq_Current;	// Copy to reference
	
	// Known state
	State.Mode.bt_Display_Mode = DISPLAY_OUTPUT;			// Change display to Display Output
	State.Mode.bt_Encoder_Operation = SET_VOLTAGE_CURRENT;	// Change operation to set_voltage_current
	State.Mode.bt_Encoder_Rotation_Mode = COARSE_ROTATION;	// Change rotation to coarse
	if(State.Mode.bt_PSU_Control == LOCAL)
	{
		State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
	}
	Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
	
	// Reset encoders
	Encoder.i_CurrentKnob_Fine_Count = QEP_FULL_ROTATIONS_COUNT/2;
	Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count; 
	EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Coarse_Count; 
	EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;
	
	// SET THE CURRENT REFERENCE
	ei_vSwitchSpiClkPolarity(DAC_8552);
	ei_vSetCurrentReference();

	SciaRegs.SCITXBUF = 0x01;
	
	// Switch the polarity back to AD7705
	ei_vSwitchSpiClkPolarity(AD7705);
}

//#####################	CAN OVP programming ###################################################################
// Function Name: ei_vCanProgramOvp
// Return Type: void
// Arguments:   Uint16 DataKey
// Description: Program the OVP of PSU through CAN. This function can either set the parameter straight to
//				MIN/MAX value or read the data from Mbox and change the parameter to user specified value.
//			1.	This function will be called with MIN/MAX argument from SINGLE_BYTE_EXTENSION
//			2.	This function will be called with MBOX_VALUE in PROGRAMMED_VOLTAGE_EXTENSION

// The type of processing is specified by the argument DataKey
//	DataKey: MIN_VALUE --> Sets paramter to minimum possible value.
//	DataKey: MAX_VALUE --> Sets paramter to maximum possible value.
//	DataKey: MBOX_VALUE --> Sets paramter from the value specified by the user in MBOX.
//#################################################################################################################
static void ei_vCanProgramOvp(Uint16 DataKey)
{
	int32 iq_Ovp;
	Uint16 ui_OvpInQepCounts;
	
	if(DataKey == MAX_VALUE)
	{
		// Present MAX Possible Value
		iq_Ovp = _IQ15rmpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(1.1));
	}
	else if(DataKey == MIN_VALUE)
	{
		if(OUTPUT_OFF)
		{
			// Minmum OVP specified for that model in ModelNO.h
			iq_Ovp = Product_Info.iq_Min_Ovp_Reference;
		}
		else	// OUTPUT ON
		{
			// Minimum is (0.95^-1) times the programmed voltage or Minimum ovp for that model whichever is greater
			iq_Ovp = _IQ15rmpy(Reference.iq_Voltage_Reference,_IQ15(MIN_DISTANCE_ABOVE_PROGRAMMED_VOLT));
			if(iq_Ovp < Product_Info.iq_Min_Ovp_Reference)
			{
				iq_Ovp = Product_Info.iq_Min_Ovp_Reference;
			}
		}
	}
	else //if(DataKey == MAX_VALUE)
	{
		int32 iq_ProgramOvpBeforeDecimal, iq_ProgramOvpAfterDecimal;
		int32 iq_LowerLimit;
		int32 iq_OvpRange;
		
		//Extract data from the CAN Data Field
		
		// Before decimal value is stored in Byte 0 and Byte 1
		iq_ProgramOvpBeforeDecimal = MboxShadow.MDL.word.HI_WORD;	
		iq_ProgramOvpBeforeDecimal = 	(iq_ProgramOvpBeforeDecimal & 0x000F)*1
										+	((iq_ProgramOvpBeforeDecimal >> 4) & 0x000F)*10
										+	((iq_ProgramOvpBeforeDecimal >> 8) & 0x000F)*100
										+	((iq_ProgramOvpBeforeDecimal >> 12) & 0x000F)*1000;
		
		// Calculate after decimal. Byte 3 and Byte 4
		iq_ProgramOvpAfterDecimal = MboxShadow.MDL.word.LOW_WORD;
		iq_ProgramOvpAfterDecimal = 	_IQ15mpy(_IQ15(iq_ProgramOvpAfterDecimal & 0x000F), _IQ15(0.0001))
						  				+ _IQ15mpy(_IQ15((iq_ProgramOvpAfterDecimal >> 4) & 0x000F), _IQ15(0.001))
						  				+ _IQ15mpy(_IQ15((iq_ProgramOvpAfterDecimal >> 8) & 0x000F), _IQ15(0.01))
						  				+ _IQ15mpy(_IQ15((iq_ProgramOvpAfterDecimal >> 12) & 0x000F), _IQ15(0.1));
		
		// Actual Current in IQ
		iq_Ovp  = _IQ15(iq_ProgramOvpBeforeDecimal) | iq_ProgramOvpAfterDecimal;
		
		// Ovp Range
		iq_OvpRange = _IQ15mpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(1.1));	// The Upper range is 110% of XVUL	
		
		// Check if value outside programmable range (negative or greater than Full_Scale)
		// Avoiding direct IQ comparisons.
		if((_IQ15rmpy(iq_Ovp,10000) > _IQ15rmpy(iq_OvpRange,10000)) || (iq_Ovp < 0)) 	
		{
			// Outside range. Reply with Error.
			ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SETTING_OUTSIDE_RANGE;
			return;
		}
		
		// Ovp within range. But still lower limit has to be checked.
		if(OUTPUT_ON)	// when o/p on limit is (0.95^-1)*Present voltage
		{
			iq_LowerLimit = _IQ15mpy(Reference.iq_Voltage_Reference,_IQ15(MIN_DISTANCE_ABOVE_PROGRAMMED_VOLT)); 
		}
		else	// minimum OVP counts for that model
		{
			iq_LowerLimit = Product_Info.iq_Min_Ovp_Reference;
		}
		
		// The limit itself cannot go lesser than Minimum for the model
		if(iq_LowerLimit < Product_Info.iq_Min_Ovp_Reference)
		{
			iq_LowerLimit = Product_Info.iq_Min_Ovp_Reference;
		}
		
		if(iq_Ovp < iq_LowerLimit)
		{
			ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_OVP_SETTING;
			return;
		}
	}
	
	// ALL OKAY. SET REFERENCE
	// Here an important checking is required.
	// Check if the OVP set has become less than UVL. In that case change the value
	// of UVL.
	// UVLmax is always equal to 0.904761904*OVP
	// ==> OVP should never be less than UVL/0.904761904 = UVL * 1.105263159
	if(iq_Ovp < _IQ15mpy(Reference.iq_UVL_Reference,_IQ15(1.105263159)))
	{
		// If it is then set UVL to 0.904761904.
		// No rounding up to be done here.
		Reference.iq_UVL_Reference = _IQ15mpy(iq_Ovp,_IQ15(0.904761904));
		
		// Make the appropriate change in the encoder vars
		Encoder.ui_Qep_UVL_Count = _IQ15rmpy(_IQ15div(Reference.iq_UVL_Reference,Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
	}
	
	// Convert to Qep counts
	ui_OvpInQepCounts = _IQ15rmpy(_IQ15div(iq_Ovp,Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
	
	// Ovp in Remote control
	Global_Flags.fg_OVP_Remote_control = TRUE;
	
	// Update OVP
	Reference.iq_OVP_Reference = iq_Ovp;
	
	// Update Encoder Vars
	Encoder.ui_Qep_OVP_Count = ui_OvpInQepCounts;
	
	// Known State
	if(State.Mode.bt_PSU_Control == LOCAL)
	{
		State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
	}
	Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
	Reference.ui_OVP_Reference_Update_Immediate_Flag = TRUE;
	
	// SET THE OVP REFERENCE
	ei_vSetOvpReference();
}

//#####################	CAN UVL programming ###################################################################
// Function Name: ei_vCanProgramUvl
// Return Type: void
// Arguments:   Uint16 DataKey
// Description: Program the OVP of PSU through CAN. This function can either set the parameter straight to
//				MIN/MAX value or read the data from Mbox and change the parameter to user specified value.
//			1.	This function will be called with MIN/MAX argument from SINGLE_BYTE_EXTENSION
//			2.	This function will be called with MBOX_VALUE in PROGRAMMED_VOLTAGE_EXTENSION

// The type of processing is specified by the argument DataKey
//	DataKey: MIN_VALUE --> Sets paramter to minimum possible value.
//	DataKey: MAX_VALUE --> Sets paramter to maximum possible value.
//	DataKey: MBOX_VALUE --> Sets paramter from the value specified by the user in MBOX.
//#################################################################################################################
static void ei_vCanProgramUvl(Uint16 DataKey)
{
	int32 iq_Uvl;
	Uint16 ui_UvlInQepCounts;
	
	if(DataKey == MAX_VALUE)
	{
		if(OUTPUT_OFF)
		{
			// When o/p off Uvl max is min(0.9047 * OVP, UVLmax specified for that model)
			iq_Uvl = _IQ15rmpy(Reference.iq_OVP_Reference,_IQ15(0.904761904));
			
			if(iq_Uvl > Product_Info.iq_Max_Uvl_Reference)
			{
				iq_Uvl = Product_Info.iq_Max_Uvl_Reference;
			}
		}
		else	// OUTPUT ON
		{
			// Maximum is (1.05^-1) times the programmed voltage
			iq_Uvl = _IQ15rmpy(Reference.iq_Voltage_Reference,_IQ15(MIN_DISTANCE_BELOW_PROGRAMMED_VOLT));
		}
	}
	else if(DataKey == MIN_VALUE)
	{
		// Minimum possible Uvl is always 0
		iq_Uvl = 0;
	}
	else //if(DataKey == MAX_VALUE)
	{
		int32 iq_ProgramUvlBeforeDecimal, iq_ProgramUvlAfterDecimal;
		int32 iq_UpperLimit;
		
		//Extract data from the CAN Data Field
		
		// Before decimal value is stored in Byte 0 and Byte 1
		iq_ProgramUvlBeforeDecimal = MboxShadow.MDL.word.HI_WORD;	
		iq_ProgramUvlBeforeDecimal = 	(iq_ProgramUvlBeforeDecimal & 0x000F)*1
										+	((iq_ProgramUvlBeforeDecimal >> 4) & 0x000F)*10
										+	((iq_ProgramUvlBeforeDecimal >> 8) & 0x000F)*100
										+	((iq_ProgramUvlBeforeDecimal >> 12) & 0x000F)*1000;
		
		// Calculate after decimal. Byte 3 and Byte 4
		iq_ProgramUvlAfterDecimal = MboxShadow.MDL.word.LOW_WORD;
		iq_ProgramUvlAfterDecimal = 	_IQ15mpy(_IQ15(iq_ProgramUvlAfterDecimal & 0x000F), _IQ15(0.0001))
						  				+ _IQ15mpy(_IQ15((iq_ProgramUvlAfterDecimal >> 4) & 0x000F), _IQ15(0.001))
						  				+ _IQ15mpy(_IQ15((iq_ProgramUvlAfterDecimal >> 8) & 0x000F), _IQ15(0.01))
						  				+ _IQ15mpy(_IQ15((iq_ProgramUvlAfterDecimal >> 12) & 0x000F), _IQ15(0.1));
		
		// Actual Current in IQ
		iq_Uvl  = _IQ15(iq_ProgramUvlBeforeDecimal) | iq_ProgramUvlAfterDecimal;
		
		
		// Check if value outside programmable range (negative or greater than Full_Scale)
		// Avoiding direct IQ comparisons.
		if((_IQ15rmpy(iq_Uvl,10000) > _IQ15rmpy(Product_Info.iq_Max_Uvl_Reference,10000)) || (iq_Uvl < 0)) 	
		{
			// Outside range. Reply with Error.
			ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_SETTING_OUTSIDE_RANGE;
			return;
		}
		
		// Uvl within range. But still Upper limit has to be checked.
		if(OUTPUT_ON)	// when o/p on, Upper limit is (1.05^-1)*Present voltage
		{
			iq_UpperLimit = _IQ15rmpy(Reference.iq_Voltage_Reference,_IQ15(MIN_DISTANCE_BELOW_PROGRAMMED_VOLT)); 
		}
		else	// when o/p OFF, upper limit is min(0.9047 * OVP, Max allowed UVL for that model)
		{
			iq_UpperLimit = _IQ15mpy(Reference.iq_OVP_Reference,_IQ15(0.904761904)); 
		}
		
		// The limit itself cannot go higher than Product Specification
		if(iq_UpperLimit > Product_Info.iq_Max_Uvl_Reference)
		{	
			iq_UpperLimit = Product_Info.iq_Max_Uvl_Reference;
		}
		
		if(iq_Uvl > iq_UpperLimit)
		{
			ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_UVL_SETTING;
			return;
		}
	}
	
	// Convert to Qep counts
	ui_UvlInQepCounts = _IQ15rmpy(_IQ15div(iq_Uvl,Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);
	
	// Uvl in Remote control
	Global_Flags.fg_UVL_Remote_control = TRUE;
	
	// Update Uvl
	Reference.iq_UVL_Reference = iq_Uvl;
	
	// Update Encoder Vars
	Encoder.ui_Qep_UVL_Count = ui_UvlInQepCounts;
	
	// Known State
	if(State.Mode.bt_PSU_Control == LOCAL)
	{
		State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
	}
	Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
}

//#####################	CAN system calibration ####################################################################
// Function Name: CanProgramOutputCurrent
// Return Type: void
// Arguments:   void
// Description: Complete calibration commands depending on which command was sent.
//#################################################################################################################
static void ei_vCanSystemCal(Uint16 CommandType)
{
	if(CommandType == SET_CALIBRATION_DATE)
	{
		// Here before writing the date we extract the rated paramters, specified paramters from the EEPROM.
		// And calculate all the necessary values for setting references and ADC scaling.
		ei_vSetPreCANCalibrationParameters();
		
		Uint16 a_uiI2cTxDataArray[3];
		
		a_uiI2cTxDataArray[0] = MboxShadow.MDL.byte.BYTE0;	// YY
		a_uiI2cTxDataArray[1] = MboxShadow.MDL.byte.BYTE1;	// MM
		a_uiI2cTxDataArray[2] = MboxShadow.MDL.byte.BYTE2;	// DD
		
		while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
		ei_uiI2CWrite(EEPROM_24LC256_ADDRESS,CAN_CALIBRATION_DATE_ADDRESS,3,a_uiI2cTxDataArray);
		RESET_EEPROM_WRITE_CYCLE_TIMER;
		
		Global_Flags.fg_Date_Entered = TRUE;
	}
	else	// Calibrate_Voltage/Current
	{
		// Extract the Measured Values from the Mailbox
		int32 iq_Value;
		int32 Value_Before_Decimal, Value_After_Decimal;
		Uint16 I2C_Tx_Data_Array[49],I2C_Rx_Data_Array[49],i;
		
		// Get Value_1 from BYTE0 to BYTE3
		// Before decimal value is stored in Byte 0 and Byte 1
		Value_Before_Decimal = MboxShadow.MDL.word.HI_WORD;	
		Value_Before_Decimal = 	(Value_Before_Decimal & 0x000F)*1
								+	((Value_Before_Decimal >> 4) & 0x000F)*10
								+	((Value_Before_Decimal >> 8) & 0x000F)*100
								+	((Value_Before_Decimal >> 12) & 0x000F)*1000;
			
		// Calculate after decimal. Byte 3 and Byte 4
		Value_After_Decimal = MboxShadow.MDL.word.LOW_WORD;
		Value_After_Decimal = 	_IQ15mpy(_IQ15(Value_After_Decimal & 0x000F), _IQ15(0.0001))
							  + _IQ15mpy(_IQ15((Value_After_Decimal >> 4) & 0x000F), _IQ15(0.001))
							  + _IQ15mpy(_IQ15((Value_After_Decimal >> 8) & 0x000F), _IQ15(0.01))
							  + _IQ15mpy(_IQ15((Value_After_Decimal >> 12) & 0x000F), _IQ15(0.1));
		
		// Value in IQ
		iq_Value  = _IQ15(Value_Before_Decimal) | Value_After_Decimal;	

		if(CommandType == CALIBRATE_VOLTAGE)
		{
			static int32 MV_2,MV_1,DV_2,DV_1;
			
			if(Global_Flags.fg_Previous_Calibration == VOLTAGE_LOW)
			{
				// The Measured value was entered after setting Voltage Low
				MV_1 = iq_Value;
				DV_1 = External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Scaled_Filtered_Value;
				Global_Flags.fg_Voltage_MV1_Entered = TRUE;
			}
			else if(Global_Flags.fg_Previous_Calibration == VOLTAGE_FULL)
			{
				MV_2 = iq_Value;
				DV_2 = External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Scaled_Filtered_Value;
				Global_Flags.fg_Voltage_MV2_Entered = TRUE;
			}
			else
				asm("	nop");
				
			// Now to calculate offset and gain
		
			// 1. Check if both values have been entered.
			if(Global_Flags.fg_Voltage_MV1_Entered == Global_Flags.fg_Voltage_MV2_Entered == TRUE)
			{
				int32 X1,X2,iq_Temp;
				
				X1 = Product_Info.iq_Specified_FS_Voltage/10;
				X2 = Product_Info.iq_Specified_FS_Voltage;
				
				Product_Info.iq_Voltage_Ref_Gain = _IQ15div((MV_2 - MV_1),(X2 - X1));	
				// Gain can never be 0
				if(Product_Info.iq_Voltage_Ref_Gain <= 0)
				{
					State.Mode.bt_Output_Status = OFF;
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_GAIN_CALCULATED;
					Global_Flags.fg_Voltage_MV1_Entered = Global_Flags.fg_Voltage_MV2_Entered = FALSE;	// Reset Both Flags. Value to be entered again.
					return;
				}
				Product_Info.iq_Voltage_Ref_Offset = MV_1 - _IQ15mpy(Product_Info.iq_Voltage_Ref_Gain, X1);
				Product_Info.iq_Voltage_Display_Gain = _IQ15div((DV_2 - DV_1),(MV_2 - MV_1));
				// Gain can never be 0
				if(Product_Info.iq_Voltage_Display_Gain <= 0)
				{
					State.Mode.bt_Output_Status = OFF;
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_GAIN_CALCULATED;
					Global_Flags.fg_Voltage_MV1_Entered = Global_Flags.fg_Voltage_MV2_Entered = FALSE;	// Reset Both Flags. Value to be entered again.
					return;
				}
				Product_Info.iq_Voltage_Display_Offset = DV_1 - _IQ15mpy(Product_Info.iq_Voltage_Display_Gain, MV_1);
				
				// Voltage offset
				iq_Temp = Product_Info.iq_Voltage_Ref_Offset;	
				I2C_Tx_Data_Array[0] = (iq_Temp >> 24) & 0x000000FF;
				I2C_Tx_Data_Array[1] = (iq_Temp >> 16) & 0x000000FF;
				I2C_Tx_Data_Array[2] = (iq_Temp >> 8) & 0x000000FF;
				I2C_Tx_Data_Array[3] = (iq_Temp) & 0x000000FF;
				
				// Voltage gain
				iq_Temp = Product_Info.iq_Voltage_Ref_Gain;	
				I2C_Tx_Data_Array[4] = (iq_Temp >> 24) & 0x000000FF;
				I2C_Tx_Data_Array[5] = (iq_Temp >> 16) & 0x000000FF;
				I2C_Tx_Data_Array[6] = (iq_Temp >> 8) & 0x000000FF;
				I2C_Tx_Data_Array[7] = (iq_Temp) & 0x000000FF;
	
				// Voltage Display offset
				iq_Temp = Product_Info.iq_Voltage_Display_Offset;	
				I2C_Tx_Data_Array[8] = (iq_Temp >> 24) & 0x000000FF;
				I2C_Tx_Data_Array[9] = (iq_Temp >> 16) & 0x000000FF;
				I2C_Tx_Data_Array[10] = (iq_Temp >> 8) & 0x000000FF;
				I2C_Tx_Data_Array[11] = (iq_Temp) & 0x000000FF;
				
				// Voltage Display gain
				iq_Temp = Product_Info.iq_Voltage_Display_Gain;	
				I2C_Tx_Data_Array[12] = (iq_Temp >> 24) & 0x000000FF;
				I2C_Tx_Data_Array[13] = (iq_Temp >> 16) & 0x000000FF;
				I2C_Tx_Data_Array[14] = (iq_Temp >> 8) & 0x000000FF;
				I2C_Tx_Data_Array[15] = (iq_Temp) & 0x000000FF;
				
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, V_OFFSET_ADDRESS, 16,I2C_Tx_Data_Array);
				RESET_EEPROM_WRITE_CYCLE_TIMER;
				
				Global_Flags.fg_Voltage_Calibration_Done = TRUE;
			}
		}
		else //if(CommandType == CALIBRATE_CURRENT)
		{
			static int32 MC_2,MC_1,DC_2,DC_1;
					
			if(Global_Flags.fg_Previous_Calibration == CURRENT_LOW)
			{
				MC_1 = iq_Value;
				DC_1 = External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Scaled_Filtered_Value;
				Global_Flags.fg_Current_MC1_Entered = TRUE;
			}
			else if(Global_Flags.fg_Previous_Calibration == CURRENT_FULL)
			{
				MC_2 = iq_Value;
				DC_2 = External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Scaled_Filtered_Value;
				Global_Flags.fg_Current_MC2_Entered = TRUE;
			}
			else
				asm("	nop");
				
			// Now to calculate offset and gain
			
			// 1. Check if both values have been entered.
			if(Global_Flags.fg_Current_MC1_Entered == Global_Flags.fg_Current_MC2_Entered == TRUE)
			{
				int32 X1,X2;
				
				X1 = Product_Info.iq_Specified_FS_Current/10;
				X2 = Product_Info.iq_Specified_FS_Current;
				
				Product_Info.iq_Current_Ref_Gain = _IQ15div((MC_2 - MC_1),(X2 - X1));
				// Gain can never be 0
				if(Product_Info.iq_Current_Ref_Gain <= 0)
				{
					State.Mode.bt_Output_Status = OFF;
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_GAIN_CALCULATED;
					Global_Flags.fg_Voltage_MV1_Entered = Global_Flags.fg_Voltage_MV2_Entered = FALSE;	// Reset Both Flags. Value to be entered again.
					return;
				}
				Product_Info.iq_Current_Ref_Offset = MC_1 - _IQ15mpy(Product_Info.iq_Current_Ref_Gain, X1);
				
				Product_Info.iq_Current_Display_Gain = _IQ15div((DC_2 - DC_1),(MC_2 - MC_1));
				// Gain can never be 0
				if(Product_Info.iq_Current_Display_Gain <= 0)
				{
					State.Mode.bt_Output_Status = OFF;
					ECanaMboxes.MBOX16.MDL.byte.BYTE1 = ECanaMboxes.MBOX16.MDL.byte.BYTE0 = CAN_BYTE_FOR_ILLEGAL_GAIN_CALCULATED;
					Global_Flags.fg_Voltage_MV1_Entered = Global_Flags.fg_Voltage_MV2_Entered = FALSE;	// Reset Both Flags. Value to be entered again.
					return;
				}
				
				Product_Info.iq_Current_Display_Offset = DC_1 - _IQ15mpy(Product_Info.iq_Current_Display_Gain, MC_1);
				
				Global_Flags.fg_Current_Calibration_Done = TRUE;
			}
				
			// This was the last step in calibration. We need to read everything from EEPROM, 
			// calculate checksum and write it back to eeprom.
			
			// Check if all operations completed
			if(Global_Flags.fg_Date_Entered == Global_Flags.fg_Rated_Volt_Entered == Global_Flags.fg_Rated_Curr_Entered == Global_Flags.fg_Specified_Volt_Entered == Global_Flags.fg_Specified_Curr_Entered == Global_Flags.fg_Voltage_Calibration_Done == Global_Flags.fg_Current_Calibration_Done == TRUE)
			{
				int32 iq_Temp;
				
				// Read until Voltage display gain
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, MODEL_NO_ADDR, 29,I2C_Rx_Data_Array)) == FAILURE)
				{
					return;
				}
				
				// Now form the write array.
				I2C_Tx_Data_Array[0] = Product_Info.ui_Model_Number;
				
				// XVM, XCM, SVUL, SCUL, Voltage gain and offset, Voltage display gain and offset can be copied from read values
				for(i = 1; i < 29; i++)
				{
					I2C_Tx_Data_Array[i] = I2C_Rx_Data_Array[i];
				}
				
				// Current offset
				iq_Temp = Product_Info.iq_Current_Ref_Offset;	
				I2C_Tx_Data_Array[29] = (iq_Temp >> 24) & 0x000000FF;
				I2C_Tx_Data_Array[30] = (iq_Temp >> 16) & 0x000000FF;
				I2C_Tx_Data_Array[31] = (iq_Temp >> 8) & 0x000000FF;
				I2C_Tx_Data_Array[32] = (iq_Temp) & 0x000000FF;
				
				// Current gain
				iq_Temp = Product_Info.iq_Current_Ref_Gain;	
				I2C_Tx_Data_Array[33] = (iq_Temp >> 24) & 0x000000FF;
				I2C_Tx_Data_Array[34] = (iq_Temp >> 16) & 0x000000FF;
				I2C_Tx_Data_Array[35] = (iq_Temp >> 8) & 0x000000FF;
				I2C_Tx_Data_Array[36] = (iq_Temp) & 0x000000FF;
				
				// Current Display offset
				iq_Temp = Product_Info.iq_Current_Display_Offset;	
				I2C_Tx_Data_Array[37] = (iq_Temp >> 24) & 0x000000FF;
				I2C_Tx_Data_Array[38] = (iq_Temp >> 16) & 0x000000FF;
				I2C_Tx_Data_Array[39] = (iq_Temp >> 8) & 0x000000FF;
				I2C_Tx_Data_Array[40] = (iq_Temp) & 0x000000FF;
				
				// Current Display gain
				iq_Temp = Product_Info.iq_Current_Display_Gain;	
				I2C_Tx_Data_Array[41] = (iq_Temp >> 24) & 0x000000FF;
				I2C_Tx_Data_Array[42] = (iq_Temp >> 16) & 0x000000FF;
				I2C_Tx_Data_Array[43] = (iq_Temp >> 8) & 0x000000FF;
				I2C_Tx_Data_Array[44] = (iq_Temp) & 0x000000FF;
				
				// Now read the calibration count.
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, CALIBRATION_COUNT_ADDRESS, 1,I2C_Rx_Data_Array)) == FAILURE)
				{
					return;
				}
	
				// Denotes calibration Count			
				// If the value is 0xFF, Initialize it to 1. Else increment it by 1.
				if(I2C_Rx_Data_Array[0] == 0xFF)
				{
					I2C_Tx_Data_Array[45] = 1;
				}
				else
				{
					I2C_Tx_Data_Array[45] = I2C_Rx_Data_Array[0] + 1;
				}
				
				// now calculate checksum
				Uint16 Checksum = 0;
				
				for(i = 0; i < 46; i++)
				{
					Checksum += I2C_Tx_Data_Array[i];
				}
				
				I2C_Tx_Data_Array[46] = Checksum & 0xFF;
				I2C_Tx_Data_Array[47] = Product_Info.ui_Model_Number;		// For CAN msg ID generation
				I2C_Tx_Data_Array[48] = Product_Info.ui_Model_Number;		// Duplicity. For Data verification in the above case when the contents are read.
				
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, MODEL_NO_ADDR, 49,I2C_Tx_Data_Array);
				RESET_EEPROM_WRITE_CYCLE_TIMER;
				
				// Calibration completed. Switch Output off
				State.Mode.bt_Output_Status = OFF;
			}
		}
	}
}

//#####################	Pre Calibration Settings ##################################################################
// Function Name: ei_vSetPreCANCalibrationParameters
// Return Type: void
// Arguments:   void
// Description: When the User enters the date of calibration of PSU through CAN, this function is called before storing
//				date on EEPROM. Since during CAN calibration we already know the specified and Rated Parameters of the PSU
//	we don't have to enter it (This process is different in RS485 calibration where even these parameters have to be
//	re entered). We just read these already stored values in EEPROM and straight away set the Model no. of the PSU.
//#################################################################################################################
static void ei_vSetPreCANCalibrationParameters()
{
	Uint16 I2C_Rx_Data_Array[13];
	int32 Temp_Storage;
	
	// Read till Specified Current.
	while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
	if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, MODEL_NO_ADDR, 13, I2C_Rx_Data_Array)) == FAILURE)
	{
		return;
	}
	
	// I2C_Rx_Data_Array[0] stores JUNK_MODEL_NO (255). Not used.
	
	// For Rated Voltage
	Temp_Storage = (I2C_Rx_Data_Array[1] >> 4) * 100 + (I2C_Rx_Data_Array[1] & 0x000F) * 10 + (I2C_Rx_Data_Array[2] >> 4);
	Temp_Storage = _IQ15(Temp_Storage);
	Temp_Storage += (_IQ15mpy(_IQ15((I2C_Rx_Data_Array[2] & 0x000F)),_IQ15(0.1)) 
					 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[3] >> 4)),_IQ15(0.01))
					 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[3] & 0x000F)),_IQ15(0.001))); // 3 digits after decimal point
	Product_Info.iq_Rated_Voltage = Temp_Storage;
	
	// For Rated current
	Temp_Storage = (I2C_Rx_Data_Array[4] >> 4) * 100 + (I2C_Rx_Data_Array[4] & 0x000F) * 10 + (I2C_Rx_Data_Array[5] >> 4);
	Temp_Storage = _IQ15(Temp_Storage);
	Temp_Storage += (_IQ15mpy(_IQ15((I2C_Rx_Data_Array[5] & 0x000F)),_IQ15(0.1)) 
					 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[6] >> 4)),_IQ15(0.01))
					 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[6] & 0x000F)),_IQ15(0.001))); // 3 digits after decimal point
	Product_Info.iq_Rated_Current = Temp_Storage;
	
	// For SVUL
	Temp_Storage = (I2C_Rx_Data_Array[7] >> 4) * 100 + (I2C_Rx_Data_Array[7] & 0x000F) * 10 + (I2C_Rx_Data_Array[8] >> 4);
	Temp_Storage = _IQ15(Temp_Storage);
	Temp_Storage += (_IQ15mpy(_IQ15((I2C_Rx_Data_Array[8] & 0x000F)),_IQ15(0.1)) 
					 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[9] >> 4)),_IQ15(0.01))
					 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[9] & 0x000F)),_IQ15(0.001))); // 3 digits after decimal point
	Product_Info.iq_Specified_FS_Voltage = Temp_Storage;
	
	// For SCUL
	Temp_Storage = (I2C_Rx_Data_Array[10] >> 4) * 100 + (I2C_Rx_Data_Array[10] & 0x000F) * 10 + (I2C_Rx_Data_Array[11] >> 4);
	Temp_Storage = _IQ15(Temp_Storage);
	Temp_Storage += (_IQ15mpy(_IQ15((I2C_Rx_Data_Array[11] & 0x000F)),_IQ15(0.1)) 
					 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[12] >> 4)),_IQ15(0.01))
					 + _IQ15mpy(_IQ15((I2C_Rx_Data_Array[12] & 0x000F)),_IQ15(0.001))); // 3 digits after decimal point
	Product_Info.iq_Specified_FS_Current = Temp_Storage;
	
	// Set the MODEL No.
	ei_vCheckAndSetModelNo();
	
	// External Adc Initialization. Must always be calculated with rated parameters.
	External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Rated_Voltage, _IQ15(CONVERSION_FACTOR_1P145V_TO_1P25V));
	External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Rated_Current, _IQ15(CONVERSION_FACTOR_1P145V_TO_1P25V));
	
	// Make all Calibration Flags true uptil this point
	Global_Flags.fg_Rated_Volt_Entered = Global_Flags.fg_Rated_Curr_Entered = Global_Flags.fg_Specified_Volt_Entered = Global_Flags.fg_Specified_Curr_Entered = TRUE;
}
