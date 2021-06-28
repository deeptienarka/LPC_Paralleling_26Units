#ifndef GLOBALVARIABLES_H_
#define GLOBALVARIABLES_H_
#include "GeneralStructureDefs.h"
#include "PSURegisterStructures.h"

// This header file is included only in main.c. All the variables here are declared as extern in ExternalVariables.h.

Uint16 ui_Interrupt_Number;
Uint16 ui_Next_Event_In_Line;
Uint16 ui_Event_Initiated;
int32 iq8_Temperature;

//	Display keyscan related vars
Uint16 ui_Fault_Led_Previous_State;
Uint16 ui_Last_Fault_Displayed;

// Communication Vars
Int8 Uart_Rx_Data_Array[MAX_UART_ARRAY_SIZE+1];
Uint16 ui_Next;
Uint16 CANFailCnt, CANFailTrig, CurRefRampDowncnt, CANFailEnable;

		// Structure Objects
// Event Array
struct EVENT Event_Array[TOTAL_NO_OF_EVENTS];

// ADC Result object.
//struct RMS_ARRAY_VARIABLES Internal_Rms_Array[16];	// For internal 12 bit ADC
struct RMS_ARRAY_VARIABLES External_Rms_Array[2];	// For external 16 bit ADC

// Product info object
struct PRODUCT_SPECIFICATION Product_Info;

// PSU_State Object that stores all state related info
struct PSU_STATE State;

// Reference object that stores all current parameters of the product
struct REFERENCE_VARIABLES Reference;

// eCAN control registers require read/write access using 32-bits. Thus we will create a set of shadow registers 
// for this example.  These shadow registers will be used to make sure the access is 32-bits.
struct ECAN_REGS ECanaShadow;

// Encoder Object
struct ENCODER_VARS Encoder;

// Global Flags
struct GLOBAL_FLAGS Global_Flags;

// Timer Object
struct TIMERS Timers;

fault_regs_t_h FecFaultHReg;
fault_regs_t_l FecFaultLReg;

// PSU Register Objects
struct FAULT Fault_Regs;
struct OPERATION_CONDITION Operation_Condition_Regs;
struct STANDARD_EVENT_STATUS Standard_Event_Status_Regs;
struct SERVICE_REQUEST Service_Request_Regs;

// Buffer and Queues
struct TRANSMIT_CIRCULAR_BUFFER Transmit_Circular_Buffer;
struct ERROR_LINEAR_QUEUE Error_Linear_Queue;
struct RECEIVE_CIRCULAR_BUFFER Receive_Circular_Buffer;

// ERROR Messages. M stands for 'Minus' in the variable name.
Int8 const * const E00 = "0,\"No Error\"";
Int8 const * const EM100 = "-100,\"Command Error\"";
Int8 const * const EM101 = "-101,\"Illegal Parameter\"";
Int8 const * const EM104 = "-104,\"Data Type Error\"";
Int8 const * const EM109 = "-109,\"Missing Parameter\"";
Int8 const * const EM222 = "-222,\"Data Out of Range\"";
Int8 const * const EM223 = "-223,\"Too Much Data\"";
Int8 const * const EM350 = "-350,\"Queue Overflow\"";
Int8 const * const E301 = "301,\"PV Above OVP\"";
Int8 const * const E302 = "302,\"PV Below UVL\"";
Int8 const * const E304 = "304,\"OVP Below PV or Minimum\"";
Int8 const * const E306 = "306,\"UVL Above PV or Maximum\"";
Int8 const * const E307 = "307,\"ON During Fault\"";
Int8 const * const E308 = "308,\"Non Voltaile Memory Failure\"";
Int8 const * const E310 = "310,\"Onboard ADC Failure\"";
Int8 const * const E311 = "311,\"Internal Communication Failure\"";
Int8 const * const E312 = "312,\"PFC Bus Failure\"";
Int8 const * const E321 = "321,\"AC Fault Shutdown\"";
Int8 const * const E322 = "322,\"Over Temperature Shutdown\"";
Int8 const * const E323 = "323,\"FoldBack Shutdown\"";
Int8 const * const E324 = "324,\"Over Voltage Shutdown\"";
Int8 const * const E325 = "325,\"Analog Shutoff Shutdown\"";
Int8 const * const E326 = "326,\"Output off Shutdown\"";
Int8 const * const E327 = "327,\"Interlock Shutdown\"";
Int8 const * const E341 = "342,\"Input Overflow\"";
Int8 const * const E342 = "341,\"Output Overflow\"";
Int8 const * const E344 = "344,\"Internal Checksum Fault\"";
Int8 const * const E345 = "345,\"Settings Conflict\"";
Int8 const * const E396 = "397,\"Illegal Result For Calculation\"";
Int8 const * const E397 = "397,\"Illegal Denominator in Calculation\"";
Int8 const * const E398 = "398,\"Critical Positive Parameter is Negative\"";
Int8 const * const E399 = "399,\"Gain Evaluates to 0 or Negative\"";

