#ifndef EXTERNALVARIABLES_H_
#define EXTERNALVARIABLES_H_
#include "GeneralStructureDefs.h"
#include "PSURegisterStructures.h"

//	This file is included in all sourse files other than main.c
extern Int8 const * const SOFTWARE_VERSION;
extern Int8 const * const BOOT_KEY;
extern Int8 const * const IEEE_LAN_SOFTWARE_REVISION;

extern Uint16 ui_Interrupt_Number;
extern Uint16 ui_Next_Event_In_Line;
extern Uint16 ui_Event_Initiated;
extern Uint16 ui_Fault_Led_Previous_State;
extern Uint16 ui_Last_Fault_Displayed;
extern int32 iq8_Temperature;
extern Int8 Uart_Rx_Data_Array[MAX_UART_ARRAY_SIZE];
extern Uint16 ui_Next;
extern Int8 sci_fecOnCommand;
extern Int8 sciQR;
extern Int8 fecOnStatus;
extern Uint16 CANFailCnt,CANFailTrig,CurRefRampDowncnt, CANFailEnable;


extern struct EVENT Event_Array[TOTAL_NO_OF_EVENTS];
//extern struct RMS_ARRAY_VARIABLES Internal_Rms_Array[16];
extern struct RMS_ARRAY_VARIABLES External_Rms_Array[2];
extern struct PRODUCT_SPECIFICATION Product_Info;
extern struct PSU_STATE State;
extern struct REFERENCE_VARIABLES Reference;
extern struct ECAN_REGS ECanaShadow;
extern struct ENCODER_VARS Encoder;
extern struct GLOBAL_FLAGS Global_Flags;
extern struct TIMERS Timers;
extern struct TRANSMIT_CIRCULAR_BUFFER Transmit_Circular_Buffer;
extern struct ERROR_LINEAR_QUEUE Error_Linear_Queue;
extern struct RECEIVE_CIRCULAR_BUFFER Receive_Circular_Buffer;

extern Int8 FaultChar[4];
extern fault_regs_t_h FecFaultHReg;
extern fault_regs_t_l FecFaultLReg;
// PSU Registers Object
extern struct FAULT Fault_Regs;
extern struct OPERATION_CONDITION Operation_Condition_Regs;
extern struct STANDARD_EVENT_STATUS Standard_Event_Status_Regs;
extern struct SERVICE_REQUEST Service_Request_Regs;

extern Int8 const * const E00;
extern Int8 const * const EM100;
extern Int8 const * const EM101;
extern Int8 const * const EM104;
extern Int8 const * const EM109;
extern Int8 const * const EM222;
extern Int8 const * const EM223;
extern Int8 const * const EM350;
extern Int8 const * const E301;
extern Int8 const * const E302;
extern Int8 const * const E304;
extern Int8 const * const E306;
extern Int8 const * const E307;
extern Int8 const * const E308;
extern Int8 const * const E310;
extern Int8 const * const E311;
extern Int8 const * const E312;
extern Int8 const * const E321;
extern Int8 const * const E322;
extern Int8 const * const E323;
extern Int8 const * const E324;
extern Int8 const * const E325;
extern Int8 const * const E326;
extern Int8 const * const E327;
extern Int8 const * const E341;
extern Int8 const * const E342;
extern Int8 const * const E343;
extern Int8 const * const E344;
extern Int8 const * const E345;
extern Int8 const * const E396;
extern Int8 const * const E397;
extern Int8 const * const E398;
extern Int8 const * const E399;

//###############################	 COMPLETE SCPI STRINGS	#####################################################//
extern Int8 const * const SPACE;
extern Int8 const * const COLON;
extern Int8 const * const JUNK_CHARACTER;

extern Int8 const * const ZERO;
extern Int8 const * const ONE;
extern Int8 const * const TWO;
extern Int8 const * const ZERO_WITH_CR;
extern Int8 const * const ONE_WITH_CR;
extern Int8 const * const TWO_WITH_CR;

extern Int8 const * const GLOBAL;
extern Int8 const * const GLOB;
extern Int8 const * const STAR_RST_WITH_CR;
extern Int8 const * const STAR_RCL;
extern Int8 const * const STAR_SAV;
extern Int8 const * const VOLTAGE;
extern Int8 const * const VOLT;
extern Int8 const * const CURRENT;
extern Int8 const * const CURR;
extern Int8 const * const OUTPUT;
extern Int8 const * const OUTP;
extern Int8 const * const STATE;
extern Int8 const * const STAT;
extern Int8 const * const AMPLITUDE;
extern Int8 const * const AMPL;

