//#################################################################################################################
//# 							enArka Instruments proprietary
//# File: ei_SciApp.c
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.
//# All Rights Reserved
//#
//#################################################################################################################

//########################################### GENERAL #############################################################
//	Dictionary for abbreviations in Names
//	QR: Query
//	CR: Carriage return
//  SP: Space
//	SCI: Serial Communication Interface
//	ADDR: Address
//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
#include "ExternalVariables.h"
//#################################################################################################################
									/*-------Global Prototypes----------*/
//void ei_vSciCommEvent();
//void ei_vUartFifoTx();
//void ei_vEnqueueFaultRegsInErrorQueue(Uint16 Fault_Flag, const Int8 *ErrorString);
//void ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(Int8 *ErrorString);
//void ei_vEnqueueOutputMessageInTransmitBuffer(Int8 *MessageToBePushed);
//#################################################################################################################
									/*-------Local Prototypes----------*/
static Uint16 ei_ulFormMessageFromReceiveBuffer();
static Uint16 ei_vMessageParsing();
static Uint16 ei_uiMatchChecksum();
static void ei_vStrUpper(Int8 *String);
static Uint16 ei_uiMatchCommonCommands(Int8 *s_ParseString);
static Uint16 ei_uiMatchInstrumentCommands();
static Uint16 ei_uiMatchGlobalCommands();
static Uint16 ei_uiMatchSystemCommands();
static Uint16 ei_uiMatchStatusCommands();
static Uint16 ei_uiMatchDisplayCommands();
static Uint16 ei_uiMatchMeasureCommands();
static Uint16 ei_uiCommandMatchingForTopLevelCommands(Int8 *s_Command, Uint16 ui_SubSystemType);
static Uint16 ei_uiMatchVoltageCommands();
static Uint16 ei_uiMatchCurrentCommands();
static Uint16 ei_uiMatchOutputCommands();
static Uint16 ei_uiMatchCalibrationCommands();
static Uint16 ei_uiMatchBootCommand();
static void ei_vOpenCloseUartCommunication(Int8* Params);
static void ei_vUartSetEnableRegisters(Uint16 TypeOfRegister, Int8 *Params);
static void ei_vUartSetPsuControl(Int8 *Params);
static void ei_vUartProgramVoltage(Int8 *Params);
static void ei_vUartProgramOVP(Int8 *Params);
static void ei_vUartProgramUVL(Int8 *Params);
static void ei_vUartProgramCurrent(Int8 *Params);
static void ei_vUartSetProtectionDelay(Int8 *Params);
static void ei_vUartVoltageCalibrationProcess(Int8 *Params);
static void ei_vUartCurrentCalibrationProcess(Int8 *Params);
static void ei_vUartSetRated_SpecifiedParams(Int8 *Params, Uint16 ui_ParameterType);
static void ei_vRespondToReceivedMessage(Uint16 Response_Key);
static void ei_vDequeueFromErrorQueue();
static Uint16 ei_uiUartParameterErrorChecker(Int8 *Parameter, Uint16 ui_Parameter_Type);
static void ei_vEnqueueCommandErrorInErrorQueue(const Int8 *ErrorString);
static void ei_vEnqueueExecutionErrorInErrorQueue(const Int8 *ErrorString);
Int8 fecOnStatus = 0;
Int8 fecOnCommand = 0;
Int8 cnt = 0;
Uint16 debug_variable=0;
Int8 rxChar = 0;
Uint16 Turn_ON_Command_To_FEC = 0;
//###################################	SCI Baud Rate Settings  ###################################################

//		LSPCLK is input to the SCI module. We operate at 100M/10 = 10M LSPCLK.
//
//		SCI Baud Rate = LSPCLK/{(BRR + 1) * 8}
//
//		BRR is Concatenation of SCIHBAUD and SCILBAUD. Both are 8bit registers.
//
//		Different settings for different Baud Rate for LSPCLK of 10M
//
//		Baudrate(bps)		SCIHBAUD(Hex)		SCILBAUD(Hex)		BRR(Hex)	BRR(Dec)		Achieved Baud(bps)
//																								 = 10M/{(BRR+1)*8}
//
//		1200				0x04			`	0x10				0x0410		1040			1200.76
//		2400				0x02				0x07				0x0207		519				2403.846
//		4800				0x01				0x03				0x0103		259				4807.692
//		9600				0x00				0x81				0x0081		129				9615.384
//	   19200				0x00				0x40				0x0040		64				19230.769
//#################################################################################################################
Int8 sciCnt = 0;
//#####################	Communication Event #######################################################################
// Function Name: ei_vSciCommEvent
// Return Type: void
// Arguments:   void
// Description: Does all the Uart communication related operations
//#################################################################################################################
void ei_vSciCommEvent()
{
	// Form the messsage from Input Buffer
	if(ei_ulFormMessageFromReceiveBuffer() == COMPLETE_COMMAND_RECEIVED)	//Check if a complete query/command Received
	{
		// If a complete Query received perform Message Parsing
		ei_vMessageParsing();
	}

	ei_vUartFifoTx();

	fecOnCommand = 0xF1;

//	if((FaultChar[0] == debug_variable)
//	        ||(FaultChar[1] == debug_variable)
//	        ||(FaultChar[2] == debug_variable)
//	        ||(FaultChar[3] == debug_variable))
//	{
//	    // DO Nothing.
//	}
//	else
//	{
        FecFaultHReg.all = FaultChar[2];
        FecFaultLReg.all = FaultChar[1];
        fecOnStatus = FaultChar[3];
//	}


    if(sci_fecOnCommand == 1)
    {
        SciaRegs.SCITXBUF = 0xAA; // ON Command to FEC
        Turn_ON_Command_To_FEC = SciaRegs.SCITXBUF;
    }
    else if(sci_fecOnCommand == 0)
    {
        SciaRegs.SCITXBUF = 0x55; // OFF Command to FEC
        Turn_ON_Command_To_FEC = SciaRegs.SCITXBUF;
    }

    if (CANFailEnable == 1)
    {
        if (CANFailTrig == 1)
        {
            CurRefRampDowncnt++;

            if (CurRefRampDowncnt >= 30)
            {
                Reference.iq_Current_Reference -= _IQ15mpy(Reference.iq_Current_Reference, _IQ15(0.05)); // percentage of current reference ramped down on CAN fail

                if (Reference.iq_Current_Reference <= _IQ15(1.0))
                {
                    Reference.iq_Current_Reference = _IQ15(0.0);
                    Fault_Regs.FAULT_REGISTER.bit.bt_CANcomm_Failure = 1;
                    State.Mode.bt_Output_Status = OFF;
                    sci_fecOnCommand = 0;
                }

                ei_vSwitchSpiClkPolarity(DAC_8552);

                ei_vSetCurrentReference();

                ei_vSwitchSpiClkPolarity(AD7705);

                CurRefRampDowncnt = 0;
            }
        }
        else
        {
            Fault_Regs.FAULT_REGISTER.bit.bt_CANcomm_Failure = 0;
        }
    }
}


//#####################	Form Message from the receive charcter buffer ######################################################################
// Function Name: ei_vFormMessageFromReceiveBuffer
// Return Type: Uint16
// Arguments:   void
// Description: Forms the command/Query from the input buffer and returns TRUE when formation complete
//#################################################################################################################
static Uint16 ei_ulFormMessageFromReceiveBuffer()
{
	// Disable the SCI interrupt here to avoid SHARED DATA PROBLEM
	SciaRegs.SCIFFRX.bit.RXFFIENA = DISABLE_INT;//1

	// Wait 10 Cycles to allow any flagged interrupts to get serviced
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");
	asm("	nop");

	if(RECEIVE_BUFFER_NOT_EMPTY)
	{
		INCREMENT_RECEIVED_BUFFER(Receive_Circular_Buffer.i_Front)
		// Form the message array by shifting the character 1 by 1 from receive buffer
		while((ui_Next <  MAX_UART_ARRAY_SIZE) && (Receive_Circular_Buffer.a_c_ReceiveBuffer[Receive_Circular_Buffer.i_Front] != '\r'))
		{
			if(Receive_Circular_Buffer.a_c_ReceiveBuffer[Receive_Circular_Buffer.i_Front] == BACKSPACE_CHARACTER)
			{
				// In case of backspace we need to delete the previously copied charcter.
				if(ui_Next == 0)
				{
					ui_Next = 0;	// Array index cannot go to -1
				}
				else
				{
					ui_Next--;
				}
			}

			// CHECK AND TEST THIS PIECE OF CODE LATER
			/*else if(IF_NON_STANDARD_ASCII_RECIEVED)
			{
				// An illegal character that is not suppoted. Error in communication link
				ui_Next = 0;
				Receive_Circular_Buffer.i_Front = EMPTY_QUEUE_INDEX;
				Receive_Circular_Buffer.i_Rear = EMPTY_QUEUE_INDEX;

				// Internal Message Fault
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E340);

				SciaRegs.SCIFFRX.bit.RXFFIENA = ENABLE_INT;//1
				return FALSE;
			}*/

			else
			{
				// Copy the value from buffer to message string if not backspace
				Uart_Rx_Data_Array[ui_Next] = Receive_Circular_Buffer.a_c_ReceiveBuffer[Receive_Circular_Buffer.i_Front];

				// Checking for $ symbol and checksum
				if(Uart_Rx_Data_Array[ui_Next] == DOLLAR_CHARACTER)
				{
					Global_Flags.fg_Checksum_Included_In_Uart_Comm = TRUE;
				}
				ui_Next++;
			}
			// Check if Queue empty
			if(RECEIVE_BUFFER_EMPTY)
			{
				Receive_Circular_Buffer.i_Front = EMPTY_QUEUE_INDEX;
				Receive_Circular_Buffer.i_Rear = EMPTY_QUEUE_INDEX;
				break;
			}
			INCREMENT_RECEIVED_BUFFER(Receive_Circular_Buffer.i_Front)
		}

		// Too Much Data. Error
		if(ui_Next == MAX_UART_ARRAY_SIZE)
		{
			ei_vEnqueueExecutionErrorInErrorQueue(EM223);

			// Flush receive buffer
			Receive_Circular_Buffer.i_Front = EMPTY_QUEUE_INDEX;
			Receive_Circular_Buffer.i_Rear = EMPTY_QUEUE_INDEX;
			ui_Next = 0;

			SciaRegs.SCIFFRX.bit.RXFFIENA = ENABLE_INT;//1
			return FALSE;
		}

		// '\r' found
		if(Receive_Circular_Buffer.a_c_ReceiveBuffer[Receive_Circular_Buffer.i_Front] == '\r')
		{
			Uart_Rx_Data_Array[ui_Next] = '\r';
			Uart_Rx_Data_Array[ui_Next+1] = '\0';
			ui_Next = 0;

			// Check if Queue empty
			if(RECEIVE_BUFFER_EMPTY)
			{
				Receive_Circular_Buffer.i_Front = EMPTY_QUEUE_INDEX;
				Receive_Circular_Buffer.i_Rear = EMPTY_QUEUE_INDEX;
			}

			SciaRegs.SCIFFRX.bit.RXFFIENA = ENABLE_INT;//1
			return COMPLETE_COMMAND_RECEIVED;
		}
	}
	else
	{
		// Safety conditons.
		Receive_Circular_Buffer.i_Front = EMPTY_QUEUE_INDEX;	// Flush buffer
//		ui_Next = 0;
//		Uart_Rx_Data_Array[0] = '\0';
	}

	SciaRegs.SCIFFRX.bit.RXFFIENA = ENABLE_INT;//1
	return FALSE;
}

//#####################	Message Parsing ###########################################################################
// Function Name: ei_vMessageParsing
// Return Type: Uint16
// Arguments:   void
// Description: The complete message parsing is done in this function. The return type is just an indication of SUCCESS or
//				FAILURE of parsing.
//				Steps:
//				1. Check if user has sent message with checksum. If included then calculate checksum of message and compare with
//				   the appended checksum
//				2. On Checksum success convert the complete message to Upper case.
//				3. Check for common commands.
//				4. If not Common commands then find the right subsystem by seperated by : using strtok
//				5. On a successful match of particular subsystem branch to the appropriate function for that subsystem
//				6. Successive calls of strtok(with the appropriate delimiter) in the respective subsystem function
//				   will check the Message at further until an error is spotted or the parameter captured.

//						NOTE: About strtok			[Source: cplusplus.com]
//SYNTAX: char * strtok ( char * str, const char * delimiters );
//USAGE: Split string into tokens
//DESC:
//A sequence of calls to this function split str into tokens, which are sequences of contiguous characters separated by any of the characters that are part of delimiters.

// On a first call, the function expects a C string as argument for str, whose first character is used as the starting
// location to scan for tokens. In subsequent calls, the function expects a null pointer and uses the position right
// after the end of last token as the new starting location for scanning.

//To determine the beginning and the end of a token, the function first scans from the starting location for the first
//character not contained in delimiters (which becomes the beginning of the token). And then scans starting from this beginning
//of the token for the first character contained in delimiters, which becomes the end of the token.

//This end of the token is automatically replaced by a null-character by the function, and the beginning of the token is returned by the function.
//Once the terminating null character of str has been found in a call to strtok, all subsequent calls to this function with a null pointer as the first argument return a null pointer.
//The point where the last token was found is kept internally by the function to be used on the next call (particular library implementations are not required to avoid data races).

//PARAMETERS
//str
//C string to truncate.
//Notice that the contents of this string are modified and broken into smaller strings (tokens).
//Alternativelly, a null pointer may be specified, in which case the function continues scanning where a previous successful call to the function ended.
//delimiters
//C string containing the delimiter characters.
//These may vary from one call to another.

//RETURN VALUE
//A pointer to the last token found in string.
//A null pointer is returned if there are no tokens left to retrieve.

