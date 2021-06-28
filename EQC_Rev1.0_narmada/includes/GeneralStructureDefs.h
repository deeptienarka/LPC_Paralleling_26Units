#ifndef STRUCTURE_DECLARATIONS_H_
#define STRUCTURE_DECLARATIONS_H_

#define TOTAL_NO_OF_EVENTS			126
#define ERROR_BUFFER_SIZE			10
#define TRANSMIT_BUFFER_SIZE		250
#define MAX_UART_ARRAY_SIZE			80
#define RECEIVE_BUFFER_SIZE			250

// ENUMS
typedef enum
{
	EI_ST_STANDBY,
	EI_ST_ARMED_POWER,
	EI_ST_FAULT
} STATE_VALUES;

// The basic structure is defined here. The objects are created in GlobalVariables.h

// Structure to store Event related Info
struct EVENT
{
	Uint16 ui_Scheduled_Occurrence;
	Uint16 (*p_fn_EventDriver)();
};

// Structure to store ADC Results. The name is from legacy code. It's got nothing to do with RMS values.
struct RMS_ARRAY_VARIABLES
{
	Uint16 iq_ADC_Result_Regs;
	int32 iq_Normalized_Adc_Values;
	int32 iq_Filtered_Value;
	int32 iq_Scaled_Filtered_Value;
   	int32 iq_Scaled_Unfiltered_Value;
	int32 iq_Full_Scale_Value;
	int32 iq_Calibrated_Value;
};

// Structure that stores Product Specifications.
struct PRODUCT_SPECIFICATION
{
	int32 iq_Rated_Voltage;
	int32 iq_Rated_Current;
	int32 iq_Specified_FS_Voltage;		// This is the voltage corresponding to 6 encoder rotations
	int32 iq_Specified_FS_Current;		// This is the rotations corresponding to 6 encoder rotations
	int32 iq_Voltage_3V_value;
	int32 iq_Current_3V_value;
	Uint16 ui_Model_Number;
	Uint16 ui_Calibration_Status:1;		// Whether the PSU has been calibrated once or not.
	int32 iq_Voltage_PWM_Gain;
	int32 iq_Current_PWM_Gain;
	int32 iq_OVP_PWM_Gain;
	int32 iq_Min_Ovp_Reference;			// Device Ovp Minimum limit in IQ
	int32 iq_Max_Uvl_Reference;			// Device Uvl Maximum limit in IQ
	Uint16 ui_Qep_Min_OVP_Count;		// Device Ovp Minimum limit in Qep counts
	int32 iq_Voltage_Ref_Offset;
	int32 iq_Voltage_Ref_Gain;
	int32 iq_Current_Ref_Offset;
	int32 iq_Current_Ref_Gain;
	Uint16 ui_Current_Rise_Time;
	int32 iq_Voltage_Dac_Gain;
	int32 iq_Current_Dac_Gain;
	int32 iq_Voltage_Display_Offset;
	int32 iq_Voltage_Display_Gain;
	int32 iq_Current_Display_Offset;
	int32 iq_Current_Display_Gain;
	Uint32 ul_Product_Power_On_Time_In_Minutes;
//	Uint16 fg_Ovp_Calibrated:1;			// Whether The ovp of the system has been calibrated or not.
//	int32 iq_Ovp_Ref_Offset;
//	int32 iq_Ovp_Ref_Gain;
};

// Structure that stores all the reference variables needed for PSU operation
struct REFERENCE_VARIABLES
{
	int32 iq_Voltage_Reference;	// Pwm generated for this value
//	int32 Voltage_Reference_Error;
	Uint16 fg_Voltage_Reference_Update_Immediate_Flag;

	int32 iq_Current_Reference;	// Pwm generated for this value
//	int32 Current_Reference_Error;
	Uint16 fg_Current_Reference_Update_Immediate_Flag;

	int32 iq_OVP_Reference;		// Pwm generated for this value
	int32 iq_OVP_Reference_Error;
	Uint16 ui_OVP_Reference_Update_Immediate_Flag;

	int32 iq_UVL_Reference;	// No Pwm generated for this value
	Uint32 ui_Set_Address;
	Uint16 ui_Set_BaudRate;
};

// Product Mode machine structure
struct PSU_MODE
{
	Uint16 bt_Encoder_Rotation_Mode:1; 		// COARSE, FINE (1 bit)
	Uint16 bt_Display_Mode:1;		  		// OUTPUT, PREVIEW (1 bit)
	Uint16 bt_FrontPanel_Access:1;     		// LOCKED,UNLOCKED (1 bit)
	Uint16 bt_Encoder_Operation:2;     		// VOLT/CURRENT, OVP,UVL, ADDR/BAUD (2 bits)
	Uint16 bt_FoldBack_Mode:2;         		// OFF, PROTECT_FROM_CV, PROTECT_FROM_CC (2 bits)
	Uint16 bt_PSU_Control:2;           		// LOCAL,REMOTE,REMOTE LOCKED (2 bits)
	Uint16 bt_Output_Status:1;         		// ON/OFF (1 bit)
	Uint16 bt_Start_Mode:1;            		// AUTO,SAFE (1 bit)
	Uint16 bt_Master_Slave_Setting:1;		// MASTER,SLAVE (1 bit)
	Uint16 bt_Multidrop_Mode:1;				// Multidrop mode enabled/disabled
	Uint16 bt_Power_On_Status_Clear:1;			// If ON Clears the enable registers every time system boots, else retain value from EEPROM
	Uint16 bt_Can_SRQ_Switch:1;				// Switch to enable/disable SRQ's on CAN
};

