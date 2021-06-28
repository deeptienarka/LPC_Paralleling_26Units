#ifndef USERDEFS_H_
#define USERDEFS_H_
			
			// PSU General Defines
// Math Constants
#define	CONVERSION_FACTOR_2P29V_TO_3V									1.310043668
#define CONVERSION_FACTOR_1P145V_TO_3V									2.620087336
#define CONVERSION_FACTOR_1P145V_TO_1P25V								1.091703057
#define	PWM_GAIN_CALCULTAION_CONSTANT									2272.727273
#define PWM_COUNTS_CORRESPONDING_TO_RATED_VALUE							1735
//#define	BASE_BAUD_RATE													1200
#define	BASE_BAUD_RATE													4800
#define	BAUD_RATE_CALCULATION_CONSTANT									20
#define	ADDRESS_CALCULATION_CONSTANT									20
#define DAC_COUNTS_CORRESPONDING_TO_RATED_VALUE							60036
#define ONE_FOURTH														0.25
#define ONE_HALF														0.50
#define THREE_FOURTH													0.75
#define	MIN_PROTECTION_DELAY_IN_COUNTS									1
#define IQ_FINE_CALCULATION_CONSTANT									393				// IQ_FINE_CALCULATION_CONSTANT = _IQ15(.2% * QEP_FULL_ROTATIONS_COUNT)/96
#define IQ_FINE_CALCULATION_CONSTANT_INVERSE							2730666			// IQ_FINE_CALCULATION_CONSTANT_INVERSE = _IQ15(IQ_FINE_CALCULATION_CONSTANT^-1)
#define	CONTROL_VOLTAGE_CORRESPONDING_TO_FULL_SCALE_OVP					2.29
#define	IQ_FULL_SCALE_OVP												_IQ15mpy(Product_Info.iq_Specified_FS_Voltage,_IQ15(1.1))
#define IQ_3P3_BY_EPWM2PRD												11				// _IQ15(3.3/10000)
#define BASE_TIME_ON_VALUE_IN_CASE_OF_DATA_FAILURE						52560000		// No of Minutes in 100 Years!!!!!
#define CHECK_FAULT_EVENT_FREQUENCY										10
#define MAX_PROTECTION_DELAY_SECONDS									25.5
#define MIN_PROTECTION_DELAY_SECONDS									0.1
#define LAST_ONE_BIT_ONLY												0x01
#define LAST_TWO_BITS_ONLY												0x03
#define LAST_FOUR_BITS													0x0F
#define LAST_EIGHT_BITS_ONLY											0xFF
#define MIN_DISTANCE_ABOVE_UVL											1.05
#define MIN_DISTANCE_BELOW_OVP											0.95
#define MIN_DISTANCE_ABOVE_PROGRAMMED_VOLT								1.052631579		//0.95^-1
#define MIN_DISTANCE_BELOW_PROGRAMMED_VOLT								0.952380952		//1.05^-1

// PSU Quick Operation Defines
#define DISABLE_FAULT_RESET												GpioDataRegs.GPASET.bit.GPIO27 = 1
#define ENABLE_FAULT_RESET												GpioDataRegs.GPACLEAR.bit.GPIO27 = 1
#define	SWITCH_OUTPUT_OFF												(GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1)
#define	SWITCH_OUTPUT_ON												(GpioDataRegs.GPBSET.bit.GPIO39 = 1)
#define PULL_PS_OK_LOW													(GpioDataRegs.GPBCLEAR.bit.GPIO53 = 1)
#define PULL_PS_OK_HIGH													(GpioDataRegs.GPBSET.bit.GPIO53 = 1)
#define TOGGLE_OUTPUT_GPIO												(GpioDataRegs.GPBTOGGLE.bit.GPIO39 = 1)
#define TOGGLE_DSP_RESET_PIN											(GpioDataRegs.GPATOGGLE.bit.GPIO27 = 1)
//#define FAN_OFF															ul_DesiredFanPeriod = 0
//#define FULL_SPEED														ul_DesiredFanPeriod = EPWM2_PERIOD_VALUE
//#define BASE_SPEED														ul_DesiredFanPeriod = 3*EPWM2_PERIOD_VALUE/10
#define CLEAR_ON_READING(x)												x = 0

