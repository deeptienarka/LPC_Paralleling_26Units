//#################################################################################################################
//#                             enArka Instruments proprietary
//# File: ei_I2cApp.c
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.
//# All Rights Reserved
//#
//#################################################################################################################

//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
#include "ExternalVariables.h"
//#################################################################################################################
// Specific defines
#define I2C_DELAY           500
#define I2C_WAIT_UNTIL(x)   { \
                                timeout_cnt = 0; \
                                while((x)) \
                                { \
                                    timeout_cnt++; \
                                    if (timeout_cnt >= I2C_DELAY) \
                                        { \
                                        Fault_Regs.FAULT_REGISTER.bit.bt_Internal_Communication_Fault = 1; \
                                        ei_vResetPsuToSafeState(); \
                                        return FAILURE; \
                                        } \
                                } \
                            }
volatile Uint16 timeout_cnt;
//#################################################################################################################
                                    /*-------Global Prototypes----------*/
//Uint16 ei_uiI2CWrite(Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Write_Array[]);
//Uint16 ei_uiI2CRead(Uint16 Slave_address, Uint16 Start_address,Uint16 No_of_databytes, Uint16 Read_Array[]);
//void ei_vReadTemperature();
//void ei_vTempSensorInitialization();
//void ei_vWriteLastSettingsEEPROM(Uint16 Cal_Mode);
//void ei_vWriteOnTimeToArray(Uint16 *I2CArray);
//#################################################################################################################

//###################################   I2C Bus Baud Rate Settings  ###############################################

//      Sysclkout is input to the I2C module. We operate at 90MHz sysclkout.
//
//      I2C Baud Rate = I2C_Input_Clock_Frequency/{ (IPSC + 1)[(ICCL + d) + (ICCH + d)] }
//
//              IPSC vs d                               Our Settings:
//          IPSC        d                               IPSC: 8 ==> d: 5
//          0           7                               ICCL: 10 ==> SCL Low time = 1.5uS   (See Notes below)
//          1           6                               ICCH: 5 ==> SCL High time = 1uS     (See Notes below)
//          > 1         5
//
//      Baud Rate set = 90M/{(8+1)[(10+5)+(5+5)]} = 400Kbps

// Important Notes:
// 1. I2C Module Clock must be between 7-12MHz [ I2C Module Clock = I2C_Input_Clock_Frequency/(IPSC +1) ]
// 2. We have set I2C Module Clock at 10M.(It is advised that to change baud rate, we play with only ICCH and ICCL. Let
//    IPSC remain at 8. And to change ICCL and ICCH read 3,4,5,6,7,8,9).
// 3. ICCL and ICCH must be non zero
// 4. Every I2C based IC device has a minimum specification for SCL low time and SCL high time. These
//    values must be factored into account while choosing the I2C baud Rate. 
// 5. SCL low time = I2C Module Clock/(ICCL + d)   AND
//    SCL high time = I2C Module Clock/(ICCH + d)
// 6. For EEPROM 24LC256                (Tlow,Thigh) >= (1.3uS,0.6uS)
// 7. For MAX6958/59                    (Tlow,Thigh) >= (1.3uS,0.6uS)
// 8. For TMP75                         (Tlow,Thigh) >= (1.3uS,0.6uS)
// 9. For our TI DSP (TMS320F2808)      (Tlow,Thigh) >= (1.3uS,0.6uS)
// 10. THE BAUD SETTING MUST NOT BE HIGHER THAN 400kbps. PERFORMANCE NOT GUARANTEED ABOVE THAT.
//#################################################################################################################