struct PSU_STATE
{
	struct PSU_MODE Mode;
	STATE_VALUES Next_St;
	STATE_VALUES Present_St;
	STATE_VALUES Previous_St;
	Uint16 ui_Cycle_Counter;					// 100ms second counter that counts the no. of 100ms that
};									// have elasped since a particular state has been attained.

struct ENCODER_VARS
{
	int16 i_VoltageKnob_Coarse_Count;
	int16 i_VoltageKnob_Fine_Count;
	int16 i_VoltageKnob_Final_Count;
	int16 i_VoltageKnob_Previous_Count; // Store Previous Count

	int16 i_CurrentKnob_Coarse_Count;
	int16 i_CurrentKnob_Fine_Count;
	int16 i_CurrentKnob_Final_Count;
	int16 i_CurrentKnob_Previous_Count; // Store Previous Count

	int32 iq_Qep_Voltage_Count;
	int32 iq_Qep_Current_Count;
	Uint16 ui_Qep_OVP_Count;
	Uint16 ui_Qep_UVL_Count;
	Uint16 ui_Qep_Address_Count;
	Uint16 ui_Qep_BaudRate_Count;
};

// Structure for all bit Global Flags
struct GLOBAL_FLAGS
{
	Uint16 fg_Scheduled_Event_Flag:1;					// To perform a scheduled event or not
	Uint16 fg_Preview_Key_Operation_Mode:1;				// Secondary Operation or not
	Uint16 fg_Active_AD7705_Channel:1;					// Active AD7705 Channel
	Uint16 fg_Next_AD7705_Channel:1;					// Next AD7705 Channel to be converted
	Uint16 fg_Display_FP_Access_Mode:1;					// To display LFP/UFP or not
	Uint16 fg_Out_Key_Operation_Mode:1;					// Secondary or primary operation for Out key
	Uint16 fg_Display_PSU_StartMode:1;					// To display AST/SAF or not
	Uint16 fg_Remote_Key_Operation_Mode:1;				// Secondary or primary operation for Rem key
	Uint16 fg_Immediate_Update_Sci_Baud:1;				// To update Sci baud or not
	Uint16 fg_Immediate_Update_PSU_Addr:1;				// To update PSU Address
	Uint16 fg_Checksum_Included_In_Uart_Comm:1;			// Command/Query has $ sign or not
	Uint16 fg_Uart_Comm_with_this_PSU:2;				// Is communication with this psu OPEN/CLOSED/COUPLED
	Uint16 fg_Voltage_Remote_control:1;					// Voltage Controlled from Remote?
	Uint16 fg_Current_Remote_control:1;					// Current Controlled from Remote?
	Uint16 fg_OVP_Remote_control:1;						// OVP Controlled from Remote?
	Uint16 fg_UVL_Remote_control:1;						// UVL Controlled from Remote?
	Uint16 fg_Date_Entered:1;							// Date Entered?
	Uint16 fg_Rated_Volt_Entered:1;						// XVM Entered?
	Uint16 fg_Rated_Curr_Entered:1;						// XCM Entered?
	Uint16 fg_Specified_Volt_Entered:1;					// SVUL Entered?
	Uint16 fg_Specified_Curr_Entered:1;					// SCUL Entered?
	Uint16 fg_Voltage_Calibration_Done:1;				// Voltage calibrated?
	Uint16 fg_Current_Calibration_Done:1;				// Current calibrated?
	Uint16 fg_Previous_Calibration:2;					// this bit sets the previous calibration setting (SVZ,SVF,SCZ,SCF)
	Uint16 fg_Voltage_MV1_Entered:1;					// Measured voltage low has been entered?
	Uint16 fg_Voltage_MV2_Entered:1;					// Measured voltage high has been entered?
	Uint16 fg_Current_MC1_Entered:1;					// Measured current low has been entered?
	Uint16 fg_Current_MC2_Entered:1;					// Measured current high has been entered?
	Uint16 fg_Output_Switched_On_From_Uart:1;			// Output switched ON Command given from Uart?
	Uint16 fg_Output_Switched_On_From_Can:1;			// Output switched ON Command given from CAN?
	Uint16 fg_Onboard_OTP_Fault:1;						// The control board temperature has caused an OTP fault or not.
	Uint16 fg_Current_Mode_Changed_From_Fine_To_Coarse:1;// Whenever we switch from Fine to Coarse this flag is set.
	Uint16 fg_Voltage_Mode_Changed_From_Fine_To_Coarse:1;// Whenever we switch from Fine to Coarse this flag is set.
	Uint16 fg_Query_For_Opc:1;					   		// User queried for OPC or not.
	Uint16 fg_Blink_Fold_Led:1;					   		// Blink the Fold Led Once.
	Uint16 fg_State_Of_Display:1;					   	// Display digits are on or off?
	Uint16 fg_CAN_Srq_Sent:1;					   		// Has the CAN SRQ been Sent?
	Uint16 fg_Uart_SRQ_Tranmsit:1;				   		// Does the UART SRQ needs to be sent?
};

