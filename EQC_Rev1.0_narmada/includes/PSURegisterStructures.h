#ifndef PSU_REGISTER_STRUCTURES_H_
#define PSU_REGISTER_STRUCTURES_H_


// PSU Register processing.
// DESC: The PSU registers have been implemented according to SCPI format.
//       SCPI requires the implementation of following registers
//		1. Questionable Status Registers (Henceforth known as Fault Register)
//		2. Operation condition Registers
//		3. StandardEvent Status Register
//		4. Service Request Register

// Each register has its enable, event (condition register in case of Operation and Fault registers).
// Each condition register bits (Operation and Fault Register), Standard Event Status register bits are set in appropriate locations
// in the code when Fault or a particular condition occurs
// 1. All Fault Registers are set mostly in CheckFaults and some are set in respective locations whenever fault detected.
// 2. The Operation Condition register bits are set at different places throughut the code whene the particular condition
//    denoted by the bit occurs.
// 3. Event Status Register bits are Set mostly when an error is pushed into the error queue. It also has Message_Valid and PON bit
//    set elsewhere.
// 4. The Status Byte bits are set in the interrupt for Fault and Operation bits. Rest are set elsewhere.
// 5. The Enable register allow the conditions to be latched to event register.
// 6. SRQ Raised when STB byte changes state from all 0s to atleast 1 bit set. STB is not cleared on reading. But it's
//	  individual bits are cleared reading the reason for which that bit got set. (Except MAV.It's cleared automatically.)
// 
// FOR EXACT IMPLEMTATION OF REGISTERS IT IS BETTER TO READ THE ENARKA PSU MANUAL. (Doc No: XXXX, Relevant Pages: XX to XX) 
 
// Fault Register
struct FAULT_REG_BITS
{				
	Uint16 bt_AC_Fault:1;							// Bit 0
	Uint16 bt_Otp_Fault:1;							// Bit 1
	Uint16 bt_Fold_Fault:1;							// Bit 2
	Uint16 bt_Ovp_Fault:1;							// Bit 3
	Uint16 bt_ShutOff_Fault:1;						// Bit 4
	Uint16 bt_Out_Fault:1;							// Bit 5
	Uint16 bt_Interlock_Fault:1;					// Bit 6
	Uint16 bt_Fan_Fault:1;							// Bit 7
	Uint16 bt_Reserved:1;							// Bit 8
	Uint16 bt_PFC_DC_Bus_Fault:1;					// Bit 9
	Uint16 bt_Internal_Communication_Fault:1;		// Bit 10	I2C, SPI
	Uint16 bt_Eeprom_Checksum_Failure:1;			// Bit 11
	Uint16 bt_On_Board_Adc_Failure:1;				// Bit 12
	Uint16 bt_Illegal_Math_Performed:1;				// Bit 13
	Uint16 bt_CANcomm_Failure:1;			        // Bit 14  CAN
	Uint16 bt_Reserved_3:1;							// Bit 15
};

union FAULT_REGS
{
	Uint16 all;
	struct FAULT_REG_BITS bit;
};
	
struct FAULT
{	
	union FAULT_REGS	FAULT_REGISTER;		// Internal Fault Flag Register
	union FAULT_REGS	FAULT_EVENT;
//	union FAULT_REGS	FAULT_EVENT_PREVIOUS;
	union FAULT_REGS	FAULT_ENABLE;
};

// Operation condition Register
struct OPERATION_CONDITION_REG_BITS
{
	Uint16 bt_Constant_Voltage:1;					// Bit 0
	Uint16 bt_Constant_Current:1;					// Bit 1
	Uint16 bt_No_Fault:1;							// Bit 2
	Uint16 bt_Reserved_1:1;						// Bit 3
	Uint16 bt_Auto_start_Enabled:1;				// Bit 4
	Uint16 bt_Fold_Enabled:1;						// Bit 5
	Uint16 bt_Local_Mode:1;						// Bit 6
	Uint16 bt_Reserved:1;							// Bit 7
	Uint16 bt_Reserved_2:1;						// Bit 8
	Uint16 bt_Reserved_3:1;						// Bit 9
	Uint16 bt_Reserved_4:1;						// Bit 10
	Uint16 bt_Current_Foldback_Mode:1;				// Bit 11
	Uint16 bt_Analog_Control:1;					// Bit 12
	Uint16 bt_Reserved_5:1;						// Bit 13
	Uint16 bt_Reserved_6:1;						// Bit 14
	Uint16 bt_Reserved_7:1;						// Bit 15
};

union OPERATION_CONDITION_REG
{
	Uint16 all;
	struct OPERATION_CONDITION_REG_BITS bit;
};

struct OPERATION_CONDITION
{
	union OPERATION_CONDITION_REG	OPERATION_CONDITION_REGISTER;		// Internal Operation Condition Flag Register
	union OPERATION_CONDITION_REG	OPERATION_CONDITION_EVENT;
//	union OPERATION_CONDITION_REG	OPERATION_CONDITION_EVENT_PREVIOUS;
	union OPERATION_CONDITION_REG	OPERATION_CONDITION_ENABLE;
};

// Standard Event Status Register
struct STANDARD_EVENT_STATUS_REG_BITS
{
	Uint16 bt_Operation_Complete:1;				// Bit 0
	Uint16 bt_Query_Error:1;						// Bit 1
	Uint16 bt_Device_Dependent_Error:1;			// Bit 2
	Uint16 bt_Execution_Error:1;					// Bit 3
	Uint16 bt_Command_Error:1;						// Bit 4
	Uint16 bt_User_Request:1;						// Bit 5
	Uint16 bt_Power_On:1;							// Bit 6
	Uint16 bt_Reserved:1;							// Bit 7
};

union STANDARD_EVENT_STATUS_REG
{
	Uint16 all;
	struct STANDARD_EVENT_STATUS_REG_BITS bit;
};

struct STANDARD_EVENT_STATUS
{
	union STANDARD_EVENT_STATUS_REG	STANDARD_EVENT_STATUS_REGISTER;					// Internal Standard Event Flag Register
//	union STANDARD_EVENT_STATUS_REG	STANDARD_EVENT_PREVIOUS_STATUS_REGISTER;		// Previous Standard Event Flag Register
	union STANDARD_EVENT_STATUS_REG	STANDARD_EVENT_STATUS_ENABLE;
};

// Service Request Register
struct SERVICE_REQUEST_REG_BITS
{
	Uint16 bt_Busy:1;								// Bit 0
	Uint16 bt_Reserved:1;							// Bit 1
	Uint16 bt_System_Error:1;						// Bit 2
	Uint16 bt_Questionable_Summary:1;				// Bit 3
	Uint16 bt_Message_Available:1;					// Bit 4
	Uint16 bt_Standard_Event_Summary:1;			// Bit 5
	Uint16 bt_Service_Request:1;					// Bit 6
	Uint16 bt_Operation_Summary:1;					// Bit 7
};

union SERVICE_REQUEST_REG
{
	Uint16 all;
	struct SERVICE_REQUEST_REG_BITS bit;
};

struct SERVICE_REQUEST
{
	union SERVICE_REQUEST_REG	STATUS_BYTE_REGISTER;				// Internal Service Request Flag Register
	union SERVICE_REQUEST_REG	STATUS_BYTE_PREVIOUS_REGISTER;		// Previous Service Request Flag Register
	union SERVICE_REQUEST_REG	SERVICE_REQUEST_ENABLE;
};

#endif /*PSU_REGISTER_STRUCTURES_H_*/