//##################### I2C_Write #################################################################################
// Function Name: ei_uiI2CWrite
// Return Type: Uint16
// Arguments:   Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Write_Array[]
// Description: I2C Write Driver. Pass Slave Address, Write location, No of databytes and the array with data. Returns
// I2C operation status (SUCCESS or FAILURE). On FAILURE PSU automatically taken to safe state and Fault bit set.  
//#################################################################################################################
Uint16 ei_uiI2CWrite(Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Write_Array[])
{
    Uint16 i = 0;

    I2caRegs.I2CSAR = Slave_address;
    I2C_WAIT_UNTIL(I2caRegs.I2CMDR.bit.STP != 0)

    //  Start bit, write mode, Higher 16 address bits, Master, Repeat mode.
     I2caRegs.I2CMDR.bit.TRX = TRANSMIT_MESSAGE;

//######## Only for EEPROM #########################//
     if(Slave_address == EEPROM_24LC256_ADDRESS)
     {
        I2caRegs.I2CDXR = (Start_address)>>8;
     }
//##################################################//
     I2caRegs.I2CMDR.all = 0x26A0;

    //(Lower 16) address bits
     I2C_WAIT_UNTIL(I2caRegs.I2CSTR.bit.ARDY != 1)
     if (I2caRegs.I2CSTR.bit.ARDY == 1)
     {
        I2caRegs.I2CDXR = Start_address;
     }

    // Data Bytes to be transmitted
     while(i < No_of_databytes)
     {
        I2C_WAIT_UNTIL(I2caRegs.I2CSTR.bit.ARDY != 1)
        if (I2caRegs.I2CSTR.bit.ARDY == 1)
        {
            I2caRegs.I2CDXR = Write_Array[i++];  // Lower 16 address bits
        }
     }

     while (I2caRegs.I2CSTR.bit.NACK == 1) // Clear if NACK received
     {
        I2caRegs.I2CSTR.bit.NACK = 1;
     }

     I2caRegs.I2CMDR.all = 0x0EA0;
     I2C_WAIT_UNTIL(I2caRegs.I2CSTR.bit.SCD != 1)
     I2caRegs.I2CSTR.bit.SCD = 1;   // Clear stop condition

     return SUCCESS;
}

//##################### I2C_Read ##################################################################################
// Function Name:ei_uiI2CRead
// Return Type: Uint16 
// Arguments:   Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Read_Array[]
// Description: I2C Read Driver. Pass Slave Address, Write location, No of databytes, Array where received 
//              will be copied. Returns I2C operation status (SUCCESS or FAILURE). On FAILURE PSU automatically
//              taken to safe state and Fault bit set.
//#################################################################################################################
Uint16 ei_uiI2CRead(Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Read_Array[])
{
    Uint16 i = 0;
    I2caRegs.I2CSAR = Slave_address;
    I2caRegs.I2CCNT = No_of_databytes; // When operating in non repeat mode, this value will determine how many bytes to receive/send.
    Uint16 *Temp_Pointer;
    Temp_Pointer = Read_Array;
 
    I2C_WAIT_UNTIL(I2caRegs.I2CMDR.bit.STP != 0)  // Detect a stop bit
 
    //  Start bit, write mode, Higher 16 address bits, Master, Non Repeat mode.
    I2caRegs.I2CMDR.bit.TRX = 1;
 
 //######## Only for EEPROM #########################//
    if(Slave_address == EEPROM_24LC256_ADDRESS)
    {
        I2caRegs.I2CDXR = (Start_address)>>8;
    }
//##################################################//
    I2caRegs.I2CMDR.all = 0x26A0;
 

    I2C_WAIT_UNTIL(I2caRegs.I2CSTR.bit.ARDY != 1)

    if (I2caRegs.I2CSTR.bit.ARDY == 1)
    {
        I2caRegs.I2CDXR = Start_address; // (Lower 16) address bits
    }
 
    I2C_WAIT_UNTIL(I2caRegs.I2CSTR.bit.ARDY != 1)

 // Put a stop bit. Because we need to change to non repeat mode. 
    I2caRegs.I2CMDR.bit.STP = 1;
 
    I2C_WAIT_UNTIL(I2caRegs.I2CMDR.bit.STP != 0)  // Detect a stop bit
    I2caRegs.I2CMDR.all = 0x2C20;
    while(i < No_of_databytes)
    {
        I2C_WAIT_UNTIL(I2caRegs.I2CSTR.bit.RRDY != 1)
        *Temp_Pointer++ = I2caRegs.I2CDRR;
        i++;
    }
    return SUCCESS;
}