struct TIMERS // Various state timers implemented in the project
{
	Uint16 ui_OVP_UVL_Timer;				// 100 milliseconds counter			// Timer to keep Psu idle time in OVP/UVL mode
	Uint16 ui_I2C_State_Timer;
	Uint16 ui_SPI_State_Timer;
	Uint16 ui_AD7705_DRDY_Pin_Timer;
	Uint16 ui_Current_Rise_Timer;			// 1 milliseconds counter			// Timer to keep Current rise time after output ON
	Uint16 ui_FoldBack_Delay_Timer;			// 100 milliseconds counter			// Timer to keep PSU's CC/CV mode time after Fold fault
	Uint16 ui_Key_Preview_Press_Timer;		// 100 milliseconds counter			// Timer to keep the Preview Key Button pressed time
	Uint16 ui_Preview_Operation_Timer;		// 100 milliseconds counter			// Timer to keep Psu idle time in Preview mode
	Uint16 ui_Key_Out_Press_Timer;			// 100 milliseconds counter			// Timer to keep the Out button pressed time.
	Uint16 ui_Addr_Baud_Timer;				// 100 milliseconds counter			// Timer to keep PSU idle time in Addr_Baud mode
	Uint16 ui_Key_Remote_Press_Timer;		// 100 milliseconds counter			// Timer to keep the remote key pressed time
	Uint16 ui_MaxFoldBackDelayInCounts;		// 100 millisecond value			// Keeps the total allowed foldback delay time of the system
	Uint16 ui_Onboard_OTP_Timer;			// 500 millisecond value.			// Keeps the total time the PSU onboard temperature is above safety limit.
	Uint16 ui_EEpromWriteCycle;				// 1 millisecond counter			// Keeps the write cycle time after every EEPROM Write
	Uint16 ui_Fanfail_Delay_Timer;			// 500 millisecond timer			// Keeps the time for which fan fail detected as low
};

struct TRANSMIT_CIRCULAR_BUFFER
{
	Int8 a_c_TransmitBuffer[TRANSMIT_BUFFER_SIZE];
	int16 i_Front;
	int16 i_Rear;										// ALWAYS POINTS TO THE LAST ITEM IN THE QUEUE.
};

struct ERROR_LINEAR_QUEUE
{
	Int8 *a_s_p_ErrorBuffer[ERROR_BUFFER_SIZE];
//	int16 i_Front;
	int16 i_Rear;										// ALWAYS POINTS TO THE LAST ITEM IN THE QUEUE.
};

struct RECEIVE_CIRCULAR_BUFFER
{
	Int8 a_c_ReceiveBuffer[RECEIVE_BUFFER_SIZE];
	int16 i_Front;
	int16 i_Rear;										// ALWAYS POINTS TO THE LAST ITEM IN THE QUEUE.
};

typedef struct {
  Uint32 dc_bus_rstundervolt:1;  // bit8
  Uint32 grid_current_violation:1; // bit9
  Uint32 inst_grid_rstcurrent_violation:1; // bit10
  Uint32 system_fault:1; // bit11
  Uint32 over_temp_voilation:1; // bit12
  Uint32 freq_out_of_Range:1; // bit13
  Uint32 inputphase_missing:1; // bit14
  Uint32 healthStatus:1;      // bit15
} FAULT_t_h;

typedef struct{
    Uint32 grid_ac_undervolt:1;    // bit0
    Uint32 grid_ac_rstundervolt:1; // bit1
    Uint32 grid_ac_overvolt:1;     // bit2
    Uint32 grid_ac_rstovervolt:1;  // bit3
    Uint32 dc_bus_overvolt:1;      // bit4
    Uint32 dc_bus_rstovervolt:1;   // bit5
    Uint32 dc_bus_undervolt:1;     // bit6
    Uint32 dc_bus_undervolt1:1 ;   // bit7
}FAULT_t_l;

typedef union fault_regs_t_h{
  FAULT_t_h bit;
  Uint32 all;
} fault_regs_t_h;

typedef union fault_regs_t_l{
  FAULT_t_l bit;
  Uint32 all;
} fault_regs_t_l;

#endif /*STRUCTURE_DECLARATIONS_H_*/