//###############################	 COMPLETE SCPI STRINGS	#####################################################//
Int8 const * const SPACE = " ";
Int8 const * const COLON =  ":";
Int8 const * const JUNK_CHARACTER =  "#";

Int8 const * const ZERO = "0";
Int8 const * const ONE = "1";
Int8 const * const TWO = "2";
Int8 const * const ZERO_WITH_CR = "0\r";
Int8 const * const ONE_WITH_CR = "1\r";
Int8 const * const TWO_WITH_CR = "2\r";

Int8 const * const STAR_RST_WITH_CR = "*RST\r";
Int8 const * const STAR_RCL = "*RCL";
Int8 const * const STAR_SAV = "*SAV";

Int8 const * const GLOBAL = "GLOBAL";
Int8 const * const GLOB = "GLOB";
Int8 const * const VOLTAGE = "VOLTAGE";
Int8 const * const VOLT = "VOLT";
Int8 const * const CURRENT = "CURRENT";
Int8 const * const CURR = "CURR";
Int8 const * const OUTPUT = "OUTPUT";
Int8 const * const OUTP = "OUTP";
Int8 const * const STATE = "STATE";
Int8 const * const STAT = "STAT";
Int8 const * const AMPLITUDE = "AMPLITUDE";
Int8 const * const AMPL = "AMPL";

Int8 const * const INSTRUMENT = "INSTRUMENT";
Int8 const * const INST = "INST";
Int8 const * const COUPLE = "COUPLE";
Int8 const * const COUPL = "COUPL";
Int8 const * const NSELECT = "NSELECT";
Int8 const * const NSEL = "NSEL";
Int8 const * const QR_NSELECT_WITH_CR = "NSELECT?\r";
Int8 const * const QR_NSEL_WITH_CR = "NSEL?\r";
Int8 const * const ALL_WITH_CR = "ALL\r";
Int8 const * const NONE_WITH_CR = "NONE\r";

Int8 const * const LLO_WITH_CR = "LLO\r";
Int8 const * const REM_WITH_CR = "REM\r";
Int8 const * const LOC_WITH_CR = "LOC\r";
Int8 const * const LLO = "LLO";
Int8 const * const REM = "REM";
Int8 const * const LOC = "LOC";
Int8 const * const PON = "PON";
Int8 const * const REMOTE = "REMOTE";
Int8 const * const SYST = "SYST";
Int8 const * const SYSTEM = "SYSTEM";
Int8 const * const QR_DATE_WITH_CR = "DATE?\r";
Int8 const * const QR_ERROR_WITH_CR = "ERROR?\r";
Int8 const * const QR_REM_WITH_CR = "REM?\r";
Int8 const * const QR_VERS_WITH_CR = "VERS?\r";
Int8 const * const QR_VERSION_WITH_CR = "VERSION?\r";
Int8 const * const QR_REMOTE_WITH_CR = "REMOTE?\r";
Int8 const * const QR_STATE_WITH_CR = "STATE?\r";
Int8 const * const QR_STAT_WITH_CR = "STAT?\r";
Int8 const * const QR_TIME_WITH_CR = "TIME?\r";

Int8 const * const STAR_CLS_WITH_CR = "*CLS\r";
Int8 const * const STAR_ESE_WITH_SP = "*ESE ";
Int8 const * const STAR_QR_ESE_WITH_CR = "*ESE?\r";
Int8 const * const STAR_QR_ESR_WITH_CR = "*ESR?\r";
Int8 const * const STAR_PSC_WITH_SP = "*PSC ";
Int8 const * const STAR_SRE_WITH_SP = "*SRE ";
Int8 const * const STAR_SAV_WITH_SP = "*SAV ";
Int8 const * const STAR_RCL_WITH_SP = "*RCL ";
Int8 const * const STAR_QR_IDN_WITH_CR = "*IDN?\r";
Int8 const * const STAR_QR_OPT_WITH_CR = "*OPT?\r";
Int8 const * const STAR_QR_PSC_WITH_CR = "*PSC?\r";
Int8 const * const STAR_QR_SRE_WITH_CR = "*SRE?\r";
Int8 const * const STAR_QR_STB_WITH_CR = "*STB?\r";
Int8 const * const STAR_OPC_WITH_CR = "*OPC\r";
Int8 const * const STAR_QR_OPC_WITH_CR = "*OPC?\r";

