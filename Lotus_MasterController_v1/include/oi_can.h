/*
 * ei_Can.h
 *
 *  Created on: Jan 13, 2019
 *      Author: enArka
 */

#ifndef SOURCE_OI_CAN_H_
#define SOURCE_OI_CAN_H_

//
// Defines
//
#include "F28x_Project.h"
#include "driverlib/can.h"
#include "inc/hw_can.h"

#define TXCOUNT  100
#define MSG_DATA_LENGTH                 2
#define MSG_DATA_LENGTH_RX              8
#define MSG_DATA_LENGTH_TX_CANB         8



#define CAN_NODEID_MASK           0X0000FF00
#define TOTAL_PSU_NODE_NUMBER     28
#define TOTAL_VS_NODE_NUMBER      6
#define TOTAL_IO_NODE_NUMBER      3


#define MAILBOX_1                 1
#define MAILBOX_2                 2
#define MAILBOX_3                 3
#define MAILBOX_4                 4
#define MAILBOX_5                 5
#define MAILBOX_6                 6
#define MAILBOX_7                 7
#define MAILBOX_8                 8
#define MAILBOX_9                 9
#define MAILBOX_10               10
#define MAILBOX_11               11
#define MAILBOX_12               12
#define MAILBOX_13               13
#define MAILBOX_14               14
#define MAILBOX_15               15
#define MAILBOX_16               16
#define MAILBOX_17               17
#define MAILBOX_18               18
#define MAILBOX_19               19
#define MAILBOX_20               20
#define MAILBOX_21               21
#define MAILBOX_22               22
#define MAILBOX_23               23
#define MAILBOX_24               24
#define MAILBOX_25               25
#define MAILBOX_26               26
#define MAILBOX_27               27
#define MAILBOX_28               28
#define MAILBOX_29               29
#define MAILBOX_30               30
#define MAILBOX_31               31
#define MAILBOX_32               32


#define TURN_ON_PSU                                                                                         0xAA
#define TURN_OFF_PSU                                                                                        0x00
#define TURN_ON_FEC                                                                                         0xBB

#define CAN_PSU_TIMEOUT      150

// General msg IDs used for msg id creation
#define CAN_BASE_ID_FOR_TRANSMIT_PSU                                                                        0x10000000
#define CAN_BASE_ID_FOR_PRODUCT_PSU                                                                         0x01000000
#define CAN_BASE_ID_FOR_FAMILY_PSU                                                                          0x00100000
#define CAN_BASE_ID_FOR_MODEL_PSU                                                                           0x000B0000


#define CAN_BASE_ID_FOR_TURNONPSU_GLOBAL                                                                    0x00000004
#define CAN_BASE_ID_FOR_VOLT_SET_GLOBAL                                                                     0x00000005
#define CAN_BASE_ID_FOR_CURR_SET_GLOBAL                                                                     0x00000006

#define MSG_ID_GLOBAL_COMMAND_TO_TURNON_PSU                                                                 CAN_BASE_ID_FOR_TRANSMIT_PSU + CAN_BASE_ID_FOR_PRODUCT_PSU + CAN_BASE_ID_FOR_FAMILY_PSU +  CAN_BASE_ID_FOR_TURNONPSU_GLOBAL
#define MSG_ID_GLOBAL_COMMAND_TO_SETVOLTAGE_PSU                                                             CAN_BASE_ID_FOR_TRANSMIT_PSU + CAN_BASE_ID_FOR_PRODUCT_PSU + CAN_BASE_ID_FOR_FAMILY_PSU +  CAN_BASE_ID_FOR_VOLT_SET_GLOBAL
#define MSG_ID_GLOBAL_COMMAND_TO_SETCURRENT_PSU                                                             CAN_BASE_ID_FOR_TRANSMIT_PSU + CAN_BASE_ID_FOR_PRODUCT_PSU + CAN_BASE_ID_FOR_FAMILY_PSU +  CAN_BASE_ID_FOR_CURR_SET_GLOBAL


#define CAN_BASE_ID_FOR_MEASURED_PARAMS                                                                     0x00000020
#define CAN_BASE_ID_FOR_FAULTS                                                                              0x00000020
#define CAN_BASE_ID_FOR_PROG_PARAMS                                                                         0x00000020