// Additional note on STRTOK:
// strtok(NULL,JUNK_CHARCTER)
// We used to use strtok(NULL,NULL) to retrieve the remaining part of string earlier. NULL as a delimiter was
// undefined. We changed this to a JUNK_CHARCTER (like : for example) which we know should not be present in that part
// of the string. It can give a more stable operation.
//#################################################################################################################
static Uint16 ei_vMessageParsing()
{
	Int8 *s_Subsystem;

	// If user has included checksum, we need to check for its correctness.
	if(Global_Flags.fg_Checksum_Included_In_Uart_Comm == TRUE)
	{
		if(ei_uiMatchChecksum() == FAILURE)	// User has violated checksum
		{
	 		// Checksum error. Clear the checksum included flag first and push the error into the queue
	 		Global_Flags.fg_Checksum_Included_In_Uart_Comm = FALSE;
	 		ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E344);
			return FAILURE;
		}
	}

	ei_vStrUpper(Uart_Rx_Data_Array);	// Convert everything to upper case so that case is not a problem while parsing further down.

	// Assign local pointer to received data.
	s_Subsystem = Uart_Rx_Data_Array;

	// Process for Common commands by checking for a leading *. ABOrt does not have leading * but we will
	// worry about it later. We can check for a leading 'A' later. No other subsystem starts and A.

	if(*s_Subsystem == '*')
	{
		ei_uiMatchCommonCommands(s_Subsystem);
		return SUCCESS;
	}

	// DO NOT CALL strtok BEFORE PROCESSING COMMON COMMANDS. IT WILL CORRUPT Uart_Rx_Data_Array
	// Get the otehr subsystem incase not common commands
	s_Subsystem = strtok(Uart_Rx_Data_Array,COLON);

	// Find the right Subsystem

	// INSTRUMENT SUBSYSYTEM
	if((strcmp(s_Subsystem,INSTRUMENT) == MATCH_FOUND) || (strcmp(s_Subsystem,INST) == MATCH_FOUND))
	{
		ei_uiMatchInstrumentCommands();
		return SUCCESS;
	}
	// GLOBAL SUBSYSTEM
	else if((strcmp(s_Subsystem,GLOBAL) == MATCH_FOUND) || (strcmp(s_Subsystem,GLOB) == MATCH_FOUND))
	{
		// Enter only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			ei_uiMatchGlobalCommands();
			return SUCCESS;
		}
		else
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
			return FAILURE;
		}
	}
	else if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
	{
		// All Subsystems after this can be accessed only if the communication with this unit open.

		// BOOT SUBSYSTEM
		if(strcmp(s_Subsystem,BOOT) == MATCH_FOUND)
		{
			// PSU will process Boot Commands only in Communication Open mode.
			// They will not work in coupled ALL mode

			if(Global_Flags.fg_Uart_Comm_with_this_PSU == OPEN)
			{
				// Boot mode works with/without calibration
				ei_uiMatchBootCommand();
			}
			return SUCCESS;
		}

		// CALIBRATE SUBSYSTEM
		if((strcmp(s_Subsystem,CALIBRATE) == MATCH_FOUND) || (strcmp(s_Subsystem,CAL) == MATCH_FOUND))
		{
			// PSU will process calibration Commands only in Communication Open mode.
			// They will not work in coupled ALL mode

			if(Global_Flags.fg_Uart_Comm_with_this_PSU == OPEN)
			{
				// Commands/Queries inside work with/without calibration
				ei_uiMatchCalibrationCommands();
			}
			return SUCCESS;
		}

		// SYSTEM SUBSYSTEM
		if((strcmp(s_Subsystem,SYSTEM) == MATCH_FOUND) || (strcmp(s_Subsystem,SYST) == MATCH_FOUND))
		{
			// Condition for calibration checked inside system commnds because one of the queries in the subsystem
			// ERROR? works without calibration also
			ei_uiMatchSystemCommands();
			return SUCCESS;
		}

		// STATUS SUBSYSTEM
		if((strcmp(s_Subsystem,STATUS) == MATCH_FOUND) || (strcmp(s_Subsystem,STAT) == MATCH_FOUND))
		{
			ei_uiMatchStatusCommands();
			return SUCCESS;
		}

		// DISPLAY SUBSYSTEM
		if((strcmp(s_Subsystem,DISPLAY) == MATCH_FOUND) || (strcmp(s_Subsystem,DISP) == MATCH_FOUND))
		{
			ei_uiMatchDisplayCommands();
			return SUCCESS;
		}

		// All commands will check for calibration after this
		// MEASURE SUBSYSTEM
		if((strcmp(s_Subsystem,MEASURE) == MATCH_FOUND) || (strcmp(s_Subsystem,MEAS) == MATCH_FOUND))
		{
			if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
			{
				ei_uiMatchMeasureCommands();
				return SUCCESS;
			}
			else
			{
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// VOLTAGE SUBSYSTEM with ROOT
		if((strcmp(s_Subsystem,VOLTAGE) == MATCH_FOUND) || (strcmp(s_Subsystem,VOLT) == MATCH_FOUND))
		{
			// Check the goto labels for processing comments
			goto VOLTAGE_SUBSYSTEM_DETECTED_WITH_MORE_TOKENS;	// Inside Optional SOURCE SUBSYSTEM
		}

		// VOLTAGE SUBSYSTEM without ROOT
		if((strncmp(s_Subsystem,VOLTAGE,LENGTH_OF_VOLTAGE) == MATCH_FOUND) || (strncmp(s_Subsystem,VOLT,LENGTH_OF_VOLT) == MATCH_FOUND))
		{
			// Check the goto labels for processing comments
			goto VOLTAGE_SUBSYSTEM_DETECTED_WITHOUT_TOKENS;	// Inside Optional SOURCE SUBSYSTEM
		}

		// CURRENT SUBSYSTEM with ROOT
		if((strcmp(s_Subsystem,CURRENT) == MATCH_FOUND) || (strcmp(s_Subsystem,CURR) == MATCH_FOUND))
		{
			// Check the goto labels for processing comments
			goto CURRENT_SUBSYSTEM_DETECTED_WITH_MORE_TOKENS;	// Inside Optional SOURCE SUBSYSTEM
		}

		// CURRENT SUBSYSTEM without ROOT
		if((strncmp(s_Subsystem,CURRENT,LENGTH_OF_CURRENT) == MATCH_FOUND) || (strncmp(s_Subsystem,CURR,LENGTH_OF_CURR) == MATCH_FOUND))
		{
			// Check the goto labels for processing comments
			goto CURRENT_SUBSYSTEM_DETECTED_WITHOUT_TOKENS;	// Inside Optional SOURCE SUBSYSTEM
		}

		// OUTPUT SUBSYSTEM with ROOT
		if((strcmp(s_Subsystem,OUTPUT) == MATCH_FOUND) || (strcmp(s_Subsystem,OUTP) == MATCH_FOUND))
		{
			// Calibration Must
			if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
			{
				// More tokens present. We have removed the OUTPut token at this point. Call the Normal OUTPut Subsystems
				// Processing command
				ei_uiMatchOutputCommands();
			}
			else
			{
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
			return SUCCESS;
		}

		// OUTPUT SUBSYSTEM without ROOT
		if((strncmp(s_Subsystem,OUTPUT,LENGTH_OF_OUTPUT) == MATCH_FOUND) || (strncmp(s_Subsystem,OUTP,LENGTH_OF_OUTP) == MATCH_FOUND))
		{
			// Calibration Must
			if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
			{
				// No more tokens present. The command is of the form OUTPut <arg> / OUTPut?
				// In this case pass the argument and Type of command to a special function that parses the commands
				// without token specifiers.
				ei_uiCommandMatchingForTopLevelCommands(s_Subsystem,OUT_PROCESSING);
				return SUCCESS;
			}
			else
			{
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// Optional SOURCE SUBSYSTEM
		if((strcmp(s_Subsystem,SOURCE) == MATCH_FOUND) || (strcmp(s_Subsystem,SOUR) == MATCH_FOUND))
		{
			// SOURCE Subsystem includes VOLTAGE, CURRENT subsystems. We will just check again for these subsystems inside SOURCE
			// Go one level further and check for the right subsystem
			s_Subsystem = strtok(NULL,COLON);
			if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
			{
				// VOLTAGE SUBSYSTEM with ROOT
				if((strcmp(s_Subsystem,VOLTAGE) == MATCH_FOUND) || (strcmp(s_Subsystem,VOLT) == MATCH_FOUND))
				{
					// More tokens present. We have removed the voltage token at this point. Call the Normal Voltage Subsystems
					// Processing command
					VOLTAGE_SUBSYSTEM_DETECTED_WITH_MORE_TOKENS:;
					ei_uiMatchVoltageCommands();
					return SUCCESS;
				}

				// VOLTAGE SUBSYSTEM without ROOT
				if((strncmp(s_Subsystem,VOLTAGE,LENGTH_OF_VOLTAGE) == MATCH_FOUND) || (strncmp(s_Subsystem,VOLT,LENGTH_OF_VOLT) == MATCH_FOUND))
				{
					// No more tokens present. The command is of the form VOLTage <arg> / VOLTage? / VOLTage?MAX/MIN
					// In this case pass the argument and Type of command to a special function that parses the commands
					// without token specifiers.
					VOLTAGE_SUBSYSTEM_DETECTED_WITHOUT_TOKENS:;
					ei_uiCommandMatchingForTopLevelCommands(s_Subsystem,V_PROCESSING);
					return SUCCESS;
				}

				// CURRENT SUBSYSTEM with ROOT
				if((strcmp(s_Subsystem,CURRENT) == MATCH_FOUND) || (strcmp(s_Subsystem,CURR) == MATCH_FOUND))
				{
					// More tokens present. We have removed the CURRent token at this point. Call the Normal Current Subsystems
					// Processing command
					CURRENT_SUBSYSTEM_DETECTED_WITH_MORE_TOKENS:;
					ei_uiMatchCurrentCommands();
					return SUCCESS;
				}

				// CURRENT SUBSYSTEM without ROOT
				if((strncmp(s_Subsystem,CURRENT,LENGTH_OF_CURRENT) == MATCH_FOUND) || (strncmp(s_Subsystem,CURR,LENGTH_OF_CURR) == MATCH_FOUND))
				{
					// No more tokens present. The command is of the form CURRent <arg> / CURRent? / CURRent?MAX/MIN
					// In this case pass the argument and Type of command to a special function that parses the commands
					// without token specifiers.
					CURRENT_SUBSYSTEM_DETECTED_WITHOUT_TOKENS:;
					ei_uiCommandMatchingForTopLevelCommands(s_Subsystem,C_PROCESSING);
					return SUCCESS;
				}

				// If we come here no legal subsystems found within source. Command Error
				ei_vEnqueueCommandErrorInErrorQueue(EM100);
				return FAILURE;
			}
			else
			{
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

	}
	else
	{
		// Communication with this PSU not open. Doesn't care about the commands. Just returns SUCCESS
		return SUCCESS;
	}

	// If we come here parsing has failed. Not matching with any subsystem. Command Error
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Checksum matching #########################################################################
// Function Name: MatchChecksum
// Return Type: Uint16
// Arguments:   void
// Description: Calculate the checksum of the received command/query and match with what user has sent.
//				If correct then return success, otherwise return failure.
//				If Success replace the Checksum symbol with '\r' and the next character with '\0'.
//#################################################################################################################
static Uint16 ei_uiMatchChecksum()
{
	Uint16 Checksum = 0;
	Uint16 i = 0;
	Int8 Reference_str[4];	// Only 3 characters + NULL required

	// Add the ascii value of all characters till $ sign found
	while(Uart_Rx_Data_Array[i] != '$')
	{
		Checksum += Uart_Rx_Data_Array[i++];
	}

	//Convert charcters after $ sign to Uppercase to avoid confusion
	ei_vStrUpper(&Uart_Rx_Data_Array[i+1]);

	// Checksum appended is actually 2 nibbles. So our Reference_Str needs to be 2 characters + '\r'.
	// So for checksum values which are less than 0x10 (last two nibbles only), we pad with 0.
	// The width specified is 2 which means minimum 2 characters to be printed.
	sprintf(Reference_str,"%02X\r",Checksum & 0x00FF);

	if(strcmp(&Uart_Rx_Data_Array[i+1],Reference_str) == MATCH_FOUND)	// Checksum correct
	{
		// Replace $ with '\r' and next character with null
		Uart_Rx_Data_Array[i] = '\r';
		Uart_Rx_Data_Array[i+1] = '\0';
		return SUCCESS;
	}
	else // Checksum error
		return FAILURE;
}

//#####################	lower to upper case #######################################################################
// Function Name: StrUpper
// Return Type: void
// Arguments:   char *String
// Description: Accepts a string and converts all lowerercase characters to uppercase. All non alphabetical characters
//				are left as it is.
//				THE ORIGINAL STRING IS MODIFIED. DOES NOT RETURN A NEW STRING.
//#################################################################################################################
static void ei_vStrUpper(Int8 *String)
{
	Int8 *temp;
	temp = String;

	while(*temp !='\0')
	{
		// check for lowercase and convert to uppercase
		if((*temp >= 97) && (*temp <= 122))
		{
			*temp -= 32;
		}
		temp++;
	}
}

//#####################	Common Commands ###########################################################################
// Function Name: ei_uiMatchCommonCommands
// Return Type: Uint16
// Arguments:   Int8 *
// Description: Match and service common commands.
//
// Common commands do not fall under any other subsystem(i.e), they are not preceded by the root(:) token or the
// Subsystem specifier(GLOB, INST etc..). They must be processed first on entering the Message parsing function.
//
// This is the only Subsytem level parsing function that takes an argument. All others use strtok() for operation.
// We don't use strtok here because the common commands doesn't cleanly split into different subsystems using any
// direct tokens. Using strtok() here will corrupt the Uart_Rx_Data_Array.

// Also Checking for calibration and Open COmmunication has to be performed individually for each condition because
// of the same problem (Not falling under any subsystem).

// 		This system works only when
//		1. PSU open for communication
//		2. PSU calibrated
//		(EXCEPT FOR *RST WHICH WORKS NO MATTER WHAT)

// Returns SUCCESS or FAILURE depending on the match found/not found inside.

//						NOTE about OPC
// OPC stands for operation complete.
// The operation complete bit is set when all Power supply operations are complete.
//
// Now. What is finally the operation of the power supply? The basic operation of the pwer supply is to
// 1. Change Voltage set point
// 2. Change Current set point
// 3. Change Ovp set point
// depending on command. All other PSU features enables the user to implement the 3 basic functions above through
// various means. Thus when we say PSU operation complete, we mean that the power supply has completed setting references.

// Now the actual action of setting the refernces (DAC or PWM drivers), does not occur immediately after user
// chooses the value (Through front Panel, Uart or CAN. Here we refer to only DSP based set points and not analog programming).
// Because of the present implementation of the scheduler this time delay can be upto 100ms. The particular device
// drivers that go and change the references in hardware are called in SetReferences().

// Therefore it is only logical that operation complete should be set in SetReferences.

// Usage Of *OPC:  OPC bit Set --> Set References operation completed and the PSU output set to the latest references.
//				   OPC bit cleared --> This bit is cleared by sending command *OPC. This bit will then automatically
//									   be set in the next call of set references.

// Quering *OPC?: Returns ASCII ‘1’ as soon as all commands and queries are completed. It checks for 2 conditions.
//				  (fg_Query_For_Opc == TRUE) and (Operation_Completed_bit == 1). It is checked in UartFifo after the
//				  the last character has been transmitted.
//#################################################################################################################
static Uint16 ei_uiMatchCommonCommands(Int8 *s_ParseString)
{
	// *CLS
	if(strcmp(s_ParseString,STAR_CLS_WITH_CR) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				// Clear the entire Status structure.
				ei_vClearPsuStatusStructure();
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// *ESE <NR1>
	if(strncmp(s_ParseString,STAR_ESE_WITH_SP,LENGTH_OF_COMMON_COMMANDS_WITH_SP) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				// The register needed to be changed and Parameter string.
				ei_vUartSetEnableRegisters(STD_EVENT_STATUS_ENABLE_REGS,&s_ParseString[5]);
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// *ESE?
	if(strcmp(s_ParseString,STAR_QR_ESE_WITH_CR) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				ei_vRespondToReceivedMessage(REPLY_WITH_STD_EVENT_STATUS_ENABLE_REGISTER);
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// *ESR?
	if(strcmp(s_ParseString,STAR_QR_ESR_WITH_CR) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				ei_vRespondToReceivedMessage(REPLY_WITH_STD_EVENT_STATUS_REGISTER);
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// *IDN?
	if(strcmp(s_ParseString,STAR_QR_IDN_WITH_CR) == MATCH_FOUND)
	{
		// Can be processed even when no calibration done. It's safe. This is because it
		// will be needed to check string during calibration.
		// Process only if Communication not closed
		if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_IDENTIFICATION_STRING);
		}
		return SUCCESS;
	}

	// *OPT?
	if(strcmp(s_ParseString,STAR_QR_OPT_WITH_CR) == MATCH_FOUND)
	{
		// Can be processed even when no calibration done. It's safe. This is because it
		// will be needed to check string during calibration.
		// Process only if Communication not closed
		if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_INSTALLED_OPTIONS);
		}
		return SUCCESS;
	}

	// *PSC <bool>
	if(strncmp(s_ParseString,STAR_PSC_WITH_SP,LENGTH_OF_COMMON_COMMANDS_WITH_SP) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				MATCH_BOOL_FOR_LOGIC_TRUE(&s_ParseString[5])
				{
					// if 1/ON then always clear at powerup
					State.Mode.bt_Power_On_Status_Clear = ON;
					return SUCCESS;
				}
				MATCH_BOOL_FOR_LOGIC_FALSE(&s_ParseString[5])
				{
					// if 0/OFF then restore last set values.
					State.Mode.bt_Power_On_Status_Clear = OFF;
					return SUCCESS;
				}

				// If code comes here. There's an error in Argument. Illegal Parameter
				ei_vEnqueueCommandErrorInErrorQueue(EM101);
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// *PSC?
	if(strcmp(s_ParseString,STAR_QR_PSC_WITH_CR) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				ei_vRespondToReceivedMessage(REPLY_WITH_POWER_ON_STATE_CLEAR_SETTING);
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// *RCL <arg>
	if(strncmp(s_ParseString,STAR_RCL_WITH_SP,LENGTH_OF_COMMON_COMMANDS_WITH_SP) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				if(strcmp(&s_ParseString[5],ONE_WITH_CR) == MATCH_FOUND)
				{
					ei_vRecallLastPSUSettings();
					return SUCCESS;
				}

				// If code comes here. There's an error in Argument. Illegal Parameter
				ei_vEnqueueCommandErrorInErrorQueue(EM101);
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		// else if(strcmp(Parameters,TWO) == MATCH_FOUND)	// Room for expansion

		return SUCCESS;
	}

	// *SAV <arg>
	if(strncmp(s_ParseString,STAR_SAV_WITH_SP,LENGTH_OF_COMMON_COMMANDS_WITH_SP) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				if(strcmp(&s_ParseString[5],ONE_WITH_CR) == MATCH_FOUND)
				{
					ei_vWriteLastSettingsEEPROM(Product_Info.ui_Calibration_Status);
					return SUCCESS;
				}

				// If code comes here. There's an error in Argument. Illegal Parameter
				ei_vEnqueueCommandErrorInErrorQueue(EM101);
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		// else if(strcmp(Parameters,TWO) == MATCH_FOUND)	// Room for expansion

		return SUCCESS;
	}

	// *RST takes no arguments. THIS IS SUPPORTED EVEN IN UNCALIBRATED MODE and COMMUNICATION CLOSED MODE
	if(strcmp(s_ParseString,STAR_RST_WITH_CR) == MATCH_FOUND)
	{
		ei_vResetPsuToSafeState();
		return SUCCESS;
	}

	// *SRE <NR1>
	if(strncmp(s_ParseString,STAR_SRE_WITH_SP,LENGTH_OF_COMMON_COMMANDS_WITH_SP) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				// The register needed to be changed and Parameter string.
				ei_vUartSetEnableRegisters(SERVICE_REQUEST_ENABLE_REGS,&s_ParseString[5]);
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// *SRE?
	if(strcmp(s_ParseString,STAR_QR_SRE_WITH_CR) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				ei_vRespondToReceivedMessage(REPLY_WITH_SERVICE_REQUEST_ENABLE_REGISTER);
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// *STB?
	if(strcmp(s_ParseString,STAR_QR_STB_WITH_CR) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				ei_vRespondToReceivedMessage(REPLY_WITH_STATUS_BYTE_REGISTER);
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// *OPC
	if(strcmp(s_ParseString,STAR_OPC_WITH_CR) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				// For a Description about *OPC see the comments on top of this function.
				Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.bit.bt_Operation_Complete = CLEARED;
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// *OPC?
	if(strcmp(s_ParseString,STAR_QR_OPC_WITH_CR) == MATCH_FOUND)
	{
		// Process only if PSU calibrated
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
		{
			// Process only if Communication not closed
			if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
			{
				// Just set the OPC queried flag. See the comment header section of this function for details.
				Global_Flags.fg_Query_For_Opc = TRUE;
			}
		}
		else	// Uncalibrated
		{
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		}

		return SUCCESS;
	}

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Instrument Subsystem ######################################################################
// Function Name: ei_uiMatchInstrumentCommands
// Return Type: Uint16
// Arguments:   void
// Description: Match and service the Instrument Subsystem commands. These commands open/close the communication with the
//				Instrument by specifying the address set from from the front panel.
//				Note:
//				COUPLE ALL: Opens Communication for all the power supplies. But no power supply will respond to any queries
//							All power supplies can be controlled with the same set of commands in Couple All mode.

// 		This system works only when
//		1. PSU open for communication
//		2. PSU calibrated

//				For understanding the parsing operation read the comments for ei_uiMatchGlobalCommands
//#################################################################################################################
static Uint16 ei_uiMatchInstrumentCommands()
{
	Int8 *p_s_NextParsePoint;
	Int8 *Parameters;

	// INSTRUMENT SUBSYTEM has been detected by MessageParser function.
	// Now we Close in on the Parameter
	p_s_NextParsePoint = strtok(NULL,SPACE);	// Everything before Space
	Parameters = strtok(NULL,JUNK_CHARACTER);	// Everything after space

	// COUPLE ALL/NONE
	if((strcmp(p_s_NextParsePoint,COUPLE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,COUPL) == MATCH_FOUND))
	{
		if(strcmp(Parameters,ALL_WITH_CR) == MATCH_FOUND)
		{
			Global_Flags.fg_Uart_Comm_with_this_PSU = COUPLED_COMMUNICATION_MODE;
			return SUCCESS;
		}

		if(strcmp(Parameters,NONE_WITH_CR) == MATCH_FOUND)
		{
			Global_Flags.fg_Uart_Comm_with_this_PSU = CLOSED;
			return SUCCESS;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// NSELECT n
	if((strcmp(p_s_NextParsePoint,NSELECT) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,NSEL) == MATCH_FOUND))
	{
		ei_vOpenCloseUartCommunication(Parameters);
		return SUCCESS;
	}

	// NSELECT?
	if((strcmp(p_s_NextParsePoint,QR_NSELECT_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_NSEL_WITH_CR) == MATCH_FOUND))
	{
		// Process only if communication open with this PSU
		if(Global_Flags.fg_Uart_Comm_with_this_PSU != CLOSED)
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_INSTRUMENT_ADDR);
		}
		return SUCCESS;
	}

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Global Subsytem ###########################################################################
// Function Name: ei_uiMatchGlobalCommands
// Return Type: Uint16
// Arguments:   void
// Description: Match and service the global commands. All PSU will take action on global commands no matter
//				which PSU is currently connected to the host.Followings things are important in global commands.
//				1. All PSU's will take action on Global commands.
//				2. PSU will not reply to any global commands irrespective of the fact that the command was executed
//				   or not. It is the responsibility of the host to send the commands correctly.
//				3. Global commands will not affect the present communication link between host and currently addressed
//					PSU
// 				GOTO used in cases where optional arguments are possible

// COMMANDS COMPULSORILY SEPERATED BY MORE THAN : MUST BE CHECKED LAST BECAUSE STRTOK WILL MODIFY THE PARSING STRING
//#################################################################################################################
static Uint16 ei_uiMatchGlobalCommands()
{
	Global_Flags.fg_Checksum_Included_In_Uart_Comm = FALSE;	// No checksum response for global commands
	Int8 *p_s_NextParsePoint;
	Int8 *Parameters;
	Int8 *OptionalHeader;

	// Here we define the literals that are used more than once inside to save space

	// GLOBAL SUBSYTEM has been detected by MessageParser function.
	// Now we Close in on the Parameter
	p_s_NextParsePoint = strtok(NULL,SPACE);	// Everything before Space
	Parameters = strtok(NULL,JUNK_CHARACTER);	// Everything after space

	// IMPORTANT TO UNDERSTAND THE PREVIOUS STEP.
	// If we have received a proper command then,
	// 1. p_s_NextParsePoint holds the string before space
	// 2. strtok functions hold the parameter string.

	// The intenetion is to correctly navigate to the parameter

	// Check for *type commands first

	// *RST takes no arguments
	if(strcmp(p_s_NextParsePoint,STAR_RST_WITH_CR) == MATCH_FOUND)
	{
		ei_vResetPsuToSafeState();
		return SUCCESS;
	}

	// *RCL takes an argument
	if(strcmp(p_s_NextParsePoint,STAR_RCL) == MATCH_FOUND)
	{
		if(strcmp(Parameters,ONE_WITH_CR) == MATCH_FOUND)
		{
			ei_vRecallLastPSUSettings();
			return SUCCESS;
		}
		// else if(strcmp(Parameters,TWO) == MATCH_FOUND)	// Room for expansion

		// If we come here Illegal Parameter.
		ei_vEnqueueCommandErrorInErrorQueue(EM101);
		return FAILURE;
	}

	// *SAV takes an argument
	if(strcmp(p_s_NextParsePoint,STAR_SAV) == MATCH_FOUND)
	{
		if(strcmp(Parameters,ONE_WITH_CR) == MATCH_FOUND)
		{
			ei_vWriteLastSettingsEEPROM(Product_Info.ui_Calibration_Status);
			return SUCCESS;
		}
		// else if(strcmp(Parameters,TWO) == MATCH_FOUND)	// Room for expansion

		// If we come here Illegal Parameter.
		ei_vEnqueueCommandErrorInErrorQueue(EM101);
		return FAILURE;
	}

	// VOLTAGE <arg>
	if((strcmp(p_s_NextParsePoint,VOLTAGE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,VOLT) == MATCH_FOUND))
	{
		// Makes life easier. Goes to the point where optional header after this parameter, incase present is checked
		goto VOLTAGE_PARAMTER_DETECTED;
	}

	// Optional Command Check for VOLTAGE:AMPITUDE
	if((strncmp(p_s_NextParsePoint,VOLTAGE,LENGTH_OF_VOLTAGE) == MATCH_FOUND) || (strncmp(p_s_NextParsePoint,VOLT,LENGTH_OF_VOLT) == MATCH_FOUND))
	{
		//VOLTAGE[:AMPLITUDE] <arg>
		// Get the next node

		p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);
		OptionalHeader = strtok(NULL,JUNK_CHARACTER);
		// Optional Header contains AMPLITUDE or AMPL

		if((strcmp(OptionalHeader,AMPLITUDE) == MATCH_FOUND) || (strcmp(OptionalHeader,AMPL) == MATCH_FOUND))
		{
			VOLTAGE_PARAMTER_DETECTED:;		// goto label from previous if test
			ei_vUartProgramVoltage(Parameters);
			return SUCCESS;
		}

		// If we come here. Command Error.
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// CURRENT <arg>
	if((strcmp(p_s_NextParsePoint,CURRENT) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,CURR) == MATCH_FOUND))
	{
		// Makes life easier. Goes to the point where optional header after this parameter, incase present is checked
		goto CURRENT_PARAMTER_DETECTED;
	}

	// Optional Command Check for CURRENT:AMPITUDE
	if((strncmp(p_s_NextParsePoint,CURRENT,LENGTH_OF_CURRENT) == MATCH_FOUND) || (strncmp(p_s_NextParsePoint,CURR,LENGTH_OF_CURR) == MATCH_FOUND))
	{
		//CURRENT[:AMPLITUDE] <arg>
		// Get the next node

		p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);
		OptionalHeader = strtok(NULL,JUNK_CHARACTER);

		// Optional Header contains AMPLITUDE or AMPL

		if((strcmp(OptionalHeader,AMPLITUDE) == MATCH_FOUND) || (strcmp(OptionalHeader,AMPL) == MATCH_FOUND))
		{
			CURRENT_PARAMTER_DETECTED:;		// goto label from previous if test
			ei_vUartProgramCurrent(Parameters);
			return SUCCESS;
		}
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// Checking for OUTPUT:STATE <bool>
	p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);
	if((strcmp((const Int8 *)p_s_NextParsePoint,OUTPUT) == MATCH_FOUND) || (strcmp((const Int8 *)p_s_NextParsePoint,OUTP) == MATCH_FOUND))
	{
		// Match found for this node.
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);	// Next Token
		if((strcmp((const Int8 *)p_s_NextParsePoint,STATE) == MATCH_FOUND) || (strcmp((const Int8 *)p_s_NextParsePoint,STAT) == MATCH_FOUND))
		{
			// Match Found at this node
			// All that remains is the parameter
			MATCH_BOOL_FOR_LOGIC_TRUE(Parameters)
			{
				State.Mode.bt_Output_Status = ON;
				Global_Flags.fg_Output_Switched_On_From_Uart = TRUE;	// A flag indicating output is switched on from uart.
				return SUCCESS;
			}
			MATCH_BOOL_FOR_LOGIC_FALSE(Parameters)
			{
				State.Mode.bt_Output_Status = OFF;
				return SUCCESS;
			}

			// If we come here then it's an illegal parameter
			ei_vEnqueueCommandErrorInErrorQueue(EM101);
			return FAILURE;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}
	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	System Subsytem ###########################################################################
// Function Name: ei_uiMatchSystemCommands
// Return Type: Uint16
// Arguments:   void
// Description: Match and service system subsystem.
//
//				System subsystem works only when
//				1. The PSU is currently addressed or coupled
//				2. Calibration done. (Except Error?)
//
// The test for calibration will be checked inside this function because, we need to always transmit error messages
// irrespective of PSU calibration status.

// COMMANDS COMPULSORILY SEPERATED BY MORE THAN : MUST BE CHECKED LAST BECAUSE STRTOK WILL MODIFY THE PARSING STRING
//#################################################################################################################
static Uint16 ei_uiMatchSystemCommands()
{
	Int8 *p_s_NextParsePoint;
	Int8 *Parameters;
	Int8 *OptionalHeader;

	// SYSTEM SUBSYTEM has been detected by MessageParser function.
	// Now we Close in on the Parameter
	p_s_NextParsePoint = strtok(NULL,SPACE);	// Everything before Space
	Parameters = strtok(NULL,JUNK_CHARACTER);	// Everything after space

	// ERROR?
	if(strcmp(p_s_NextParsePoint,QR_ERROR_WITH_CR) == MATCH_FOUND)
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_SYSTEM_ERROR);
		return SUCCESS;
	}

	// DATE?
	if(strcmp(p_s_NextParsePoint,QR_DATE_WITH_CR) == MATCH_FOUND)
	{
		// Can be processed even when no calibration done. It's safe. This is because it
		// will be needed to check string during calibration.
		ei_vRespondToReceivedMessage(REPLY_WITH_CALIBRATION_DATE);
		return SUCCESS;
	}

	// All Commands after this will be checked only if PSU calibrated
	if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
	{
		// VERSION?
		if((strcmp(p_s_NextParsePoint,QR_VERSION_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_VERS_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_SOFTWARE_VERSION);
			return SUCCESS;
		}

		// REMOTE <arg>
		if((strcmp(p_s_NextParsePoint,REMOTE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,REM) == MATCH_FOUND))
		{
			// Makes life easier. Goes to the point where optional header after this parameter, incase present is checked
			goto REMOTE_SETTING_PARAMETER_DETECTED;
		}

		// REMOTE?
		if((strcmp(p_s_NextParsePoint,QR_REMOTE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_REM_WITH_CR) == MATCH_FOUND))
		{
			// Makes life easier. Goes to the point where optional header after this parameter, incase present is checked
			goto REMOTE_QUERY_DETECTED;
		}

		// Optional Command Check for REMOTE:STATE <arg>
		if((strncmp(p_s_NextParsePoint,REMOTE,LENGTH_OF_REMOTE) == MATCH_FOUND) || (strncmp(p_s_NextParsePoint,REM,LENGTH_OF_REM) == MATCH_FOUND))
		{
			//REMOTE[:STATE]?
			// Get the next node

			p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);
			OptionalHeader = strtok(NULL,JUNK_CHARACTER);

			// Optional Header contains STATE or STAT

			if((strcmp(OptionalHeader,STATE) == MATCH_FOUND) || (strcmp(OptionalHeader,STAT) == MATCH_FOUND))
			{
				REMOTE_SETTING_PARAMETER_DETECTED:;		// goto label
				ei_vUartSetPsuControl(Parameters);
				return SUCCESS;
			}

			if((strcmp(OptionalHeader,QR_STATE_WITH_CR) == MATCH_FOUND) || (strcmp(OptionalHeader,QR_STAT_WITH_CR) == MATCH_FOUND))
			{
				REMOTE_QUERY_DETECTED:;					// goto label
				ei_vRespondToReceivedMessage(REPLY_WITH_STATE_OF_PSU_CONTROL);
				return SUCCESS;
			}

			ei_vEnqueueCommandErrorInErrorQueue(EM100);
			return FAILURE;
		}
	}
	else
	{
		ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
		return FAILURE;
	}

	// Checking for PON:TIME?
	// This Too Works without calibration
	p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);
	if(strcmp((const Int8 *)p_s_NextParsePoint,PON) == MATCH_FOUND)
	{
		// Match found for this node.
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);	// Next Token
		if(strcmp((const Int8 *)p_s_NextParsePoint,QR_TIME_WITH_CR) == MATCH_FOUND)
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_POWER_ON_TIME);
			return SUCCESS;
		}
	}

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;

}

//#####################	Status Subsytem ###########################################################################
// Function Name: ei_uiMatchStatusCommands
// Return Type: Uint16
// Arguments:   void
// Description: Match and service status subsystem.
//
//				System subsystem works only when
//				1. The PSU is currently addressed or coupled
//
// This subsytem will be parsed even if PSU calibration not done. Because this subsytem enables user to
// query fault and operation conditional registers.

// COMMANDS COMPULSORILY SEPERATED BY MORE THAN : MUST BE CHECKED LAST BECAUSE STRTOK WILL MODIFY THE PARSING STRING
//#################################################################################################################
static Uint16 ei_uiMatchStatusCommands()
{
	Int8 *p_s_NextParsePoint;
	Int8 *Parameters;

	// STATUS SUBSYTEM has been detected by MessageParser function.
	// Now we Close in on the Parameter
	p_s_NextParsePoint = strtok(NULL,SPACE);	// Everything before Space
	Parameters = strtok(NULL,JUNK_CHARACTER);	// Everything after space

	// This Subsystem has no commands at the First level

	// We move to the next level
	// Commands seperated from Voltage level by ::
	p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);

	// OPERation:
	if((strcmp(p_s_NextParsePoint,OPERATION) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,OPER) == MATCH_FOUND))
	{
		// Extract everything after OPERation:
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

		if((strcmp(p_s_NextParsePoint,ENABLE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,ENAB) == MATCH_FOUND))
		{
			ei_vUartSetEnableRegisters(OPERATION_CONDITION_ENABLE_REGS, Parameters);
			return SUCCESS;
		}

		if((strcmp(p_s_NextParsePoint,QR_ENABLE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_ENAB_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_OPERATION_CONDITION_ENABLE_REGISTER);
			return SUCCESS;
		}

		if((strcmp(p_s_NextParsePoint,QR_EVENT_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_EVEN_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_OPERATION_CONDITION_EVENT_REGISTER);
			return SUCCESS;
		}

		if((strcmp(p_s_NextParsePoint,QR_CONDITION_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_COND_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_OPERATION_CONDITION_REGISTER);
			return SUCCESS;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// QUEStionable:
	if((strcmp(p_s_NextParsePoint,QUESTIONABLE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QUES) == MATCH_FOUND))
	{
		// Extract everything after OPERation:
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

		if((strcmp(p_s_NextParsePoint,ENABLE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,ENAB) == MATCH_FOUND))
		{
			ei_vUartSetEnableRegisters(QUESTIONABLE_CONDITION_ENABLE_REGS, Parameters);
			return SUCCESS;
		}

		if((strcmp(p_s_NextParsePoint,QR_ENABLE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_ENAB_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_QUESTIONABLE_CONDITION_ENABLE_REGISTER);
			return SUCCESS;
		}

		if((strcmp(p_s_NextParsePoint,QR_EVENT_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_EVEN_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_QUESTIONABLE_CONDITION_EVENT_REGISTER);
			return SUCCESS;
		}

		if((strcmp(p_s_NextParsePoint,QR_CONDITION_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_COND_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_QUESTIONABLE_CONDITION_REGISTER);
			return SUCCESS;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Display Subsytem ###########################################################################
// Function Name: ei_uiMatchDisplayCommands
// Return Type: Uint16
// Arguments:   void
// Description: Match and service Display subsystem.
//
//				Measure subsystem works only when
//				1. The PSU is currently addressed or coupled
//
//	This subsytem will be parsed even if PSU calibration not done. Because this subsytem enables user to
// control the display.

	// DISPLAY:STATE OFF
// Display State OFF turns off voltage and current display, but keeps all LEDs active.
// This is kind of a save the power mode. Also PSU must be Local lockout, so that the settings
// cannot be changed when Displays are OFF.
// User can bring from local lockout to local from communication and violate the PSU in this mode.
// No protection includede for that.

	// DISPLAY:FLASH
// Will perform resetting of the Display ICs causing aflash effect.
// COMMANDS COMPULSORILY SEPERATED BY MORE THAN : MUST BE CHECKED LAST BECAUSE STRTOK WILL MODIFY THE PARSING STRING
//#################################################################################################################
static Uint16 ei_uiMatchDisplayCommands()
{
	Int8 *p_s_NextParsePoint;
	Int8 *Parameters;

	// DISPLAY SUBSYTEM has been detected by MessageParser function.
	// Now we Close in on the Parameter
	p_s_NextParsePoint = strtok(NULL,SPACE);	// Everything before Space
	Parameters = strtok(NULL,JUNK_CHARACTER);	// Everything after space

	// Commands sepearted from Status level by :
	// STATe:
	if((strcmp(p_s_NextParsePoint,STATE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,STAT) == MATCH_FOUND))
	{
		// Match Found at this node
		// All that remains is the parameter

		MATCH_BOOL_FOR_LOGIC_TRUE(Parameters)
		{
			// Disables all the digits of the front panel.
			// This puts the front panel in Local lockout.
			/*Uint16 I2C_Tx_Data_Array[1];
			I2C_Tx_Data_Array[0] = ENABLE_DIG0_TO_DIG3;
			ei_uiI2CWrite(MAX6958B_ADDRESS, SCAN_LIMIT_REGISTER_ADDRESS, 1, I2C_Tx_Data_Array);
			ei_uiI2CWrite(MAX6959A_ADDRESS, SCAN_LIMIT_REGISTER_ADDRESS, 1, I2C_Tx_Data_Array);*/
			Global_Flags.fg_State_Of_Display = ON;

			return SUCCESS;
		}

		MATCH_BOOL_FOR_LOGIC_FALSE(Parameters)
		{
			// Disables all the digits of the front panel.
			// This puts the front panel in Local lockout.
			/*Uint16 I2C_Tx_Data_Array[1];
			I2C_Tx_Data_Array[0] = DISABLE_DIG0_TO_DIG3;
			ei_uiI2CWrite(MAX6958B_ADDRESS, SCAN_LIMIT_REGISTER_ADDRESS, 1, I2C_Tx_Data_Array);
			ei_uiI2CWrite(MAX6959A_ADDRESS, SCAN_LIMIT_REGISTER_ADDRESS, 1, I2C_Tx_Data_Array);*/
			State.Mode.bt_PSU_Control = LOCAL_LOCKOUT;
			Global_Flags.fg_State_Of_Display = OFF;

			return SUCCESS;
		}

		// If we come here then it's an illegal parameter
		ei_vEnqueueCommandErrorInErrorQueue(EM101);
		return FAILURE;
	}

	// STATe?
	if((strcmp(p_s_NextParsePoint,QR_STATE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_STAT_WITH_CR) == MATCH_FOUND))
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_STATE_OF_DISPLAY);
		return SUCCESS;
	}

	// RESet\r
	if((strcmp(p_s_NextParsePoint,RESET_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,RES_WITH_CR) == MATCH_FOUND))
	{
		// This will flash the Display Once
		// Reinitialising the ICs should cause a Flashing Effect.
		Fault_Regs.FAULT_REGISTER.bit.bt_Internal_Communication_Fault = 0;	// Reset Fault
	    ei_vMax625_X_Y_I2C_initialization(MAX6959A_ADDRESS);
	    ei_vMax625_X_Y_I2C_initialization(MAX6958B_ADDRESS);
		return SUCCESS;
	}

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Measure Subsytem ###########################################################################
// Function Name: ei_uiMatchMeasureCommands
// Return Type: Uint16
// Arguments:   void
// Description: Match and service Measure subsystem.
//
//				Measure subsystem works only when
//				1. The PSU is currently addressed or coupled
//				2. Calibration done.
//
//	No Parameters for this subsystem.

// COMMANDS COMPULSORILY SEPERATED BY MORE THAN : MUST BE CHECKED LAST BECAUSE STRTOK WILL MODIFY THE PARSING STRING
//#################################################################################################################
static Uint16 ei_uiMatchMeasureCommands()
{
	Int8 *p_s_NextParsePoint;
	Int8 *OptionalHeader;

	// MEASURE SUBSYTEM has been detected by MessageParser function.
	p_s_NextParsePoint = strtok(NULL,SPACE);

	// CURRENT?
	if((strcmp(p_s_NextParsePoint,QR_CURRENT_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_CURR_WITH_CR) == MATCH_FOUND))
	{
		goto QUERY_CURRENT_DETECTED;
	}

	// TEMPerature?
	if((strcmp(p_s_NextParsePoint,QR_TEMP_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_TEMPERATURE_WITH_CR) == MATCH_FOUND))
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_MEASURED_TEMPERATURE);
		return SUCCESS;
	}

	// Optional Command Check for CURRENT:DC?
	if((strncmp(p_s_NextParsePoint,CURRENT,LENGTH_OF_CURRENT) == MATCH_FOUND) || (strncmp(p_s_NextParsePoint,CURR,LENGTH_OF_CURR) == MATCH_FOUND))
	{
		//CURRENT:DC?
		// Get the next node

		p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);
		OptionalHeader = strtok(NULL,JUNK_CHARACTER);

		// Optional Header contains DC?
		if(strcmp(OptionalHeader,QR_DC_WITH_CR) == MATCH_FOUND)
		{
			QUERY_CURRENT_DETECTED:;
			ei_vRespondToReceivedMessage(REPLY_WITH_MEASURED_DC_CURRENT);
			return SUCCESS;
		}
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// VOLTAGE?
	if((strcmp(p_s_NextParsePoint,QR_VOLTAGE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_VOLT_WITH_CR) == MATCH_FOUND))
	{
		goto QUERY_VOLTAGE_DETECTED;
	}

	// Optional Command Check for VOLTAGE:DC?
	if((strncmp(p_s_NextParsePoint,VOLTAGE,LENGTH_OF_VOLTAGE) == MATCH_FOUND) || (strncmp(p_s_NextParsePoint,VOLT,LENGTH_OF_VOLT) == MATCH_FOUND))
	{
		//VOLTAGE:DC?
		// Get the next node

		p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);
		OptionalHeader = strtok(NULL,JUNK_CHARACTER);

	// Optional Header contains DC?
		if(strcmp(OptionalHeader,QR_DC_WITH_CR) == MATCH_FOUND)
		{
			QUERY_VOLTAGE_DETECTED:;
			ei_vRespondToReceivedMessage(REPLY_WITH_MEASURED_DC_VOLTAGE);
			return SUCCESS;
		}
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// POWER?
	if((strcmp(p_s_NextParsePoint,QR_POWER_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_POW_WITH_CR) == MATCH_FOUND))
	{
		goto QUERY_POWER_DETECTED;
	}

	// Optional Command Check for POWER:DC?
	if((strncmp(p_s_NextParsePoint,POWER,LENGTH_OF_POWER) == MATCH_FOUND) || (strncmp(p_s_NextParsePoint,POW,LENGTH_OF_POW) == MATCH_FOUND))
	{
		//POWER:DC?
		// Get the next node

		p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);
		OptionalHeader = strtok(NULL,JUNK_CHARACTER);

		// Optional Header contains AMPLITUDE or AMPL

		if(strcmp(OptionalHeader,QR_DC_WITH_CR) == MATCH_FOUND)
		{
			QUERY_POWER_DETECTED:;
			ei_vRespondToReceivedMessage(REPLY_WITH_MEASURED_DC_POWER);
			return SUCCESS;
		}
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Special Processing #########################################################################
// Function Name: ei_uiCommandMatchingForTopLevelCommands
// Return Type: Uint16
// Arguments:   Int8 *Command, Uint16 SubSystemType
// Description: This function processes those commands that will be too difficult to process in their respective subsystems.
//				It is used for those type of commands that do not have a : at all.

//				All subsystems here work only when
//				1. The PSU is currently addressed or coupled
//				2. Calibration done.
//
//				eg: VOLTage <Nrf+>, VOLTage?, VOLTage?MAX
//					Same for current.
//	The first argument is the complete command string. The second argument is SubSystemType that the command falls
//  in.
//#################################################################################################################
static Uint16 ei_uiCommandMatchingForTopLevelCommands(Int8 *s_Command, Uint16 ui_SubSystemType)
{
	// Seperate into arguments and commands
	Int8 *p_s_NextParsePoint;
	Int8 *Parameters;

	// Now we Close in on the Parameter
	p_s_NextParsePoint = strtok(s_Command,SPACE);	// Everything before Space
	Parameters = strtok(NULL,JUNK_CHARACTER);	// Everything after space

	// FOR VOLTAGE
	if(ui_SubSystemType == V_PROCESSING)
	{
		//VOLTage <arg>
		if((strcmp(p_s_NextParsePoint,VOLTAGE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,VOLT) == MATCH_FOUND))
		{
			ei_vUartProgramVoltage(Parameters);
			return SUCCESS;
		}

		//VOLTage?
		if((strcmp(p_s_NextParsePoint,QR_VOLTAGE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_VOLT_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_PROGRAMMED_VOLTAGE);
			return SUCCESS;
		}

		// VOLTage?MIN
		if((strcmp(p_s_NextParsePoint,QR_VOLTAGE_MIN_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_VOLT_MIN_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_MIN_PROGRAMMABLE_VOLTAGE);
			return SUCCESS;
		}

		// VOLTage?MAX
		if((strcmp(p_s_NextParsePoint,QR_VOLTAGE_MAX_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_VOLT_MAX_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_MAX_PROGRAMMABLE_VOLTAGE);
			return SUCCESS;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// FOR CURRENT
	if(ui_SubSystemType == C_PROCESSING)
	{
		//CURRent <arg>
		if((strcmp(p_s_NextParsePoint,CURRENT) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,CURR) == MATCH_FOUND))
		{
			ei_vUartProgramCurrent(Parameters);
			return SUCCESS;
		}

		//CURRent?
		if((strcmp(p_s_NextParsePoint,QR_CURRENT_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_CURR_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_PROGRAMMED_CURRENT);
			return SUCCESS;
		}

		// CURRent?MIN
		if((strcmp(p_s_NextParsePoint,QR_CURRENT_MIN_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_CURR_MIN_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_MIN_PROGRAMMABLE_CURRENT);
			return SUCCESS;
		}

		// CURRent?MAX
		if((strcmp(p_s_NextParsePoint,QR_CURRENT_MAX_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_CURR_MAX_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_MAX_PROGRAMMABLE_CURRENT);
			return SUCCESS;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	if(ui_SubSystemType == OUT_PROCESSING)
	{
		//OUTPut <arg>
		if((strcmp(p_s_NextParsePoint,OUTPUT) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,OUTP) == MATCH_FOUND))
		{
			MATCH_BOOL_FOR_LOGIC_TRUE(Parameters)
			{
				State.Mode.bt_Output_Status = ON;
				Global_Flags.fg_Output_Switched_On_From_Uart = TRUE;	// A flag indicating output is switched on from uart.
				return SUCCESS;
			}
			MATCH_BOOL_FOR_LOGIC_FALSE(Parameters)
			{
				State.Mode.bt_Output_Status = OFF;
				return SUCCESS;
			}

			// If we come here then it's an illegal parameter
			ei_vEnqueueCommandErrorInErrorQueue(EM101);
			return FAILURE;
		}

		//OUTPut?
		if((strcmp(p_s_NextParsePoint,QR_OUTPUT_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_OUTP_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_OUTPUT_STATE);
			return SUCCESS;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Voltage Subsytem ###########################################################################
// Function Name: ei_uiMatchVoltageCommands
// Return Type: Uint16
// Arguments:   void
// Description: Match and service Voltage subsystem.
//
//				Voltage subsystem works only when
//				1. The PSU is currently addressed or coupled
//				2. Calibration done.
//
//#################################################################################################################
static Uint16 ei_uiMatchVoltageCommands()
{
	Int8 *p_s_NextParsePoint;
	Int8 *Parameters;

	// VOLTAGE SUBSYTEM has been detected by Message Parser function.
	// Now we Close in on the Parameter
	p_s_NextParsePoint = strtok(NULL,SPACE);	// Everything before Space
	Parameters = strtok(NULL,JUNK_CHARACTER);		// Everything after space

	// Commands sepearted from Voltage level by :

							//LEVel <arg>
	if((strcmp(p_s_NextParsePoint,LEVEL) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,LEV) == MATCH_FOUND))
	{
		goto PROGRAM_VOLTAGE_COMMAND_FOUND;
	}
							//LEVel?
	if((strcmp(p_s_NextParsePoint,QR_LEVEL_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_LEV_WITH_CR) == MATCH_FOUND))
	{
		goto VOLTAGE_QUERY_FOUND;
	}

	// Others not implemented now at this level
	// MODE <CRD>
	// MODE?
	// TRIGgered <arg>
	// TRIGerred?
	// TRIGerred?MAX
	// TRIGerred?MIN

	// We move to the next level
	// Commands seperated from Voltage level by ::
	p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);

	// Valid parameter @ this level
	// LEVel:
	if((strcmp(p_s_NextParsePoint,LEVEL)==MATCH_FOUND) || (strcmp(p_s_NextParsePoint,LEV)==MATCH_FOUND))
	{
		// Extract everything after LEVel:
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

		// IMMediate <arg>
		if((strcmp(p_s_NextParsePoint,IMMEDIATE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,IMM) == MATCH_FOUND))
		{
			goto PROGRAM_VOLTAGE_COMMAND_FOUND;
		}

		// IMMediate?
		if((strcmp(p_s_NextParsePoint,QR_IMMEDIATE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_IMM_WITH_CR) == MATCH_FOUND))
		{
			goto VOLTAGE_QUERY_FOUND;
		}

		// Left Out @ this level
//		TRIGgered <arg>
//		TRIGgered?

		// We move to the next level
		// Commands seperated from Voltage by :::
		p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);

		//IMMediate:
		if((strcmp(p_s_NextParsePoint,IMMEDIATE)==MATCH_FOUND) || (strcmp(p_s_NextParsePoint,IMM)==MATCH_FOUND))
		{
			// Extract everything after LEVel:IMMediate:
			p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

//			AMPLitude <arg>
			// AMPLitude <arg>
			if((strcmp(p_s_NextParsePoint,AMPLITUDE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,AMPL) == MATCH_FOUND))
			{
				PROGRAM_VOLTAGE_COMMAND_FOUND:;
				ei_vUartProgramVoltage(Parameters);
				return SUCCESS;
			}

			// AMPLitude?
			if((strcmp(p_s_NextParsePoint,QR_AMPLITUDE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_AMPL_WITH_CR) == MATCH_FOUND))
			{
				VOLTAGE_QUERY_FOUND:;
				ei_vRespondToReceivedMessage(REPLY_WITH_PROGRAMMED_VOLTAGE);
				return SUCCESS;
			}

			// If we come here parsing has failed
			ei_vEnqueueCommandErrorInErrorQueue(EM100);
			return FAILURE;
		}

		// Left Out @ this Level
		// TRIGgered:

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// PROTection:
	if((strcmp(p_s_NextParsePoint,PROTECTION)==MATCH_FOUND) || (strcmp(p_s_NextParsePoint,PROT)==MATCH_FOUND))
	{
		// Extract everything after PROTection:
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

		// LEVel <arg>
		if((strcmp(p_s_NextParsePoint,LEVEL) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,LEV) == MATCH_FOUND))
		{
			ei_vUartProgramOVP(Parameters);
			return SUCCESS;
		}

		// LEVel?
		if((strcmp(p_s_NextParsePoint,QR_LEVEL_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_LEV_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_PROGRAMMED_OVP);
			return SUCCESS;
		}

		// LEVel?MAX
		if((strcmp(p_s_NextParsePoint,QR_LEVEL_MAX_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_LEV_MAX_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_MAX_PROGRAMMABLE_OVP);
			return SUCCESS;
		}

		// LEVel?MIN
		if((strcmp(p_s_NextParsePoint,QR_LEVEL_MIN_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_LEV_MIN_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_MIN_PROGRAMMABLE_OVP);
			return SUCCESS;
		}

		// LOW <arg>
		if(strcmp(p_s_NextParsePoint,LOW) == MATCH_FOUND)
		{
			ei_vUartProgramUVL(Parameters);
			return SUCCESS;
		}

		// LOW?
		if(strcmp(p_s_NextParsePoint,QR_LOW_WITH_CR) == MATCH_FOUND)
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_PROGRAMMED_UVL);
			return SUCCESS;
		}

		// LOW?MAX
		if(strcmp(p_s_NextParsePoint,QR_LOW_MAX_WITH_CR) == MATCH_FOUND)
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_MAX_PROGRAMMABLE_UVL);
			return SUCCESS;
		}

		// LOW?MIN
		if(strcmp(p_s_NextParsePoint,QR_LOW_MIN_WITH_CR) == MATCH_FOUND)
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_MIN_PROGRAMMABLE_UVL);
			return SUCCESS;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// Others not implemented now at this level
//	TRIGgered:

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Current Subsytem ###########################################################################
// Function Name: ei_uiMatchCurrentCommands
// Return Type: Uint16
// Arguments:   void
// Description: Match and service Current subsystem.
//
//				Current subsystem works only when
//				1. The PSU is currently addressed or coupled
//				2. Calibration done.
//
//#################################################################################################################
static Uint16 ei_uiMatchCurrentCommands()
{
	Int8 *p_s_NextParsePoint;
	Int8 *Parameters;

	// CURRENT SUBSYTEM has been detected by Message Parser function.
	// Now we Close in on the Parameter
	p_s_NextParsePoint = strtok(NULL,SPACE);	// Everything before Space
	Parameters = strtok(NULL,JUNK_CHARACTER);		// Everything after space

	// Commands sepearted from Current level by :

							//LEVel <arg>
	if((strcmp(p_s_NextParsePoint,LEVEL) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,LEV) == MATCH_FOUND))
	{
		goto PROGRAM_CURRENT_COMMAND_FOUND;
	}

	//LEVel?
	if((strcmp(p_s_NextParsePoint,QR_LEVEL_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_LEV_WITH_CR) == MATCH_FOUND))
	{
		goto CURRENT_QUERY_FOUND;
	}

	// Others not implemented now at this level
	// MODE <CRD>
	// MODE?
	// TRIGgered <arg>
	// TRIGerred?
	// TRIGerred?MAX
	// TRIGerred?MIN

	// We move to the next level
	// Commands sepearted from Current level by ::
	p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);

	// Valid parameter @ this level
	// LEVel:
	if((strcmp(p_s_NextParsePoint,LEVEL) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,LEV) == MATCH_FOUND))
	{
		// Extract everything after LEVel:
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

		// IMMediate <arg>
		if((strcmp(p_s_NextParsePoint,IMMEDIATE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,IMM) == MATCH_FOUND))
		{
			goto PROGRAM_CURRENT_COMMAND_FOUND;
		}

		// IMMediate?
		if((strcmp(p_s_NextParsePoint,QR_IMMEDIATE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_IMM_WITH_CR) == MATCH_FOUND))
		{
			goto CURRENT_QUERY_FOUND;
		}

		// Left Out @ this level
//		TRIGgered <arg>
//		TRIGgered?

		// We move to the next level
		// Commands sepearted from Current level by ::
		p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);

		//IMMediate:
		if((strcmp(p_s_NextParsePoint,IMMEDIATE)==MATCH_FOUND) || (strcmp(p_s_NextParsePoint,IMM)==MATCH_FOUND))
		{
			// Extract everything after LEVel:IMMediate:
			p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

//			AMPLitude <arg>
			// AMPLitude <arg>
			if((strcmp(p_s_NextParsePoint,AMPLITUDE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,AMPL) == MATCH_FOUND))
			{
				PROGRAM_CURRENT_COMMAND_FOUND:;
				ei_vUartProgramCurrent(Parameters);
				return SUCCESS;
			}

			// AMPLitude?
			if((strcmp(p_s_NextParsePoint,QR_AMPLITUDE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_AMPL_WITH_CR) == MATCH_FOUND))
			{
				CURRENT_QUERY_FOUND:;
				ei_vRespondToReceivedMessage(REPLY_WITH_PROGRAMMED_CURRENT);
				return SUCCESS;
			}

			// If we come here parsing has failed
			ei_vEnqueueCommandErrorInErrorQueue(EM100);
			return FAILURE;
		}

		// Left Out @ this Level
		// TRIGgered:

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// Others not implemented now at this level
//	TRIGgered:

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Output Subsytem ###########################################################################
// Function Name: ei_uiMatchOutputCommands
// Return Type: Uint16
// Arguments:   void
// Description: Match and service Output subsystem.
//
//				Output subsystem works only when
//				1. The PSU is currently addressed or coupled
//				2. Calibration done.
//
//#################################################################################################################
static Uint16 ei_uiMatchOutputCommands()
{
	Int8 *p_s_NextParsePoint;
	Int8 *Parameters;

	// OUTPUT SUBSYTEM has been detected by Message Parser function.
	// Now we Close in on the Parameter
	p_s_NextParsePoint = strtok(NULL,SPACE);	// Everything before Space
	Parameters = strtok(NULL,JUNK_CHARACTER);		// Everything after space

	// Commands sepearted from Output level by :

	//STATe <arg>
	if((strcmp(p_s_NextParsePoint,STATE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,STAT) == MATCH_FOUND))
	{
		MATCH_BOOL_FOR_LOGIC_TRUE(Parameters)
		{
			State.Mode.bt_Output_Status = ON;
			Global_Flags.fg_Output_Switched_On_From_Uart = TRUE;	// A flag indicating output is switched on from uart.
			return SUCCESS;
		}
		MATCH_BOOL_FOR_LOGIC_FALSE(Parameters)
		{
			State.Mode.bt_Output_Status = OFF;
			return SUCCESS;
		}

		// If we come here then it's an illegal parameter
		ei_vEnqueueCommandErrorInErrorQueue(EM101);
		return FAILURE;
	}

	//STATe?
	if((strcmp(p_s_NextParsePoint,QR_STATE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_STAT_WITH_CR) == MATCH_FOUND))
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_OUTPUT_STATE);
		return SUCCESS;
	}

	// PON <arg>
	if(strcmp(p_s_NextParsePoint,PON) == MATCH_FOUND)
	{
		goto POWER_ON_STATE_SET_COMMAND_DETECTED;
	}

	//PON?
	if(strcmp(p_s_NextParsePoint,QR_PON_WITH_CR) == MATCH_FOUND)
	{
		goto POWER_ON_STATE_QUERY_FOUND;
	}

	//MODE?
	if(strcmp(p_s_NextParsePoint,QR_MODE_WITH_CR) == MATCH_FOUND)
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_OUTPUT_MODE);
		return SUCCESS;
	}

	// We move to the next level
	// Commands sepearted from Current level by ::
	p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);

	// Valid parameter @ this level
	// PON:
	if(strcmp(p_s_NextParsePoint,PON) == MATCH_FOUND)
	{
		// Extract everything after LEVel:
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

		//STATe <arg>
		if((strcmp(p_s_NextParsePoint,STATE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,STAT) == MATCH_FOUND))
		{
			POWER_ON_STATE_SET_COMMAND_DETECTED:;
			MATCH_BOOL_FOR_LOGIC_TRUE(Parameters)
			{
				State.Mode.bt_Start_Mode = AUTO_RESTART;
				Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Auto_start_Enabled = SET;
				return SUCCESS;
			}
			MATCH_BOOL_FOR_LOGIC_FALSE(Parameters)
			{
				State.Mode.bt_Start_Mode = SAFE_START;
				Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Auto_start_Enabled = CLEARED;
				return SUCCESS;
			}

			// If we come here then it's an illegal parameter
			ei_vEnqueueCommandErrorInErrorQueue(EM101);
			return FAILURE;
		}

		//STATe?
		if((strcmp(p_s_NextParsePoint,QR_STATE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_STAT_WITH_CR) == MATCH_FOUND))
		{
			POWER_ON_STATE_QUERY_FOUND:;
			ei_vRespondToReceivedMessage(REPLY_WITH_PON_STATE);
			return SUCCESS;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	//PROTection:
	if((strcmp(p_s_NextParsePoint,PROTECTION)==MATCH_FOUND) || (strcmp(p_s_NextParsePoint,PROT)==MATCH_FOUND))
	{
		// Extract everything after PROTection:
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

		//CLEar
		if((strcmp(p_s_NextParsePoint,CLEAR_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,CLE_WITH_CR) == MATCH_FOUND))
		{
			State.Mode.bt_Output_Status = OFF;
			return SUCCESS;
		}

		//DELay
		if((strcmp(p_s_NextParsePoint,DELAY) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,DEL) == MATCH_FOUND))
		{
			ei_vUartSetProtectionDelay(Parameters);
			return SUCCESS;
		}

		//DELay?
		if((strcmp(p_s_NextParsePoint,QR_DELAY_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_DEL_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_SET_PROTECTION_DELAY);
			return SUCCESS;
		}

		//FOLDback <arg>
		if((strcmp(p_s_NextParsePoint,FOLDBACK) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,FOLD) == MATCH_FOUND))
		{
			goto FOLDBACK_SET_COMMAND_FOUND;
		}

		//FOLDback?
		if((strcmp(p_s_NextParsePoint,QR_FOLDBACK_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_FOLD_WITH_CR) == MATCH_FOUND))
		{
			goto FOLDBACK_STATE_QUERY_FOUND;
		}

		// We move to the next level
		// Commands sepearted from Current level by :::
		p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);

		//FOLDback:
		if((strcmp(p_s_NextParsePoint,FOLDBACK) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,FOLD) == MATCH_FOUND))
		{
			// Extract everything after FOLDback:
			p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

			//MODE <arg>
			if(strcmp(p_s_NextParsePoint,MODE) == MATCH_FOUND)
			{
				FOLDBACK_SET_COMMAND_FOUND:;
				if((strcmp(Parameters,ZERO_WITH_CR) == MATCH_FOUND) || (strcmp(Parameters,OFF_WITH_CR) == MATCH_FOUND))
				{
					State.Mode.bt_FoldBack_Mode = FOLDBACK_DISARMED;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Fold_Enabled = CLEARED;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Current_Foldback_Mode = CLEARED;
					return SUCCESS;
				}
				if((strcmp(Parameters,ONE_WITH_CR) == MATCH_FOUND) || (strcmp(Parameters,CC_WITH_CR) == MATCH_FOUND))
				{
					State.Mode.bt_FoldBack_Mode = FOLDBACK_ARMED_TO_PROTECT_FROM_CC;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Fold_Enabled = SET;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Current_Foldback_Mode = SET;
					return SUCCESS;
				}

				// If we come here then Illegal Parameters
				ei_vEnqueueCommandErrorInErrorQueue(EM101);
				return FAILURE;
			}

			//MODE?
			if(strcmp(p_s_NextParsePoint,QR_MODE_WITH_CR) == MATCH_FOUND)
			{
				FOLDBACK_STATE_QUERY_FOUND:;
				ei_vRespondToReceivedMessage(REPLY_WITH_SET_FOLDBACK_MODE);
				return SUCCESS;
			}

			// If we come here parsing has failed
			ei_vEnqueueCommandErrorInErrorQueue(EM100);
			return FAILURE;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// Others not implemented now at this level
	// ILC
	// TTLTrg
	// RELAY1(2)

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Calibration commands ######################################################################
// Function Name: ei_uiMatchCalibrationCommands
// Return Type: Uint16
// Arguments:   void
// Description: Match and service the calibration commands.
//				Important..
//				1. These commands will get corrupted if they PSU's are connected in series/parallel
//				2. Calibration should always be done individually for each PSU while unconnected.
//				3. They will work only when the PSU open for communication
//				4. The commands here will work only when the Psu is in FACTORY_DEFAULT.
//				5. The Queries here will work always.
//				6. The Command RESet will work only when PSU is ONE_TIME_CALIBRATED
//#################################################################################################################
static Uint16 ei_uiMatchCalibrationCommands()
{
	Int8 *p_s_NextParsePoint;
	Int8 *Parameters;

	// CALIBRATE SUBSYTEM has been detected by Message Parser function.
	// Now we Close in on the Parameter
	p_s_NextParsePoint = strtok(NULL,SPACE);	// Everything before Space
	Parameters = strtok(NULL,JUNK_CHARACTER);		// Everything after space

	// Commands sepearted from Output level by :

	//DATE <arg>
	if(strcmp(p_s_NextParsePoint,DATE) == MATCH_FOUND)
	{
		// Allowed only if PSU not calibrated
		if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
		{
			if(strlen(Parameters) != (DATE_LENGTH + 1))	// Including '\r'
			{
				// If we come here then Too much data.
				ei_vEnqueueExecutionErrorInErrorQueue(EM223);
				return FAILURE;
			}
			else
			{
				// Writing only DATE_LENGTH charcters in EEPROM removes '\r'
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, DATE_START_ADDRESS, DATE_LENGTH, (Uint16 *)Parameters);	// Excluding '\r'
				RESET_EEPROM_WRITE_CYCLE_TIMER;
				Global_Flags.fg_Date_Entered = TRUE;
				return SUCCESS;
			}
		}
		else
		{
			// Settings conflict
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
			return FAILURE;
		}
	}

	//IDN <arg>
	if(strcmp(p_s_NextParsePoint,IDN) == MATCH_FOUND)
	{
		// Allowed only if PSU not calibrated
		if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
		{
			// First Check wheteher "ENARKA,VBV" has been received
			if(strncmp(Parameters,ENARKA_VBV,LENGTH_OF_ENARKA_AUM) == MATCH_FOUND)
			{
				// Start measuring strlen of paramter after enarka,aum
				if(strlen(&Parameters[10]) >= (MAX_IDN_LENGTH + 1))	// Including '\r'
				{
					// If we come here then Too much data.
					ei_vEnqueueExecutionErrorInErrorQueue(EM223);
					return FAILURE;
				}
				else
				{
					// From parameters remove '\r. It will be located at [strlen - 1] offset
					Parameters[strlen(Parameters) - 1] = '\0';

					// Now write from the first character after 'M' in "ENARKA,AUM" to NULL in EEPROM. (NULL is also written).
					// That is why we pass strlen(&Parameters[10])+1 as the 3rd paramter to write function.
					while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
					ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, IDN_ADDRESS, strlen(&Parameters[10])+1, (Uint16 *)&Parameters[10]);	// Excluding '\r'
					RESET_EEPROM_WRITE_CYCLE_TIMER;
					return SUCCESS;
				}
			}
			else	// "ENARKA,AUM" not detected. Illegal Parameter
			{
				// If we come here then Illegal Parameters
				ei_vEnqueueCommandErrorInErrorQueue(EM101);
				return FAILURE;
			}
		}
		else
		{
			// Settings conflict
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
			return FAILURE;
		}
	}

	//OPT <arg>
	if(strcmp(p_s_NextParsePoint,OPT) == MATCH_FOUND)
	{
		// Allowed only if PSU not calibrated
		if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
		{
			if(strcmp(Parameters,ZERO_WITH_CR) == MATCH_FOUND)
			{
				Uint16 a_ui_I2cTxDataArray[1];
				a_ui_I2cTxDataArray[0] = 0;
				// Writing only DATE_LENGTH charcters in EEPROM removes '\r'
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, OPTION_INSTALLED_DETAILS_ADDRESS, 1, a_ui_I2cTxDataArray);	// Excluding '\r'
				RESET_EEPROM_WRITE_CYCLE_TIMER;
				return SUCCESS;
			}
			// If we come here then Illegal Parameters
			ei_vEnqueueCommandErrorInErrorQueue(EM101);
			return FAILURE;
		}
		else
		{
			// Settings conflict
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
			return FAILURE;
		}
	}
	// RESet
	if((strcmp(p_s_NextParsePoint,RESET_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,RES_WITH_CR) == MATCH_FOUND))
	{
		// You can reset the calibration only when PSU CALIBRATED
		if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)
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
			return SUCCESS;
		}
		else
		{
			// Settings conflict
			ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
			return FAILURE;
		}
	}

	// COUNt?
	if((strcmp(p_s_NextParsePoint,QR_COUNT_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_COUN_WITH_CR) == MATCH_FOUND))
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_CALIBRATION_COUNT);
		return SUCCESS;
	}

	// MODel?
	if((strcmp(p_s_NextParsePoint,QR_MODEL_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_MOD_WITH_CR) == MATCH_FOUND))
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_PSU_MODEL_NUMBER);
		return SUCCESS;
	}

	// WIPE			[For testing team and enArka Only. Should not be used by customer.]
	// Comment this code when final product shipped
	if(strcmp(p_s_NextParsePoint,WIPE_WITH_CR) == MATCH_FOUND)
	{
		// This command completely wipes the following Pages of EEPROM (writes of 0xFF)
		// Page 1:0x40-0x007F
		// Page 2:0x80-0x00BF
		// Page 3:0xC0-0x00FF
		// THIS FUNCTION WILL CAUSE UPTO 15ms DELAY

		Uint16 a_ui_I2cTxDataArray[64],i;
		for(i = 0; i < 64; i++)
		{
			a_ui_I2cTxDataArray[i] = 0xFF;
		}

		// Erase Page 1
		while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
		ei_uiI2CWrite(EEPROM_24LC256_ADDRESS,0x0040,64,a_ui_I2cTxDataArray);
		RESET_EEPROM_WRITE_CYCLE_TIMER;

		// Erase Page 2
		while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
		ei_uiI2CWrite(EEPROM_24LC256_ADDRESS,0x0080,64,a_ui_I2cTxDataArray);
		RESET_EEPROM_WRITE_CYCLE_TIMER;

		// Erase Page 3
		while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
		ei_uiI2CWrite(EEPROM_24LC256_ADDRESS,0x00C0,64,a_ui_I2cTxDataArray);
		RESET_EEPROM_WRITE_CYCLE_TIMER;

		// Reset PSU to safe state and put it in uncalibrated mode.
		ei_vResetPsuToSafeState();
		Product_Info.ui_Calibration_Status = FACTORY_DEFAULT;
		Product_Info.ui_Model_Number = DEFAULT_1V_1A_MODEL;

		return SUCCESS;
	}

	// We move to the next level
	// Commands sepearted from Current level by ::
	p_s_NextParsePoint = strtok(p_s_NextParsePoint,COLON);

	// Valid parameter @ this level
//	VOLTage:
	if((strcmp(p_s_NextParsePoint,VOLTAGE) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,VOLT) == MATCH_FOUND))
	{
		// Extract everything after VOLTage:
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

		// RATed <arg>
		if((strcmp(p_s_NextParsePoint,RATED) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,RAT) == MATCH_FOUND))
		{
			// Allowed only if PSU not calibrated
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				ei_vUartSetRated_SpecifiedParams(Parameters,RATED_VOLTAGE);
				return SUCCESS;
			}
			else
			{
				// Settings conflict
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// RATed?
		if((strcmp(p_s_NextParsePoint,QR_RATED_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_RAT_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_RATED_VOLTAGE);
			return SUCCESS;
		}

		// SPECIFIED <arg>
		if((strcmp(p_s_NextParsePoint,SPECIFIED) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,SPEC) == MATCH_FOUND))
		{
			// Allowed only if PSU not calibrated
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				ei_vUartSetRated_SpecifiedParams(Parameters,SPECIFIED_VOLTAGE);
				return SUCCESS;
			}
			else
			{
				// Settings conflict
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// SPECified?
		if((strcmp(p_s_NextParsePoint,QR_SPECIFIED_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_SPEC_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_SPECIFIED_VOLTAGE);
			return SUCCESS;
		}

		// LOW\r
		if(strcmp(p_s_NextParsePoint,LOW_WITH_CR) == MATCH_FOUND)
		{
			// Allowed only if PSU not calibrated
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				if((Global_Flags.fg_Rated_Volt_Entered & Global_Flags.fg_Specified_Volt_Entered & Global_Flags.fg_Rated_Curr_Entered & Global_Flags.fg_Specified_Curr_Entered) == TRUE)
				{
					// Enter Only If these parameters are already enetered
					ei_vSetVoltageCurrentForCalib(VOLTAGE_LOW);
				}
				return SUCCESS;
			}
			else
			{
				// Settings conflict
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// HIGH\r
		if(strcmp(p_s_NextParsePoint,HIGH_WITH_CR) == MATCH_FOUND)
		{
			// Allowed only if PSU not calibrated
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				if((Global_Flags.fg_Rated_Volt_Entered & Global_Flags.fg_Specified_Volt_Entered & Global_Flags.fg_Rated_Curr_Entered & Global_Flags.fg_Specified_Curr_Entered) == TRUE)
				{
					// Enter Only If these parameters are already enetered
					ei_vSetVoltageCurrentForCalib(VOLTAGE_FULL);
				}
				return SUCCESS;
			}
			else
			{
				// Settings conflict
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// MEASured <arg>
		if((strcmp(p_s_NextParsePoint,MEASURED) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,MEAS) == MATCH_FOUND))
		{
			// Allowed only if PSU not calibrated
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				ei_vUartVoltageCalibrationProcess(Parameters);
				return SUCCESS;
			}
			else
			{
				// Settings conflict
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// VALue?
		if((strcmp(p_s_NextParsePoint,QR_VALUE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_VAL_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_VOLTAGE_CALIBRATION_PARAMETERS);
			return SUCCESS;
		}

		// DISPlay?
		if((strcmp(p_s_NextParsePoint,QR_DISPLAY_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_DISP_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_VOLTAGE_DISPLAY_CALIBRATION_PARAMETERS);
			return SUCCESS;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	//	CURRent:
	if((strcmp(p_s_NextParsePoint,CURRENT) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,CURR) == MATCH_FOUND))
	{
		// Extract everything after CURRent:
		p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

		// RATed <arg>
		if((strcmp(p_s_NextParsePoint,RATED) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,RAT) == MATCH_FOUND))
		{
			// Allowed only if PSU not calibrated
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				ei_vUartSetRated_SpecifiedParams(Parameters,RATED_CURRENT);
				return SUCCESS;
			}
			else
			{
				// Settings conflict
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// RATed?
		if((strcmp(p_s_NextParsePoint,QR_RATED_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_RAT_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_RATED_CURRENT);
			return SUCCESS;
		}

		// SPECIFIED <arg>
		if((strcmp(p_s_NextParsePoint,SPECIFIED) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,SPEC) == MATCH_FOUND))
		{
			// Allowed only if PSU not calibrated
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				ei_vUartSetRated_SpecifiedParams(Parameters,SPECIFIED_CURRENT);
				return SUCCESS;
			}
			else
			{
				// Settings conflict
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// SPECified?
		if((strcmp(p_s_NextParsePoint,QR_SPECIFIED_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_SPEC_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_SPECIFIED_CURRENT);
			return SUCCESS;
		}

		// LOW\r
		if(strcmp(p_s_NextParsePoint,LOW_WITH_CR) == MATCH_FOUND)
		{
			// Allowed only if PSU not calibrated
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				if((Global_Flags.fg_Rated_Volt_Entered & Global_Flags.fg_Specified_Volt_Entered & Global_Flags.fg_Rated_Curr_Entered & Global_Flags.fg_Specified_Curr_Entered) == TRUE)
				{
					// Enter Only If these parameters are already enetered
					ei_vSetVoltageCurrentForCalib(CURRENT_LOW);
				}
				return SUCCESS;
			}
			else
			{
				// Settings conflict
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// HIGH\r
		if(strcmp(p_s_NextParsePoint,HIGH_WITH_CR) == MATCH_FOUND)
		{
			// Allowed only if PSU not calibrated
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				if((Global_Flags.fg_Rated_Volt_Entered & Global_Flags.fg_Specified_Volt_Entered & Global_Flags.fg_Rated_Curr_Entered & Global_Flags.fg_Specified_Curr_Entered) == TRUE)
				{
					// Enter Only If these parameters are already enetered
					ei_vSetVoltageCurrentForCalib(CURRENT_FULL);
				}
				return SUCCESS;
			}
			else
			{
				// Settings conflict
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// MEASured <arg>
		if((strcmp(p_s_NextParsePoint,MEASURED) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,MEAS) == MATCH_FOUND))
		{
			// Allowed only if PSU not calibrated
			if(Product_Info.ui_Calibration_Status == FACTORY_DEFAULT)
			{
				ei_vUartCurrentCalibrationProcess(Parameters);
				return SUCCESS;
			}
			else
			{
				// Settings conflict
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
				return FAILURE;
			}
		}

		// VALue?
		if((strcmp(p_s_NextParsePoint,QR_VALUE_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_VAL_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_CURRENT_CALIBRATION_PARAMETERS);
			return SUCCESS;
		}

		// DISPlay?
		if((strcmp(p_s_NextParsePoint,QR_DISPLAY_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_DISP_WITH_CR) == MATCH_FOUND))
		{
			ei_vRespondToReceivedMessage(REPLY_WITH_CURRENT_DISPLAY_CALIBRATION_PARAMETERS);
			return SUCCESS;
		}

		// If we come here parsing has failed
		ei_vEnqueueCommandErrorInErrorQueue(EM100);
		return FAILURE;
	}

	// If we come here parsing has failed
	ei_vEnqueueCommandErrorInErrorQueue(EM100);
	return FAILURE;
}

//#####################	Boot Command ##############################################################################
// Function Name: ei_uiMatchBootCommand
// Return Type: void
// Arguments:   void
// Description: Checks for the BOOT Key to determine which processor to update firmware.
//#################################################################################################################
static Uint16 ei_uiMatchBootCommand()
{
	Int8 *p_s_NextParsePoint;
	Uint16 a_ui_I2cTxDataArray[1];

	// BOOT SUBSYTEM has been detected by MessageParser function.
	p_s_NextParsePoint = strtok(NULL,JUNK_CHARACTER);

	// Check for the boot Key
	if(strcmp(p_s_NextParsePoint,BOOT_KEY) == MATCH_FOUND)
	{
		// Correct Boot Key
		// Go the the boot mode

		// Disable Interrupts as the first step
		DINT;

		// Write the boot command received in EEPROM memory
		a_ui_I2cTxDataArray[0] = BOOT_COMMAND_RECEIVED;
		ei_uiI2CWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_STATUS_ADDRESS,1,a_ui_I2cTxDataArray);
		EEPROM_WRITE_CYCLE_DELAY;

		// Call the Boot Function
		ei_vDspBootMode();
	}

	// CHK?
	if(strcmp(p_s_NextParsePoint,QR_CHK_WITH_CR) == MATCH_FOUND)
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_FLASH_CHECKSUM_STORED_IN_EEPROM);
		return SUCCESS;
	}

	// STATus?
	if((strcmp(p_s_NextParsePoint,QR_STATUS_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_STAT_WITH_CR) == MATCH_FOUND))
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_FLASH_PROGRAMMING_STATUS);
		return SUCCESS;
	}

	// FAIL?
	if(strcmp(p_s_NextParsePoint,QR_FAIL_WITH_CR) == MATCH_FOUND)
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_FLASH_FAILURE_STATUS_STORED_IN_EEPROM);
		return SUCCESS;
	}

	// MISREAD?
	if((strcmp(p_s_NextParsePoint,QR_MISREAD_WITH_CR) == MATCH_FOUND) || (strcmp(p_s_NextParsePoint,QR_MISR_WITH_CR) == MATCH_FOUND))
	{
		ei_vRespondToReceivedMessage(REPLY_WITH_FLASH_ILLEGAL_READ_VALUE_STORED_IN_EEPROM);
		return SUCCESS;
	}

	// Boot commands don't match. Just return
	return FAILURE;
}

//#####################	Open/Close Comm. ##########################################################################
// Function Name: OpenCloseUartCommunication
// Return Type: void
// Arguments:   void
// Description: Open or Close communication with the PSU by checking for INSTrument command. In case of an erreneous
//				parameter in the INSTRument command, the state of communication does not change. (i.e The communication
//				line with the currently open line does not close).
//#################################################################################################################
static void ei_vOpenCloseUartCommunication(Int8* Params)
{
	Uint16 ui_ParameterStatus;
	ui_ParameterStatus = 0;

	// 1. Run the Parameter through the error checker which returns either VALID_PARAMTER or INVALID_PARAMTER or MAX_PARAMTER or MIN_PARAMTER.
	ui_ParameterStatus = ei_uiUartParameterErrorChecker(Params,NR1);

	if(ui_ParameterStatus != VALID_PARAMETER_NOT_FOUND)
	{
		int32 l_SentAddress;
		Int8 *EndPtr;

		l_SentAddress = strtol(Params,&EndPtr,DECIMAL);

		//	Check if all charcters up to '\r' have been used up. That means end ptr must point to '\r'
		if(*EndPtr != '\r')
		{
			// Illegal Parameter
			ei_vEnqueueCommandErrorInErrorQueue(EM101);
			return;
		}

		// Now check for Out of Range errors using LONG MAX
		if((l_SentAddress > MAX_ALLOWED_ADDR) || (l_SentAddress < MIN_ALLOWED_ADDR))
		{
			ei_vEnqueueCommandErrorInErrorQueue(EM222);
			return;
		}

		if(l_SentAddress == Reference.ui_Set_Address)	// Address matches. Open communication
		{
			Global_Flags.fg_Uart_Comm_with_this_PSU = OPEN;
		}
		else	// Address does not match. Close communication
		{
			Global_Flags.fg_Uart_Comm_with_this_PSU = CLOSED;
		}
	}
	// In case of an Invalid parameter do nothing
	return;
}

//#####################	Set Registers from Uart ###################################################################
// Function Name: ei_vUartSetEnableRegisters
// Return Type: void
// Arguments:   Uint16 TypeOfRegister, Int8 *Params
// Description: Sets the various enable registers with values specified in the Uart string.
//				The first argument is a value denoting Specific event register to be modified and the second argument
// is the string parameter which needs to be converted to Uint and written to the register.
//#################################################################################################################
static void ei_vUartSetEnableRegisters(Uint16 TypeOfRegister, Int8 *Params)
{
	Uint16 ui_ParameterStatus;
	ui_ParameterStatus = 0;

	// 1. Run the Parameter through the error checker which returns either VALID_PARAMTER or INVALID_PARAMTER or MAX_PARAMTER or MIN_PARAMTER.
	ui_ParameterStatus = ei_uiUartParameterErrorChecker(Params,NR1);

	if(ui_ParameterStatus != VALID_PARAMETER_NOT_FOUND)
	{
		int32 l_TempStorage;
		Int8 *EndPtr;

		l_TempStorage = strtol(Params,&EndPtr,DECIMAL);

		//	Check if all charcters up to '\r' have been used up. That means end ptr must point to '\r'
		if(*EndPtr != '\r')
		{
			// Illegal Parameter
			ei_vEnqueueCommandErrorInErrorQueue(EM101);
			return;
		}

		// The values cannot be less than 0. The Command checker itself does not allow the '-' character for NR1 type
		// but no harm in checking again
		if(l_TempStorage < 0)
		{
			ei_vEnqueueCommandErrorInErrorQueue(EM222);
			return;
		}

		// Move on checking the first argument and ciopying the converted value to specific register.
		// Also the upper bounds are checked inside the respective ifs below because some registers are 16 bit and some 8 bit.

		if(TypeOfRegister == STD_EVENT_STATUS_ENABLE_REGS)
		{
			// 8 bit register. 255 MAX value
			if(l_TempStorage > MAX_VAL_FOR_8_BIT_REG)
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}

			Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_ENABLE.all = l_TempStorage;
			return;
		}

		if(TypeOfRegister == SERVICE_REQUEST_ENABLE_REGS)
		{
			// 8 bit register. 255 MAX value
			if(l_TempStorage > MAX_VAL_FOR_8_BIT_REG)
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}

			Service_Request_Regs.SERVICE_REQUEST_ENABLE.all = l_TempStorage;
			return;
		}

		if(TypeOfRegister == OPERATION_CONDITION_ENABLE_REGS)
		{
			// 8 bit register. 65535 MAX value
			if(l_TempStorage > MAX_VAL_FOR_16_BIT_REG)
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}

			Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all = l_TempStorage;
			return;
		}

		if(TypeOfRegister == QUESTIONABLE_CONDITION_ENABLE_REGS)
		{
			// 8 bit register. 65535 MAX value
			if(l_TempStorage > MAX_VAL_FOR_16_BIT_REG)
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}

			Fault_Regs.FAULT_ENABLE.all = l_TempStorage;
			return;
		}
	}
}

//#####################	PSU Control ###############################################################################
// Function Name: ei_vUartSetPsuControl
// Return Type: void
// Arguments:   Int8 *Params
// Description: Changes the state of PSU Control to either LOCAL, REMOTE_NON_LATCHED, LOCAL_LOCKOUT depending on the
//				parameters. If illegal parameters pushes the command error in error queue.
//#################################################################################################################
static void ei_vUartSetPsuControl(Int8 *Params)
{
	// In case of no arguments push missing parameter error in the Queue.
	if(*Params == '\r')
	{
		ei_vEnqueueCommandErrorInErrorQueue(EM109);
		return;
	}

	if((strcmp(Params,ZERO_WITH_CR) == MATCH_FOUND) || (strcmp(Params,LOC_WITH_CR) == MATCH_FOUND))
	{
		State.Mode.bt_PSU_Control = LOCAL;
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 1;
		return;
	}

	if((strcmp(Params,ONE_WITH_CR) == MATCH_FOUND) || (strcmp(Params,REM_WITH_CR) == MATCH_FOUND))
	{
		State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
		return;
	}

	if((strcmp(Params,TWO_WITH_CR) == MATCH_FOUND) || (strcmp(Params,LLO_WITH_CR) == MATCH_FOUND))
	{
		State.Mode.bt_PSU_Control = LOCAL_LOCKOUT;
		Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
		return;
	}

	// If we come here the user has specified illegal parameters
	ei_vEnqueueCommandErrorInErrorQueue(EM101);
	return;
}

//#####################	Program Voltage ###########################################################################
// Function Name: ei_vUartProgramVoltage
// Return Type: void
// Arguments:   Int8 *
// Description: Program the output voltage from uart. Puts PSU in remote mode. This directly sets the value in
//				Reference structure and bypasses reference setting through Qep. This is important because remote
//				mode always has better resolution than local mode(References set through Qep). Qep values are still
//				calculated and stored in respective registers because Qep needs to follow the remote programming always
//				and also because QEP only enables OVP and UVL limits calcultaion.

//				1. Pass the Parameter through Parameter Error Checker
//				2. Set the Output VOltage depending on various states.

//				NOTES:			strtod function				[Source: cpluscplus.com]
//HEADER: <cstdlib>
//SYNTAX: double strtod (const char* str, char** endptr);
//OPERATION: Convert string to double
//
//DESC: Parses the C-string str interpreting its content as a floating point number and returns its value as a double.
// If endptr is not a null pointer, the function also sets the value of endptr to point to the first character after the number.
//
// The function first discards as many whitespace characters (as in isspace) as necessary until the first non-whitespace character is found.
// Then, starting from this character, takes as many characters as possible that are valid following a syntax resembling that of floating point literals (see below),
// and interprets them as a numerical value. A pointer to the rest of the string after the last valid character is stored in the object pointed by endptr.
//
// C90 (C++98)C99/C11 (C++11)
//A valid floating point number for strtod using the "C" locale is formed by an optional sign character (+ or -), followed
//by a sequence of digits, optionally containing a decimal-point character (.), optionally followed by an exponent part (an e or E character followed by an optional sign and a sequence of digits).

//If the first sequence of non-whitespace characters in str does not form a valid floating-point number as just described, or if no such sequence exists because either
//str is empty or contains only whitespace characters, no conversion is performed and the function returns a zero value.

//PARAMETERS
//str : C-string beginning with the representation of a floating-point number.
//endptr :Reference to an already allocated object of type char*, whose value is set by the function to the next character in str after the numerical value.
//        This parameter can also be a null pointer, in which case it is not used.
//
//RETURN VALUE
//On success, the function returns the converted floating point number as a value of type double.
//If no valid conversion could be performed, the function returns zero (0.0).
//If the correct value is out of the range of representable values for the type, a positive or negative HUGE_VAL is returned, and errno is set to ERANGE.
//C90 (C++98)C99/C11 (C++11)
//If the correct value would cause underflow, the function returns a value whose magnitude is no greater than the smallest normalized positive number and sets errno to ERANGE.
//#################################################################################################################
static void ei_vUartProgramVoltage(Int8 *Params)
{
	int32 iq_Voltage, iq_VoltageRange, iq_UpperLimit, iq_LowerLimit, iq_ProgramVoltageInQepCounts;
	Uint16 ui_ParameterStatus;

	// 1. Run the Parameter through the error checker which returns either VALID_PARAMTER or INVALID_PARAMTER or MAX_PARAMTER or MIN_PARAMTER.
	ui_ParameterStatus = ei_uiUartParameterErrorChecker(Params,NRF_PLUS);

	if(ui_ParameterStatus != VALID_PARAMETER_NOT_FOUND)
	{
		if(strlen(Params) > (PARAMETER_LENGTH_ALLOWED+1))
		{
			// Too Much Data
			ei_vEnqueueExecutionErrorInErrorQueue(EM223);
			return;
		}
		else // Command within allowed resolution
		{
			// Check if MIN and MAX Parameters received
			if(ui_ParameterStatus == PARAMETER_MAX)
			{
				// Present MAX Possible Value
				iq_Voltage = _IQ15mpy(Reference.iq_OVP_Reference,_IQ15(MIN_DISTANCE_BELOW_OVP));
				if(iq_Voltage > Product_Info.iq_Specified_FS_Voltage)
				{
					iq_Voltage = Product_Info.iq_Specified_FS_Voltage;
				}
			}
			else if(ui_ParameterStatus == PARAMETER_MIN)
			{
				// Present MIN Possible Value
				iq_Voltage = _IQ15mpy(Reference.iq_UVL_Reference,_IQ15(MIN_DISTANCE_ABOVE_UVL));
			}
			else //if(ui_ParameterStatus == VALID_PARAMETER)
			{
				float32 ft_TempStorage;
				Int8 *EndPtr;

											  // Strtod takes a pointer to pointer as one of it's argument
				ft_TempStorage = strtod(Params,&EndPtr);

//				Check if all charcters up to '\r' have been used up. That means end ptr must point to '\r'
				if(*EndPtr != '\r')
				{
					// Illegal Parameter
					ei_vEnqueueCommandErrorInErrorQueue(EM101);
					return;
				}

				// Now check for Out of Range errors using HUGE VALUE
				if((ft_TempStorage == HUGE_VAL) || (ft_TempStorage < 0))
				{
					ei_vEnqueueCommandErrorInErrorQueue(EM222);
					return;
				}

				// No Errors found. Proceed to conversion from float to _IQ15
				iq_Voltage = _IQ15(ft_TempStorage);
			}

			// The comparison method with allowed range was changed on 30-10-2012. It has been observed that representing full scale using atoIQ
			// does not give the same result as representing the same no. by using _IQ(from say #define or by dividing using place value). The variations occur
			// after 5 or 6 places after decimal point and therefore the comparisons between 2 same IQ nos. each obtained by one of the above method will give
			// invalid results. Therefore for Uart commands alone we will multiply(with rounding) both LHS and RHS by 10000 and then compare

			iq_VoltageRange = Product_Info.iq_Specified_FS_Voltage;	// The Upper range is XVUL

			if(_IQ15rmpy(iq_Voltage,10000) > _IQ15rmpy(iq_VoltageRange,10000))
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}
			else // Value is within the product's range. We still need to check for OVP and UVL
			{

				iq_UpperLimit = _IQ15mpy(Reference.iq_OVP_Reference,_IQ15(MIN_DISTANCE_BELOW_OVP));
				iq_LowerLimit = _IQ15mpy(Reference.iq_UVL_Reference,_IQ15(MIN_DISTANCE_ABOVE_UVL));

				// Above OVP? or below UVL?
				if(iq_Voltage > iq_UpperLimit)
				{
					ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E301);	// Setting above OVP
					return;
				}
				else if(iq_Voltage < iq_LowerLimit)
				{
					ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E302);	// Setting below UVL
					return;
				}
				else
				{

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
					State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;

					// Reset encoders
					Encoder.i_VoltageKnob_Final_Count = Encoder.i_VoltageKnob_Coarse_Count;
					EQep1Regs.QPOSINIT = Encoder.i_VoltageKnob_Coarse_Count;
					EQep1Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;

					// SET THE VOLTAGE HERE
					ei_vSwitchSpiClkPolarity(DAC_8552);

#if 				MODE_OF_OPERATION == EQC
					ei_vCurrentCurveFitting();
					ei_vSetCurrentReference();
#endif
					ei_vSetVoltageReference();

					// Switch the polarity back to AD7705
					ei_vSwitchSpiClkPolarity(AD7705);
				}
			}
		}
	}
}

//#####################	Program OVP ###############################################################################
// Function Name: ei_vUartProgramOVP
// Return Type: void
// Arguments:   Int8 *Params
// Description: Program the over voltage from uart. Puts the PSU in Remote mode and Copies the value durectly
//				into the Reference structure. The Parameter is passed as argument.
//#################################################################################################################
static void ei_vUartProgramOVP(Int8 *Params)
{
	int32 iq_Ovp, iq_OvpRange, iq_LowerLimit;
	Uint16 ui_ParameterStatus, ui_OvpInQepCounts;

	// 1. Run the Parameter through the error checker which returns either VALID_PARAMTER or INVALID_PARAMTER or MAX_PARAMTER or MIN_PARAMTER.
	ui_ParameterStatus = ei_uiUartParameterErrorChecker(Params,NRF_PLUS);

	if(ui_ParameterStatus != VALID_PARAMETER_NOT_FOUND)
	{
		if(strlen(Params) > (PARAMETER_LENGTH_ALLOWED+1))
		{
			// Too Much Data
			ei_vEnqueueExecutionErrorInErrorQueue(EM223);
			return;
		}
		else // Command within allowed resolution
		{
			// Check if MIN and MAX Parameters received
			if(ui_ParameterStatus == PARAMETER_MAX)
			{
				// Present MAX Possible Value
				iq_Ovp = _IQ15rmpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(1.1));
			}
			else if(ui_ParameterStatus == PARAMETER_MIN)
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
			else //if(ui_ParameterStatus == VALID_PARAMETER)
			{
				float32 ft_TempStorage;
				Int8 *EndPtr;

											  // Strtod takes a pointer to pointer as one of it's argument
				ft_TempStorage = strtod(Params,&EndPtr);

//				Check if all charcters up to '\r' have been used up. That means end ptr must point to '\r'
				if(*EndPtr != '\r')
				{
					// Illegal Parameter
					ei_vEnqueueCommandErrorInErrorQueue(EM101);
					return;
				}

				// Now check for Out of Range errors using HUGE VALUE
				if((ft_TempStorage == HUGE_VAL) || (ft_TempStorage < 0))
				{
					ei_vEnqueueCommandErrorInErrorQueue(EM222);
					return;
				}

				// No Errors found. Proceed to conversion from float to _IQ15
				iq_Ovp = _IQ15(ft_TempStorage);
			}


			iq_OvpRange = _IQ15mpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(1.1));	// The Upper range is 110% of XVUL

			// Ovp out of range test
			if(_IQ15rmpy(iq_Ovp,10000) > _IQ15rmpy(iq_OvpRange,10000))
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}
			else	// Voltage within range. We still need to check if lower limit is met
			{

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
					ei_vEnqueueCommandErrorInErrorQueue(E304);	// Ovp below PV or Minimum Voltage
					return;
				}
				else	// Everything okay. Update OVP
				{
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
					State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
					Reference.ui_OVP_Reference_Update_Immediate_Flag = TRUE;

					// SET THE OVP REFERENCE
					ei_vSetOvpReference();
				}
			}
		}
	}
}

//#####################	Program UVL ###############################################################################
// Function Name: ei_vUartProgramUVL
// Return Type: void
// Arguments:   Int8 *Params
// Description: Program the under voltage from uart. Puts the PSU in remote mode and copies value to reference structure
//				directly. Takes the parameter string as argument
//#################################################################################################################
static void ei_vUartProgramUVL(Int8 *Params)
{
	int32 iq_Uvl, iq_UpperLimit;
	Uint16 ui_ParameterStatus, ui_UvlInQepCounts;

	// 1. Run the Parameter through the error checker which returns either VALID_PARAMTER or INVALID_PARAMTER or MAX_PARAMTER or MIN_PARAMTER.
	ui_ParameterStatus = ei_uiUartParameterErrorChecker(Params,NRF_PLUS);

	if(ui_ParameterStatus != VALID_PARAMETER_NOT_FOUND)
	{
		if(strlen(Params) > (PARAMETER_LENGTH_ALLOWED+1))
		{
			// Too Much Data
			ei_vEnqueueExecutionErrorInErrorQueue(EM223);
			return;
		}
		else // Command within allowed resolution
		{
			// Check if MIN and MAX Parameters received
			if(ui_ParameterStatus == PARAMETER_MAX)
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
			else if(ui_ParameterStatus == PARAMETER_MIN)
			{
				// Minimum possible Uvl is always 0
				iq_Uvl = 0;
			}
			else //if(ui_ParameterStatus == VALID_PARAMETER)
			{
				float32 ft_TempStorage;
				Int8 *EndPtr;

											  // Strtod takes a pointer to pointer as one of it's argument
				ft_TempStorage = strtod(Params,&EndPtr);

//				Check if all charcters up to '\r' have been used up. That means end ptr must point to '\r'
				if(*EndPtr != '\r')
				{
					// Illegal Parameter
					ei_vEnqueueCommandErrorInErrorQueue(EM101);
					return;
				}

				// Now check for Out of Range errors using HUGE VALUE
				if((ft_TempStorage == HUGE_VAL) || (ft_TempStorage < 0))
				{
					ei_vEnqueueCommandErrorInErrorQueue(EM222);
					return;
				}

				// No Errors found. Proceed to conversion from float to _IQ15
				iq_Uvl = _IQ15(ft_TempStorage);
			}

			// UVL out of range test
			if(_IQ15rmpy(iq_Uvl,10000) > _IQ15rmpy(Product_Info.iq_Max_Uvl_Reference,10000))
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}
			else	// UVL within range. We still need to check if Upper limit is met
			{
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
					ei_vEnqueueCommandErrorInErrorQueue(E306);	// Uvl above PV or Maximum Voltage
					return;
				}
				else	// Everything okay. Update UVL
				{
					// Convert to Qep counts
					ui_UvlInQepCounts = _IQ15rmpy(_IQ15div(iq_Uvl,Product_Info.iq_Specified_FS_Voltage),QEP_FULL_ROTATIONS_COUNT);

					// Uvl in Remote control
					Global_Flags.fg_UVL_Remote_control = TRUE;

					// Update Uvl
					Reference.iq_UVL_Reference = iq_Uvl;

					// Update Encoder Vars
					Encoder.ui_Qep_UVL_Count = ui_UvlInQepCounts;

					// Known State
					State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
					Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;
				}
			}
		}
	}
}