// PSU Mode Defs
#define	FACTORY_DEFAULT				0
#define	ONE_TIME_CALIBRATED			1
#define	LOCAL_LOCKOUT				2
//#define	ANALOG_REMOTE				2
#define	NON_LATCHED_REMOTE			1
#define	LOCAL						0
#define AUTO_RESTART				1
#define	SAFE_START					0
#define	FOLDBACK_DISARMED			0
#define	COARSE_ROTATION				0
#define	FINE_ROTATION				1
#define DISPLAY_OUTPUT				0
#define	DISPLAY_PREVIEW				1
#define SET_VOLTAGE_CURRENT			0
#define	SET_OVP						1
#define	SET_UVL						2
#define	SET_ADDR_BAUD				3
#define	FRONTPANEL_LOCKED			1
#define	FRONTPANEL_UNLOCKED			0
#define CLOSED						0
#define OPEN						1
#define COUPLED_COMMUNICATION_MODE	2
#define PSU_MASTER					0
#define PSU_SLAVE					1

#define	FOLDBACK_ARMED_TO_PROTECT_FROM_CC								1
#define	FOLDBACK_ARMED_TO_PROTECT_FROM_CV								2
#define FAULT_STATE_FOR_UNDER_VOLTAGES									1
#define UNDER_VOLTAGE_LIMIT_ONLY										0
#define	CV_MODE															(GpioDataRegs.GPBDAT.bit.GPIO41 == 0)
#define	CC_MODE															(GpioDataRegs.GPBDAT.bit.GPIO41 == 1)
#define	OUTPUT_ON														(GpioDataRegs.GPBDAT.bit.GPIO39 == 1)
#define	OUTPUT_OFF														(GpioDataRegs.GPBDAT.bit.GPIO39 == 0)
#define PSU_IN_ANALOG_REMOTE											(GpioDataRegs.GPADAT.bit.GPIO16 == 1)
#define	CHECKSUM_SUCCESS												Checksum_Operation_Result
#define MAX_ALLOWED_ADDR												QEP_MAX_ADDRESS_COUNT/ADDRESS_CALCULATION_CONSTANT
#define MIN_ALLOWED_ADDR												1
#define PRODUCT_NOT_SWITCHED_ON_FOR_FIRST_TIME							0xAA
#define PRODUCT_SWITCHED_ON_FOR_FIRST_TIME								a_ui_I2cRxDataArray[0] == 0xFF
#define FRONTPANEL_IS_UNLOCKED											(State.Mode.bt_FrontPanel_Access == FRONTPANEL_UNLOCKED)

// PSU Fault defines
//#define SUNON_FAULT					1
//#define PROTECHNIC_FAULT			0
#define	AC_FAULT					(GpioDataRegs.GPADAT.bit.GPIO11)
//#define FAN_FAIL					(ul_DesiredFanPeriod) && (GpioDataRegs.GPADAT.bit.GPIO1 == SUNON_FAULT)
//#define FAN_FAIL					(ul_DesiredFanPeriod) && (GpioDataRegs.GPADAT.bit.GPIO1 == PROTECHNIC_FAULT)
#define	OTP_FAULT					(((GpioDataRegs.GPBDAT.bit.GPIO57 == 1) && (PFC_DC_BUS_FAULT == 0))|| (Global_Flags.fg_Onboard_OTP_Fault == TRUE))
#define	OVP_FAULT					(GpioDataRegs.GPBDAT.bit.GPIO51)
#define	INTERLOCK_FAULT				(GpioDataRegs.GPADAT.bit.GPIO12)
#define	PFC_DC_BUS_FAULT			(GpioDataRegs.GPADAT.bit.GPIO23)
#define I2C_BUS_FAIL				(Fault_Regs.FAULT_REGISTER.bit.Internal_Communication_Fault)

#define	SHUTOFF_FAULT				(GpioDataRegs.GPADAT.bit.GPIO13)
#define	OUTPUT_OFF_FAULT			(State.Mode.bt_Output_Status == OFF)
#define ANY_FAULTS					Fault_Regs.FAULT_REGISTER.all
#define BLINK_LED_FAULTS			(CANFailCnt == CAN_FAIL_SET_CNT)||(ui_Last_Fault_Displayed == AC_FAULT_FLAG)||(ui_Last_Fault_Displayed == OTP_FAULT_FLAG)||(ui_Last_Fault_Displayed == FOLD_FAULT_FLAG)||(ui_Last_Fault_Displayed == OVP_FAULT_FLAG)||(ui_Last_Fault_Displayed == INTERLOCK_FAULT_FLAG)||(ui_Last_Fault_Displayed == UNDISPLAYED_FAULTS)//||(ui_Last_Fault_Displayed == FAN_FAULT_FLAG)
#define CAN_FAIL_SET_CNT            4000
#define	LAST_FAULT_CLEARED												(Fault_Regs.FAULT_REGISTER.all & ui_Last_Fault_Displayed) == 0
#define	LAST_FAULT_DISPLAYED_WAS_NOT_OUT_FAULT							((ui_Last_Fault_Displayed & OUT_FAULT_FLAG)==0)
#define	ANY_FAULT_OTHER_THAN_OUT_FAULT									(Fault_Regs.FAULT_REGISTER.all & ~OUT_FAULT_FLAG & ~PFC_FAULT_FLAG)
#define	NO_FAULT_OTHER_THAN_OUT_FAULT									((Fault_Regs.FAULT_REGISTER.all & ~OUT_FAULT_FLAG & ~PFC_FAULT_FLAG)==0)
#define INTERNAL_COMMUNICATION_FAULT_SET								Fault_Regs.FAULT_REGISTER.bit.bt_Internal_Communication_Fault == SET
#define EEPROM_FAILURE_FAULT_SET										Fault_Regs.FAULT_REGISTER.bit.bt_Eeprom_Checksum_Failure == SET
#define ONBOARD_ADC_FAULT_SET											Fault_Regs.FAULT_REGISTER.bit.bt_On_Board_Adc_Failure == SET
#define FAULT_NOT_SET_PREVIOUSLY	0