#define CAN_BASE_ID_FOR_TURNON_PSU                                                                           0x00000010
#define CAN_BASE_ID_FOR_VOLT_SET_PSU                                                                         0x00000011
#define CAN_BASE_ID_FOR_CURR_SET_PSU                                                                         0x00000012


#define MSG_ID_MODEL_SPECIFIC_QUERY_TO_TURNON_PSU                                                           CAN_BASE_ID_FOR_TRANSMIT_PSU + CAN_BASE_ID_FOR_PRODUCT_PSU + CAN_BASE_ID_FOR_FAMILY_PSU + CAN_BASE_ID_FOR_MODEL_PSU + CAN_BASE_ID_FOR_TURNON_PSU
#define MSG_ID_MODEL_SPECIFIC_QUERY_TO_SETVOLTAGE_PSU                                                       CAN_BASE_ID_FOR_TRANSMIT_PSU + CAN_BASE_ID_FOR_PRODUCT_PSU + CAN_BASE_ID_FOR_FAMILY_PSU + CAN_BASE_ID_FOR_MODEL_PSU + CAN_BASE_ID_FOR_VOLT_SET_PSU
#define MSG_ID_MODEL_SPECIFIC_QUERY_TO_SETCURRENT_PSU                                                       CAN_BASE_ID_FOR_TRANSMIT_PSU + CAN_BASE_ID_FOR_PRODUCT_PSU + CAN_BASE_ID_FOR_FAMILY_PSU + CAN_BASE_ID_FOR_MODEL_PSU + CAN_BASE_ID_FOR_CURR_SET_PSU
#define MSG_ID_MODEL_SPECIFIC_QUERY_TO_MEASURED_PARAMS_PSU                                                  CAN_BASE_ID_FOR_TRANSMIT_PSU + CAN_BASE_ID_FOR_PRODUCT_PSU + CAN_BASE_ID_FOR_FAMILY_PSU + CAN_BASE_ID_FOR_MODEL_PSU + CAN_BASE_ID_FOR_MEASURED_PARAMS
#define MSG_ID_MODEL_SPECIFIC_QUERY_TO_PROG_PARAMS_PSU                                                      CAN_BASE_ID_FOR_TRANSMIT_PSU + CAN_BASE_ID_FOR_PRODUCT_PSU + CAN_BASE_ID_FOR_FAMILY_PSU + CAN_BASE_ID_FOR_MODEL_PSU + CAN_BASE_ID_FOR_PROG_PARAMS

#define MSG_ID_MODEL_SPECIFIC_QUERY_TO_FAULTS_PSU                                                           CAN_BASE_ID_FOR_TRANSMIT_PSU + CAN_BASE_ID_FOR_PRODUCT_PSU + CAN_BASE_ID_FOR_FAMILY_PSU + CAN_BASE_ID_FOR_MODEL_PSU + CAN_BASE_ID_FOR_FAULTS


// MACROS FOR I/O CARD

#define IO_DONTCARE   0
#define IO_DO_SET     1
#define IO_DO_CLEAR   2

#define IO_GENERAL_QUERY          0
#define IO_COMMAND_QUERY_AIBLK1   1
#define IO_COMMAND_QUERY_AIBLK2   2
#define IO_COMMAND_QUERY_THERMO   3
#define IO_COMMAND_QUERY_GENERIC  4

#define IO_QUERY_AIBLK1   0x01
#define IO_QUERY_AIBLK2   0x02
#define IO_QUERY_THERMO   0x03
#define IO_QUERY_GENERIC  0x04

#define LPC_100_NODE    0
#define LHC_101_NODE    1
#define LHC_102_NODE    2

#define CAN_LPC_100_NODE    1
#define CAN_LHC_101_NODE    2
#define CAN_LHC_102_NODE    3

#define DEFAULT_SETDO_LPC_100 0x00
#define DEFAULT_SETDO_LHC_101 0x00
#define DEFAULT_SETDO_LHC_102 0x00

#define DEFAULT_SETAI_LPC_100 0x00
#define DEFAULT_SETAI_LHC_101 0x00
#define DEFAULT_SETAI_LHC_102 0x00


// Voltage Sensing card




// General msg IDs used for msg id creation
#define CAN_BASE_ID_FOR_TRANSMIT_VS                                                                        0x10000000
#define CAN_BASE_ID_FOR_PRODUCT_VS                                                                         0x04000000
#define CAN_BASE_ID_FOR_FAMILY_VS                                                                          0x00100000
#define CAN_BASE_ID_FOR_MODEL_VS                                                                           0x00010000