extern Int8 const * const INSTRUMENT;
extern Int8 const * const INST;
extern Int8 const * const COUPLE;
extern Int8 const * const COUPL;
extern Int8 const * const NSELECT;
extern Int8 const * const NSEL;
extern Int8 const * const QR_NSELECT_WITH_CR;
extern Int8 const * const QR_NSEL_WITH_CR;
extern Int8 const * const ALL_WITH_CR;
extern Int8 const * const NONE_WITH_CR;

extern Int8 const * const LLO_WITH_CR;
extern Int8 const * const REM_WITH_CR;
extern Int8 const * const LOC_WITH_CR;
extern Int8 const * const LLO;
extern Int8 const * const REM;
extern Int8 const * const LOC;
extern Int8 const * const PON;
extern Int8 const * const REMOTE;
extern Int8 const * const SYST;
extern Int8 const * const SYSTEM;
extern Int8 const * const QR_DATE_WITH_CR;
extern Int8 const * const QR_ERROR_WITH_CR;
extern Int8 const * const QR_REM_WITH_CR;
extern Int8 const * const QR_VERS_WITH_CR;
extern Int8 const * const QR_VERSION_WITH_CR;
extern Int8 const * const QR_REMOTE_WITH_CR;
extern Int8 const * const QR_STATE_WITH_CR;
extern Int8 const * const QR_STAT_WITH_CR;
extern Int8 const * const QR_TIME_WITH_CR;

extern Int8 const * const STAR_CLS_WITH_CR;
extern Int8 const * const STAR_ESE_WITH_SP;
extern Int8 const * const STAR_QR_ESE_WITH_CR;
extern Int8 const * const STAR_QR_ESR_WITH_CR;
extern Int8 const * const STAR_PSC_WITH_SP;
extern Int8 const * const STAR_SRE_WITH_SP;
extern Int8 const * const STAR_SAV_WITH_SP;
extern Int8 const * const STAR_RCL_WITH_SP;
extern Int8 const * const STAR_QR_IDN_WITH_CR;
extern Int8 const * const STAR_QR_OPT_WITH_CR;
extern Int8 const * const STAR_QR_PSC_WITH_CR;
extern Int8 const * const STAR_QR_SRE_WITH_CR;
extern Int8 const * const STAR_QR_STB_WITH_CR;
extern Int8 const * const STAR_OPC_WITH_CR;
extern Int8 const * const STAR_QR_OPC_WITH_CR;

extern Int8 const * const MEASURE;
extern Int8 const * const MEASURED;
extern Int8 const * const MEAS;
extern Int8 const * const QR_VOLTAGE_WITH_CR;
extern Int8 const * const QR_VOLT_WITH_CR;
extern Int8 const * const QR_CURRENT_WITH_CR;
extern Int8 const * const QR_CURR_WITH_CR;
extern Int8 const * const QR_POWER_WITH_CR;
extern Int8 const * const QR_POW_WITH_CR;
extern Int8 const * const POWER;
extern Int8 const * const POW;
extern Int8 const * const QR_DC_WITH_CR;
extern Int8 const * const QR_TEMPERATURE_WITH_CR;
extern Int8 const * const QR_TEMP_WITH_CR;

extern Int8 const * const SOURCE;
extern Int8 const * const SOUR;

extern Int8 const * const QR_VOLTAGE_MAX_WITH_CR;
extern Int8 const * const QR_VOLT_MAX_WITH_CR;
extern Int8 const * const QR_VOLTAGE_MIN_WITH_CR;
extern Int8 const * const QR_VOLT_MIN_WITH_CR;
extern Int8 const * const LEVEL;
extern Int8 const * const LEV;
extern Int8 const * const PROTECTION;
extern Int8 const * const PROT;
extern Int8 const * const IMMEDIATE;
extern Int8 const * const IMM;
extern Int8 const * const QR_AMPLITUDE_WITH_CR;
extern Int8 const * const QR_AMPL_WITH_CR;
extern Int8 const * const QR_IMMEDIATE_WITH_CR;
extern Int8 const * const QR_IMM_WITH_CR;
extern Int8 const * const QR_LEVEL_WITH_CR;
extern Int8 const * const QR_LEV_WITH_CR;
extern Int8 const * const QR_LEVEL_WITH_CR;
extern Int8 const * const QR_LEV_WITH_CR;
extern Int8 const * const QR_LEVEL_MAX_WITH_CR;
extern Int8 const * const QR_LEVEL_MIN_WITH_CR;
extern Int8 const * const QR_LEV_MAX_WITH_CR;
extern Int8 const * const QR_LEV_MIN_WITH_CR;
extern Int8 const * const LOW;
extern Int8 const * const QR_LOW_WITH_CR;
extern Int8 const * const QR_LOW_MAX_WITH_CR;
extern Int8 const * const QR_LOW_MIN_WITH_CR;