//#####################	Program Current ###########################################################################
// Function Name: UartProgramCurrent
// Return Type: void
// Arguments:   Int8 *
// Description: Program the output current from uart. Puts in remote mode and sets values in the reference structure
//				directly.
//				1. Pass the Parameter through Parameter Error Checker
//				2. Set the Output Current depending on various states.
//#################################################################################################################
static void ei_vUartProgramCurrent(Int8 *Params)
{
	int32 iq_Current,iq_ProgramCurrentInQepCounts;
	Uint16 ui_ParameterStatus;

	// 1. Run the Parameter through the error checker which returns either VALID_PARAMTER or INVALID_PARAMTER or MAX_PARAMTER or MIN_PARAMTER.
	ui_ParameterStatus = ei_uiUartParameterErrorChecker(Params,NRF_PLUS);

	if(ui_ParameterStatus != VALID_PARAMETER_NOT_FOUND)
	{
		if(strlen(Params) > (PARAMETER_LENGTH_ALLOWED+1))
		{
			// Too Much Data
			ei_vEnqueueExecutionErrorInErrorQueue(EM223);
			return;
		}
		else // Command within allowed resolution
		{
			// Check if MIN and MAX Parameters received
			if(ui_ParameterStatus == PARAMETER_MAX)
			{
				// Present MAX Possible Value
				iq_Current = Product_Info.iq_Specified_FS_Current;
			}
			else if(ui_ParameterStatus == PARAMETER_MIN)
			{
				// Present MIN Possible Value
				iq_Current = 0;
			}
			else if(ui_ParameterStatus == VALID_PARAMETER)
			{
				float32 ft_TempStorage;
				Int8 *EndPtr;

												// Strtod takes a pointer to pointer as one of it's argument
				ft_TempStorage = strtod(Params,&EndPtr);

//				Check if all charcters up to '\r' have been used up. That means end ptr must point to '\r'
				if(*EndPtr != '\r')
				{
					// Illegal Parameter
					ei_vEnqueueCommandErrorInErrorQueue(EM101);
					return;
				}

				// Now check for Out of Range errors using HUGE VALUE
				if((ft_TempStorage == HUGE_VAL) || (ft_TempStorage < 0))
				{
					ei_vEnqueueCommandErrorInErrorQueue(EM222);
					return;
				}

				// No Errors found. Proceed to conversion from float to _IQ15
				iq_Current = _IQ15(ft_TempStorage);
			}

			// Same As Voltage. Comparing with Int Instead of IQ.
			if(_IQ15rmpy(iq_Current,10000) > _IQ15rmpy(Product_Info.iq_Specified_FS_Current,10000))
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}
			else // Value is within the product's range.
			{
				// Convert Current to QEP Counts
				iq_ProgramCurrentInQepCounts = _IQ15rmpy(_IQ15div(iq_Current,Product_Info.iq_Specified_FS_Current),_IQ15(QEP_FULL_ROTATIONS_COUNT));

				// Current in remote control
				Global_Flags.fg_Current_Remote_control = TRUE;

				// Copy values to encoder vars
				Encoder.iq_Qep_Current_Count = iq_ProgramCurrentInQepCounts;
				// Coarse takes the integer value. Got by multiplying with 1 rather than IQint because round up required.
				Encoder.i_CurrentKnob_Coarse_Count = _IQ15rmpy(iq_ProgramCurrentInQepCounts,1);	// Coarse takes the integer value
				Reference.iq_Current_Reference = iq_Current;	// Copy to reference

				// Known state
				State.Mode.bt_Display_Mode = DISPLAY_OUTPUT;			// Change display to Display Output
				State.Mode.bt_Encoder_Operation = SET_VOLTAGE_CURRENT;	// Change operation to set_voltage_current
				State.Mode.bt_Encoder_Rotation_Mode = COARSE_ROTATION;	// Change rotation to coarse
				State.Mode.bt_PSU_Control = NON_LATCHED_REMOTE;
				Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.bit.bt_Local_Mode = 0;

				// Reset encoders
				Encoder.i_CurrentKnob_Fine_Count = QEP_FULL_ROTATIONS_COUNT/2;
				Encoder.i_CurrentKnob_Final_Count = Encoder.i_CurrentKnob_Coarse_Count;
				EQep2Regs.QPOSINIT = Encoder.i_CurrentKnob_Coarse_Count;
				EQep2Regs.QEPCTL.bit.SWI = QEP_CTR_SOFTWARE_INIT;

				// SET THE CURRENT REFERENCE
				ei_vSwitchSpiClkPolarity(DAC_8552);
#if				MODE_OF_OPERATION == EQC
				ei_vVoltageCurveFitting();
				ei_vSetVoltageReference();
#endif
				ei_vSetCurrentReference();

				// Switch the polarity back to AD7705
				ei_vSwitchSpiClkPolarity(AD7705);
			}
		}
	}
}