#define	AC_FAULT_FLAG				0x0001
#define	OTP_FAULT_FLAG				0x0002
#define	FOLD_FAULT_FLAG				0x0004
#define	OVP_FAULT_FLAG				0x0008
#define	SHUTOFF_FAULT_FLAG			0x0010
#define	OUT_FAULT_FLAG				0x0020
#define	INTERLOCK_FAULT_FLAG		0x0040
//#define FAN_FAULT_FLAG				0x0080
#define PFC_FAULT_FLAG				0x0200
#define INTERNAL_COMM_FAULT_FLAG	0x0400
#define EEPROM_FAILURE_FAULT_FLAG	0x0800
#define ONBOARD_ADC_FAULT_FLAG		0x1000
#define ILLEGAL_MATH_FAULT_FLAG		0x2000
#define CAN_COMM_FAULT_FLAG         0x4000
#define UNDISPLAYED_FAULTS			0xFE00

			// Psu Timer Operations
#define INCREMENT_OVP_UVL_TIMER											Timers.ui_OVP_UVL_Timer++
#define RESET_OVP_UVL_TIMER												Timers.ui_OVP_UVL_Timer = 0
#define	OVP_UVL_TIMER_EXPIRED											Timers.ui_OVP_UVL_Timer >= 50

#define CURRENT_RISE_TIMER_EXPIRED										(Timers.ui_Current_Rise_Timer >= Product_Info.ui_Current_Rise_Time)
#define INCREMENT_CURRENT_RISE_TIMER 									Timers.ui_Current_Rise_Timer++
#define	RESET_CURRENT_RISE_TIMER										Timers.ui_Current_Rise_Timer = 0

#define	FOLDBACK_DELAY_TIME_EXPIRED										(Timers.ui_FoldBack_Delay_Timer >= Timers.ui_MaxFoldBackDelayInCounts)
#define	INCREMENT_FOLDBACK_DELAY_TIMER									Timers.ui_FoldBack_Delay_Timer++
#define RESET_FOLD_BACK_DELAY_TIMER										Timers.ui_FoldBack_Delay_Timer = 0

#define PREVIEW_KEY_SECONDARY_OPERATION_DETECTED						Timers.ui_Key_Preview_Press_Timer >= 30
#define INCREMENT_KEY_PREVIEW_PRESSED_TIMER								Timers.ui_Key_Preview_Press_Timer++
#define	RESET_KEY_PREVIEW_PRESSED_TIMER									Timers.ui_Key_Preview_Press_Timer = 0

#define PREVIEW_OPEARTION_TIMER_EXPIRED									Timers.ui_Preview_Operation_Timer >= 50
#define	INCREMENT_PREVIEW_OPERATION_TIMER								Timers.ui_Preview_Operation_Timer++
#define RESET_PREVIEW_OPERATION_TIMER									Timers.ui_Preview_Operation_Timer = 0

#define OUT_KEY_SECONDARY_OPERATION_DETECTED							Timers.ui_Key_Out_Press_Timer >= 30
#define INCREMENT_KEY_OUT_PRESSED_TIMER									Timers.ui_Key_Out_Press_Timer++
#define RESET_KEY_OUT_PRESSED_TIMER										Timers.ui_Key_Out_Press_Timer = 0

#define ADDR_BAUD_TIMER_EXPIRED											Timers.ui_Addr_Baud_Timer >= 50
#define INCREMENT_ADDR_BAUD_TIMER										Timers.ui_Addr_Baud_Timer++
#define RESET_ADDR_BAUD_TIMER											Timers.ui_Addr_Baud_Timer = 0