Int8 const * const MEASURE = "MEASURE";
Int8 const * const MEASURED = "MEASURED";
Int8 const * const MEAS = "MEAS";
Int8 const * const QR_VOLTAGE_WITH_CR = "VOLTAGE?\r";
Int8 const * const QR_VOLT_WITH_CR = "VOLT?\r";
Int8 const * const QR_CURRENT_WITH_CR = "CURRENT?\r";
Int8 const * const QR_CURR_WITH_CR = "CURR?\r";
Int8 const * const QR_POWER_WITH_CR = "POWER?\r";
Int8 const * const QR_POW_WITH_CR = "POW?\r";
Int8 const * const POWER = "POWER";
Int8 const * const POW = "POW";
Int8 const * const QR_DC_WITH_CR = "DC?\r";
Int8 const * const QR_TEMPERATURE_WITH_CR = "TEMPERATURE?\r";
Int8 const * const QR_TEMP_WITH_CR = "TEMP?\r";

Int8 const * const SOURCE = "SOURCE";
Int8 const * const SOUR = "SOUR";

Int8 const * const QR_VOLTAGE_MAX_WITH_CR = "VOLTAGE?MAX\r";
Int8 const * const QR_VOLT_MAX_WITH_CR = "VOLT?MAX\r";
Int8 const * const QR_VOLTAGE_MIN_WITH_CR = "VOLTAGE?MIN\r";
Int8 const * const QR_VOLT_MIN_WITH_CR = "VOLT?MIN\r";
Int8 const * const LEVEL = "LEVEL";
Int8 const * const LEV = "LEV";
Int8 const * const PROTECTION = "PROTECTION";
Int8 const * const PROT = "PROT";
Int8 const * const IMMEDIATE = "IMMEDIATE";
Int8 const * const IMM = "IMM";
Int8 const * const QR_AMPLITUDE_WITH_CR = "AMPLITUDE?\r";
Int8 const * const QR_AMPL_WITH_CR = "AMPL?\r";
Int8 const * const QR_IMMEDIATE_WITH_CR = "IMMEDIATE?\r";
Int8 const * const QR_IMM_WITH_CR = "IMM?\r";
Int8 const * const QR_LEVEL_WITH_CR = "LEVEL?\r";
Int8 const * const QR_LEV_WITH_CR = "LEV?\r";
Int8 const * const QR_LEVEL_MAX_WITH_CR = "LEVEL?MAX\r";
Int8 const * const QR_LEVEL_MIN_WITH_CR = "LEVEL?MIN\r";
Int8 const * const QR_LEV_MAX_WITH_CR = "LEV?MAX\r";
Int8 const * const QR_LEV_MIN_WITH_CR = "LEV?MIN\r";
Int8 const * const LOW = "LOW";
Int8 const * const QR_LOW_WITH_CR = "LOW?\r";
Int8 const * const QR_LOW_MAX_WITH_CR = "LOW?MAX\r";
Int8 const * const QR_LOW_MIN_WITH_CR = "LOW?MIN\r";

Int8 const * const QR_CURRENT_MAX_WITH_CR = "CURRENT?MAX\r";
Int8 const * const QR_CURRENT_MIN_WITH_CR = "CURRENT?MIN\r";
Int8 const * const QR_CURR_MIN_WITH_CR = "CURR?MIN\r";
Int8 const * const QR_CURR_MAX_WITH_CR = "CURR?MAX\r";