extern Int8 const* const QR_CURRENT_MAX_WITH_CR;
extern Int8 const* const QR_CURRENT_MIN_WITH_CR;
extern Int8 const* const QR_CURR_MIN_WITH_CR;
extern Int8 const* const QR_CURR_MAX_WITH_CR;

extern Int8 const * const CC;
extern Int8 const * const CV;
extern Int8 const * const OFF_STRING;
extern Int8 const * const QR_MODE_WITH_CR;
extern Int8 const * const QR_OUTP_WITH_CR;
extern Int8 const * const QR_OUTPUT_WITH_CR;
extern Int8 const * const QR_PON_WITH_CR;
extern Int8 const * const CC_WITH_CR;
extern Int8 const * const CV_WITH_CR;
extern Int8 const * const CLEAR_WITH_CR;
extern Int8 const * const CLE_WITH_CR;
extern Int8 const * const DEL;
extern Int8 const * const DELAY;
extern Int8 const * const FOLD;
extern Int8 const * const FOLDBACK;
extern Int8 const * const MODE;
extern Int8 const * const OFF_WITH_CR;
extern Int8 const * const QR_DEL_WITH_CR;
extern Int8 const * const QR_DELAY_WITH_CR;
extern Int8 const * const QR_FOLD_WITH_CR;
extern Int8 const * const QR_FOLDBACK_WITH_CR;

extern Int8 const * const CALIBRATE;
extern Int8 const * const CAL;
extern Int8 const * const DATE;
extern Int8 const * const ENARKA_VBV;
extern Int8 const * const LOW_WITH_CR;
extern Int8 const * const HIGH_WITH_CR;
extern Int8 const * const IDN;
extern Int8 const * const OPT;
extern Int8 const * const QR_COUNT_WITH_CR;
extern Int8 const * const QR_COUN_WITH_CR;
extern Int8 const * const QR_DISPLAY_WITH_CR;
extern Int8 const * const QR_DISP_WITH_CR;
extern Int8 const * const QR_RATED_WITH_CR;
extern Int8 const * const QR_RAT_WITH_CR;
extern Int8 const * const QR_SPECIFIED_WITH_CR;
extern Int8 const * const QR_SPEC_WITH_CR;
extern Int8 const * const QR_VALUE_WITH_CR;
extern Int8 const * const QR_VAL_WITH_CR;
extern Int8 const * const RATED;
extern Int8 const * const RAT;
extern Int8 const * const SPECIFIED;
extern Int8 const * const SPEC;
extern Int8 const * const VALUE;
extern Int8 const * const VAL;
extern Int8 const * const RESET_WITH_CR;
extern Int8 const * const RES_WITH_CR;
extern Int8 const * const WIPE_WITH_CR;
extern Int8 const * const QR_MOD_WITH_CR;
extern Int8 const * const QR_MODEL_WITH_CR;

extern Int8 const * const STATUS;
extern Int8 const * const OPERATION;
extern Int8 const * const OPER;
extern Int8 const * const QUESTIONABLE;
extern Int8 const * const QUES;
extern Int8 const * const ENABLE;
extern Int8 const * const ENAB;
extern Int8 const * const QR_ENABLE_WITH_CR;
extern Int8 const * const QR_ENAB_WITH_CR;
extern Int8 const * const QR_EVENT_WITH_CR;
extern Int8 const * const QR_EVEN_WITH_CR;
extern Int8 const * const QR_CONDITION_WITH_CR;
extern Int8 const * const QR_COND_WITH_CR;

extern Int8 const * const DISPLAY;
extern Int8 const * const DISP;

extern Int8 const * const BOOT;
extern Int8 const * const QR_CHK_WITH_CR;
extern Int8 const * const QR_STATUS_WITH_CR;
extern Int8 const * const QR_FAIL_WITH_CR;
extern Int8 const * const QR_MISR_WITH_CR;
extern Int8 const * const QR_MISREAD_WITH_CR;

extern Int8 const * const MAX_WITH_CR;
extern Int8 const * const MIN_WITH_CR;
//extern Int8 const * const TEMPOR;

#endif /*EXTERNALVARIABLES_H_*/
