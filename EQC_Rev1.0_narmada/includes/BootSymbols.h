#ifndef BOOTSYMBOLS_H_
#define BOOTSYMBOLS_H_

#define	CHECKSUM_EXPECTED 			0xFFFFB926
#define FLASH_RANGE       			65535L
#define PROGRAMMABLE_FLASH_RANGE	57343L
#define DEST_ADDR		  			0x003E8000
#define BLOCK_SIZE					PROGRAMMABLE_FLASH_RANGE + 1L
#define RAM_BUFFER_SIZE				4096
#define BOOT_SWITCH_OUTPUT_OFF		(GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1)
#define BOOT_PULL_PS_OK_LOW			(GpioDataRegs.GPBCLEAR.bit.GPIO53 = 1)
#define DISABLE_INT					0
#define RESET_TRANSMIT_RECEIVE 		1
#define ENABLE_FIFO_ENHANCEMENTS	1
#define UART_BUFFER_SIZE			4
#define UART_BUFFER_FULL			4
#define UART_BUFFER_EMPTY			0
#define HOLD_IN_RESET				0
#define ASCII_FOR_0					48
#define ASCII_FOR_9					57
#define ASCII_FOR_A					65
#define ASCII_FOR_F					70
#define BOOT_COMMUNICATION_ESTABLISHED_SUCCESSFULLY						97	// 'a'
#define CHARACTER_LIES_BETWEEN_0_AND_9									(c_ChecksumCharacter >= ASCII_FOR_0) && (c_ChecksumCharacter <= ASCII_FOR_9) 
#define CHARACTER_LIES_BETWEEN_A_AND_F									(c_ChecksumCharacter >= ASCII_FOR_A) && (c_ChecksumCharacter <= ASCII_FOR_F)
#define FLASH_PROGRAMMING_SUCCESSFULL									0xAA 
#define BOOT_COMMAND_RECEIVED											0xCC
#define EEPROM_24LC256_ADDRESS											0x0050 // 7 bit: 1010000 
#define FLASH_PROG_STATUS_ADDRESS										0x0073
#define FLASH_PROG_STATUS_ILLEGAL_READ_ADDRESS							0x0079
#define FLASH_PROG_ERROR_ADDRESS										0x007A
#define SUCCESS						1
#define FAILURE						0
#define	EEPROM_WRITE_CYCLE_DELAY	DELAY_US(5000)
#define I2C_DELAY					500
#define BOOT_ENABLE_UART_TRANSMISSION									GpioDataRegs.GPASET.bit.GPIO17 = 1
#define BOOT_DISABLE_UART_TRANSMISSION									GpioDataRegs.GPACLEAR.bit.GPIO17 = 1
#define I2C_WAIT_UNTIL(x)	{ \
								timeout_cnt_2 = 0; \
								while((x)) \
								{ \
									timeout_cnt_2++; \
									if (timeout_cnt_2 >= I2C_DELAY) \
										{ \
										return FAILURE; \
										} \
								} \
							}
							
// Error Defs for storing in EEPROM
#define FLASH_CHECKSUM_FAILURE		0xC1
#define COMM_NOT_ESTABLISHED		0xC2
#define COMMUNICATION_FAILURE		0xC3
#define FILE_SIZE_MISMATCH_FAILURE										0xC4

// Error given by the flash Apis for storing in EEPROM
#define HEX_STATUS_FAIL_CSM_LOCKED										0x10
#define HEX_STATUS_FAIL_REVID_INVALID									0x11
#define HEX_STATUS_FAIL_ADDR_INVALID									0x12
#define HEX_STATUS_FAIL_INCORRECT_PARTID								0x13
#define HEX_STATUS_FAIL_API_SILICON_MISMATCH							0x14
#define HEX_STATUS_FAIL_NO_SECTOR_SPECIFIED								0x20
#define HEX_STATUS_FAIL_PRECONDITION									0x21
#define HEX_STATUS_FAIL_ERASE											0x22
#define HEX_STATUS_FAIL_COMPACT											0x23
#define HEX_STATUS_FAIL_PRECOMPACT										0x24
#define HEX_STATUS_FAIL_PROGRAM											0x30
#define HEX_STATUS_FAIL_ZERO_BIT_ERROR									0x31
#define HEX_UNKNOWN_ERROR												0x00

			// MAX6958/59 defs
			// segment led with 4 digits & keyscan
#define	MAX6959A_ADDRESS			0x0038
#define	MAX6958B_ADDRESS			0x0039

#define	CONFIGURATION_REGISTER_ADDRESS									0x04
#define	DECODE_MODE_REGISTER_ADDRESS									0x01
#define	INTENSITY_REGISTER_ADDRESS										0x02
#define	PORT_CONFIGURATION_REGISTER_ADDRESS								0x06
#define	SCAN_LIMIT_REGISTER_ADDRESS										0x03
#define	DIGIT_DISPLAY_BEGIN_ADDRESS										0x20

#define	DIGIT_DISPLAY_DECIMAL_POINT_AND_SEGMENT_LED_ADDRESS				0x24

#define	KEY_DEBOUNCE_REGISTER_ADDRESS									0x08
#define	KEY_PRESS_REGISTER_ADDRESS										0x0C
#define	SEGMENT_LED_REGISTER_ADDRESS									0x24

#define	ENABLE_DIG0_TO_DIG3			0x0F
#define DISABLE_DIG0_TO_DIG3		0x00
#define	SEGMENT9_FUNCTION			0x80
#define	ENABLE_KEYSCAN0				0x10
#define	ENABLE_KEYSCAN1				0x08
#define	LEVEL_SELECTOR				0x0E
#define	GLOBAL_CLEAR_MAX			0x20
#define	NORMAL_OPERATION			0x01
#define	SHUTDOWN_MAXIC				0x00