#define REMOTE_KEY_SECONDARY_OPERATION_DETECTED							Timers.ui_Key_Remote_Press_Timer >= 30
#define INCREMENT_KEY_REMOTE_PRESSED_TIMER								Timers.ui_Key_Remote_Press_Timer++
#define RESET_KEY_REMOTE_PRESSED_TIMER									Timers.ui_Key_Remote_Press_Timer = 0

#define ONBOARD_OTP_TIMER_EXPIRED										Timers.ui_Onboard_OTP_Timer == 6
#define INCREMENT_ONBOARD_OTP_TIMER										Timers.ui_Onboard_OTP_Timer++
#define RESET_ONBOARD_OTP_TIMER											Timers.ui_Onboard_OTP_Timer = 0

#define EEPROM_WRITE_CYCLE_TIMER_EXPIRED								(Timers.ui_EEpromWriteCycle == 6)
#define INCREMENT_EEPROM_WRITE_CYCLE_TIMER								Timers.ui_EEpromWriteCycle++
#define RESET_EEPROM_WRITE_CYCLE_TIMER									Timers.ui_EEpromWriteCycle = 0

#define FANFAIL_DELAY_TIME_EXPIRED										(Timers.ui_Fanfail_Delay_Timer >= FAN_FAIL_TIMEOUT)
#define INCREMENT_FANFAIL_DELAY_TIMER									Timers.ui_Fanfail_Delay_Timer++
#define RESET_FANFAIL_DELAY_TIMER										Timers.ui_Fanfail_Delay_Timer = 0

			// State Defs
#define INCREMENT_STATE_CYCLE_TIMER										State.ui_Cycle_Counter++
#define RESET_STATE_CYCLE_TIMER											State.ui_Cycle_Counter = 0
#define ONE_CYCLE_COUNT													1
#define TWO_CYCLES_COUNT												2
#define THREE_CYCLES_COUNT												3
#define FOUR_CYCLES_COUNT												4
#define TEN_CYCLES_COUNT												10
//#define FAN_FAIL_TIMEOUT												10

			// Scheduler and Events
#define	NO_OF_INTERRUPTS_IN_ONE_CYCLE									500

			//Peripheral Operation Defs
// PIE Defs
#define	ENABLE_INT_PIE_LEVEL		1
#define	CLEAR_INTERRUPT				1
#define	DISABLE_INT					0
#define	ENABLE_INT					1
			
//GPIO Defs
#define	GPIO_FUNCTION				0
#define	PERIPHERAL_ECANA			1
#define	PERIPHERAL_PWM_PIN			1
#define	PERIPHERAL_I2C				1
#define	PERIPHERAL_EQEP1			1
#define	PERIPHERAL_EQEP2			2
#define	PERIPHERAL_SCIA				1
#define	PERIPHERAL_SPIA				1

#define AIO_DISABLE					2 //2 or 3. Both are valid. (The register bits can be 11b or 10b)

#define	OUTPUT_PIN					1
#define	INPUT_PIN					0

#define	PULLUP_ENABLE   			0
#define	PULLUP_DISABLE				1

#define	HIGH						1
#define	SET_1_TO_CLEAR				1
#define	WRITE_1_TO_SET				1

// Input Qualification Definitions
#define	SYNC_TO_SYSCLKOUT			0
#define SIX_SAMPLES_FOR_INPUT_QUAL	2
#define INPUT_SAMPLE_PERIOD_20SYS	10
#define	INPUT_PIN_SAMPLE_PERIOD		255

//Epwm definitions
//#define     HR_PWM_SHIFT			0x180
//#define   	MEP_SF     				55				// HRPWM Scaling factor

#define	CLKDIV_1					0
#define	DISABLE_PHASE_SYNC			3	
#define	PHASE_SYNC_AT_CTR_ZERO		1	
#define	DISABLE_PHASE				0
#define	ENABLE_PHASE				1
#define	UP_DOWN_MODE				2
#define	DIV_1						0

#define	LOAD_IN_SHADOW_REG			0	
#define	LOAD_DURING_UNDERFLOW		0

#define	PULL_HIGH					2
#define	PULL_LOW					1
	

#define	DISABLE_SELECTON			0
#define	DISABLE_INT_PERIOD			0
#define	INTERRUPT_FIRST_EVENT		1
#define	INTERRUPT_UNDERFLOW			1