Int8 const * const CC = "CC";
Int8 const * const CV = "CV";
Int8 const * const OFF_STRING = "OFF";	// So that we dont get confused with OFF bool
Int8 const * const QR_MODE_WITH_CR = "MODE?\r";
Int8 const * const QR_OUTP_WITH_CR = "OUTP?\r";
Int8 const * const QR_OUTPUT_WITH_CR = "OUTPUT?\r";
Int8 const * const QR_PON_WITH_CR = "PON?\r";
Int8 const * const CC_WITH_CR = "CC\r";
Int8 const * const CV_WITH_CR = "CV\r";
Int8 const * const CLEAR_WITH_CR = "CLEAR\r";
Int8 const * const CLE_WITH_CR = "CLE\r";
Int8 const * const DEL = "DEL";
Int8 const * const DELAY = "DELAY";
Int8 const * const FOLD = "FOLD";
Int8 const * const FOLDBACK = "FOLDBACK";
Int8 const * const MODE = "MODE";
Int8 const * const OFF_WITH_CR = "OFF\r";
Int8 const * const QR_DEL_WITH_CR = "DEL?\r";
Int8 const * const QR_DELAY_WITH_CR = "DELAY?\r";
Int8 const * const QR_FOLD_WITH_CR = "FOLD?\r";
Int8 const * const QR_FOLDBACK_WITH_CR = "FOLDBACK?\r";

Int8 const * const CALIBRATE = "CALIBRATE";
Int8 const * const CAL = "CAL";
Int8 const * const DATE = "DATE";
Int8 const * const ENARKA_VBV = "ENARKA,VBV";
Int8 const * const LOW_WITH_CR = "LOW\r";
Int8 const * const HIGH_WITH_CR = "HIGH\r";
Int8 const * const IDN = "IDN";
Int8 const * const OPT = "OPT";
Int8 const * const QR_COUNT_WITH_CR = "COUNT?\r";
Int8 const * const QR_COUN_WITH_CR = "COUN?\r";
Int8 const * const QR_DISPLAY_WITH_CR = "DISPLAY?\r";
Int8 const * const QR_DISP_WITH_CR = "DISP?\r";
Int8 const * const QR_RATED_WITH_CR = "RATED?\r";
Int8 const * const QR_RAT_WITH_CR = "RAT?\r";
Int8 const * const QR_SPECIFIED_WITH_CR = "SPECIFIED?\r";
Int8 const * const QR_SPEC_WITH_CR = "SPEC?\r";
Int8 const * const QR_VALUE_WITH_CR = "VALUE?\r";
Int8 const * const QR_VAL_WITH_CR = "VAL?\r";
Int8 const * const RATED = "RATED";
Int8 const * const RAT = "RAT";
Int8 const * const SPECIFIED = "SPECIFIED";
Int8 const * const SPEC = "SPEC";
Int8 const * const VALUE = "VALUE";
Int8 const * const VAL = "VAL";
Int8 const * const RESET_WITH_CR = "RESET\r";
Int8 const * const RES_WITH_CR = "RES\r";
Int8 const * const WIPE_WITH_CR = "WIPE\r";
Int8 const * const QR_MOD_WITH_CR = "MOD?\r";
Int8 const * const QR_MODEL_WITH_CR = "MODEL?\r";

Int8 const * const STATUS = "STATUS";
Int8 const * const OPERATION = "OPERATION";
Int8 const * const OPER = "OPER";
Int8 const * const QUESTIONABLE = "QUESTIONABLE";
Int8 const * const QUES = "QUES";
Int8 const * const ENABLE = "ENABLE";
Int8 const * const ENAB = "ENAB";
Int8 const * const QR_ENABLE_WITH_CR = "ENABLE?\r";
Int8 const * const QR_ENAB_WITH_CR = "ENAB?\r";
Int8 const * const QR_EVENT_WITH_CR = "EVENT?\r";
Int8 const * const QR_EVEN_WITH_CR = "EVEN?\r";
Int8 const * const QR_CONDITION_WITH_CR = "CONDITION?\r";
Int8 const * const QR_COND_WITH_CR = "COND?\r";

Int8 const * const DISPLAY = "DISPLAY";
Int8 const * const DISP = "DISP";

Int8 const * const BOOT = "BOOT";
Int8 const * const QR_CHK_WITH_CR = "CHK?\r";
Int8 const * const QR_STATUS_WITH_CR = "STATUS?\r";
Int8 const * const QR_FAIL_WITH_CR = "FAIL?\r";
Int8 const * const QR_MISR_WITH_CR = "MISR?\r";
Int8 const * const QR_MISREAD_WITH_CR = "MISREAD?\r";

Int8 const * const MAX_WITH_CR = "MAX\r";
Int8 const * const MIN_WITH_CR = "MIN\r";
//Int8 const * const TEMPOR = "TEMP\r";

#endif /*GLOBALVARIABLES_H_*/