#define CAN_BASE_ID_FOR_MEAS_CELL_VOLT_VS                                                                  0x00000001



#define MSG_ID_MODEL_SPECIFIC_QUERY_TO_MEAS_CELL_VS                                                        CAN_BASE_ID_FOR_TRANSMIT_VS + CAN_BASE_ID_FOR_PRODUCT_VS + CAN_BASE_ID_FOR_FAMILY_VS + CAN_BASE_ID_FOR_MODEL_VS + CAN_BASE_ID_FOR_MEAS_CELL_VOLT_VS

#define MSG_ID_MODEL_SPECIFIC_QUERY_IO                                                                     0x1421000A
#define MSG_ID_MODEL_SPECIFIC_QUERY_TO_LHC101                                                              0x1421020A
#define MSG_ID_MODEL_SPECIFIC_QUERY_TO_LHC102                                                              0x1421030A

typedef struct  // Various state timers implemented in the project
{
    Uint16 TX_count;
    Uint16 TX_count1;
    Uint16 RX_count;
    Uint16 TX_count_VS;
    Uint16 RX_count_VS;
    Uint16 RX_count_IO;
    Uint16 TX_count_Global;
    Uint16 PSU_Count;
    Uint16 RX_count_Node1;
    Uint16 CAN_Fail_count_Node[7];
    Uint16 CAN_Fail_count_Node1;
    Uint16 CAN_Fail_count_Node2;
    Uint16 CAN_Fail_count_Node3;
    Uint16 CAN_Fail_count_Node4;
    Uint16 CAN_Fail_count_Node5;
    Uint16 CAN_Fail_count_Node6;
    Uint16 CAN_failCountPSU_Node1;
    Uint16 CAN_failCountPSU_Node2;
    Uint16 CAN_failCountPSU_Node3;
    Uint16 CAN_failCountPSU_Node4;
    Uint16 CAN_failCountPSU_Node5;
    Uint16 CAN_failCountPSU_Node6;
    Uint16 CAN_failCountPSU_Node7;
    Uint16 CAN_failCountPSU_Node8;
    Uint16 CAN_failCountPSU_Node9;
    Uint16 CAN_failCountPSU_Node10;
    Uint16 CAN_failCountPSU_Node11;
    Uint16 CAN_failCountPSU_Node12;
    Uint16 CAN_failCountPSU_Node13;
    Uint16 CAN_failCountPSU_Node14;
    Uint16 CAN_failCountPSU_Node15;
    Uint16 CAN_failCountPSU_Node16;
    Uint16 CAN_failCountPSU_Node17;
    Uint16 CAN_failCountPSU_Node18;
    Uint16 CAN_failCountPSU_Node19;
    Uint16 CAN_failCountPSU_Node20;
    Uint16 CAN_failCountPSU_Node21;
    Uint16 CAN_failCountPSU_Node22;
    Uint16 CAN_failCountPSU_Node23;
    Uint16 CAN_failCountPSU_Node24;
    Uint16 CAN_failCountPSU_Node25;
    Uint16 CAN_failCountPSU_Node26;
    Uint16 CAN_failCountPSU_Node27;

    Uint16 TX_count_IO_Query;
    Uint16 TX_count_IO_Query2;
    Uint16 TX_count_IO_Purge;
    Uint16 Time_count_IO_Purge;
    Uint16 Seconds_count_IO_Purge;
    Uint16 Minutes_count_IO_Purge;
    Uint16 TX_count_IO;
    Uint16 Minutes_flag_IO_Purge;

}CANA_timers_t;


typedef struct  // Various state timers implemented in the project
{
    Uint16 Node;
    float32 Voltage_Set;
    float32 Current_Set;
    float32 Prev_Current_Set;
    Uint16 CurrRefChange;
    float32 Total_Curr_Set;

}CANA_Ui_Regs_R;

typedef struct
{
    Uint16 OFF_Command;
    Uint16 FSM_State;
    float32 Max_Current_Reference;
    Uint16 Max_Current_Reference_int;
    Uint16 Max_Current_Reference_fract;
    float32 Voltage_Reference;
    Uint16 Voltage_Reference_int;
    Uint16 Voltage_Reference_fract;
    Uint16 StateChange;
    Uint16 StateChangeStandBy;
    uint16_t MeasVoltParams_PSU[9];
    uint16_t MeasCurrParams_PSU[9];

}Site_CANCommands;