#define	EPWM2_PERIOD_VALUE			4500		// Set at 10K
//#define	EPWM3_PERIOD_VALUE			45			// Set at 1M
#define	ADC7705_MCLK_PRD			45			// Set at 1M
#define	EPWM4_PERIOD_VALUE			692	        // Set at 32.5K Earlier 50K
#define	EPWM5_PERIOD_VALUE			692			// Set at 32.5K
#define DEADBAND_UC3825				65  		// Measured blanking time 1.5usec
#define	EPWM8_PERIOD_VALUE			4500		// Set at 10K
#define	EPWM6_PERIOD_VALUE			45000		// Set at 1K
#define PWM_COUNTS_CORRESPONDING_TO_OVP_RATED_VALUE						3122			// 2.29*EPWM8_PERIOD/3.3

//Scia Defs 								
#define RESET_TRANSMIT_RECEIVE 		1
#define ENABLE_FIFO_ENHANCEMENTS	1
#define UART_BUFFER_SIZE			4
#define UART_BUFFER_FULL			4
#define UART_BUFFER_EMPTY			0
#define HOLD_IN_RESET				0
#define SINGLE_CHARACTER_FOR_UART_INTERRUPT								1

//EQep Defs
#define	QUADRATURE_COUNT_MODE		0

#define	POSITION_CONTROL_SYNC_OUTPUT_DISABLE  							0

#define	RESOLUTION_2X			  	0
#define	RESET_ON_MAX_POSITION		1
#define ANTICLOCKWISE				0
#define	CLOCKWISE					1
#define	ENABLE_COUNTER				1

// Different QEP constants used in the application
#define	EQEP1						1
#define	EQEP2						2
#define	QEP_CTR_SOFTWARE_INIT		1
#define	BASE_ADDRESS_QEP_COUNT											20
//#define	QEP_DEFAULT_BAUD_RATE_COUNT										60
#define	QEP_DEFAULT_BAUD_RATE_COUNT										39
#define	QEP_DEFAULT_ADDR_COUNT											140
#define	QEP_MAX_OVP_COUNT												634		// 1.1 * QEP_FULL_ROTATIONS_COUNT
#define	QEP_SAFETY_NET													200
#define	QEP_FULL_ROTATIONS_COUNT										576
#define QEP_FINE_COUNT_DIVIDER											10
#define QEP_MAX_ADDRESS_COUNT											639
//#define QEP_MAX_BAUDRATE_COUNT											99
#define QEP_MAX_BAUDRATE_COUNT											59
#define QEP_MAX_UVL_COUNT												547		// 0.95 * QEP_FULL_ROTATIONS_COUNT

// Math to calculate the below values
//CURRENT_COARSE_COUNT_FOR_WHICH_FINE_HAS_A_LOWER_LIMIT = (QEP_FULL_ROTATIONS_COUNT/2)*FINE_CALCULATION_CONSTANT
//CURRENT_COARSE_COUNT_FOR_WHICH_FINE_HAS_A_UPPER_LIMIT = QEP_FULL_ROTATIONS_COUNT - (QEP_FULL_ROTATIONS_COUNT/2)*FINE_CALCULATION_CONSTANT
#define CURRENT_COARSE_COUNT_FOR_WHICH_FINE_HAS_A_LOWER_LIMIT		3
#define	CURRENT_COARSE_COUNT_FOR_WHICH_FINE_HAS_AN_UPPER_LIMIT		573
// Calculated using the same logic above but because of OVP and UVL limits, this value is always 3
#define VOLTAGE_COUNT_DIFFERENCE_FOR_WHICH_FINE_HAS_A_LOWER_LIMIT	3
#define VOLTAGE_COUNT_DIFFERENCE_FOR_WHICH_FINE_HAS_AN_UPPER_LIMIT	3

//I2C defs
#define	I2C_RESET					0
#define	TRANSMIT_MESSAGE			1

//ADC Defs
#define ADC_CLOCK_DIVIDE_BY_2											1											
#define INTERRUPT_AT_END_OF_CONVERSION									1
#define ENABLE_ADCINB5													0
#define ENABLE_ADCINA5													0
#define DISABLE_SAMPLE_AND_CONVERSION_OVERLAP							1
#define ADC_FLAGS_CLEAR													0x01FF
#define NON_CONTINUOUS_INTERRUPT_MODE									0
#define INT_SRC_EOC_4													0x04													
#define INT_SRC_EOC_5													0x05
#define INT_SRC_EOC_6													0x06
#define ENABLE_ADCINT													1
#define SOFTWARE_TRIGGER												0
#define SAMPLE_WINDOW_SIZE												6
#define ADCINA0						0
#define ADCINA1						1
#define ADCINA2						2
#define ADCINA3						3
#define ADCINA4						4
#define ADCINA5						5
#define ADCINA7						7
#define NUM_OF_ADC_CHANNELS_USED	5