//##################### Last settings save ########################################################################
// Function Name: ei_vWriteLastSettingsEEPROM
// Return Type: void
// Arguments:   Uint16 Mode
// Description: Write last settings into the EEPROM depending on whether Product is Factory default or one time
//              calibrated.
//
//      This function can be understood by using Excel sheet "AUM_1U_System_Details.xlsx" as a reference.
//      Single bit and 2 bit values are Ored and stored in same location.
//
//      This calls a seperate function to Write Time On value to appropriate locations in the Array and dumps
//      the whole array into EEPROM at a time.
//#################################################################################################################
void ei_vWriteLastSettingsEEPROM(Uint16 Cal_Mode)
{
    Uint16 I2C_Tx_Data_Array[48];
    Uint16 i,Checksum;
    Checksum = 0;
    int32 Temp;
    Temp = 0;
    switch(Cal_Mode)
    {
        case(FACTORY_DEFAULT):
        {
                //      Write default last settings in the EEPROM
            // PV_C:0x0000, PV_F:0x0000,PC_C: QEP_FULL_ROTATIONS_COUNT, PC_F: QEP_FULL_ROTATIONS_COUNT/2, OVP: QEP_MAX_OVP_COUNT, UVL: 0x0000, OUT: 0(Output off),
            // FOLD: 0(Foldback off), AST: 0(Safe start),Address: 120, Baudrate: 70, REM/LOC: 0(Remote), LFP/UFP: 0(Unlocked FrontPanel), M/S: 0 (Slave), Foldback delay: 0
            // All References are 0.
            // All Enable registers are 0.
            for(i = 0; i < 2; i++)
            {
                I2C_Tx_Data_Array[i] = 0;
            }
            // Voltage Fine
            I2C_Tx_Data_Array[2] = ((QEP_FULL_ROTATIONS_COUNT/2) >> 8) & 0x00FF;    // Higher 8 bits
            I2C_Tx_Data_Array[3] = (QEP_FULL_ROTATIONS_COUNT/2) & 0x00FF;           // Lower 8 bits

            // current default value must be at max according to specs.
            // Current Coarse
            I2C_Tx_Data_Array[4] = (QEP_FULL_ROTATIONS_COUNT) >> 8; // Higher 8 bits
            I2C_Tx_Data_Array[5] = (QEP_FULL_ROTATIONS_COUNT) & 0x00FF; // Lower 8 bits

            // Current Fine
            I2C_Tx_Data_Array[6] = ((QEP_FULL_ROTATIONS_COUNT/2) >> 8) & 0x00FF;    // Higher 8 bits
            I2C_Tx_Data_Array[7] = (QEP_FULL_ROTATIONS_COUNT/2) & 0x00FF;           // Lower 8 bits


            I2C_Tx_Data_Array[8] = (QEP_MAX_OVP_COUNT)>> 8; // Higher 8 bits
            I2C_Tx_Data_Array[9] = (QEP_MAX_OVP_COUNT) & 0x00FF;       // Lower 8 bits

            I2C_Tx_Data_Array[10] = 0;
            I2C_Tx_Data_Array[11] = 0;

            // Power On Clear status is ON (1) in factory default. So we are writing 1 to bit 7 in this location of EEPROM.
            I2C_Tx_Data_Array[12] = (ON << 7) & 0x80;   // Mask Lower 6

            I2C_Tx_Data_Array[13] = (QEP_DEFAULT_ADDR_COUNT) >> 8;      //  Higher 8 bits
            I2C_Tx_Data_Array[14] = (QEP_DEFAULT_ADDR_COUNT) & 0x00FF;  //  Lower 8 bits
            I2C_Tx_Data_Array[15] = (QEP_DEFAULT_BAUD_RATE_COUNT)>> 8;      // Higher 8 bits
            I2C_Tx_Data_Array[16] = (QEP_DEFAULT_BAUD_RATE_COUNT) & 0x00FF; // Lower 8 bits
            I2C_Tx_Data_Array[17] = MIN_PROTECTION_DELAY_IN_COUNTS;

            for(i = 18; i <= 40; i++)
            {
                I2C_Tx_Data_Array[i] = 0;
            }

            // Checksum calculation. Add all values to be written and the store the last two nibbles of the sum
            // in EEPROM

            for(i = 0; i < 41; i++)
            {
                Checksum += I2C_Tx_Data_Array[i];
            }

            I2C_Tx_Data_Array[41] = Checksum & 0x00FF;

    //      Call the Time On write function and pass the address of next location
            ei_vWriteOnTimeToArray(&I2C_Tx_Data_Array[42]);

            ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, PV_ADDRESS, 48, I2C_Tx_Data_Array);
            break;
        }
        case(ONE_TIME_CALIBRATED):
        {
            I2C_Tx_Data_Array[0] = (Encoder.i_VoltageKnob_Coarse_Count) >> 8; // Higher 8 bits
            I2C_Tx_Data_Array[1] = (Encoder.i_VoltageKnob_Coarse_Count) & 0x00FF; // Lower 8 bits

            I2C_Tx_Data_Array[2] = (Encoder.i_VoltageKnob_Fine_Count) >> 8; // Higher 8 bits
            I2C_Tx_Data_Array[3] = (Encoder.i_VoltageKnob_Fine_Count) & 0x00FF; // Lower 8 bits

            I2C_Tx_Data_Array[4] = (Encoder.i_CurrentKnob_Coarse_Count) >> 8; // Higher 8 bits
            I2C_Tx_Data_Array[5] = (Encoder.i_CurrentKnob_Coarse_Count) & 0x00FF; // Lower 8 bits

            I2C_Tx_Data_Array[6] = (Encoder.i_CurrentKnob_Fine_Count) >> 8; // Higher 8 bits
            I2C_Tx_Data_Array[7] = (Encoder.i_CurrentKnob_Fine_Count) & 0x00FF; // Lower 8 bits

            I2C_Tx_Data_Array[8] = (Encoder.ui_Qep_OVP_Count) >> 8; // Higher 8 bits
            I2C_Tx_Data_Array[9] = (Encoder.ui_Qep_OVP_Count) & 0x00FF; // Lower 8 bits

            I2C_Tx_Data_Array[10] = (Encoder.ui_Qep_UVL_Count) >> 8; // Higher 8 bits
            I2C_Tx_Data_Array[11] = (Encoder.ui_Qep_UVL_Count) & 0x00FF; // Lower 8 bits

            // Form the Bit Info Set 1  register (Reference excel sheet)
            Temp = (((Uint16)State.Mode.bt_Power_On_Status_Clear) << 7) | (((Uint16)State.Mode.bt_Master_Slave_Setting) << 6)
                    | (((Uint16)State.Mode.bt_FrontPanel_Access) << 5) | (((Uint16)State.Mode.bt_PSU_Control) << 3)
                    | (((Uint16)State.Mode.bt_Start_Mode) << 2) | (State.Mode.bt_Output_Status);

            I2C_Tx_Data_Array[12] = Temp;

            I2C_Tx_Data_Array[13] = (Encoder.ui_Qep_Address_Count) >> 8;        // Higher 8 bits
            I2C_Tx_Data_Array[14] = (Encoder.ui_Qep_Address_Count) & 0x00FF;    // Lower 8 bits
            I2C_Tx_Data_Array[15] = (Encoder.ui_Qep_BaudRate_Count)>> 8;        // Higher 8 bits
            I2C_Tx_Data_Array[16] = (Encoder.ui_Qep_BaudRate_Count) & 0x00FF;   // Lower 8 bits
            I2C_Tx_Data_Array[17] = Timers.ui_MaxFoldBackDelayInCounts;     // Copy from any one of the protection delays

            // store reference values which is a 32bit IQ no. in 4 EEPROM locations each

            // Voltage_Ref
            Temp = Reference.iq_Voltage_Reference;
            I2C_Tx_Data_Array[18] = (Temp >> 24) & 0x000000FF;
            I2C_Tx_Data_Array[19] = (Temp >> 16) & 0x000000FF;
            I2C_Tx_Data_Array[20] = (Temp >> 8) & 0x000000FF;
            I2C_Tx_Data_Array[21] = (Temp) & 0x000000FF;

            // Current_Ref
            Temp = Reference.iq_Current_Reference;
            I2C_Tx_Data_Array[22] = (Temp >> 24) & 0x000000FF;
            I2C_Tx_Data_Array[23] = (Temp >> 16) & 0x000000FF;
            I2C_Tx_Data_Array[24] = (Temp >> 8) & 0x000000FF;
            I2C_Tx_Data_Array[25] = (Temp) & 0x000000FF;

            // OVP_Ref
            Temp = Reference.iq_OVP_Reference;
            I2C_Tx_Data_Array[26] = (Temp >> 24) & 0x000000FF;
            I2C_Tx_Data_Array[27] = (Temp >> 16) & 0x000000FF;
            I2C_Tx_Data_Array[28] = (Temp >> 8) & 0x000000FF;
            I2C_Tx_Data_Array[29] = (Temp) & 0x000000FF;

            // UVL_Ref
            Temp = Reference.iq_UVL_Reference;
            I2C_Tx_Data_Array[30] = (Temp >> 24) & 0x000000FF;
            I2C_Tx_Data_Array[31] = (Temp >> 16) & 0x000000FF;
            I2C_Tx_Data_Array[32] = (Temp >> 8) & 0x000000FF;
            I2C_Tx_Data_Array[33] = (Temp) & 0x000000FF;

            // All enable registers

//          FAULT_ENABLE is 16 bit.
            I2C_Tx_Data_Array[34] = (Fault_Regs.FAULT_ENABLE.all) >> 8; // Higher 8 bits
            I2C_Tx_Data_Array[35] = (Fault_Regs.FAULT_ENABLE.all) & 0x00FF; // Lower 8 bits

//          OPERATION_CONDITION_ENABLE is 16 bit.
            I2C_Tx_Data_Array[36] = (Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all) >> 8; // Higher 8 bits
            I2C_Tx_Data_Array[37] = (Operation_Condition_Regs.OPERATION_CONDITION_ENABLE.all) & 0x00FF; // Lower 8 bits

//          STANDARD_EVENT_STATUS_ENABLE is 8 bit
            I2C_Tx_Data_Array[38] = Standard_Event_Status_Regs.STANDARD_EVENT_STATUS_ENABLE.all;

//          SERVICE_REQUEST_ENABLE is 8 bit
            I2C_Tx_Data_Array[39] = Service_Request_Regs.SERVICE_REQUEST_ENABLE.all;

//          Form Bit Info Set 2 (Refer EEPROM Locations in System Details document
            I2C_Tx_Data_Array[40] = (((Uint16)State.Mode.bt_Can_SRQ_Switch)<<2) | (State.Mode.bt_FoldBack_Mode & LAST_TWO_BITS_ONLY);

            // Checksum calculation. Add all values to be written and the store the last two nibbles of the sum
            // in EEPROM

            for(i = 0; i < 41; i++)
            {
                Checksum += I2C_Tx_Data_Array[i];
            }

            I2C_Tx_Data_Array[41] = Checksum & 0x00FF;

    //      Call the Time On write function and pass the address of next location
            ei_vWriteOnTimeToArray(&I2C_Tx_Data_Array[42]);

            while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
            ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, PV_ADDRESS, 48, I2C_Tx_Data_Array);
            RESET_EEPROM_WRITE_CYCLE_TIMER;

            break;
        }
    }
}

