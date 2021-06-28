#ifndef COMMAND_DEFS_H_
#define COMMAND_DEFS_H_

// This file contains all command related defs. Delete this if there aren't many commands. 
// No need to include this file separately. It is included in UserDefs

#include "string.h"
#include "stdio.h"
#include "ctype.h"
#include "limits.h"
#include "stdlib.h"
#include "math.h"

// SCPI SRQ related
#define BIT_6															0x40
#define MSS_RQS_BIT														BIT_6

// Uart transmit enable pin
#define ENABLE_UART_TRANSMISSION	GpioDataRegs.GPASET.bit.GPIO23 = 1
#define DISABLE_UART_TRANSMISSION	GpioDataRegs.GPACLEAR.bit.GPIO23 = 1

// Response Keys
#define REPLY_WITH_INSTRUMENT_ADDR	1
#define REPLY_WITH_SYSTEM_ERROR		2
#define REPLY_WITH_SOFTWARE_VERSION	3
#define REPLY_WITH_CALIBRATION_DATE	4
#define REPLY_WITH_POWER_ON_TIME	5

#define REPLY_WITH_STATE_OF_PSU_CONTROL									6	
#define REPLY_WITH_STD_EVENT_STATUS_ENABLE_REGISTER						7
#define REPLY_WITH_STD_EVENT_STATUS_REGISTER							8
#define REPLY_WITH_IDENTIFICATION_STRING								9
#define REPLY_WITH_INSTALLED_OPTIONS									10
#define REPLY_WITH_POWER_ON_STATE_CLEAR_SETTING							11
#define REPLY_WITH_SERVICE_REQUEST_ENABLE_REGISTER						12
#define REPLY_WITH_STATUS_BYTE_REGISTER									13
#define REPLY_WITH_MEASURED_DC_VOLTAGE									14
#define REPLY_WITH_MEASURED_DC_CURRENT									15
#define REPLY_WITH_MEASURED_DC_POWER									16
#define REPLY_WITH_PROGRAMMED_VOLTAGE									17
#define REPLY_WITH_MIN_PROGRAMMABLE_VOLTAGE								18
#define REPLY_WITH_MAX_PROGRAMMABLE_VOLTAGE								19
#define REPLY_WITH_PROGRAMMED_OVP										20
#define REPLY_WITH_MIN_PROGRAMMABLE_OVP									21
#define REPLY_WITH_MAX_PROGRAMMABLE_OVP									22
#define REPLY_WITH_PROGRAMMED_UVL										23
#define REPLY_WITH_MIN_PROGRAMMABLE_UVL									24
#define REPLY_WITH_MAX_PROGRAMMABLE_UVL									25
#define REPLY_WITH_UNDER_VOLTAGE_PROTECTION_MODE						26
#define REPLY_WITH_PROGRAMMED_CURRENT									27
#define REPLY_WITH_MIN_PROGRAMMABLE_CURRENT								28
#define REPLY_WITH_MAX_PROGRAMMABLE_CURRENT								29
#define REPLY_WITH_OUTPUT_STATE											30
#define REPLY_WITH_PON_STATE											31
#define REPLY_WITH_OUTPUT_MODE											32
#define REPLY_WITH_SET_PROTECTION_DELAY									33
#define REPLY_WITH_SET_FOLDBACK_MODE									34
#define REPLY_WITH_RATED_VOLTAGE										35
#define REPLY_WITH_RATED_CURRENT										36
#define REPLY_WITH_VOLTAGE_CALIBRATION_PARAMETERS						37
#define REPLY_WITH_VOLTAGE_DISPLAY_CALIBRATION_PARAMETERS				38
#define REPLY_WITH_SPECIFIED_VOLTAGE									39
#define REPLY_WITH_SPECIFIED_CURRENT									40
#define REPLY_WITH_CURRENT_CALIBRATION_PARAMETERS						41
#define REPLY_WITH_CURRENT_DISPLAY_CALIBRATION_PARAMETERS				42
#define REPLY_WITH_CALIBRATION_COUNT									43
#define REPLY_WITH_PSU_MODEL_NUMBER										44
#define REPLY_WITH_OPERATION_CONDITION_ENABLE_REGISTER					45
#define REPLY_WITH_OPERATION_CONDITION_EVENT_REGISTER					46
#define REPLY_WITH_OPERATION_CONDITION_REGISTER							47
#define REPLY_WITH_QUESTIONABLE_CONDITION_ENABLE_REGISTER				48
#define REPLY_WITH_QUESTIONABLE_CONDITION_EVENT_REGISTER				49
#define REPLY_WITH_QUESTIONABLE_CONDITION_REGISTER						50
#define REPLY_WITH_STATE_OF_DISPLAY										51
#define REPLY_WITH_FLASH_CHECKSUM_STORED_IN_EEPROM						52
#define REPLY_WITH_FLASH_PROGRAMMING_STATUS								53
#define REPLY_WITH_MEASURED_TEMPERATURE									54
#define REPLY_WITH_FLASH_FAILURE_STATUS_STORED_IN_EEPROM				55
#define REPLY_WITH_FLASH_ILLEGAL_READ_VALUE_STORED_IN_EEPROM			56

// ASCII Values
#define ASCII_FOR_0					48
#define ASCII_FOR_9					57
#define ASCII_FOR_A					65
#define ASCII_FOR_F					70

#define OPERATION_COMPLETE_ASCII_CHARCTER								49

// Error Checker Defs
#define OUTPUT_CONTROL_COMMAND		0
#define HEXADECIMAL_COMMANDS		1
#define	OTHER_COMMANDS				2
#define VALID_PARAMETER_NOT_FOUND	0
#define VALID_PARAMETER				1
#define PARAMETER_MAX				2
#define PARAMETER_MIN				3