//#####################	Program Foldback delay ####################################################################
// Function Name: ei_vUartSetProtectionDelay
// Return Type: void
// Arguments:   Int8 *Params
// Description: Programs the protection delay for FOLD. User sends a Value from 0.1 to 25.5 seconds.
//				Minimum allowed is 0.1ms. To calculate the actual delay we have to give in the code we need to
//	find number of 100milliseconds in the specified time. This is because Check faults Occurs once in 100ms in the code.
//
//				Math
//	Protection_Delay (Counts) = Time_Specified/(0.1s) = Time_Specifed * 10;
//#################################################################################################################
static void ei_vUartSetProtectionDelay(Int8 *Params)
{
	Uint16 ui_ParameterStatus;
	int32 iq_ProtectionDelayinSecs;

	// 1. Run the Parameter through the error checker which returns either VALID_PARAMTER or INVALID_PARAMTER or MAX_PARAMTER or MIN_PARAMTER.
	ui_ParameterStatus = ei_uiUartParameterErrorChecker(Params,NRF_PLUS);

	if(ui_ParameterStatus != VALID_PARAMETER_NOT_FOUND)
	{
		if(strlen(Params) > (PARAMETER_LENGTH_ALLOWED+1))
		{
			// Too Much Data
			ei_vEnqueueExecutionErrorInErrorQueue(EM223);
			return;
		}
		else // Command within allowed resolution
		{
			// Check if MIN and MAX Parameters received
			if(ui_ParameterStatus == PARAMETER_MAX)
			{
				// Present MAX Possible Value
				iq_ProtectionDelayinSecs = _IQ15(MAX_PROTECTION_DELAY_SECONDS);
			}
			else if(ui_ParameterStatus == PARAMETER_MIN)
			{
				// Present MIN Possible Value
				iq_ProtectionDelayinSecs = _IQ15(MIN_PROTECTION_DELAY_SECONDS);
			}
			else if(ui_ParameterStatus == VALID_PARAMETER)
			{
				float32 ft_TempStorage;
				Int8 *EndPtr;

												// Strtod takes a pointer to pointer as one of it's argument
				ft_TempStorage = strtod(Params,&EndPtr);

//				Check if all charcters up to '\r' have been used up. That means end ptr must point to '\r'
				if(*EndPtr != '\r')
				{
					// Illegal Parameter
					ei_vEnqueueCommandErrorInErrorQueue(EM101);
					return;
				}

				// Now check for Out of Range errors using HUGE VALUE
				if((ft_TempStorage == HUGE_VAL) || (ft_TempStorage < 0))
				{
					ei_vEnqueueCommandErrorInErrorQueue(EM222);
					return;
				}

				// No Errors found. Proceed to conversion from float to _IQ15
				iq_ProtectionDelayinSecs = _IQ15(ft_TempStorage);
			}

			// Iq comparisons unstable
			if((_IQ15rmpy(iq_ProtectionDelayinSecs,10) > _IQ15rmpy(_IQ15(MAX_PROTECTION_DELAY_SECONDS),10)) || (_IQ15rmpy(iq_ProtectionDelayinSecs,10) < _IQ15rmpy(_IQ15(MIN_PROTECTION_DELAY_SECONDS),10)))
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
					return;
			}
			else	// All OK
			{
				// The below variable unit is 100ms
				Timers.ui_MaxFoldBackDelayInCounts = _IQ15rmpy(iq_ProtectionDelayinSecs,CHECK_FAULT_EVENT_FREQUENCY);

				// Reset the timer here.
				RESET_FOLD_BACK_DELAY_TIMER;
			}
		}
	}
}