//##################### Write Timer ##############################################################################
// Function Name: ei_vWriteOnTimeToArray
// Return Type: void
// Arguments:   Uint16 *
// Description: Stores the time on related data in the array sent in the argument.
//              Byte key (PRODUCT_NOT_SWITCHED_ON_FOR_FIRST_TIME) in the first location
//              Time On value in the next 4 locations
//              Checksum of the above 5 locations in the last location.

//              A failure case writes every location to 0. (see the if inside)
//#################################################################################################################
void ei_vWriteOnTimeToArray(Uint16 *I2CArray)
{
    Uint16 i;
    // If timer not corrupted
    if(Product_Info.ul_Product_Power_On_Time_In_Minutes >= BASE_TIME_ON_VALUE_IN_CASE_OF_DATA_FAILURE)
    {
        // Data Corrupted. No Point Writing anything to EEPROM.

        // THIS REALLY REALLY SHOULD NOT HAPPEN.

        // Initialize everything to 0.
        I2CArray[0] = 0;
        I2CArray[1] = 0;
        I2CArray[2] = 0;
        I2CArray[3] = 0;
        I2CArray[4] = 0;
        I2CArray[5] = 0;
    }
    else
    {
        Uint16 Checksum;
        Checksum = 0;
        // Write the key
        I2CArray[0] = PRODUCT_NOT_SWITCHED_ON_FOR_FIRST_TIME;

        // Write the Total Calculated on time
        I2CArray[1] = (Product_Info.ul_Product_Power_On_Time_In_Minutes >> 24) & 0x000000FF;
        I2CArray[2] = (Product_Info.ul_Product_Power_On_Time_In_Minutes >> 16) & 0x000000FF;
        I2CArray[3] = (Product_Info.ul_Product_Power_On_Time_In_Minutes >> 8) & 0x000000FF;
        I2CArray[4] = (Product_Info.ul_Product_Power_On_Time_In_Minutes) & 0x000000FF;

        for(i = 0; i < 5; i++)
        {
            Checksum += I2CArray[i];
        }

        I2CArray[5]  = Checksum & 0x00FF;
    }
}