typedef struct
{
  Uint16 MBox1;
  Uint16 MBox2;
  Uint16 MBox3;
  Uint16 MBox4;
  Uint16 MBox5;
  Uint16 MBox6;
  Uint16 MBox7;
  Uint16 MBox12;



  Uint16 MBox16;
  Uint16 MBox17;
  Uint16 MBox18;
  Uint16 MBox19;



  Uint16 MsgId1_PSU;
  Uint16 MsgId2_PSU;
  Uint16 MsgId3_PSU;

  Uint16 MsgId4_VS;

  Uint16 IO_AIBLK1_MSGID;
  Uint16 IO_AIBLK2_MSGID;
  Uint16 IO_THERMO_MSGID;
  Uint16 IO_GENERIC_MSGID;

  Uint16 MsgIdVSInt;

  Uint16 MsgID1_Site;
  Uint16 MsgID2_Site;
  Uint16 MsgID3_Site;



}CAN_Read;


// ENUMS
typedef enum
{
   IDLE_PSU,
   SET_VOLT,
   SET_CURR,
   QUERY_PROGPARAM,
   TURN_ON,
   TURN_ONFEC,
   TURN_OFF,
   QUERY_OP_PARAM,
   QUERY_FLTS,
   SAVE_SETTINGS,
   LOCAL_REMOTE

}PSU_QUERY_VALUE;

typedef enum
{
   LPC_100,
   LHC_101,
   LHC_102
} IO_CARD_VALUE;

typedef enum
{
   IDLE_VS,
   MEAS_CELL_VOLT
}VS_QUERY_VALUE;

typedef enum
{
    IDLE_IO,
    AI_BLK1_QUERY,
    AI_BLK2_QUERY,
    THERMO_QUERY,
    GENERIC_QUERY,
    PRT_ENABLE_QUERY,
    PRT_READ_VFD_ON,
    PURGE_FAN1_ON,
    PURGE_FAN2_ON,
    PURGE_FAN3_ON,
    LHC_POWER_ON,
    LHC101_DEFAULT,
    LHC102_DEFAULT,
    LHC101_QUERY_LVL,
    DEFAULT

}IO_QUERY_VALUE;

struct IO_DO_REG_BITS
{
    Uint16 DO_bit0:1;
    Uint16 DO_bit1:1;
    Uint16 DO_bit2:1;
    Uint16 DO_bit3:1;
    Uint16 DO_bit4:1;
    Uint16 DO_bit5:1;
    Uint16 DO_bit6:1;
    Uint16 DO_bit7:1;
};

union IO_DO_REG
{
    Uint16 all;
    struct IO_DO_REG_BITS bit;
};

struct IO_DI_REG_BITS
{
    Uint16 DI_bit0:1;
    Uint16 DI_bit1:1;
    Uint16 DI_bit2:1;
    Uint16 DI_bit3:1;
    Uint16 DI_bit4:1;
    Uint16 DI_bit5:1;
    Uint16 DI_bit6:1;
    Uint16 DI_bit7:1;
};

union IO_DI_REG
{
    Uint16 all;
    struct IO_DI_REG_BITS bit;
};

typedef struct
{
  Uint16 PSU_Node;
  Uint16 Hi_PSU_Node;
  Uint16 VS_Node;
  Uint16 VS_NodeInt;
  Uint16 MsgId1;
  IO_CARD_VALUE IOCard_Type;
  Uint16 IOCard_Node;
}CAN_Node;

struct AI_ENABLE_REG_BITS
{
    Uint16 AI_EN_bit0:1;
    Uint16 AI_EN_bit1:1;
    Uint16 AI_EN_bit2:1;
    Uint16 AI_EN_bit3:1;
    Uint16 AI_EN_bit4:1;
    Uint16 AI_EN_bit5:1;
    Uint16 AI_EN_bit6:1;
    Uint16 AI_EN_bit7:1;
};

union AI_ENABLE_REG
{
    Uint16 all;
    struct AI_ENABLE_REG_BITS bit;
};