#define SOC_0						1
#define SOC_1						2
#define SOC_2						4
#define SOC_3						8
#define SOC_4						16
#define SOC_6						64

#define ADC_SOC(x)					AdcRegs.ADCSOCFRC1.all = x

		// AD7705 Defs
#define AD7705					0			
/*
#define	CHIP_SELECT_AD7705		(GpioDataRegs.GPBCLEAR.bit.GPIO57 = 1)
#define	CHIP_RELEASE_AD7705		(GpioDataRegs.GPBSET.bit.GPIO57 = 1)
*/
#define	CHIP_SELECT_AD7705		(GpioDataRegs.GPACLEAR.bit.GPIO10 = 1)
#define	CHIP_RELEASE_AD7705		(GpioDataRegs.GPASET.bit.GPIO10 = 1)
//#define CS_AD7705				0x00080000
//#define CS_AD7705				1
#define CS_AD7705				0x00000400
#define	DRDY_PIN				(GpioDataRegs.GPBDAT.bit.GPIO50)
#define	AD7705_READY_TO_BE_READ	0
#define	STARTUP_TIME_DELAY		20000

#define	NO_OF_RESULTS			1
#define JUNK					255
#define CH1						0
#define CH2						1
#define CURRENT_MEASURE_CHANNEL	1
#define VOLTAGE_MEASURE_CHANNEL	0
#define NO_OF_AD7705_SAMPLES	4
//#define NO_OF_AD7705_SAMPLES	8
#define RIGHT_SHIFT_FACTOR_FOR_AD7705_LPF								2
//#define RIGHT_SHIFT_FACTOR_FOR_AD7705_LPF								3

#define AD7705_CLOCK_REGISTER	0x20
#define AD7705_SETUP_REGISTER	0x10
#define AD7705_DATA_REGISTER	0x30
#define AD7705_WRITE			0x00
#define AD7705_READ				0x08
#define AD7705_DISABLE_MCLK_OUT	0x10
#define AD7705_CLKDIV_1			0x00
#define AD7705_MCLK_1MHz		0x40

#define AD7705_OUTPUT_DATA_RATE_25										0x01

#define	AD7705_SELF_CALIBRATION	0x40
#define AD7705_UNITY_GAIN		0x00
#define	ADD7705_UNIPOLAR		0x04
#define AD7705_UNBUFFERED_INPUT	0x00
#define AD7705_CLEAR_FSYNC		0x00
#define AD7705_CH1				0x00
#define AD7705_CH2				0x01

		// DAC 8552
/*
#define	CHIP_SELECT_DAC			(GpioDataRegs.GPACLEAR.bit.GPIO15 = 1)
#define	CHIP_RELEASE_DAC		(GpioDataRegs.GPASET.bit.GPIO15 = 1)		
*/
#define DAC_8552				1		
//#define CS_DAC					0x00000080
//#define CS_DAC					2
#define CS_DAC					0x00008000
#define DAC_LOAD_A				0x10
#define DAC_LOAD_B				0x20
#define DAC_CHANNEL_A			0x00
#define DAC_CHANNEL_B			0x04
#define	DAC_NORMAL_MODE			0x00

			// EEPROM
#define EEPROM_24LC256_ADDRESS		0x0050 // 7 bit: 1010000
#define	EEPROM_WRITE_CYCLE_DELAY	DELAY_US(5000)

#define VALUE_AT_ANY_LOCATION_IN_A_NEW_EEPROM							0xFF

			// Temp Sensor
#define TEMP_SENSOR_ADDRESS			0x0048 // 7 bit: 1001000
#define	TEMP_REG_ADDR  				0x00
#define T_CONFIG_ADDR				0x01
#define	T_LOW_ADDR					0x02
#define	T_HIGH_ADDR					0x03
#define	TEMP_HIGH_LIMIT_MSB			0x37						
#define	TEMP_HIGH_LIMIT_LSB			0x00						
#define	TEMP_LOW_LIMIT_MSB			0xEC
#define	TEMP_LOW_LIMIT_LSB			0x00
#define UPPER_LIMIT_FOR_ONBOARD_TEMPERATURE								_IQ8(80)

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
		
#define	DIGIT3_DECIMAL_POINT		0x80
#define	DIGIT2_DECIMAL_POINT		0x40
#define	DIGIT1_DECIMAL_POINT		0x20
#define	DIGIT0_DECIMAL_POINT		0x10
#define	NO_DECIMAL_POINT			0x00

// Display and KeyScan Defs
#define BLINK						1
#define DONT_BLINK					0