// Argument Types for different functions
#define NR1							1
#define NR2							2
#define NR3							3
#define NRF							4
#define NRF_PLUS					5

#define V_PROCESSING				1
#define C_PROCESSING				2
#define OUT_PROCESSING				3

#define RATED_VOLTAGE				1
#define RATED_CURRENT				2
#define SPECIFIED_VOLTAGE			3
#define SPECIFIED_CURRENT			4

// Genearal Comm Defs
#define	MATCH_FOUND					0
#define PARAMETER_LENGTH_ALLOWED    12
#define PARAMTER_LENGTH_FOR_STATUS_CONTROL_COMMANDS						4
#define DATE_LENGTH					10
#define MAX_IDN_LENGTH				39
#define SN_LENGTH					21
#define COMPLETE_COMMAND_RECEIVED	1
#define NEXT_CHARCTER				1
#define DECIMAL						0

// Which Register to set Values
#define STD_EVENT_STATUS_ENABLE_REGS									0 
#define SERVICE_REQUEST_ENABLE_REGS										1
#define OPERATION_CONDITION_ENABLE_REGS									2
#define QUESTIONABLE_CONDITION_ENABLE_REGS								3

// Special characters
#define BACKSPACE_CHARACTER												8
#define CARRIAGE_RETURN													13
#define DOLLAR_CHARACTER												36
#define LINE_FEED_CHARACTER												10
#define COLON_CHARACTER													58
#define SPACE_CHARACTER													32

// 
#define EXTENDED_ASCII_SET			128

//SCPI Comparison Defs
#define MATCH_BOOL_FOR_LOGIC_TRUE(x)									if((strcmp(x,"1\r")== MATCH_FOUND) || (strcmp(x,"ON\r")== MATCH_FOUND))
#define MATCH_BOOL_FOR_LOGIC_FALSE(x)									if((strcmp(x,"0\r")== MATCH_FOUND) || (strcmp(x,"OFF\r")== MATCH_FOUND))

// Character length of specific english words
#define LENGTH_OF_GLOBAL												6
#define LENGTH_OF_GLOB													4
#define LENGTH_OF_OUTPUT												6
#define LENGTH_OF_OUTP													4
#define LENGTH_OF_STATE													5
#define LENGTH_OF_STAT													4
#define LENGTH_OF_REMOTE												6
#define LENGTH_OF_REM													3
#define LENGTH_OF_VOLTAGE												7								
#define LENGTH_OF_VOLT													4
#define LENGTH_OF_CURRENT												7
#define LENGTH_OF_CURR													4
#define LENGTH_OF_COMMON_COMMANDS_WITH_SP								5
#define LENGTH_OF_POWER													5
#define LENGTH_OF_POW													3
#define LENGTH_OF_ENARKA_AUM											10

// Queue Concepts
#define EMPTY_QUEUE_INDEX												-1
#define TILL_LAST_TRANSMIT_BUFFER_ITEM_TRANSMITTED						Transmit_Circular_Buffer.i_Front != Transmit_Circular_Buffer.i_Rear
#define LAST_TRANSMIT_BUFFER_ITEM_TRANSMITTED							Transmit_Circular_Buffer.i_Front == Transmit_Circular_Buffer.i_Rear
#define OUTPUT_QUEUE_FULL												Transmit_Circular_Buffer.i_Rear == Transmit_Circular_Buffer.i_Front
#define START_OF_QUEUE													0
#define TRANSMIT_BUFFER_NOT_EMPTY										Transmit_Circular_Buffer.i_Rear != EMPTY_QUEUE_INDEX
#define ERROR_QUEUE_EMPTY												Error_Linear_Queue.i_Rear == EMPTY_QUEUE_INDEX
#define QUEUE_HEAD														0
#define ERROR_QUEUE_OVERFLOW											Error_Linear_Queue.i_Rear == ERROR_BUFFER_SIZE
#define RECEIVE_BUFFER_EMPTY											Receive_Circular_Buffer.i_Front == Receive_Circular_Buffer.i_Rear
#define RECEIVE_BUFFER_FULL												Receive_Circular_Buffer.i_Rear == Receive_Circular_Buffer.i_Front
#define RECEIVE_BUFFER_NOT_EMPTY										Receive_Circular_Buffer.i_Rear != EMPTY_QUEUE_INDEX
#define IF_NON_STANDARD_ASCII_RECIEVED									Receive_Circular_Buffer.a_c_ReceiveBuffer[Receive_Circular_Buffer.i_Front] > 127

#define INCREMENT_RECEIVED_BUFFER(x)	if(x == (RECEIVE_BUFFER_SIZE - 1)) \
										{ \
											x = START_OF_QUEUE; \
										} \
										else \
										{ \
											x++; \
										}
#define DECREMENT_RECEIVED_BUFFER(x)	if(x == (EMPTY_QUEUE_INDEX+1)) \
										{ \
											x = (RECEIVE_BUFFER_SIZE - 1); \
										} \
										else \
										{ \
											x--; \
										}
#define INCREMENT_TRANSMIT_BUFFER(x)	if(x == (TRANSMIT_BUFFER_SIZE - 1)) \
										{ \
											x = START_OF_QUEUE; \
										} \
										else \
										{ \
											x++; \
										}
#define DECREMENT_TRANSMIT_BUFFER(x)		if(x == (EMPTY_QUEUE_INDEX+1)) \
										{ \
											x = (TRANSMIT_BUFFER_SIZE - 1); \
										} \
										else \
										{ \
											x--; \
										}
#endif /*COMMAND_DEFS_H_*/