//#####################	Rated and Specified Settings ##############################################################
// Function Name: ei_vUartSetRated_SpecifiedParams
// Return Type:  void
// Arguments:   Int8 *Params, Uint16 ui_ParameterType
// Description: Sets the Rated Voltage, Rated Current, Specified Voltage, Speciifed Current depending on user input.
//				The Checking mechanism included checks for negative value and for values greater than MAX_VOLTAGE
//				and MAX CURRENT in the Family.
//		NOTE THAT MAX and MIN are NOT valid Inputs here. Therefore the format is NRF and not NRF_PLUS

//							IMPORTANT NOTE ON THE USE OF THIS FUNCTION:
//	When in calibration mode, after Date, IDN and OPT, user has to enter
// 1. Rated Voltage/Current
// 2. Specified Voltage/Current
//
// Now when the PSU comes out from the assembly line the EEPROM is empty. At the time enArka guys at the line will
// perform the first calibration. This is when the PSU parameters are specified to the DSP.
// Now in the field during periodic recalibration, the user must not be allowed to change the Rated/Specified Params
// set in enArka. But still, the intial step in calibration requires setting of Rated/Specified Params.

// Now how to ensure that the user does change these parameters (or rather ensure that user enters the same parameters
//  that the model has been shipped for)?
// {eg. What if in the field the user specifies 100V as rated voltage during recalibration of a 60V model???)