// Front panel Leds
//MAX6959 controlled
#define	CV_LED						0x08
#define OUT_LED						0x04
#define	FINE_LED					0x02
#define	PREVIEW_LED					0x01
//MAX6958 controlled
#define	CC_LED						0x08
#define	FOLD_LED					0x04
#define	REMOTE_LED					0x02
#define FAULT_LED					0x01

// FrontPanel Switches
#define	KEY_FINE_PRESSED			0x20
#define	KEY_OUT_PRESSED				0x40
#define	KEY_PREVIEW_PRESSED			0x80
#define	KEY_FOLD_PRESSED			0x02
#define	KEY_REMOTE_PRESSED			0x04
#define	KEY_OVP_UVL_PRESSED			0x08

// Digit Register Mapping
#define NUMBER_OF_DIGITS_DISPLAY	4
#define	DISPLAY_BLANK				255			

#define	DIGIT_REGISTER_MAPPING_FOR_DISPLAY_BLANK 						0x00

#define	DIGIT_REGISTER_MAPPING_FOR_0									0x7E
#define	DIGIT_REGISTER_MAPPING_FOR_1									0x30
#define	DIGIT_REGISTER_MAPPING_FOR_2									0x6D
#define	DIGIT_REGISTER_MAPPING_FOR_3									0x79
#define	DIGIT_REGISTER_MAPPING_FOR_4									0x33
#define	DIGIT_REGISTER_MAPPING_FOR_5									0x5B
#define	DIGIT_REGISTER_MAPPING_FOR_6									0x5F
#define	DIGIT_REGISTER_MAPPING_FOR_7									0x70
#define	DIGIT_REGISTER_MAPPING_FOR_8									0x7F
#define	DIGIT_REGISTER_MAPPING_FOR_9									0x7B
#define	DIGIT_REGISTER_MAPPING_FOR_A									0x77
#define	DIGIT_REGISTER_MAPPING_FOR_B									0x1F
#define	DIGIT_REGISTER_MAPPING_FOR_C									0x4E
#define	DIGIT_REGISTER_MAPPING_FOR_D									0x3D
#define	DIGIT_REGISTER_MAPPING_FOR_E									0x4F
#define	DIGIT_REGISTER_MAPPING_FOR_F									0x47
#define DIGIT_REGISTER_MAPPING_FOR_H									0x37
#define	DIGIT_REGISTER_MAPPING_FOR_U									0x3E
#define	DIGIT_REGISTER_MAPPING_FOR_P									0x67
#define	DIGIT_REGISTER_MAPPING_FOR_T									0x0F
#define	DIGIT_REGISTER_MAPPING_FOR_N									0x15
#define	DIGIT_REGISTER_MAPPING_FOR_L									0x0E

			// IQ Math Defs
#define	_Q12toIQ26(A)				((long)A << 14)
#define _Q16toIQ23(A)				((long)A << 19)
#define	_IQ26toIQ20(A)				(A >> 6)
#define	_IQ26toIQ23(A)				(A >> 3)
#define	_IQ10toIQ20(A)				(A << 10)
#define	_IQ20toIQ10(A)				(A >> 10)
#define	_IQ20toIQ15(A)				(A >> 5)
#define	_IQ15toIQ20(A)				(A << 5)
#define	_IQ26toIQ15(A)				(A >> 11)
#define	_IQ23toIQ15(A)				(A >> 8)
#define	_IQ15toIQ23(A)				(A << 8)
#define	_Q16toIQ26(A)               ((long)A << 10)
#define	_IQ20toIQ23(A)				(A << 3)
#define	_IQ23toIQ20(A)				(A >> 3)
#define	_IQ10toIQ23(A)				(A << 13)
#define	_IQ23toIQ10(A)				(A >> 13)
#define	_IQ15toIQ10(A)				(A >> 5)
#define	_IQ20toIQ19(A)				(A >> 1)
#define	_IQ23toIQ19(A)				(A >> 4)
#define _IQ26toQ12(A)				((long)A << 14)
#define _IQ15toQ12(A)				((long)A << 3)
			
			// Execution Time
#define	EXECUTION_TIME_GPIO_SET			GpioDataRegs.GPASET.bit.GPIO14 = 1 
#define	EXECUTION_TIME_GPIO_RESET		GpioDataRegs.GPACLEAR.bit.GPIO14 = 1
#define	EXECUTION_TIME_GPIO_TOGGLE		GpioDataRegs.GPATOGGLE.bit.GPIO14 = 1
#define	EXECUTION_TIME_GPIO_STATUS		GpioDataRegs.GPADAT.bit.GPIO14

			// Imporatant EEPROM Addresses