typedef struct
{
    PSU_QUERY_VALUE PSU;
    VS_QUERY_VALUE VS;
    IO_QUERY_VALUE IO;

}Query_Type;

typedef struct
{
    Uint16 RefVolt_max;
    Uint16 RefCurr_max;
}SystemParams;


typedef struct
{
    Uint16 AO_1;
    Uint16 AO_2;
}AOx_Data;

typedef struct
{
    float32 AI0_Data;
    float32 AI1_Data;
    float32 AI2_Data;
    float32 AI3_Data;
    float32 AI4_Data;
    float32 AI5_Data;
    float32 AI6_Data;
    float32 AI7_Data;
}AIx_Data;

typedef struct
{
    Uint16 T0_Data;
    Uint16 T1_Data;
    Uint16 T2_Data;
    Uint16 T3_Data;
}THERMODATA;

typedef struct
{
    float HYS_101;
    float HYS_501;
    float HYS_401;

    float LVL_101;
    float LVL_102;
    float PRT_101;
    float PRT_103;
    float PRT_301;
    float PRT_401;

    float LVL_401;
    float PRT_102;
    float COS_101;



}AI_SensorData1;

typedef struct
{
    Uint16 PurgeFansON;
    bool PurgeFan1ON_ACK;
    bool PurgeFan2ON_ACK;
    bool PurgeFan3ON_ACK;
    bool LHC_POwerON;

    bool LPC100_DefaultACK;
    bool LHC101_DefaultACK;
    bool LHC102_DefaultACK;

    bool VFD101_ONAck;

    bool hydrogenValuesOK;
    bool hydrogenValuesOK2;
    bool WaterLevelOk;
    bool PressureNotOK;

}IO_FLAG;

typedef struct
{
   Uint16  Volt_int[TOTAL_PSU_NODE_NUMBER+1];
   Uint16 Volt_frac[TOTAL_PSU_NODE_NUMBER+1];
   float32 Volt_Node[TOTAL_PSU_NODE_NUMBER+1];
   Uint16  Curr_int[TOTAL_PSU_NODE_NUMBER+1];
   Uint16 Curr_frac[TOTAL_PSU_NODE_NUMBER+1];
   float32 Curr_Node[TOTAL_PSU_NODE_NUMBER+1];
   Uint16  Prog_Volt_int[TOTAL_PSU_NODE_NUMBER+1];
   Uint16 Prog_Volt_frac[TOTAL_PSU_NODE_NUMBER+1];
   float32 Prog_Volt_Node[TOTAL_PSU_NODE_NUMBER+1];
   Uint16  Prog_Curr_int[TOTAL_PSU_NODE_NUMBER+1];
   Uint16 Prog_Curr_frac[TOTAL_PSU_NODE_NUMBER+1];
   float32 Prog_Curr_Node[TOTAL_PSU_NODE_NUMBER+1];


   float32 Meas_Cell_Volt[8][33];
   Uint16 Channel_ID_VS;
   float32 Cell_Voltage[33];

   // In Can.c

   float32 Max_Cell_Volt1[8];
   float32 Max_Cell_Volt2[8];

   float32 Min_Cell_Volt1[8];
   float32 Min_Cell_Volt2[8];

   Uint16 Max_Cell_Num1[8];
   Uint16 Max_Cell_Num2[8];

   Uint16 Min_Cell_Num1[8];
   Uint16 Min_Cell_Num2[8];


   float32 Highest_Cell_Volt[2];
   Uint16 Highest_Cell_Num[2];
   Uint16 Highest_Cell_Volt_Node[2];

   float32 Lowest_Cell_Volt[2];
   Uint16 Lowest_Cell_Num[2];
   Uint16 Lowest_Cell_Volt_Node[2];

   float32 Max_Cell_Voltage_All_Nodes[13];
   Uint16  Max_Cell_Number_All_Nodes[13];
   Uint16  Max_Cell_Voltage_Node_Num[13];

   float32 Min_Cell_Voltage_All_Nodes[13];
   Uint16  Min_Cell_Number_All_Nodes[13];
   Uint16  Min_Cell_Voltage_Node_Num[13];

   // In ISR.c

   float32 Max_Cell_Volt1_Int[8];
   float32 Max_Cell_Volt2_Int[8];

   float32 Min_Cell_Volt1_Int[8];
   float32 Min_Cell_Volt2_Int[8];

   Uint16 Max_Cell_Num1_Int[8];
   Uint16 Max_Cell_Num2_Int[8];

   Uint16 Min_Cell_Num1_Int[8];
   Uint16 Min_Cell_Num2_Int[8];


   float32 Highest_Cell_Volt_Int[2];
   Uint16 Highest_Cell_Num_Int[2];
   Uint16 Highest_Cell_Volt_Node_Int[2];

   float32 Lowest_Cell_Volt_Int[2];
   Uint16 Lowest_Cell_Num_Int[2];
   Uint16 Lowest_Cell_Volt_Node_Int[2];

   float32 Max_Cell_Voltage_All_Nodes_Int[13];
   Uint16  Max_Cell_Number_All_Nodes_Int[13];
   Uint16  Max_Cell_Voltage_Node_Num_Int[13];

   float32 Min_Cell_Voltage_All_Nodes_Int[13];
   Uint16  Min_Cell_Number_All_Nodes_Int[13];
   Uint16  Min_Cell_Voltage_Node_Num_Int[13];



}CAN_Recd_Params;