#define	PERMIT_ACTUAL_VALUES_IN_DIGIT_REGISTER							0x0F

#define	NO_HEXADECIMAL_DECODE_MODE_IN_DIGIT_REGISTER					0X00
#define NUMBER_OF_DIGITS_DISPLAY	4

#define DIGIT_REGISTER_MAPPING_FOR_DISPLAY_BLANK						0x00
#define DIGIT_REGISTER_MAPPING_FOR_F									0x47
#define DIGIT_REGISTER_MAPPING_FOR_L									0x0E
#define DIGIT_REGISTER_MAPPING_FOR_5									0x5B
#define DIGIT_REGISTER_MAPPING_FOR_H									0x37
#define DIGIT_REGISTER_MAPPING_FOR_T									0x0F

#pragma DATA_SECTION(EraseStatus,"Boot_Symbols");
FLASH_ST EraseStatus;
#pragma DATA_SECTION(ProgStatus,"Boot_Symbols");
FLASH_ST ProgStatus;
#pragma DATA_SECTION(VerifyStatus,"Boot_Symbols");
FLASH_ST VerifyStatus;

#pragma DATA_SECTION(BlockBuffer1, "BlockTransferBuffer1");
#pragma DATA_SECTION(BlockBuffer2, "BlockTransferBuffer2");
Uint16 	BlockBuffer1[4096];
Uint16 	BlockBuffer2[4096];

#pragma DATA_SECTION(BlockBufferPtr,"Boot_Symbols");
Uint16	*BlockBufferPtr;
#pragma DATA_SECTION(BlockRemain,"Boot_Symbols");
Uint32	BlockRemain;
#pragma DATA_SECTION(BufferIndexMax,"Boot_Symbols");
Uint16	BufferIndexMax;
#pragma DATA_SECTION(MyCallbackCounter,"Boot_Symbols");
Uint32 MyCallbackCounter; // Just increment a counter in the callback function
#pragma DATA_SECTION(tempL,"Boot_Symbols");
Uint32	tempL;
#pragma DATA_SECTION(i,"Boot_Symbols");
Uint16 	i=0;
#pragma DATA_SECTION(ul_ChecksumReceived,"Boot_Symbols");
Uint32  ul_ChecksumReceived;
#pragma DATA_SECTION(timeout_cnt_2,"Boot_Symbols");
volatile Uint16 timeout_cnt_2; 
#pragma DATA_SECTION(a_ui_I2cTxDataArray,"Boot_Symbols");
Uint16 a_ui_I2cTxDataArray[4]; 
#pragma DATA_SECTION(a_ui_I2C_Rx_Data_Array,"Boot_Symbols");
Uint16 a_ui_I2C_Rx_Data_Array[1]; 

#pragma DATA_SECTION(PERIOD,"Boot_Symbols");
char PERIOD[] = ".";
#pragma DATA_SECTION(B00,"Boot_Symbols");
char B00[] = "B00\r\n";
#pragma DATA_SECTION(B01,"Boot_Symbols");
char B01[] = "B01\r\n";
#pragma DATA_SECTION(B02,"Boot_Symbols");
char B02[] = "B02\r\n";
#pragma DATA_SECTION(B03,"Boot_Symbols");
char B03[] = "B03\r\n";
#pragma DATA_SECTION(B04,"Boot_Symbols");
char B04[] = "B04\r\n";
#pragma DATA_SECTION(C01,"Boot_Symbols");
char C01[] = "C01\r\n";
#pragma DATA_SECTION(C99,"Boot_Symbols");
char C99[] = "C99\r\n";
#pragma DATA_SECTION(C02,"Boot_Symbols");
char C02[] = "C02\r\n";
#pragma DATA_SECTION(C03,"Boot_Symbols");
char C03[] = "C03\r\n";
#pragma DATA_SECTION(C04,"Boot_Symbols");
char C04[] = "C04\r\n";
#pragma DATA_SECTION(E10,"Boot_Symbols");
char E10[] = "E10\r\n";
#pragma DATA_SECTION(E11,"Boot_Symbols");
char E11[] = "E11\r\n";
#pragma DATA_SECTION(E12,"Boot_Symbols");
char E12[] = "E12\r\n";
#pragma DATA_SECTION(E13,"Boot_Symbols");
char E13[] = "E13\r\n";
#pragma DATA_SECTION(E14,"Boot_Symbols");
char E14[] = "E14\r\n";
#pragma DATA_SECTION(E20,"Boot_Symbols");
char E20[] = "E20\r\n";
#pragma DATA_SECTION(E21,"Boot_Symbols");
char E21[] = "E21\r\n";
#pragma DATA_SECTION(E22,"Boot_Symbols");
char E22[] = "E22\r\n";
#pragma DATA_SECTION(E23,"Boot_Symbols");
char E23[] = "E23\r\n";
#pragma DATA_SECTION(E24,"Boot_Symbols");
char E24[] = "E24\r\n";
#pragma DATA_SECTION(E30,"Boot_Symbols");
char E30[] = "E30\r\n";
#pragma DATA_SECTION(E31,"Boot_Symbols");
char E31[] = "E31\r\n";
#pragma DATA_SECTION(E_UNKNOWN,"Boot_Symbols");
char E_UNKNOWN[] = "E00\r\n";

#endif /*BOOTSYMBOLS_H_*/