// To prevent this misuse, the following logic has been implemented in this function.

// For a fresh PSU (Straight from line), the EEPROM will contain FFs in all location. So everytime one of the parameters
// is specified, the code reads the storage location for that parameter and sees if all FFs has been written. In that
// case write the value specified in the parameter (after all the necessary Range checks Of course!!) to EEPROM.
// In subsequent recalibration the EEPROM will no more contain FFs, but correct values entered during factory
// calibration. Now read this location and match it with the value user has specified. If it matches, proceed
// with calibration. If it does not match then:
//					 1.Discard the parameter user has entered
//					 2.Notify him with a settings conflict error
//					 3.Wait for user to re-enter that parameter.

// This check will be performed for all parameters andled by this function. Thus user cannot change anything in the field
// other than the values of measured Voltages and Current. Thus we are "As Safe as Houses".

// The above logic makes 2 risky assumptions:
// 1. The enArka Operator has entered the values correctly the first time.
// 2. EEPROM never fails in the field.
//#################################################################################################################
static void ei_vUartSetRated_SpecifiedParams(Int8 *Params, Uint16 ui_ParameterType)
{
	int16 ui_ParameterStatus;
	int32 iq_Value;
	Uint16 I2C_Tx_Data_Array[3];

	// 1. Run the Parameter through the error checker which returns either VALID_PARAMTER or INVALID_PARAMTER
	ui_ParameterStatus = ei_uiUartParameterErrorChecker(Params,NRF);

	if(ui_ParameterStatus != VALID_PARAMETER_NOT_FOUND)
	{
		if(strlen(Params) > (PARAMETER_LENGTH_ALLOWED+1))
		{
			// Too Much Data
			ei_vEnqueueExecutionErrorInErrorQueue(EM223);
			return;
		}
		else // Command within allowed resolution
		{
			float32 ft_TempStorage;
			Int8 *EndPtr;

											// Strtod takes a pointer to pointer as one of it's argument
			ft_TempStorage = strtod(Params,&EndPtr);

//				Check if all charcters up to '\r' have been used up. That means end ptr must point to '\r'
			if(*EndPtr != '\r')
			{
				// Illegal Parameter
				ei_vEnqueueCommandErrorInErrorQueue(EM101);
				return;
			}

			// Now check for Out of Range errors using HUGE VALUE. Zero is out of range for these parameters.
			if((ft_TempStorage == HUGE_VAL) || (ft_TempStorage <= 0))
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}

			// No Errors found. Proceed to conversion from float to _IQ15
			iq_Value = _IQ15(ft_TempStorage);
		}

		Uint32 ui_ValueTimes1000;
		ui_ValueTimes1000 = 0;

		// Process according to type of parameter received
		if(ui_ParameterType == RATED_VOLTAGE)
		{
			// Check if Value Greater than the greatest voltage model in the family
			if(_IQ15rmpy(iq_Value,100) > MAX_RATED_VOLTAGE_FOR_EQC_MODEL_TIMES_100)
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}
			else	// Within Range. Process.
			{
				// Read Previously programmed Rated Voltage from EEPROM
				Uint16 I2C_Rx_Data_Array[3];
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, RATED_VOLTAGE_ADDRESS, 3,I2C_Rx_Data_Array)) == FAILURE)
				{
					return;
				}

				ui_ValueTimes1000 = _IQ15rmpy(iq_Value,1000);		// Integer value times 1000. Used for I2C write.

				// Truncate everything after 3 decimal points.
				iq_Value = _IQ15(ui_ValueTimes1000/1000) + _IQ15div(_IQ15(ui_ValueTimes1000%1000),_IQ15(1000));

				// Calculate the value to be stored in EEPROM
				I2C_Tx_Data_Array[0] = (ui_ValueTimes1000/SIX_DIGIT) << 4 | ((ui_ValueTimes1000/FIVE_DIGIT)%10);				// 1st location contains 100th and the 10th place.
				I2C_Tx_Data_Array[1] = ((ui_ValueTimes1000/FOUR_DIGIT)%10) << 4 | ((ui_ValueTimes1000/THREE_DIGIT)%10);			// 2nd location contains unit's and the 10^-1 place.
				I2C_Tx_Data_Array[2] = ((ui_ValueTimes1000/TWO_DIGIT)%10) << 4 | (ui_ValueTimes1000%10);						// 3rd location contains 10^-2 and the 10^-3 place.

				// Now Read value and the value to be written must be same
				if((I2C_Tx_Data_Array[0] == I2C_Rx_Data_Array[0])&&(I2C_Tx_Data_Array[1] == I2C_Rx_Data_Array[1])&&(I2C_Tx_Data_Array[2] == I2C_Rx_Data_Array[2]))
				{
					// All okay. Values Match. No need to write again actually.

					Global_Flags.fg_Rated_Volt_Entered = TRUE;				// Set the appropriate flag
					Product_Info.iq_Rated_Voltage = iq_Value;

					// External Adc Initialization
					External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Rated_Voltage, _IQ15(CONVERSION_FACTOR_1P145V_TO_1P25V));
				}
				        // Syntax for checking multiple values == Value. Bitwise 'AND' all the variables and check whether they are equal to that single variable.
				else if(((I2C_Rx_Data_Array[0] & I2C_Rx_Data_Array[1]) & I2C_Rx_Data_Array[2]) == VALUE_AT_ANY_LOCATION_IN_A_NEW_EEPROM)
				{
					// New EEPROM. Enter whatever values given.
					while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
					ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, RATED_VOLTAGE_ADDRESS, 3,I2C_Tx_Data_Array);
					RESET_EEPROM_WRITE_CYCLE_TIMER;

					Global_Flags.fg_Rated_Volt_Entered = TRUE;				// Set the appropriate flag
					Product_Info.iq_Rated_Voltage = iq_Value;

					// External Adc Initialization
					External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Rated_Voltage, _IQ15(CONVERSION_FACTOR_1P145V_TO_1P25V));
				}
				else	// PARAMETER NOT MATCHING WITH THAT OF EEPROM
				{
					// Settings conflict
					ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
					return;
				}
			}

			// After Rated Voltage or current entered we check to see if the model can be caluclated.
			// The goto label is inside RATED_CURRENT parameter parsing.
			goto MODEL_NO_CALCULATION;
		}
		else if(ui_ParameterType == RATED_CURRENT)
		{
			// Check if Value Greater than the greatest Current Model in the family
			if(_IQ15rmpy(iq_Value,100) > MAX_RATED_CURRENT_FOR_EQC_MODEL_TIMES_100)
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}
			else	// Within Range. Process.
			{
				// Read Previously programmed Rated Voltage from EEPROM
				Uint16 I2C_Rx_Data_Array[3];
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, RATED_CURRENT_ADDRESS, 3,I2C_Rx_Data_Array)) == FAILURE)
				{
					return;
				}

				ui_ValueTimes1000 = _IQ15rmpy(iq_Value,1000);			// Integer value times 1000. Used for I2C write.

				// Truncate everything after 3 decimal points.
				iq_Value = _IQ15(ui_ValueTimes1000/1000) + _IQ15div(_IQ15(ui_ValueTimes1000%1000),_IQ15(1000));

				// Calculate the value to be stored in EEPROM
				I2C_Tx_Data_Array[0] = (ui_ValueTimes1000/SIX_DIGIT) << 4 | ((ui_ValueTimes1000/FIVE_DIGIT)%10);				// 1st location contains 100th and the 10th place.
				I2C_Tx_Data_Array[1] = ((ui_ValueTimes1000/FOUR_DIGIT)%10) << 4 | ((ui_ValueTimes1000/THREE_DIGIT)%10);			// 2nd location contains unit's and the 10^-1 place.
				I2C_Tx_Data_Array[2] = ((ui_ValueTimes1000/TWO_DIGIT)%10) << 4 | (ui_ValueTimes1000%10);						// 3rd location contains 10^-2 and the 10^-3 place.

								// Now Read value and the value to be written must be same
				if((I2C_Tx_Data_Array[0] == I2C_Rx_Data_Array[0])&&(I2C_Tx_Data_Array[1] == I2C_Rx_Data_Array[1])&&(I2C_Tx_Data_Array[2] == I2C_Rx_Data_Array[2]))
				{
					// All okay. Values Match. No need to write again actually.

					Global_Flags.fg_Rated_Curr_Entered = TRUE;				// Set the appropriate flag
					Product_Info.iq_Rated_Current = iq_Value;

					// External Adc Initialization
					External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Rated_Current, _IQ15(CONVERSION_FACTOR_1P145V_TO_1P25V));
				}
				// Syntax for checking multiple values == Value. Bitwise 'AND' all the variables and check whether they are equal to that single variable.
				else if(((I2C_Rx_Data_Array[0] & I2C_Rx_Data_Array[1]) & I2C_Rx_Data_Array[2]) == VALUE_AT_ANY_LOCATION_IN_A_NEW_EEPROM)
				{
					// New EEPROM. Enter whatever values given.
					while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
					ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, RATED_CURRENT_ADDRESS, 3,I2C_Tx_Data_Array);
					RESET_EEPROM_WRITE_CYCLE_TIMER;

					Global_Flags.fg_Rated_Curr_Entered = TRUE;				// Set the appropriate flag
					Product_Info.iq_Rated_Current = iq_Value;

					// External Adc Initialization
					External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Full_Scale_Value = _IQ15mpy(Product_Info.iq_Rated_Current, _IQ15(CONVERSION_FACTOR_1P145V_TO_1P25V));
				}
				else	// PARAMETER NOT MATCHING WITH THAT OF EEPROM
				{
					// Settings conflict
					ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
					return;
				}

				MODEL_NO_CALCULATION:;
				// To check if model number can be calculated
				if((Global_Flags.fg_Rated_Volt_Entered & Global_Flags.fg_Rated_Curr_Entered) == TRUE)
				{
					// Model can be checked and set.
					ei_vCheckAndSetModelNo();
				}
			}
		}
		else if(ui_ParameterType == SPECIFIED_VOLTAGE)
		{
			// Check if Value Greater than the greatest voltage model in the family
			if(_IQ15rmpy(iq_Value,100) > MAX_RATED_VOLTAGE_FOR_EQC_MODEL_TIMES_100)
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}
			else	// Within Range. Process.
			{
				// Read Previously programmed Rated Voltage from EEPROM
				Uint16 I2C_Rx_Data_Array[3];
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, SPECIFIED_VOLTAGE_ADDRESS, 3,I2C_Rx_Data_Array)) == FAILURE)
				{
					return;
				}

				ui_ValueTimes1000 = _IQ15rmpy(iq_Value,1000);		// Integer value times 1000. Used for I2C write.
				// Truncate everything after 3 decimal points.
				iq_Value = _IQ15(ui_ValueTimes1000/1000) + _IQ15div(_IQ15(ui_ValueTimes1000%1000),_IQ15(1000));

				// Calculate the value to be stored in EEPROM
				I2C_Tx_Data_Array[0] = (ui_ValueTimes1000/SIX_DIGIT) << 4 | ((ui_ValueTimes1000/FIVE_DIGIT)%10);				// 1st location contains 100th and the 10th place.
				I2C_Tx_Data_Array[1] = ((ui_ValueTimes1000/FOUR_DIGIT)%10) << 4 | ((ui_ValueTimes1000/THREE_DIGIT)%10);			// 2nd location contains unit's and the 10^-1 place.
				I2C_Tx_Data_Array[2] = ((ui_ValueTimes1000/TWO_DIGIT)%10) << 4 | (ui_ValueTimes1000%10);						// 3rd location contains 10^-2 and the 10^-3 place.


				// Now Read value and the value to be written must be same
				if((I2C_Tx_Data_Array[0] == I2C_Rx_Data_Array[0])&&(I2C_Tx_Data_Array[1] == I2C_Rx_Data_Array[1])&&(I2C_Tx_Data_Array[2] == I2C_Rx_Data_Array[2]))
				{
					// All okay. Values Match. No need to write again actually.

					Global_Flags.fg_Specified_Volt_Entered = TRUE;				// Set the appropriate flag
					Product_Info.iq_Specified_FS_Voltage = iq_Value;
				}
				// Syntax for checking multiple values == Value. Bitwise 'AND' all the variables and check whether they are equal to that single variable.
				else if(((I2C_Rx_Data_Array[0] & I2C_Rx_Data_Array[1]) & I2C_Rx_Data_Array[2]) == VALUE_AT_ANY_LOCATION_IN_A_NEW_EEPROM)
				{
					// New EEPROM. Enter whatever values given.
					while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
					ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, SPECIFIED_VOLTAGE_ADDRESS, 3,I2C_Tx_Data_Array);
					RESET_EEPROM_WRITE_CYCLE_TIMER;

					Global_Flags.fg_Specified_Volt_Entered = TRUE;				// Set the appropriate flag
					Product_Info.iq_Specified_FS_Voltage = iq_Value;
				}
				else	// PARAMETER NOT MATCHING WITH THAT OF EEPROM
				{
					// Settings conflict
					ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
					return;
				}
			}
		}
		else //if(ui_ParameterType == SPECIFIED_CURRENT)
		{
			// Check if Value Greater than the greatest Current Model in the family
			if(_IQ15rmpy(iq_Value,100) > MAX_RATED_CURRENT_FOR_EQC_MODEL_TIMES_100)
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}
			else	// Within Range. Process.
			{
				// Read Previously programmed Rated Voltage from EEPROM
				Uint16 I2C_Rx_Data_Array[3];
				while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
				if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, SPECIFIED_CURRENT_ADDRESS, 3,I2C_Rx_Data_Array)) == FAILURE)
				{
					return;
				}

				ui_ValueTimes1000 = _IQ15rmpy(iq_Value,1000);			// Integer value times 1000. Used for I2C write.
				// Truncate everything after 3 decimal points.
				iq_Value = _IQ15(ui_ValueTimes1000/1000) + _IQ15div(_IQ15(ui_ValueTimes1000%1000),_IQ15(1000));

				// Calculate the value to be stored in EEPROM
				I2C_Tx_Data_Array[0] = (ui_ValueTimes1000/SIX_DIGIT) << 4 | ((ui_ValueTimes1000/FIVE_DIGIT)%10);				// 1st location contains 100th and the 10th place.
				I2C_Tx_Data_Array[1] = ((ui_ValueTimes1000/FOUR_DIGIT)%10) << 4 | ((ui_ValueTimes1000/THREE_DIGIT)%10);			// 2nd location contains unit's and the 10^-1 place.
				I2C_Tx_Data_Array[2] = ((ui_ValueTimes1000/TWO_DIGIT)%10) << 4 | (ui_ValueTimes1000%10);						// 3rd location contains 10^-2 and the 10^-3 place.


				// Now Read value and the value to be written must be same
				if((I2C_Tx_Data_Array[0] == I2C_Rx_Data_Array[0])&&(I2C_Tx_Data_Array[1] == I2C_Rx_Data_Array[1])&&(I2C_Tx_Data_Array[2] == I2C_Rx_Data_Array[2]))
				{
					// All okay. Values Match. No need to write again actually.

					Global_Flags.fg_Specified_Curr_Entered = TRUE;				// Set the appropriate flag
					Product_Info.iq_Specified_FS_Current = iq_Value;
				}
				// Syntax for checking multiple values == Value. Bitwise 'AND' all the variables and check whether they are equal to that single variable.
				else if(((I2C_Rx_Data_Array[0] & I2C_Rx_Data_Array[1]) & I2C_Rx_Data_Array[2]) == VALUE_AT_ANY_LOCATION_IN_A_NEW_EEPROM)
				{
					// New EEPROM. Enter whatever values given.
					while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
					ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, SPECIFIED_CURRENT_ADDRESS, 3,I2C_Tx_Data_Array);
					RESET_EEPROM_WRITE_CYCLE_TIMER;

					Global_Flags.fg_Specified_Curr_Entered = TRUE;				// Set the appropriate flag
					Product_Info.iq_Specified_FS_Current = iq_Value;
				}
				else	// PARAMETER NOT MATCHING WITH THAT OF EEPROM
				{
					// Settings conflict
					ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E345);
					return;
				}
			}
		}
	}
}

//#####################	Uart voltage calibration ##################################################################
// Function Name: ei_vUartVoltageCalibrationProcess
// Return Type: void
// Arguments:   Int8 *Params
// Description: Voltage calibration using Uart. Gets the measured low and high voltage.
//				Calculates Voltage reference Offset and Gain. (X --> Set points; Y --> Measured value)
//				Calculates Voltage display offset and Gain.	  (X --> Measured Value; Y --> Set Point)