#define	PV_ADDRESS					0x00C0
#define	MODEL_NO_ADDR				0x0080
#define	CAN_MODEL_NO_ADDR			0x00AF
#define RATED_VOLTAGE_ADDRESS					0x0081
#define RATED_CURRENT_ADDRESS					0x0084
#define	SPECIFIED_VOLTAGE_ADDRESS				0x0087
#define	SPECIFIED_CURRENT_ADDRESS				0x008A
#define	V_OFFSET_ADDRESS			0x008D
#define	C_OFFSET_ADDRESS			0x0095
#define DATE_START_ADDRESS			0x0069
#define IDN_ADDRESS					0x0040
#define SN_START_ADDRESS			0x0054
#define OVP_INFO_ADDRESS			0x00B0
#define POWER_ON_TIME_STORAGE_ADDR	0x00EA
#define CALIBRATION_COUNT_ADDRESS	0x00AD
#define OPTION_INSTALLED_DETAILS_ADDRESS								0x0068
#define FLASH_PROG_STATUS_ADDRESS										0x0073
#define FLASH_CHECKSUM_ADDR												0x0074
#define FLASH_PROG_STATUS_ILLEGAL_READ_ADDRESS							0x0079
#define FLASH_PROG_ERROR_ADDRESS										0x007A
#define CAN_CALIBRATION_DATE_ADDRESS									0x0076

			// Calibration Operations
#define VOLTAGE_LOW					0
#define VOLTAGE_FULL				1
#define CURRENT_LOW					2
#define CURRENT_FULL				3

			// General Defs
#define NO_EVENT					0
#define EMPTY						0
#define UNITY						1
#define SUCCESS						1
#define	FAILURE						0
#define	FALSE						0
#define TRUE						1
#define SET							1
#define CLEARED						0
#define NONE						0
#define OFF							0
#define ON							1
#define	PRIMARY_OPERATION			0
#define	SECONDARY_OPERATION			1
#define	TWO_DIGIT					10
#define	THREE_DIGIT					100
#define	FOUR_DIGIT					1000
#define	FIVE_DIGIT					10000
#define	SIX_DIGIT					100000
#define	SEVEN_DIGIT					1000000
#define	EIGHT_DIGIT					10000000
#define MAX_VAL_FOR_8_BIT_REG		255
#define MAX_VAL_FOR_16_BIT_REG		65535

				// Boot related
#define BOOT_COMMAND_RECEIVED		0xCC

				// SAFETY CHECKS
#define VALUE_NEGATIVE_CHECK(x)											if(x < 0) \
																		{ \
																			ei_vEnqueueFaultRegsInErrorQueue(ILLEGAL_MATH_FAULT_FLAG,E398);\
																			return; \
																		}

#define DENOMINATOR_ZERO_NEGATIVE_CHECK(x)								if(x <= 0) \
																		{ \
																			ei_vEnqueueFaultRegsInErrorQueue(ILLEGAL_MATH_FAULT_FLAG,E397);\
																			return; \
																		}

#define UINT16_SATURATION(x)											if(x == UINT_MAX) \
																		{ \
																			ei_vEnqueueFaultRegsInErrorQueue(ILLEGAL_MATH_FAULT_FLAG,E396);\
																			return; \
																		}

#define IQ_DIVISION_RESULT_NEG_AND_POSITIVE_SATURATION_CHECK(x)			if((x == LONG_MAX) || (x == LONG_MIN)) \
																		{ \
																			ei_vEnqueueFaultRegsInErrorQueue(ILLEGAL_MATH_FAULT_FLAG,E396);\
																			return; \
																		}

/*
#define VALUE_NEGATIVE_CHECK(x)											if(x < 0) \
																		{ \
																			Fault_Regs.FAULT_REGISTER.bit.bt_Illegal_Math_Performed = SET;\
																			return; \
																		}

#define DENOMINATOR_ZERO_NEGATIVE_CHECK(x)								if(x <= 0) \
																		{ \
																			Fault_Regs.FAULT_REGISTER.bit.bt_Illegal_Math_Performed = SET;\
																			return; \
																		}

#define UINT16_SATURATION(x)											if(x == UINT_MAX) \
																		{ \
																			Fault_Regs.FAULT_REGISTER.bit.bt_Illegal_Math_Performed = SET;\
																			return; \
																		}

#define IQ_DIVISION_RESULT_NEG_AND_POSITIVE_SATURATION_CHECK(x)			if((x == LONG_MAX) || (x == LONG_MIN)) \
																		{ \
																			Fault_Regs.FAULT_REGISTER.bit.bt_Illegal_Math_Performed = SET;\
																			return; \
																		}
*/

#endif /*USERDEFS_H_*/