void TEST_I2CWrite()
{
    Uint16 a_ui_I2cTxDataArray[49];
//  Uint16 a_ui_I2cRxDataArray[49];
    Uint16 i;
    Uint16 Checksum;

    Checksum = 0;

    a_ui_I2cTxDataArray[0] = 11;
    a_ui_I2cTxDataArray[1] = 0x40;
    a_ui_I2cTxDataArray[2] = 0x00;
    a_ui_I2cTxDataArray[3] = 0x00;
    a_ui_I2cTxDataArray[4] = 0x04;
    a_ui_I2cTxDataArray[5] = 0x15;
    a_ui_I2cTxDataArray[6] = 0x00;
    a_ui_I2cTxDataArray[7] = 0x40;
    a_ui_I2cTxDataArray[8] = 0x00;
    a_ui_I2cTxDataArray[9] = 0x00;
    a_ui_I2cTxDataArray[10] = 0x04;
    a_ui_I2cTxDataArray[11] = 0x15;
    a_ui_I2cTxDataArray[12] = 0x00;
    a_ui_I2cTxDataArray[13] = 0x00;
    a_ui_I2cTxDataArray[14] = 0x00;
    a_ui_I2cTxDataArray[15] = 0x00;
    a_ui_I2cTxDataArray[16] = 0x00;
    a_ui_I2cTxDataArray[17] = 0x00;
    a_ui_I2cTxDataArray[18] = 0x00;
    a_ui_I2cTxDataArray[19] = 0x80;
    a_ui_I2cTxDataArray[20] = 0x00;
    a_ui_I2cTxDataArray[21] = 0x00;
    a_ui_I2cTxDataArray[22] = 0x00;
    a_ui_I2cTxDataArray[23] = 0x00;
    a_ui_I2cTxDataArray[24] = 0x00;
    a_ui_I2cTxDataArray[25] = 0x00;
    a_ui_I2cTxDataArray[26] = 0x00;
    a_ui_I2cTxDataArray[27] = 0x80;
    a_ui_I2cTxDataArray[28] = 0x00;
    a_ui_I2cTxDataArray[29] = 0x00;
    a_ui_I2cTxDataArray[30] = 0x00;
    a_ui_I2cTxDataArray[31] = 0x00;
    a_ui_I2cTxDataArray[32] = 0x00;
    a_ui_I2cTxDataArray[33] = 0x00;
    a_ui_I2cTxDataArray[34] = 0x00;
    a_ui_I2cTxDataArray[35] = 0x80;
    a_ui_I2cTxDataArray[36] = 0x00;
    a_ui_I2cTxDataArray[37] = 0x00;
    a_ui_I2cTxDataArray[38] = 0x00;
    a_ui_I2cTxDataArray[39] = 0x00;
    a_ui_I2cTxDataArray[40] = 0x00;
    a_ui_I2cTxDataArray[41] = 0x00;
    a_ui_I2cTxDataArray[42] = 0x00;
    a_ui_I2cTxDataArray[43] = 0x80;
    a_ui_I2cTxDataArray[44] = 0x00;
    a_ui_I2cTxDataArray[45] = 0x01;
    a_ui_I2cTxDataArray[46] = 0x0E;
    a_ui_I2cTxDataArray[47] = a_ui_I2cTxDataArray[0];
    a_ui_I2cTxDataArray[48] = a_ui_I2cTxDataArray[0];

    // Checksum
    for( i = 0; i < 46; i++)
    {
        Checksum += a_ui_I2cTxDataArray[i];
    }
    a_ui_I2cTxDataArray[46] = Checksum & 0xFF;

    while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
    ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, MODEL_NO_ADDR, 49, a_ui_I2cTxDataArray);
//  RESET_EEPROM_WRITE_CYCLE_TIMER;
    EEPROM_WRITE_CYCLE_DELAY;
//  a_ui_I2cTxDataArray[0] = 0x0B;
//    a_ui_I2cTxDataArray[1] = 0x0B;
//  while(!EEPROM_WRITE_CYCLE_TIMER_EXPIRED);
//      ei_uiI2CWrite(EEPROM_24LC256_ADDRESS, CAN_MODEL_NO_ADDR, 2, a_ui_I2cTxDataArray);
//      EEPROM_WRITE_CYCLE_DELAY;

}