//	GAIN should never go to 0. Checking mechanism implemented internally.
//#################################################################################################################
static void ei_vUartVoltageCalibrationProcess(Int8 *Params)
{
	int16 ui_ParameterStatus;
	int32 iq_Value;
	Uint16 I2C_Tx_Data_Array[16];
	static int32 MV_1,MV_2,DV_1,DV_2;

	// 1. Run the Parameter through the error checker which returns either VALID_PARAMTER or INVALID_PARAMTER
	ui_ParameterStatus = ei_uiUartParameterErrorChecker(Params,NRF);

	if(ui_ParameterStatus != VALID_PARAMETER_NOT_FOUND)
	{
		if(strlen(Params) > (PARAMETER_LENGTH_ALLOWED+1))
		{
			// Too Much Data
			ei_vEnqueueExecutionErrorInErrorQueue(EM223);
			return;
		}
		else // Command within allowed resolution
		{
			float32 ft_TempStorage;
			Int8 *EndPtr;

											// Strtod takes a pointer to pointer as one of it's argument
			ft_TempStorage = strtod(Params,&EndPtr);

//			Check if all charcters up to '\r' have been used up. That means end ptr must point to '\r'
			if(*EndPtr != '\r')
			{
				// Illegal Parameter
				ei_vEnqueueCommandErrorInErrorQueue(EM101);
				return;
			}

			// Now check for Out of Range errors using HUGE VALUE. Zero is out of range for these parameters.
			if((ft_TempStorage == HUGE_VAL) || (ft_TempStorage <= 0))
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}

			// No Errors found. Proceed to conversion from float to _IQ15
			iq_Value = _IQ15(ft_TempStorage);
		}

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
		if((Global_Flags.fg_Voltage_MV1_Entered & Global_Flags.fg_Voltage_MV2_Entered) == TRUE)
		{
			int32 X1,X2,Temp;

			X1 = Product_Info.iq_Specified_FS_Voltage/10;
			X2 = Product_Info.iq_Specified_FS_Voltage;

			Product_Info.iq_Voltage_Ref_Gain = _IQ15div((MV_2 - MV_1),(X2 - X1));
			// Gain can never be 0
			if(Product_Info.iq_Voltage_Ref_Gain <= 0)
			{
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E399);
				State.Mode.bt_Output_Status = OFF;
				Global_Flags.fg_Voltage_MV1_Entered = Global_Flags.fg_Voltage_MV2_Entered = FALSE;	// Reset Both Flags. Value to be entered again.
				return;
			}
			Product_Info.iq_Voltage_Ref_Offset = MV_1 - _IQ15mpy(Product_Info.iq_Voltage_Ref_Gain, X1);
			Product_Info.iq_Voltage_Display_Gain = _IQ15div((DV_2 - DV_1),(MV_2 - MV_1));
			// Gain can never be 0
			if(Product_Info.iq_Voltage_Display_Gain <= 0)
			{
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E399);
				State.Mode.bt_Output_Status = OFF;
				Global_Flags.fg_Voltage_MV1_Entered = Global_Flags.fg_Voltage_MV2_Entered = FALSE;	// Reset Both Flags. Value to be entered again.
				return;
			}
			Product_Info.iq_Voltage_Display_Offset = DV_1 - _IQ15mpy(Product_Info.iq_Voltage_Display_Gain, MV_1);

			// Voltage offset
			Temp = Product_Info.iq_Voltage_Ref_Offset;
			I2C_Tx_Data_Array[0] = (Temp >> 24) & 0x000000FF;
			I2C_Tx_Data_Array[1] = (Temp >> 16) & 0x000000FF;
			I2C_Tx_Data_Array[2] = (Temp >> 8) & 0x000000FF;
			I2C_Tx_Data_Array[3] = (Temp) & 0x000000FF;

			// Voltage gain
			Temp = Product_Info.iq_Voltage_Ref_Gain;
			I2C_Tx_Data_Array[4] = (Temp >> 24) & 0x000000FF;
			I2C_Tx_Data_Array[5] = (Temp >> 16) & 0x000000FF;
			I2C_Tx_Data_Array[6] = (Temp >> 8) & 0x000000FF;
			I2C_Tx_Data_Array[7] = (Temp) & 0x000000FF;

			// Voltage Display offset
			Temp = Product_Info.iq_Voltage_Display_Offset;
			I2C_Tx_Data_Array[8] = (Temp >> 24) & 0x000000FF;
			I2C_Tx_Data_Array[9] = (Temp >> 16) & 0x000000FF;
			I2C_Tx_Data_Array[10] = (Temp >> 8) & 0x000000FF;
			I2C_Tx_Data_Array[11] = (Temp) & 0x000000FF;

			// Voltage Display gain
			Temp = Product_Info.iq_Voltage_Display_Gain;
			I2C_Tx_Data_Array[12] = (Temp >> 24) & 0x000000FF;
			I2C_Tx_Data_Array[13] = (Temp >> 16) & 0x000000FF;
			I2C_Tx_Data_Array[14] = (Temp >> 8) & 0x000000FF;
			I2C_Tx_Data_Array[15] = (Temp) & 0x000000FF;

			while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
			ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, V_OFFSET_ADDRESS, 16,I2C_Tx_Data_Array);
			RESET_EEPROM_WRITE_CYCLE_TIMER;

			Global_Flags.fg_Voltage_Calibration_Done = TRUE;
		}
	}
}

//#####################	Uart current calibration ##################################################################
// Function Name: ei_vUartCurrentCalibrationProcess
// Return Type: void
// Arguments:   Int8 *Params
// Description: Current calibration using Uart. Gets the measured low and high voltage.
//				Calculates Current reference Offset and Gain. (X --> Set points; Y --> Measured value)
//				Calculates Current display offset and Gain.	  (X --> Measured Value; Y --> Set Point)

//	GAIN should never go to 0. Checking mechanism implemented internally.
//#################################################################################################################
static void ei_vUartCurrentCalibrationProcess(Int8 *Params)
{
	int16 ui_ParameterStatus;
	int32 iq_Value,iq_Temp;
	Uint16 i;
	Uint16 I2C_Tx_Data_Array[49],I2C_Rx_Data_Array[46];
	static int32 MC_1,MC_2,DC_1,DC_2;


	// 1. Run the Parameter through the error checker which returns either VALID_PARAMTER or INVALID_PARAMTER
	ui_ParameterStatus = ei_uiUartParameterErrorChecker(Params,NRF);

	if(ui_ParameterStatus != VALID_PARAMETER_NOT_FOUND)
	{
		if(strlen(Params) > (PARAMETER_LENGTH_ALLOWED+1))
		{
			// Too Much Data
			ei_vEnqueueExecutionErrorInErrorQueue(EM223);
			return;
		}
		else // Command within allowed resolution
		{
			float32 ft_TempStorage;
			Int8 *EndPtr;

											// Strtod takes a pointer to pointer as one of it's argument
			ft_TempStorage = strtod(Params,&EndPtr);

//			Check if all charcters up to '\r' have been used up. That means end ptr must point to '\r'
			if(*EndPtr != '\r')
			{
				// Illegal Parameter
				ei_vEnqueueCommandErrorInErrorQueue(EM101);
				return;
			}

			// Now check for Out of Range errors using HUGE VALUE. Zero is out of range for these parameters.
			if((ft_TempStorage == HUGE_VAL) || (ft_TempStorage <= 0))
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM222);
				return;
			}

			// No Errors found. Proceed to conversion from float to _IQ15
			iq_Value = _IQ15(ft_TempStorage);
		}

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
		if((Global_Flags.fg_Current_MC1_Entered & Global_Flags.fg_Current_MC2_Entered) == TRUE)
		{
			int32 X1,X2;

			X1 = Product_Info.iq_Specified_FS_Current/10;
			X2 = Product_Info.iq_Specified_FS_Current;

			Product_Info.iq_Current_Ref_Gain = _IQ15div((MC_2 - MC_1),(X2 - X1));
			// Gain can never be 0
			if(Product_Info.iq_Current_Ref_Gain <= 0)
			{
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E399);
				State.Mode.bt_Output_Status = OFF;
				Global_Flags.fg_Current_MC1_Entered = Global_Flags.fg_Current_MC2_Entered = FALSE;	// Reset Both Flags. Value to be entered again.
				return;
			}
			Product_Info.iq_Current_Ref_Offset = MC_1 - _IQ15mpy(Product_Info.iq_Current_Ref_Gain, X1);

			Product_Info.iq_Current_Display_Gain = _IQ15div((DC_2 - DC_1),(MC_2 - MC_1));
			// Gain can never be 0
			if(Product_Info.iq_Current_Display_Gain <= 0)
			{
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E399);
				State.Mode.bt_Output_Status = OFF;
				Global_Flags.fg_Current_MC1_Entered = Global_Flags.fg_Current_MC2_Entered = FALSE;	// Reset Both Flags. Value to be entered again.
				return;
			}

			Product_Info.iq_Current_Display_Offset = DC_1 - _IQ15mpy(Product_Info.iq_Current_Display_Gain, MC_1);

			Global_Flags.fg_Current_Calibration_Done = TRUE;
		}

		// This was the last step in calibration. We need to read everything from EEPROM,
		// calculate checksum and write it back to eeprom.

		// Check if all operations completed
		if((Global_Flags.fg_Date_Entered & Global_Flags.fg_Rated_Volt_Entered & Global_Flags.fg_Rated_Curr_Entered & Global_Flags.fg_Specified_Volt_Entered & Global_Flags.fg_Specified_Curr_Entered & Global_Flags.fg_Voltage_Calibration_Done & Global_Flags.fg_Current_Calibration_Done) == TRUE)
		{
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

//#####################	Respond to the command received ###########################################################
// Function Name: ei_vRespondToReceivedMessage
// Return Type: void
// Arguments:   Uint16 Response_Key
// Description: Whatever response the PSU generates will be generated from here.
//#################################################################################################################
static void ei_vRespondToReceivedMessage(Uint16 Response_Key)
{
	Uint16 Checksum = 0;
	Uint16 String_length = 0;
	Uint16 i = 0;

	Int8 Uart_Tx_Data_Array[MAX_UART_ARRAY_SIZE+1];

	// Initialise
	Uart_Tx_Data_Array[0] = 0;

	switch(Response_Key)
	{
		case REPLY_WITH_INSTRUMENT_ADDR:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Reference.ui_Set_Address);
			break;
		}

		case REPLY_WITH_SYSTEM_ERROR:
		{
			// Dequeue the error located at offset 0.
			ei_vDequeueFromErrorQueue(Uart_Tx_Data_Array);
			break;
		}


		case REPLY_WITH_SOFTWARE_VERSION:
		{
			strcpy(Uart_Tx_Data_Array,SOFTWARE_VERSION);
			break;
		}

		case REPLY_WITH_CALIBRATION_DATE:
		{
			Uint16 I2C_Rx_Data_Array[DATE_LENGTH+1];

			// First read the no. of characters that make up the .
			while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
			if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, DATE_START_ADDRESS, DATE_LENGTH, I2C_Rx_Data_Array)) == FAILURE)
			{
				return;
			}

			I2C_Rx_Data_Array[DATE_LENGTH] = '\0';		// The last character must be null so that the array can be used as a string
														// in the next step.

			strcpy(Uart_Tx_Data_Array,(Int8 *)I2C_Rx_Data_Array);
			break;
		}

		case REPLY_WITH_POWER_ON_TIME:
		{
			sprintf(Uart_Tx_Data_Array,"%lu",Product_Info.ul_Product_Power_On_Time_In_Minutes);
			break;
		}

		case REPLY_WITH_STATE_OF_PSU_CONTROL:
		{
			if(State.Mode.bt_PSU_Control == LOCAL)
			{
				strcpy(Uart_Tx_Data_Array,LOC);
			}
			else if(State.Mode.bt_PSU_Control == NON_LATCHED_REMOTE)
			{
				strcpy(Uart_Tx_Data_Array,REM);
			}
			else // Local Lockout
			{
				strcpy(Uart_Tx_Data_Array,LLO);
			}
			break;
		}

		case REPLY_WITH_STD_EVENT_STATUS_ENABLE_REGISTER:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_ENABLE.all);
			break;
		}

		case REPLY_WITH_STD_EVENT_STATUS_REGISTER:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.all);
			CLEAR_ON_READING(Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.all);

			// Clear the ESE bit in STB
			Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_Standard_Event_Summary = CLEARED;
			break;
		}

		case REPLY_WITH_IDENTIFICATION_STRING:
		{
			Uint16 I2C_Rx_Data_Array[40];

			// First read the no. of characters that make up the ID.
			while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
			if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, IDN_ADDRESS, 40, I2C_Rx_Data_Array)) == FAILURE)
			{
				return;
			}
			I2C_Rx_Data_Array[39] = '\0';	// The last character must be null so that the array can be used as a string
											// in the next step. This is protection in case only 0xFFs are stored in EEPROM
			sprintf(Uart_Tx_Data_Array,"ENARKA,VBV%s,%s-%s",I2C_Rx_Data_Array,SOFTWARE_VERSION,IEEE_LAN_SOFTWARE_REVISION);
			break;
		}

		case REPLY_WITH_INSTALLED_OPTIONS:
		{
			Uint16 I2C_Rx_Data_Array[1];

			// First read the no. of characters that make up the ID.
			while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
			if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, OPTION_INSTALLED_DETAILS_ADDRESS, 1, I2C_Rx_Data_Array)) == FAILURE)
			{
				return;
			}

			sprintf(Uart_Tx_Data_Array,"%u",I2C_Rx_Data_Array[0]);
			break;
		}

		case REPLY_WITH_POWER_ON_STATE_CLEAR_SETTING:
		{
			sprintf(Uart_Tx_Data_Array,"%u",State.Mode.bt_Power_On_Status_Clear);
			break;
		}

		case REPLY_WITH_SERVICE_REQUEST_ENABLE_REGISTER:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Service_Request_Regs.SERVICE_REQUEST_ENABLE.all);
			break;
		}

		case REPLY_WITH_STATUS_BYTE_REGISTER:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Service_Request_Regs.STATUS_BYTE_REGISTER.all);
			break;
		}

		case REPLY_WITH_MEASURED_DC_VOLTAGE:
		{
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Calibrated_Value));
			break;
		}

		case REPLY_WITH_MEASURED_DC_CURRENT:
		{
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Calibrated_Value));
			break;
		}

		case REPLY_WITH_MEASURED_DC_POWER:
		{
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(_IQ15rmpy(External_Rms_Array[VOLTAGE_MEASURE_CHANNEL].iq_Calibrated_Value,External_Rms_Array[CURRENT_MEASURE_CHANNEL].iq_Calibrated_Value)));
			break;
		}

		case REPLY_WITH_MEASURED_TEMPERATURE:
		{
			sprintf(Uart_Tx_Data_Array,"%f",_IQ8toF(iq8_Temperature));
			break;
		}

		case REPLY_WITH_PROGRAMMED_VOLTAGE:
		{
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(Reference.iq_Voltage_Reference));
			break;
		}

		case REPLY_WITH_MIN_PROGRAMMABLE_VOLTAGE:
		{
			// Floor is always 1.05 times UVL
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(_IQ15rmpy(Reference.iq_UVL_Reference,_IQ15(MIN_DISTANCE_ABOVE_UVL))));
			break;
		}

		case REPLY_WITH_MAX_PROGRAMMABLE_VOLTAGE:
		{
			// CEILING is always 0.95 times OVP or SVUL whichever is lower
			int32 iq_Temp;
			iq_Temp = _IQ15rmpy(Reference.iq_OVP_Reference,_IQ15(MIN_DISTANCE_BELOW_OVP));

			// Is SVUL Lower?
			if(iq_Temp > Product_Info.iq_Specified_FS_Voltage)
			{
				iq_Temp = Product_Info.iq_Specified_FS_Voltage;
			}
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(iq_Temp));
			break;
		}

		case REPLY_WITH_PROGRAMMED_OVP:
		{
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(Reference.iq_OVP_Reference));
			break;
		}

		case REPLY_WITH_MAX_PROGRAMMABLE_OVP:
		{
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(_IQ15rmpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(1.1))));
			break;
		}

		case REPLY_WITH_MIN_PROGRAMMABLE_OVP:
		{
			int32 iq_Temp;

			if(OUTPUT_OFF)
			{
				// Minmum OVP specified for that model in ModelNO.h
				iq_Temp = Product_Info.iq_Min_Ovp_Reference;
			}
			else	// OUTPUT ON
			{
				// Minimum is (0.95^-1) times the programmed voltage or Minimum ovp for that model whichever is greater
				iq_Temp = _IQ15mpy(Reference.iq_Voltage_Reference,_IQ15(MIN_DISTANCE_ABOVE_PROGRAMMED_VOLT));
				if(iq_Temp < Product_Info.iq_Min_Ovp_Reference)
				{
					iq_Temp = Product_Info.iq_Min_Ovp_Reference;
				}
			}

			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(iq_Temp));
			break;
		}

		case REPLY_WITH_PROGRAMMED_UVL:
		{
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(Reference.iq_UVL_Reference));
			break;
		}

		case REPLY_WITH_MIN_PROGRAMMABLE_UVL:
		{
			sprintf(Uart_Tx_Data_Array,"%f",(float)0);
			break;
		}

		case REPLY_WITH_MAX_PROGRAMMABLE_UVL:
		{
			int32 iq_Temp;

			if(OUTPUT_OFF)
			{
				// When o/p off Uvl max is min(0.9047 * OVP, UVLmax specified for that model)
				iq_Temp = _IQ15mpy(Reference.iq_OVP_Reference,_IQ15(0.904761904));

				if(iq_Temp > Product_Info.iq_Max_Uvl_Reference)
				{
					iq_Temp = Product_Info.iq_Max_Uvl_Reference;
				}
			}
			else	// OUTPUT ON
			{
				// Maximum is (1.05^-1) times the programmed voltage
				iq_Temp = _IQ15mpy(Reference.iq_Voltage_Reference,_IQ15(MIN_DISTANCE_BELOW_PROGRAMMED_VOLT));
			}

			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(iq_Temp));
			break;
		}

		case REPLY_WITH_PROGRAMMED_CURRENT:
		{
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(Reference.iq_Current_Reference));
			break;
		}

		case REPLY_WITH_MAX_PROGRAMMABLE_CURRENT:
		{
			sprintf(Uart_Tx_Data_Array,"%f",_IQ15toF(Product_Info.iq_Specified_FS_Current));
			break;
		}

		case REPLY_WITH_MIN_PROGRAMMABLE_CURRENT:
		{
			sprintf(Uart_Tx_Data_Array,"%f",(float)0);
			break;
		}

		case REPLY_WITH_OUTPUT_STATE:
		{
			if(OUTPUT_ON)
			{
				strcpy(Uart_Tx_Data_Array,ONE);
			}
			else	// OUTPUT OFF
			{
				strcpy(Uart_Tx_Data_Array,ZERO);
			}
			break;
		}

		case REPLY_WITH_PON_STATE:
		{
			if(State.Mode.bt_Start_Mode == AUTO_RESTART)
			{
				strcpy(Uart_Tx_Data_Array,ONE);
			}
			else
			{
				strcpy(Uart_Tx_Data_Array,ZERO);
			}
			break;
		}

		case REPLY_WITH_OUTPUT_MODE:
		{
			if(OUTPUT_ON)
			{
				if(CV_MODE)
				{
					strcpy(Uart_Tx_Data_Array,CV);
				}
				else	// CC mode
				{
					strcpy(Uart_Tx_Data_Array,CC);
				}
			}
			else	// Output off
			{
				strcpy(Uart_Tx_Data_Array,OFF_STRING);
			}
			break;
		}

		case REPLY_WITH_SET_PROTECTION_DELAY:
		{
			// Use any 1. Either Uvp_Timer or FoldBack_Timer

			sprintf(Uart_Tx_Data_Array,"%.1f",_IQ15toF(_IQ15div(_IQ15(Timers.ui_MaxFoldBackDelayInCounts),_IQ15(CHECK_FAULT_EVENT_FREQUENCY))));
			break;
		}

		case REPLY_WITH_SET_FOLDBACK_MODE:
		{
			if(State.Mode.bt_FoldBack_Mode == FOLDBACK_DISARMED)
			{
				strcpy(Uart_Tx_Data_Array,ZERO);
			}
			else if(State.Mode.bt_FoldBack_Mode == FOLDBACK_ARMED_TO_PROTECT_FROM_CC)
			{
				strcpy(Uart_Tx_Data_Array,ONE);
			}
			else //if(State.Mode.bt_FoldBack_Mode == FOLDBACK_ARMED_TO_PROTECT_FROM_CC)
			{
				strcpy(Uart_Tx_Data_Array,TWO);
			}
			break;
		}

		case REPLY_WITH_RATED_VOLTAGE:
		{
			sprintf(Uart_Tx_Data_Array,"%.3f",_IQ15toF(Product_Info.iq_Rated_Voltage));
			break;
		}

		case REPLY_WITH_SPECIFIED_VOLTAGE:
		{
			sprintf(Uart_Tx_Data_Array,"%.3f", _IQ15toF(Product_Info.iq_Specified_FS_Voltage));
			break;
		}

		case REPLY_WITH_VOLTAGE_CALIBRATION_PARAMETERS:
		{
			sprintf(Uart_Tx_Data_Array,"%f , %f",_IQ15toF(Product_Info.iq_Voltage_Ref_Offset),_IQ15toF(Product_Info.iq_Voltage_Ref_Gain));
			break;
		}

		case REPLY_WITH_VOLTAGE_DISPLAY_CALIBRATION_PARAMETERS:
		{
			sprintf(Uart_Tx_Data_Array,"%f , %f",_IQ15toF(Product_Info.iq_Voltage_Display_Offset),_IQ15toF(Product_Info.iq_Voltage_Display_Gain));
			break;
		}

		case REPLY_WITH_RATED_CURRENT:
		{
			sprintf(Uart_Tx_Data_Array,"%.3f",_IQ15toF(Product_Info.iq_Rated_Current));
			break;
		}

		case REPLY_WITH_SPECIFIED_CURRENT:
		{
			sprintf(Uart_Tx_Data_Array,"%.3f",_IQ15toF(Product_Info.iq_Specified_FS_Current));
			break;
		}

		case REPLY_WITH_CURRENT_CALIBRATION_PARAMETERS:
		{
			sprintf(Uart_Tx_Data_Array,"%f , %f",_IQ15toF(Product_Info.iq_Current_Ref_Offset),_IQ15toF(Product_Info.iq_Current_Ref_Gain));
			break;
		}

		case REPLY_WITH_CURRENT_DISPLAY_CALIBRATION_PARAMETERS:
		{
			sprintf(Uart_Tx_Data_Array,"%f , %f",_IQ15toF(Product_Info.iq_Current_Display_Offset),_IQ15toF(Product_Info.iq_Current_Display_Gain));
			break;
		}

		case REPLY_WITH_CALIBRATION_COUNT:
		{
			Uint16 I2C_Rx_Data_Array[1];
			// Read the calibration count.
			while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
			if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, CALIBRATION_COUNT_ADDRESS, 1,I2C_Rx_Data_Array)) == FAILURE)
			{
				return;
			}
			sprintf(Uart_Tx_Data_Array,"%u",I2C_Rx_Data_Array[0]);
			break;
		}

		case REPLY_WITH_PSU_MODEL_NUMBER:
		{
			Uint16 I2C_Rx_Data_Array[1];
			// Read the calibration count.
			while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
			if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, MODEL_NO_ADDR, 1,I2C_Rx_Data_Array)) == FAILURE)
			{
				return;
			}
			sprintf(Uart_Tx_Data_Array,"%u",I2C_Rx_Data_Array[0]);
			break;
		}

		case REPLY_WITH_OPERATION_CONDITION_ENABLE_REGISTER:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all);
			break;
		}

		case REPLY_WITH_OPERATION_CONDITION_EVENT_REGISTER:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all);
			CLEAR_ON_READING(Operation_Condition_Regs.OPERATION_CONDITION_EVENT.all);

			// Clear the Operation Summary bit in STB register
			Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_Operation_Summary = CLEARED;
			break;
		}

		case REPLY_WITH_OPERATION_CONDITION_REGISTER:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Operation_Condition_Regs.OPERATION_CONDITION_REGISTER.all);
			break;
		}

		case REPLY_WITH_QUESTIONABLE_CONDITION_ENABLE_REGISTER:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Fault_Regs.FAULT_ENABLE.all);
			break;
		}

		case REPLY_WITH_QUESTIONABLE_CONDITION_EVENT_REGISTER:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Fault_Regs.FAULT_EVENT.all);
			CLEAR_ON_READING(Fault_Regs.FAULT_EVENT.all);

			// Clear the Questionable Summary bit in STB register
			Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_Questionable_Summary = CLEARED;
			break;
		}

		case REPLY_WITH_QUESTIONABLE_CONDITION_REGISTER:
		{
			sprintf(Uart_Tx_Data_Array,"%u",Fault_Regs.FAULT_REGISTER.all);
			break;
		}

		case REPLY_WITH_STATE_OF_DISPLAY:
		{
			if(Global_Flags.fg_State_Of_Display == ON)
			{
				strcpy(Uart_Tx_Data_Array,ONE);
			}
			else //if(Global_Flags.fg_State_Of_Display == OFF)
			{
				strcpy(Uart_Tx_Data_Array,ZERO);
			}

			break;
		}

		case REPLY_WITH_FLASH_CHECKSUM_STORED_IN_EEPROM:
		{
			Uint16 a_ui_I2CRxDataArray[2];

			// Read the checksum of the currently programmed firmware.
			while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
			if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, FLASH_CHECKSUM_ADDR, 2,a_ui_I2CRxDataArray)) == FAILURE)
			{
				return;
			}

			sprintf(Uart_Tx_Data_Array,"0x%04X",(a_ui_I2CRxDataArray[0] << 8) | (a_ui_I2CRxDataArray[1]));

			break;
		}

		case REPLY_WITH_FLASH_PROGRAMMING_STATUS:
		{
			Uint16 a_ui_I2CRxDataArray[1];

			// Read the checksum of the currently programmed firmware.
			while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
			if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, FLASH_PROG_STATUS_ADDRESS, 1,a_ui_I2CRxDataArray)) == FAILURE)
			{
				return;
			}

			sprintf(Uart_Tx_Data_Array,"0x%02X",a_ui_I2CRxDataArray[0]);

			break;
		}

		case REPLY_WITH_FLASH_FAILURE_STATUS_STORED_IN_EEPROM:
		{
			Uint16 a_ui_I2CRxDataArray[1];

			// Read the checksum of the currently programmed firmware.
			while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
			if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, FLASH_PROG_ERROR_ADDRESS, 1,a_ui_I2CRxDataArray)) == FAILURE)
			{
				return;
			}

			sprintf(Uart_Tx_Data_Array,"0x%02X",a_ui_I2CRxDataArray[0]);

			break;
		}

		case REPLY_WITH_FLASH_ILLEGAL_READ_VALUE_STORED_IN_EEPROM:
		{
			Uint16 a_ui_I2CRxDataArray[1];

			// Read the checksum of the currently programmed firmware.
			while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
			if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, FLASH_PROG_STATUS_ILLEGAL_READ_ADDRESS, 1,a_ui_I2CRxDataArray)) == FAILURE)
			{
				return;
			}

			sprintf(Uart_Tx_Data_Array,"0x%02X",a_ui_I2CRxDataArray[0]);

			break;
		}


		default:
			break;
	}

	if(Uart_Tx_Data_Array[0] != '\0')	// Something to be transmitted
	{
		// Reply with checksum if user has included $ in his Command/Query
		// Only Last two nibbles of the checksum are included.
		String_length = strlen(Uart_Tx_Data_Array);
		if(Global_Flags.fg_Checksum_Included_In_Uart_Comm == TRUE)
		{
			// calculate checksum
			for(i = 0; i < String_length; i++)
			{
				Checksum += Uart_Tx_Data_Array[i];
			}
			sprintf(Uart_Tx_Data_Array,"%s$%X\r\n",Uart_Tx_Data_Array,(Checksum&0x00FF));	// append $checksum\n
			Global_Flags.fg_Checksum_Included_In_Uart_Comm = FALSE;
		}
		else
		{
			Uart_Tx_Data_Array[String_length] = '\r';					// append line feed
			Uart_Tx_Data_Array[String_length + 1] = '\n';				// append carriage return
			Uart_Tx_Data_Array[String_length + 2] = '\0';				// append null character
		}
	}

	// The response will be generated when asked when the communication with the power supply is either COUPLED or OPEN.
	// But the power supply must send the response only when OPEN and not in COUPLED_COMMUNICATION_MODE.
	if(Global_Flags.fg_Uart_Comm_with_this_PSU != COUPLED_COMMUNICATION_MODE)
	{
		// Enqueue in buffer only in Communication OPEN mode.
		ei_vEnqueueOutputMessageInTransmitBuffer(Uart_Tx_Data_Array);
	}
	else	// Clear Uart_Tx_Data
	{
		// Clear Tx Data everytime while leaving.
		Uart_Tx_Data_Array[0] = '\0';
	}
}