// Fault Register
struct FAULT_REG_BITS
{
    Uint16 bt_AC_Fault:1;                           // Bit 0
    Uint16 bt_Otp_Fault:1;                          // Bit 1
    Uint16 bt_Fold_Fault:1;                         // Bit 2
    Uint16 bt_Ovp_Fault:1;                          // Bit 3
    Uint16 bt_ShutOff_Fault:1;                      // Bit 4
    Uint16 bt_Out_Fault:1;                          // Bit 5
    Uint16 bt_Interlock_Fault:1;                    // Bit 6
    Uint16 bt_Fan_Fault:1;                          // Bit 7
    Uint16 bt_Reserved:1;                           // Bit 8
    Uint16 bt_PFC_DC_Bus_Fault:1;                   // Bit 9
    Uint16 bt_Internal_Communication_Fault:1;       // Bit 10   I2C, SPI
    Uint16 bt_Eeprom_Checksum_Failure:1;            // Bit 11
    Uint16 bt_On_Board_Adc_Failure:1;               // Bit 12
    Uint16 bt_Illegal_Math_Performed:1;             // Bit 13
    Uint16 bt_Reserved_2:1;                         // Bit 14
    Uint16 bt_Reserved_3:1;                         // Bit 15
};

union FAULT_REGS
{
    Uint16 all;
    struct FAULT_REG_BITS bit;
};


// Fault Register
struct DC_FAULT_REG_BITS
{
    Uint16 bt_rsvd:1;        // Bit 0
    Uint16 bt_node1:1;       // Bit 1
    Uint16 bt_node2:1;       // Bit 2
    Uint16 bt_node3:1;       // Bit 3
    Uint16 bt_node4:1;       // Bit 4
    Uint16 bt_node5:1;       // Bit 5
    Uint16 bt_node6:1;       // Bit 6
    Uint16 bt_node7:1;       // Bit 7
    Uint16 bt_node8:1;       // Bit 8
    Uint16 bt_node9:1;       // Bit 9
    Uint16 bt_node10:1;       // Bit 10
    Uint16 bt_node11:1;       // Bit 11
    Uint16 bt_node12:1;       // Bit 12
    Uint16 bt_node13:1;       // Bit 13
    Uint16 bt_node14:1;       // Bit 14
    Uint16 bt_node15:1;       // Bit 15
    Uint16 bt_node16:1;       // Bit 16
    Uint16 bt_node17:1;       // Bit 17
    Uint16 bt_node18:1;       // Bit 18
    Uint16 bt_node19:1;       // Bit 19
    Uint16 bt_node20:1;       // Bit 20
    Uint16 bt_node21:1;       // Bit 21
    Uint16 bt_node22:1;       // Bit 22
    Uint16 bt_node23:1;       // Bit 23
    Uint16 bt_node24:1;       // Bit 24
    Uint16 bt_node25:1;       // Bit 25
    Uint16 bt_node26:1;       // Bit 26
    Uint16 bt_node27:1;       // Bit 27
    Uint16 bt_node28:1;       // Bit 28
    Uint16 bt_node29:1;       // Bit 29
    Uint16 bt_node30:1;       // Bit 30
    Uint16 bt_node31:1;       // Bit 31
};

union DC_FAULT_REGS
{
    Uint32 all;
    struct DC_FAULT_REG_BITS bit;
};

struct NODE_FAULT_REG_BITS
{
    Uint16 bt_rsvd:1;        // Bit 0
    Uint16 bt_node1:1;       // Bit 1
    Uint16 bt_node2:1;       // Bit 2
    Uint16 bt_node3:1;       // Bit 3
    Uint16 bt_node4:1;       // Bit 4
    Uint16 bt_node5:1;       // Bit 5
    Uint16 bt_node6:1;       // Bit 6
    Uint16 bt_node7:1;       // Bit 7
    Uint16 bt_node8:1;       // Bit 8
    Uint16 bt_node9:1;       // Bit 9
    Uint16 bt_node10:1;       // Bit 10
    Uint16 bt_node11:1;       // Bit 11
    Uint16 bt_node12:1;       // Bit 12
    Uint16 bt_node13:1;       // Bit 13
    Uint16 bt_node14:1;       // Bit 14
    Uint16 bt_node15:1;       // Bit 15
    Uint16 bt_node16:1;       // Bit 16
    Uint16 bt_node17:1;       // Bit 17
    Uint16 bt_node18:1;       // Bit 18
    Uint16 bt_node19:1;       // Bit 19
    Uint16 bt_node20:1;       // Bit 20
    Uint16 bt_node21:1;       // Bit 21
    Uint16 bt_node22:1;       // Bit 22
    Uint16 bt_node23:1;       // Bit 23
    Uint16 bt_node24:1;       // Bit 24
    Uint16 bt_node25:1;       // Bit 25
    Uint16 bt_node26:1;       // Bit 26
    Uint16 bt_node27:1;       // Bit 27
    Uint16 bt_node28:1;       // Bit 28
    Uint16 bt_node29:1;       // Bit 29
    Uint16 bt_node30:1;       // Bit 30
    Uint16 bt_node31:1;       // Bit 31
};

union NODE_FAULT_REGS
{
    Uint32 all;
    struct NODE_FAULT_REG_BITS bit;
};

// Active Node Register
struct ACTIVE_NODE_BITS
{
    Uint16 bt_rsvd:1;        // Bit 0
    Uint16 bt_node1:1;       // Bit 1
    Uint16 bt_node2:1;       // Bit 2
    Uint16 bt_node3:1;       // Bit 3
    Uint16 bt_node4:1;       // Bit 4
    Uint16 bt_node5:1;       // Bit 5
    Uint16 bt_node6:1;       // Bit 6
    Uint16 bt_node7:1;       // Bit 7
    Uint16 bt_node8:1;       // Bit 8
    Uint16 bt_node9:1;       // Bit 9
    Uint16 bt_node10:1;       // Bit 10
    Uint16 bt_node11:1;       // Bit 11
    Uint16 bt_node12:1;       // Bit 12
    Uint16 bt_node13:1;       // Bit 13
    Uint16 bt_node14:1;       // Bit 14
    Uint16 bt_node15:1;       // Bit 15
};

typedef struct ACTIVE_NODE_BITS1
{
    Uint16 bt_rsvd1:1;       // Bit 16
    Uint16 bt_node16:1;       // Bit 17
    Uint16 bt_node17:1;       // Bit 18
    Uint16 bt_node18:1;       // Bit 19
    Uint16 bt_node19:1;       // Bit 20
    Uint16 bt_node20:1;       // Bit 21
    Uint16 bt_node21:1;       // Bit 22
    Uint16 bt_node22:1;       // Bit 23
    Uint16 bt_node23:1;       // Bit 24
    Uint16 bt_node24:1;       // Bit 25
    Uint16 bt_node25:1;       // Bit 26
    Uint16 bt_node26:1;       // Bit 27
    Uint16 bt_node27:1;       // Bit 28
    Uint16 bt_node28:1;       // Bit 29
    Uint16 bt_node29:1;       // Bit 30
    Uint16 bt_node30:1;       // Bit 31
};

union ACTIVE_NODE_REGS
{
    Uint16 all;
    struct ACTIVE_NODE_BITS bit;
};

union ACTIVE_NODE_REGS1
{
    Uint16 all;
    struct ACTIVE_NODE_BITS1 bit;
};


#endif /* SOURCE_EI_CAN_H_ */