//#####################	Uart Transmit Driver ######################################################################
// Function Name: ei_vUartFifoTx
// Return Type: void
// Arguments:   void
// Description: Put characters in the uart buffer. Note that even though the buffer size in Uart module of 2808 is
// 				16, we should use only 4 for compatibility with Piccollo series.
//#################################################################################################################
void ei_vUartFifoTx()
{
	Uint16 Current_Buffer_Status = 0;

	Current_Buffer_Status = SciaRegs.SCIFFTX.bit.TXFFST; // copy the value of the buffer status
	if(TRANSMIT_BUFFER_NOT_EMPTY)
	{
		if(Transmit_Circular_Buffer.i_Front == EMPTY_QUEUE_INDEX)
		{
			Transmit_Circular_Buffer.i_Front++;
		}
		while((Current_Buffer_Status != UART_BUFFER_FULL) && (TILL_LAST_TRANSMIT_BUFFER_ITEM_TRANSMITTED))
		{
			ENABLE_UART_TRANSMISSION;	// Pull Trans enable high.
			SciaRegs.SCITXBUF = Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Front];
			Current_Buffer_Status = SciaRegs.SCIFFTX.bit.TXFFST;
			INCREMENT_TRANSMIT_BUFFER(Transmit_Circular_Buffer.i_Front);
		}

		// Last character yet to be transmitted. Wait till buffer is free but do not use while loops
		if((LAST_TRANSMIT_BUFFER_ITEM_TRANSMITTED)&&(Current_Buffer_Status != UART_BUFFER_FULL))
		{
			SciaRegs.SCITXBUF = Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Front];
			Transmit_Circular_Buffer.i_Rear = EMPTY_QUEUE_INDEX;
			Transmit_Circular_Buffer.i_Front = EMPTY_QUEUE_INDEX;

			// No more messages valid
			Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_Message_Available = CLEARED;

			DISABLE_UART_TRANSMISSION;	// Pull Trans enable low.
		}
	}

	// Whenever the output queue empty,these check needs to be performed.

	// OPC
	if((Global_Flags.fg_Query_For_Opc == TRUE) && (Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.bit.bt_Operation_Complete == SET))
	{
//		We need to push these characters in the Output buffer when operation completed and its query detected.
//		See comments section of ei_uiMatchCommonCommands() for details on OPC.

		Transmit_Circular_Buffer.i_Rear++;
		Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Rear] = OPERATION_COMPLETE_ASCII_CHARCTER;
		Transmit_Circular_Buffer.i_Rear++;
		Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Rear] = '\r';
		Transmit_Circular_Buffer.i_Rear++;
		Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Rear] = '\n';

		Global_Flags.fg_Query_For_Opc = FALSE;
	}

	// SRQ
	if(Global_Flags.fg_Uart_SRQ_Tranmsit == TRUE)
	{
//		We need to push these characters in the Output buffer when an SRQ has to be transmitted.
//		SRQ Format: !nn\r\n

		Transmit_Circular_Buffer.i_Rear++;
		Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Rear] = '!';
		Transmit_Circular_Buffer.i_Rear++;
		Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Rear] = (Reference.ui_Set_Address/10) + ASCII_FOR_0;
		Transmit_Circular_Buffer.i_Rear++;
		Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Rear] = (Reference.ui_Set_Address%10) + ASCII_FOR_0;
		Transmit_Circular_Buffer.i_Rear++;
		Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Rear] = '\r';
		Transmit_Circular_Buffer.i_Rear++;
		Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Rear] = '\n';

		Global_Flags.fg_Uart_SRQ_Tranmsit = FALSE;
	}
}

//#####################	Error checker #############################################################################
// Function Name: ei_uiUartParameterErrorChecker
// Return Type: Uint16
// Arguments:   Uint16 Paramter_Type
// Description: Takes the type of command as the argument and then depending on the command.
//				NR1-Integer
//				NR2-Decimal Point Number
//				NR3-Explicit Decimal point and exponent
//				NRf-Includes all of the above
//				NRf_PLUS - Includes all of the above and MIN,MAX

// Reports the following Erros
//		1. -101 Illegal Parameter
//		2. -104 Data Type Error
//		3. -109 Missing Parameter
//
//		Dicitonary: CR --> Carriage Return
//#################################################################################################################
static Uint16 ei_uiUartParameterErrorChecker(Int8 *Parameter, Uint16 ui_Parameter_Type)
{
	// 1. Check for Missing paramter. The Very first location is '\r';
	if(*Parameter == '\r')
	{
		ei_vEnqueueCommandErrorInErrorQueue(EM109);
		return VALID_PARAMETER_NOT_FOUND;
	}

	// 2. Check for MAX
	if(strcmp((const Int8*)Parameter,MAX_WITH_CR) == MATCH_FOUND)
	{
		// Any other Parameter type raises Data Type Error
		if(ui_Parameter_Type != NRF_PLUS)
		{
			ei_vEnqueueCommandErrorInErrorQueue(EM104);
			return VALID_PARAMETER_NOT_FOUND;
		}
		else
		{
			return PARAMETER_MAX;
		}
	}

	// 3. Check for MIN
	if(strcmp((const Int8*)Parameter,MIN_WITH_CR) == MATCH_FOUND)
	{
		// Any other Parameter type raises Data Type Error
		if(ui_Parameter_Type != NRF_PLUS)
		{
			ei_vEnqueueCommandErrorInErrorQueue(EM104);
			return VALID_PARAMETER_NOT_FOUND;
		}
		else
		{
			return PARAMETER_MIN;
		}
	}

	// Check for Illegal Characters depending on the Command Type
	// Nr1 Takes only + and numerical characters
	if(ui_Parameter_Type == NR1)
	{
		while(*Parameter != '\r')
		{
			if((isdigit(*Parameter) == FALSE) && (*Parameter != '+'))
			{
				// The below characters are allowed but not for NR1
				if((*Parameter == '.') || (*Parameter == 'E') || (*Parameter == '-'))
				{
					ei_vEnqueueCommandErrorInErrorQueue(EM104);
					return VALID_PARAMETER_NOT_FOUND;
				}
				else	// Some illegal charcter not supported by any parameter
				{
					ei_vEnqueueCommandErrorInErrorQueue(EM101);
					return VALID_PARAMETER_NOT_FOUND;
				}
			}
			Parameter++;
		}
		return VALID_PARAMETER;
	}

	/*// Nr2 Supports only decimal point and +
	if(ui_Parameter_Type == NR2)
	{
		while(*Parameter != '\r')
		{
			if((isdigit(*Parameter) == FALSE) && (*Parameter != '+') && (*Parameter != '.'))
			{
				// The below characters are allowed but not for NR2
				if((*Parameter == 'E') || (*Parameter == '-'))
				{
					ei_vEnqueueCommandErrorInErrorQueue(EM104);
					return VALID_PARAMETER_NOT_FOUND;
				}
				else	// Some illegal charcter not supported by any parameter
				{
					ei_vEnqueueCommandErrorInErrorQueue(EM101);
					return VALID_PARAMETER_NOT_FOUND;
				}
			}
			Parameter++;
		}
		return VALID_PARAMETER;
	}*/

	// Nr3,NRF and NRF+ Supports only decimal point ,Exponent forms and +. All three are the same except NRF_PLUS supports
	// MIN and MAX strings, whose processing has been already done.
	if((ui_Parameter_Type != NR1) && (ui_Parameter_Type != NR2))
	{
		while(*Parameter != '\r')
		{
			if((isdigit(*Parameter) == FALSE) && (*Parameter != '+') && (*Parameter != '.') && (*Parameter != 'E') && (*Parameter != '-'))
			{
				ei_vEnqueueCommandErrorInErrorQueue(EM101);
				return VALID_PARAMETER_NOT_FOUND;
			}
			Parameter++;
		}
		return VALID_PARAMETER;
	}

	// Should not come Here
	return VALID_PARAMETER_NOT_FOUND;
}

//#####################	Dequeue from Error Queue ##################################################################
// Function Name: ei_vDequeueFromErrorQueue
// Return Type: void
// Arguments:   Int8 *
// Description: Dequeue the element stored at offset 0 of the error queue and copy that to the output buffer. All
// the remaining elements in the queue need to be shifted up by 1 position. This function also checks if the error
// is empty and pushes "No Error" in the output buffer.

// 				The error is copied to the string passed in the argument
//#################################################################################################################
static void ei_vDequeueFromErrorQueue(Int8 *s_Storage)
{
	Uint16 i;
	if(ERROR_QUEUE_EMPTY)
	{
		strcpy(s_Storage,E00);
	}
	else	// Error Queue Not Empty
	{
		// Push the Top of the queue to Output Buffer
		strcpy(s_Storage,Error_Linear_Queue.a_s_p_ErrorBuffer[QUEUE_HEAD]);

		// Shift all the elements in the queue by 1
		for (i=0;i<Error_Linear_Queue.i_Rear;i++)
	     Error_Linear_Queue.a_s_p_ErrorBuffer[i]= Error_Linear_Queue.a_s_p_ErrorBuffer[i+1];

		//	    Point the rear to no fault
		Error_Linear_Queue.a_s_p_ErrorBuffer[Error_Linear_Queue.i_Rear] = (Int8 *)E00;
	    // Decrement Rear by 1
	    Error_Linear_Queue.i_Rear--;

	    if(Error_Linear_Queue.i_Rear == EMPTY_QUEUE_INDEX)
	    {
	    	// We need to clear system error in STATUS BYTE whenever user queries the last error messagein the queue.
	    	Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_System_Error = CLEARED;
	    }
	}
}

//#####################	Enqueue Command error in Error Queue #######################################################
// Function Name: ei_vEnqueueCommandErrorInErrorQueue
// Return Type: void
// Arguments:   char *ErrorString
// Description: Different Command Errors are enqueued in the Error Queue.The function takes the pointer to the
//				specific error thats needs to be enqueued as the argument. It also sets the Approriate
//				Status Byte and Service Request registers.
//#################################################################################################################
static void ei_vEnqueueCommandErrorInErrorQueue(const Int8 *ErrorString)
{
	Error_Linear_Queue.i_Rear++;	// Increment Error Queue Rear
	if(ERROR_QUEUE_OVERFLOW)
	{
		// Push "Queue overflow" error in the Queue
		Error_Linear_Queue.a_s_p_ErrorBuffer[ERROR_BUFFER_SIZE - 1] = (Int8 *)EM350;
		Error_Linear_Queue.i_Rear = ERROR_BUFFER_SIZE - 1;
	}
	else
	{
		// Push appropriate error in the queue
		Error_Linear_Queue.a_s_p_ErrorBuffer[Error_Linear_Queue.i_Rear] = (Int8*)ErrorString;
	}
	// Set the appropriate registers.
	Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.bit.bt_Command_Error = SET;
	Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_System_Error = SET;
}

//#####################	Enqueue execution error in Error Queue #######################################################
// Function Name: ei_vEnqueueExecutionErrorInErrorQueue
// Return Type: void
// Arguments:   char *ErrorString
// Description: Different Execution Errors are enqueued in the Error Queue. The function takes the pointer to the
//				specific error thats needs to be enqueued as the argument. It also sets the Approriate
//				Status Byte and Service Request registers.
//#################################################################################################################
static void ei_vEnqueueExecutionErrorInErrorQueue(const Int8 *ErrorString)
{
	Error_Linear_Queue.i_Rear++;	// Increment Error Queue Rear
	if(ERROR_QUEUE_OVERFLOW)
	{
		// Push "Queue overflow" error in the Queue
		Error_Linear_Queue.a_s_p_ErrorBuffer[ERROR_BUFFER_SIZE - 1] = (Int8 *)EM350;
		Error_Linear_Queue.i_Rear = ERROR_BUFFER_SIZE - 1;
	}
	else
	{
		// Push appropriate error in the queue
		Error_Linear_Queue.a_s_p_ErrorBuffer[Error_Linear_Queue.i_Rear] = (Int8*)ErrorString;
	}
	// Set the appropriate registers.
	Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.bit.bt_Execution_Error = SET;
	Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_System_Error = SET;
}

//#####################	Enqueue execution error in Error Queue ####################################################
// Function Name: ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue
// Return Type: void
// Arguments:   char *ErrorString
// Description: The Device Dependent errors other than Fault Regs are enqueued in the Error Queue. The function takes
//				the pointer to the specific error thats needs to be enqueued as the argument. It also sets the Approriate
//				Status Byte and Service Request registers.
//#################################################################################################################
void ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(const Int8 *ErrorString)
{
	Error_Linear_Queue.i_Rear++;	// Increment Error Queue Rear
	if(ERROR_QUEUE_OVERFLOW)
	{
		// Push "Queue overflow" error in the Queue
		Error_Linear_Queue.a_s_p_ErrorBuffer[ERROR_BUFFER_SIZE - 1] = (Int8 *)EM350;
		Error_Linear_Queue.i_Rear = ERROR_BUFFER_SIZE - 1;
	}
	else
	{
		// Push appropriate error in the queue
		Error_Linear_Queue.a_s_p_ErrorBuffer[Error_Linear_Queue.i_Rear] = (Int8*)ErrorString;
	}
	// Set the appropriate registers.
	Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.bit.bt_Device_Dependent_Error = SET;
	Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_System_Error = SET;
}

//#####################	Enqueue Fault Register Errors in Error Queue ##############################################
// Function Name: ei_vEnqueueFaultRegsInErrorQueue
// Return Type: void
// Arguments:   Uint16 Fault_Flag, char *ErrorString
// Description: Input the Device dependent Fault flag and the corresponding error message to be dequeued. The function will 
//				1. Check if the fault is being detected for the first time after it was cleared last. If so
//					then enqueue the fault.
//				2. If Not 1 then check if fault already exists in the queue. If it does not then enqueue fault
//				3. If the fault enqueued then set device dependent flag error.
//				4. If queue overflow then set that particular error
//#################################################################################################################
void ei_vEnqueueFaultRegsInErrorQueue(Uint16 Fault_Flag, const Int8 *ErrorString)
{
	int16 i = 0;
	if((Fault_Regs.FAULT_REGISTER.all & Fault_Flag) == FAULT_NOT_SET_PREVIOUSLY)
	{
		// Enter the fault
		Fault_Regs.FAULT_REGISTER.all |= Fault_Flag;
		Error_Linear_Queue.i_Rear++;	// Increment Error Queue Rear
		if(ERROR_QUEUE_OVERFLOW)
		{
			// Push "Queue overflow" error in the Queue
			Error_Linear_Queue.a_s_p_ErrorBuffer[ERROR_BUFFER_SIZE - 1] = (Int8 * )EM350;
			Error_Linear_Queue.i_Rear = ERROR_BUFFER_SIZE - 1;
		}
		else
		{
			// Push error in the queue
			Error_Linear_Queue.a_s_p_ErrorBuffer[Error_Linear_Queue.i_Rear] = (Int8 *)ErrorString;
		}
	}
	else	// Fault has been set already.
	{
		Error_Linear_Queue.i_Rear++;	// Increment Error Queue Rear
		
		// Search whether the error string exists in the queue already.
		for(i = 0; i <= Error_Linear_Queue.i_Rear - 1; i++)
		{
			if(Error_Linear_Queue.a_s_p_ErrorBuffer[i] == ErrorString)
			{
				// Break from the loop on finding a match.
				break;
			}
		}
		
		if(i == Error_Linear_Queue.i_Rear)
		{
			if(ERROR_QUEUE_OVERFLOW)
			{
				// Push "Queue overflow" error in the Queue
				Error_Linear_Queue.a_s_p_ErrorBuffer[ERROR_BUFFER_SIZE - 1] = (Int8 *)EM350;
				Error_Linear_Queue.i_Rear = ERROR_BUFFER_SIZE - 1;
			}
			else
			{
				// Does not exist. Enqueue it.
				Error_Linear_Queue.a_s_p_ErrorBuffer[Error_Linear_Queue.i_Rear] = (Int8 *)ErrorString;
			}
		}
		else
		{
//			Already exists
			Error_Linear_Queue.i_Rear--;
		}
	}
	// Set the appropriate registers.
	Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_REGISTER.bit.bt_Device_Dependent_Error = SET;
	Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_System_Error = SET;
}

//#####################	Output Queue Operartion ####################################################################
// Function Name: ei_vEnqueueOutputMessageInTransmitBuffer
// Return Type: void
// Arguments:   Int8 *MessageToBePushed
// Description: Pushes the message passed in the argument to a circular transmit buffer TRANSMIT_BUFFER_SIZE wide.
// 				The charcters in the output message are copied to the transmit buffer 1 by 1 till '\0' encountered.

//	NOTE: The Transmit buffer is not NULL Terminated.
//#################################################################################################################
void ei_vEnqueueOutputMessageInTransmitBuffer(Int8 *MessageToBePushed)
{
	// Buffer the present Output Message in the queue.
	if(MessageToBePushed[0] != '\0')
	{
		Uint16 i;
		for(i = 0; MessageToBePushed[i] != '\0'; i++)
		{
			// Increment Rear.
			INCREMENT_TRANSMIT_BUFFER(Transmit_Circular_Buffer.i_Rear);

			// Buffer Overflow Check. If the buffer is found to overflow before the complete message
			// can be enqueued, no action taken to dequeue partially buffered message. The user might receive
			// only the partial sting of the message that causedthe overflow.
			// It is not pssible to retrieve the message in anyway because Uart_Tx_Data could be overwritten
			// next time the SciOpeartion is called.
			// Buffer full error messages will always be set in time of overflow.
			if(OUTPUT_QUEUE_FULL)
			{
				ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(E342);

				// Take rear a step back to its previous location
				DECREMENT_TRANSMIT_BUFFER(Transmit_Circular_Buffer.i_Rear);
				break;
			}
			else	// BUFFER NOT FULL
			{
				Transmit_Circular_Buffer.a_c_TransmitBuffer[Transmit_Circular_Buffer.i_Rear] = MessageToBePushed[i];
			}
		}

		// Buffering complete
		MessageToBePushed[0] = '\0';
		Service_Request_Regs.STATUS_BYTE_REGISTER.bit.bt_Message_Available = SET;
	}
}
