//#################################################################################################################
//#                             enArka Instruments proprietary
//# File: ei_Boot_Program.c
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.
//# All Rights Reserved
//#
//#################################################################################################################

//######################################### INCLUDE FILES #########################################################
#include "F2806x_Cla_typedefs.h"// F2806x CLA Type definitions
#include "F2806x_Device.h"      // F2806x Headerfile Include File
#include "F2806x_Examples.h"    // F2806x Examples Include File
#include "Flash2806x_API_Config.h"
#include "Flash2806x_API_Library.h"
#include "BootSymbols.h"
//#################################################################################################################

//###################################   enArka Boot File  #########################################################

//      Firmware Update of the OnBoard DSP.
//
//      IMPORTANT NOTES:
//      1. This file must be completely run from RAM.
//      2. All Symbols and text must be linked only in RAM.
//      3. All the functions that are used in this file are run from RAM.
//      4. No functions from the other part of the project are used/called in this file. Even for similar operations
//         like I2C_Write, SCI_Peripheral_Init etc.. are rewritten in this file to avoid running code inadvertantly
//         from Flash.
//      5. From here ON the SCI Baud Rate will be set to 19200bps.
//      6. In case rts functions are used (mandatory rts functions are boot.obj, they must be extracted from rts library
//         using archiver and placed in a user created library. This librray must be placed in "critical section". It is
//         also important to ensure that the compiler looks up these rts functions from custom created library than the
//         default rts library(This can be done changing the link order of the libraries in build properties for the project).
//#################################################################################################################
                                    /*-------Global Prototypes----------*/
//void ei_vDspBootMode();
//void MyCallbackFunction(void);
//#################################################################################################################
#pragma CODE_SECTION(main,"Critical_Section");
#pragma CODE_SECTION(ei_vBoot_InitI2c,"Critical_Section");

#pragma CODE_SECTION(ei_vDspBootMode,"ramfuncs");
#pragma CODE_SECTION(ei_vBoot_PsuSafeState,"ramfuncs");
#pragma CODE_SECTION(ei_vBoot_InitScia,"ramfuncs");
#pragma CODE_SECTION(ei_vBoot_CommVerificationWithHost,"ramfuncs");
#pragma CODE_SECTION(ei_vBoot_SciTransmitMessage,"ramfuncs");
#pragma CODE_SECTION(SCIA_BlockProcessing,"ramfuncs");
#pragma CODE_SECTION(SCIA_FIFO_GetWordData_swapEndian,"ramfuncs");
#pragma CODE_SECTION(MyCallbackFunction,"ramfuncs");
#pragma CODE_SECTION(ei_vBoot_ReceiveChecksum,"ramfuncs");
#pragma CODE_SECTION(ei_uiConvertCharacterToNibble,"ramfuncs");
#pragma CODE_SECTION(ei_iChecksumFlash,"ramfuncs");
#pragma CODE_SECTION(ei_vBoot_I2cWrite,"ramfuncs");
#pragma CODE_SECTION(ei_vBoot_I2CRead,"ramfuncs");
#pragma CODE_SECTION(ei_vDisplayIndicationDuringBootMode,"ramfuncs");
#pragma CODE_SECTION(ei_vDisplayFlashFlt,"ramfuncs");

static void ei_vBoot_PsuSafeState();
static void ei_vBoot_InitScia();
static void ei_vBoot_InitI2c();
static void ei_vBoot_CommVerificationWithHost();
static void ei_vBoot_SciTransmitMessage(char *msg);
static void SCIA_BlockProcessing();
static Uint16 SCIA_FIFO_GetWordData_swapEndian();
static void ei_vBoot_ReceiveChecksum();
static Uint16 ei_uiConvertCharacterToNibble(Int8 c_ChecksumCharacter);
static Uint32 ei_iChecksumFlash();
static Uint16 ei_vBoot_I2cWrite(Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Write_Array[]);
static Uint16 ei_vBoot_I2CRead(Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Write_Array[]);
static void ei_vDisplayIndicationDuringBootMode();
static void ei_vDisplayFlashFlt();

void main(void)
{
    //  Step 1. Initialize System Control:
//  PLL, WatchDog, enable Peripheral Clocks
//  This example function is found in the DSP280x_SysCtrl.c file.

    InitSysCtrl();
    MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);

    /******************************************************************/
    // For Piccolo, we dont need to copy the API from Flash as it is
    // present in BOOT ROM
    /******************************************************************/
//  MemCopy(&Flash28_API_LoadStart,&Flash28_API_LoadEnd,&Flash28_API_RunStart); // Copy Flash APIs to RAM
   
   
//  Step 2. Clear all interrupts and initialize PIE vector table:
//  Disable CPU interrupts 
    DINT;

//  Initialize PIE control registers to their default state.
//  The default state is all PIE interrupts disabled and flags
//  are cleared.  
//  This function is found in the DSP280x_PieCtrl.c file.
    InitPieCtrl();
    InitPieVectTable();
    IER = 0x0000;
    IFR = 0x0000;
    
//  Call Flash Initialization to setup flash waitstates
//  This function must reside in RAM
    InitFlash();

// Initialise the Scia and I2C Gpios here. No additional functions because we are trying to reduce functions
// here.
    EALLOW;
    // SCIa
    GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;     //Scia Rx Pin
    GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;      // enable
    GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 0;

    GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;     //Scia Tx Pin
    GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;      // enable

    // I2C Gpios
    GpioCtrlRegs.GPBMUX1.bit.GPIO32 = 1;        //SDAA
    GpioCtrlRegs.GPBPUD.bit.GPIO32 = 0;         // enable

    GpioCtrlRegs.GPBMUX1.bit.GPIO33 = 1;        //SCLA
    GpioCtrlRegs.GPBPUD.bit.GPIO33 = 0;         // enable

    //  ON_OFF_DSP Output Gpio
    GpioCtrlRegs.GPBMUX1.bit.GPIO39 = 0;
    GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1;       //Set low.
    GpioCtrlRegs.GPBDIR.bit.GPIO39 = 1;         // Configure pin as output
    GpioCtrlRegs.GPBPUD.bit.GPIO39 = 1;         // enable

    //  PS_GOOD Output Gpio
    GpioCtrlRegs.GPBMUX2.bit.GPIO53 = 0;
    GpioDataRegs.GPBCLEAR.bit.GPIO53 = 1;       //Set low.
    GpioCtrlRegs.GPBDIR.bit.GPIO53 = 1;         // Configure pin as output
    GpioCtrlRegs.GPBPUD.bit.GPIO53 = 1;         // enable

    //  TRANS_ENABLE
    GpioCtrlRegs.GPAMUX2.bit.GPIO17 = 0;
    GpioDataRegs.GPACLEAR.bit.GPIO17 = 1;       //Set Low.
    GpioCtrlRegs.GPADIR.bit.GPIO17 = 1;         // Configure pin as output
    GpioCtrlRegs.GPAPUD.bit.GPIO17 = 1;         // disable

    EDIS;
    // DELAY before doing I2C related operations.
    DELAY_US(500000);   // 500ms
    DELAY_US(500000);   // 500ms

    // Initialise Peripherals. Only I2C initialises
    // I2C
    ei_vBoot_InitI2c();

    // Read the EEPROM Status for Boot location to know the status of the last Firmware flashing from RS485
    ei_vBoot_I2CRead(EEPROM_24LC256_ADDRESS,FLASH_PROG_STATUS_ADDRESS,1,a_ui_I2C_Rx_Data_Array);

    if((a_ui_I2C_Rx_Data_Array[0] == FLASH_PROGRAMMING_SUCCESSFULL) || (a_ui_I2C_Rx_Data_Array[0] == 0xFF))
    {
        main2();
    }
    else if(a_ui_I2C_Rx_Data_Array[0] == BOOT_COMMAND_RECEIVED) // Must go to boot mode. Boot command read
    {
        ei_vDspBootMode();
    }
    else    // Some other junk data read. Copy this in a particular location in EEPROM and then go to boot mode
    {
        ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_STATUS_ILLEGAL_READ_ADDRESS,1,a_ui_I2C_Rx_Data_Array);
        EEPROM_WRITE_CYCLE_DELAY;
        ei_vDspBootMode();
    }
}

//##################### Boot Process ##############################################################################
// Function Name: ei_vDspBootMode
// Return Type: void
// Arguments:   void
// Description: The Complete Boot processing is done here.
//              Must be executed from outside flash/OTP
//#################################################################################################################
void ei_vDspBootMode()
{
    Uint16 Status;

    // Step 1: Take Power Supply to Safe State.
    ei_vBoot_PsuSafeState();

    // Step 1.1: Put Indicator in Boot Mode
    ei_vDisplayIndicationDuringBootMode();

    // Step 2: Reintialize SCI. Changing baud rate to higher value.
    ei_vBoot_InitScia();

    // Step 3: Communication verification
    ei_vBoot_CommVerificationWithHost();

    // Step 4: Transmit Boot Message B01
    ei_vBoot_SciTransmitMessage(B01);

    // Step 5: Initialize the boot variables to proper values
    EALLOW;
    Flash_CPUScaleFactor = SCALE_FACTOR;
    Flash_CallbackPtr = &MyCallbackFunction;
    EDIS;
    MyCallbackCounter = 0; // Increment this counter in the callback function

    // Step 6: Erase complete Flash Memory from sector B to sector H
    // Step 6: Erase complete Flash Memory
    Status = Flash_Erase(SECTORB|SECTORC|SECTORD|SECTORE|SECTORF|SECTORG|SECTORH,&EraseStatus);

    // Step 7: Flash Erased?
    if(Status != STATUS_SUCCESS)
    {
       // FAILURE 3. Flash Error
       // Error Type Test. Check for the Exact error returned, send it Over Uart and also store it in EEPROM.
       if(Status == STATUS_FAIL_CSM_LOCKED)
       {
            ei_vBoot_SciTransmitMessage(E10);
            a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_CSM_LOCKED;
       }
       else if(Status == STATUS_FAIL_REVID_INVALID)
       {
            ei_vBoot_SciTransmitMessage(E11);
            a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_REVID_INVALID;
       }
       else if(Status == STATUS_FAIL_ADDR_INVALID)
       {
            ei_vBoot_SciTransmitMessage(E12);
            a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_ADDR_INVALID;
       }
       /*else if(Status == STATUS_FAIL_INCORRECT_PARTID)
       {
            ei_vBoot_SciTransmitMessage(E13);
            a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_INCORRECT_PARTID;
       }*/
       /*else if(Status == STATUS_FAIL_API_SILICON_MISMATCH)
       {
            ei_vBoot_SciTransmitMessage(E14);
            a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_API_SILICON_MISMATCH;
       }*/
       else if(Status == STATUS_FAIL_NO_SECTOR_SPECIFIED)
       {
            ei_vBoot_SciTransmitMessage(E20);
            a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_NO_SECTOR_SPECIFIED;
       }
       else if(Status == STATUS_FAIL_PRECONDITION)
       {
            ei_vBoot_SciTransmitMessage(E21);
            a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_PRECONDITION;
       }
       else if(Status == STATUS_FAIL_ERASE)
       {
            ei_vBoot_SciTransmitMessage(E22);
            a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_ERASE;
       }
       else if(Status == STATUS_FAIL_COMPACT)
       {
            ei_vBoot_SciTransmitMessage(E23);
            a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_COMPACT;
       }
       else if(Status == STATUS_FAIL_PRECOMPACT)
       {
            ei_vBoot_SciTransmitMessage(E24);
            a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_PRECOMPACT;
       }
       else
       {
            ei_vBoot_SciTransmitMessage(E_UNKNOWN);
            a_ui_I2cTxDataArray[0] = HEX_UNKNOWN_ERROR;
       }

        ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_ERROR_ADDRESS,1,a_ui_I2cTxDataArray);
        EEPROM_WRITE_CYCLE_DELAY;

//     asm("    ESTOP0"); // FOR TESTING ONLY
        ei_vDisplayFlashFlt();
        for(;;);
    }

    ei_vBoot_SciTransmitMessage(B00);   // Successful Flash Erase

    // Step 8: Transmit Boot Message B02
    ei_vBoot_SciTransmitMessage(B02);

    // Step 9: Block Processing Commences
    SCIA_BlockProcessing();

    // Step 10: Flashing Completed. Transmit Boot Message B03
    ei_vBoot_SciTransmitMessage(B03);

    // Step 11: Receive Checksum and store received checksum in EEPROM
    ei_vBoot_ReceiveChecksum();

    // Step 12: Comapare with flashed Checksum and Generate Appropriate Response
    tempL = ei_iChecksumFlash();
    
    // Step 13: All Process Completed. Generate Software reset Using Watchdog.
    if((ul_ChecksumReceived - tempL) == 0)
    {
        // Transmit programming success Over Uart.
        ei_vBoot_SciTransmitMessage(B04);

        // Store status and checksum in EEPROM
        a_ui_I2cTxDataArray[0] = FLASH_PROGRAMMING_SUCCESSFULL;
        a_ui_I2cTxDataArray[1] = ul_ChecksumReceived >> 8;          // MSB of the checksum
        a_ui_I2cTxDataArray[2] = (ul_ChecksumReceived&0x00FF);      // LSB of the checksum

        ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_STATUS_ADDRESS,3,a_ui_I2cTxDataArray);
        EEPROM_WRITE_CYCLE_DELAY;

        EALLOW;
        SysCtrlRegs.SCSR = 0x0000;
        SysCtrlRegs.WDCR = 0x0000;  // Immediate Software Reset
        EDIS;
        for(;;);
    }

    // FAILURE 7 is checksum failure
    // Transmit FAILURE 7 Over Uart.
    ei_vBoot_SciTransmitMessage(C01);

    // Store the FAILURE 7 in EEPROM
    a_ui_I2cTxDataArray[0] = FLASH_CHECKSUM_FAILURE;

    ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_ERROR_ADDRESS,1,a_ui_I2cTxDataArray);
    EEPROM_WRITE_CYCLE_DELAY;

//    asm("    ESTOP0"); // FOR TESTING ONLY
    ei_vDisplayFlashFlt();
    for(;;);
}

//##################### PSU Safe State ############################################################################
// Function Name: ei_vBoot_PsuSafeState
// Return Type: void
// Arguments:   void
// Description: Take PSU to safe state by 
//              1. pulling ON/OFF (Switch OFF this PSU)
//              2.PS_GOOD pin low.(Switches OFF Other PSU's incase connected)

//#################################################################################################################
static void ei_vBoot_PsuSafeState()
{
    BOOT_SWITCH_OUTPUT_OFF;
    BOOT_PULL_PS_OK_LOW;
}

//##################### SCI Peripheral Init #######################################################################
// Function Name: ei_vBoot_InitScia
// Return Type: void
// Arguments:   void
// Description: Reinitialises SCI peripheral.
//              The Baud Rate is changed to 19200bps
//              Sci now operates in non interrupt mode.
//#################################################################################################################
static void ei_vBoot_InitScia()
{
    //  Reset Transmit and Receive, FIFO enable, Hold FIFO TX in reset,
//  Disable Interrupt, Clear Interrupt
//  SciaRegs.SCIFFTX.all=0xC040; 
    SciaRegs.SCIFFTX.bit.SCIRST = RESET_TRANSMIT_RECEIVE;
    SciaRegs.SCIFFTX.bit.SCIFFENA = ENABLE_FIFO_ENHANCEMENTS;
    SciaRegs.SCIFFTX.bit.TXFIFOXRESET = HOLD_IN_RESET;
    SciaRegs.SCIFFTX.bit.TXFFIENA = DISABLE_INT;
    SciaRegs.SCIFFTX.bit.TXFFIL = 0;

//  Clear Overflow, Hold FIFORX in reset
//  SciaRegs.SCIFFRX.all = 0x405F;

    SciaRegs.SCIFFRX.bit.RXFFOVRCLR = 1; // Clear Overflow Flag
    SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1; // Clear Interrupt Flag
    SciaRegs.SCIFFRX.bit.RXFIFORESET = HOLD_IN_RESET;
//      SciaRegs.SCIFFRX.bit.RXFFIENA = ENABLE_INT;
    SciaRegs.SCIFFRX.bit.RXFFIENA = DISABLE_INT;

//  SciaRegs.SCIFFRX.bit.RXFFIL = 1;  // Interrupt is raised when 1 character is received in Uart FIFO.

//  Auto Baud Disable and no FIFO TX delay.
    SciaRegs.SCIFFCT.all = 0x0000;
   
//  Note: Clocks were turned on to the SCIA peripheral in
//  the InitSysCtrl() function

    SciaRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
                                   // No parity,8 char bits,
                                   // async mode, idle-line protocol
    SciaRegs.SCICTL1.all =0x0003;  // enable TX, RX, internal SCICLK, Hold SCI in Reset
                                   // Disable RX ERR, SLEEP, TXWAKE
    SciaRegs.SCICTL2.all =0x0000;  // Rx/BKDT interrupt  and TX(non FIFO mode) interrupt disable


    /*
    // 1199.87 baud @LSPCLK = 22.5MHz. (1199.87 = 22.5M/{2344*8})
    SciaRegs.SCIHBAUD    =0x0009;       // 2343 = 0x0927 =>  SCIHBAUD = 0x09
    SciaRegs.SCILBAUD    =0x0027;       //                   SCILBAUD = 0x28

    // 2399.74 baud @LSPCLK = 22.5MHz. (2399.74 = 22.5M/{1172*8})
    SciaRegs.SCIHBAUD    =0x0004;       // 1171 = 0x0493 =>  SCIHBAUD = 0x04
    SciaRegs.SCILBAUD    =0x0093;       //                   SCILBAUD = 0x93

    // 4799.49 baud @LSPCLK = 22.5MHz. (4799.49 = 22.5M/{586*8})
    SciaRegs.SCIHBAUD    =0x0004;       //  585 = 0x0259 =>  SCIHBAUD = 0x02
    SciaRegs.SCILBAUD    =0x0093;       //                   SCILBAUD = 0x59

    // 9598.98 baud @LSPCLK = 22.5MHz. (9598.98 = 22.5M/{293*8})
    SciaRegs.SCIHBAUD    =0x0001;       // 292 = 0x0124 =>  SCIHBAUD = 0x01
    SciaRegs.SCILBAUD    =0x0024;       //                  SCILBAUD = 0x24
    */

    // 19263.69 baud @LSPCLK = 22.5MHz. (19263.69 = 22.5M/{296*8})
    SciaRegs.SCIHBAUD    =0x0000;       // 145 = 0x0091 =>  SCIHBAUD = 0x00
    SciaRegs.SCILBAUD    =0x0091;       //                  SCILBAUD = 0x91

    SciaRegs.SCIFFTX.bit.TXFIFOXRESET = 1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;

    SciaRegs.SCICTL1.all =0x0023;  // Relinquish SCI from Reset
}

//##################### I2C Peripheral Init #######################################################################
// Function Name: ei_vBoot_InitI2c
// Return Type: void
// Arguments:   void
// Description: Initialises I2C peripheral.
//#################################################################################################################
static void ei_vBoot_InitI2c()
{
    I2caRegs.I2CSAR = 0x00;                     //Dummy
    I2caRegs.I2CMDR.bit.IRS = 0;        // Hold in reset
    I2caRegs.I2CPSC.all = 8;                    // d value becomes 5. See I2CApp.c for details on Baudrate

//  I2caRegs.I2CCLKL = 20;          // For 250kbps
//  I2caRegs.I2CCLKH = 10;

    I2caRegs.I2CCLKL = 10;          // For 400kbps
    I2caRegs.I2CCLKH = 5;

    I2caRegs.I2CIER.all = 0x00;

    I2caRegs.I2CMDR.bit.MST = 1;                // Master
    I2caRegs.I2CMDR.bit.XA = 0;
    I2caRegs.I2CMDR.bit.RM = 1;
    I2caRegs.I2CMDR.bit.BC = 0;
    I2caRegs.I2CMDR.bit.STP = 0;
    I2caRegs.I2CMDR.bit.IRS = 1;                // Take I2C out of reset
}

//##################### SCI Comm Verify ###########################################################################
// Function Name: ei_vBoot_CommVerificationWithHost
// Return Type: void
// Arguments:   void
// Description: This function waits 'a' received from Host. Recieving this character implies that the communication
//              with host is established successfully at the new BaudRate.
//#################################################################################################################
static void ei_vBoot_CommVerificationWithHost()
{
    // Wait to receive 'a' character.
    while(SciaRegs.SCIFFRX.bit.RXFFST == 0);    // TIMEOUT IS FAILURE 1

    if(SciaRegs.SCIRXBUF.all == BOOT_COMMUNICATION_ESTABLISHED_SUCCESSFULLY)
    {
        // All okay. Return to calling function
        return;
    }
    else
    {
        // Communication Not Established.
        // FAILURE 2: Illegal Character
        ei_vBoot_SciTransmitMessage(C02);

        // Store FAILURE 2 in EEPROM
        a_ui_I2cTxDataArray[0] = COMM_NOT_ESTABLISHED;

        ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_ERROR_ADDRESS,1,a_ui_I2cTxDataArray);
        EEPROM_WRITE_CYCLE_DELAY;

//      for(;;); // FOR TESTING ONLY
    }
//  asm("    ESTOP0"); // FOR TESTING ONLY
    ei_vDisplayFlashFlt();
    for(;;);
}

//##################### SCI Transmit Message ######################################################################
// Function Name: ei_vBoot_SciTransmitMessage
// Return Type: void
// Arguments:   char *msg
// Description: This function transmits a message over Sci indicating various Boot Statuses.
//              Must be executed from outside flash/OTP
//              Must be executed from outside flash/OTP
//#################################################################################################################
static void ei_vBoot_SciTransmitMessage(char *msg)
{
    BOOT_ENABLE_UART_TRANSMISSION;
    DELAY_US(100);  // wait for sometime for the pin to settle
    while(*msg)
    {
        while((SciaRegs.SCIFFTX.bit.TXFFST<UART_BUFFER_SIZE) && *msg)
            SciaRegs.SCITXBUF = *msg++;

        while(SciaRegs.SCIFFTX.bit.TXFFST) ;    //TX FIFO empty?
    }
    DELAY_US(1000); // wait for the last character to be transmitted. Minimum time to wait: 10/19200 = 521us
    BOOT_DISABLE_UART_TRANSMISSION;

    return;
}

//##################### SCI Transmit Message ######################################################################
// Function Name: SCIA_BlockProcessing
// Return Type: void
// Arguments:   void
// Description: The Block Processing function receives code from user into 2 RAM buffers. The Flash APIs then 
//              transfer this code from RAM to FLASH.
//              Must be executed from outside flash/OTP
//#################################################################################################################
static void SCIA_BlockProcessing()
{
   struct HEADER {
     Uint32 BlockSize;
     Uint32 DestAddr;
   } BlockHeader;
  
    Uint16  BufferSize;
    Uint16 Status;
    Uint16 *Flash_ptr;     // Pointer to a location in flash
    Uint32 Length;         // Number of 16-bit values to be programmed

    //HW definitions, RAM buffer is constant, block size and dest
    //addr dependent on 28062
    BufferSize = RAM_BUFFER_SIZE;
    BlockHeader.BlockSize = BLOCK_SIZE;
    BlockHeader.DestAddr = DEST_ADDR;

    //Init processing variables
    BlockRemain = BlockHeader.BlockSize;
    BufferIndexMax = BufferSize;
    BlockBufferPtr = BlockBuffer1;

    //Transfer the program application code
    while(BufferIndexMax > 0)
    {
        ei_vBoot_SciTransmitMessage(PERIOD);
        for(i = 0; i <= (BufferIndexMax-1); i++)
        {
            //RAM Transfer
            *BlockBufferPtr++ = SCIA_FIFO_GetWordData_swapEndian();
            if(SciaRegs.SCIRXST.bit.RXERROR || SciaRegs.SCIFFRX.bit.RXFFOVF)
            {
                // FAILURE 4 is SCI Comm Failure

                ei_vBoot_SciTransmitMessage(C03);

                // Store the FAILURE 4 in EEPROM
                a_ui_I2cTxDataArray[0] = COMMUNICATION_FAILURE;

                ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_ERROR_ADDRESS,1,a_ui_I2cTxDataArray);
                EEPROM_WRITE_CYCLE_DELAY;

//              asm("    ESTOP0"); // FOR TESTING ONLY
                ei_vDisplayFlashFlt();
                for(;;);
            }
        }

        //Program data block into flash at address specified at start of data block,
        //callback function continues transfer in other buffer

        //Config RAM buffer pointer for next RAM transfer
        if(BlockBufferPtr <= BlockBuffer2)
            BufferIndexMax = BlockBufferPtr - BlockBuffer1;
        else
            BufferIndexMax = BlockBufferPtr - BlockBuffer2;

        Flash_ptr = (Uint16 *)BlockHeader.DestAddr;
        Length = BufferIndexMax;
        BlockHeader.DestAddr += Length;

        //Callback function config
        MyCallbackCounter = 0;
        BlockRemain -= BufferIndexMax;

        //Program flash
        if(BlockRemain < BufferSize)
            BufferIndexMax = BlockRemain;
        else
            BufferIndexMax = BufferSize;

        if(BlockBufferPtr <= BlockBuffer2)
        {
            BlockBufferPtr = BlockBuffer2;
            Status = Flash_Program(Flash_ptr,BlockBuffer1,Length,&ProgStatus);
        }
        else
        {
            BlockBufferPtr = BlockBuffer1;
            Status = Flash_Program(Flash_ptr,BlockBuffer2,Length,&ProgStatus);
        }
        if(Status != STATUS_SUCCESS)
        {
            // FAILURE 5. Flash Programming Failure.
            // Error Type Test. Check for the Exact error returned, send it Over Uart and also store it in EEPROM.
            if(Status == STATUS_FAIL_PROGRAM)
            {
                ei_vBoot_SciTransmitMessage(E30);
                a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_PROGRAM;
            }
            else if(Status == STATUS_FAIL_ZERO_BIT_ERROR)
            {
                ei_vBoot_SciTransmitMessage(E31);
                a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_ZERO_BIT_ERROR;
            }
            else if(Status == STATUS_FAIL_ADDR_INVALID)
            {
                ei_vBoot_SciTransmitMessage(E12);
                a_ui_I2cTxDataArray[0] = HEX_STATUS_FAIL_ADDR_INVALID;
            }
            else
            {
                ei_vBoot_SciTransmitMessage(E_UNKNOWN);
                a_ui_I2cTxDataArray[0] = HEX_UNKNOWN_ERROR;
            }

            ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_ERROR_ADDRESS,1,a_ui_I2cTxDataArray);
            EEPROM_WRITE_CYCLE_DELAY;

//          asm("    ESTOP0"); // FOR TESTING ONLY
            ei_vDisplayFlashFlt();
            for(;;);
        }

        //Receive the rest of data block not transferred by callback function
        // and store in buffer 2
        while((SciaRegs.SCIFFRX.bit.RXFFST>1) && (BufferIndexMax>0))
        {
            *BlockBufferPtr++ = SCIA_FIFO_GetWordData_swapEndian();
            BufferIndexMax--;
        }
        if(SciaRegs.SCIRXST.bit.RXERROR || SciaRegs.SCIFFRX.bit.RXFFOVF)
        {
            if(BlockRemain == 0)
            {
                // FAILURE 6 is Illegal Code size or file not in Binary mode error
                ei_vBoot_SciTransmitMessage(C04);

                // Store the FAILURE 6 in EEPROM
                a_ui_I2cTxDataArray[0] = FILE_SIZE_MISMATCH_FAILURE;

                ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_ERROR_ADDRESS,1,a_ui_I2cTxDataArray);
                EEPROM_WRITE_CYCLE_DELAY;

//              asm("    ESTOP0"); // FOR TESTING ONLY
                ei_vDisplayFlashFlt();
                for(;;);
            }
            else
            {
                // FAILURE 4 is SCI Comm Failure

                ei_vBoot_SciTransmitMessage(C03);

                // Store the FAILURE 4 in EEPROM
                a_ui_I2cTxDataArray[0] = COMMUNICATION_FAILURE;

                ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_ERROR_ADDRESS,1,a_ui_I2cTxDataArray);
                EEPROM_WRITE_CYCLE_DELAY;

//              asm("    ESTOP0"); // FOR TESTING ONLY
                ei_vDisplayFlashFlt();
                for(;;);
            }
        }

    }   //end of IndexMax While-Loop
   return;
}

//##################### SCI Get Word ##############################################################################
// Function Name: SCIA_FIFO_GetWordData_swapEndian
// Return Type: Uint16
// Arguments:   void
// Description: Gets the word from Uart and swaps the endian. 
//              Must be executed from outside flash/OTP
//#################################################################################################################
static Uint16 SCIA_FIFO_GetWordData_swapEndian()
{
   Uint16 wordData;
   Uint16 byteData;
  
   wordData = 0x0000;
   byteData = 0x0000;
   
   // Fetch the LSB and verify back to the host
   while(SciaRegs.SCIFFRX.bit.RXFFST < 2) { }   // FAILURE 1: Communication Timeout to be implemented here.

   byteData =  (Uint16)SciaRegs.SCIRXBUF.bit.RXDT;
//   SCIARegs.SCITXBUF = wordData;

   // Fetch the MSB and verify back to the host
   wordData =  (Uint16)SciaRegs.SCIRXBUF.bit.RXDT;
//   SCIARegs.SCITXBUF = byteData;
   
   // form the wordData from the MSB:LSB
   wordData |= (byteData << 8);

   return wordData;
}

//##################### Callback Function #########################################################################
// Function Name: MyCallbackFunction
// Return Type: void
// Arguments:   void
// Description: Callback function 
//              Must be executed from outside flash/OTP
//#################################################################################################################
void MyCallbackFunction(void)
{       

    if(SciaRegs.SCIFFRX.bit.RXFFST == UART_BUFFER_SIZE)
    {
        while((SciaRegs.SCIFFRX.bit.RXFFST>1) && (BufferIndexMax>0))
        {
            *BlockBufferPtr++ = SCIA_FIFO_GetWordData_swapEndian();
            BufferIndexMax--;
        }
        if(SciaRegs.SCIRXST.bit.RXERROR || SciaRegs.SCIFFRX.bit.RXFFOVF)
        {
            if(BlockRemain == 0)
            {
                // FAILURE 6 is Illegal Code size or file not in Binary mode error
                ei_vBoot_SciTransmitMessage(C04);

                // Store the FAILURE 6 in EEPROM
                a_ui_I2cTxDataArray[0] = FILE_SIZE_MISMATCH_FAILURE;

                ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_ERROR_ADDRESS,1,a_ui_I2cTxDataArray);
                EEPROM_WRITE_CYCLE_DELAY;

//              asm("    ESTOP0"); // FOR TESTING ONLY
                ei_vDisplayFlashFlt();
                for(;;);
            }
            else
            {
                // FAILURE 4 is SCI Comm Failure

                ei_vBoot_SciTransmitMessage(C03);

                // Store the FAILURE 4 in EEPROM
                a_ui_I2cTxDataArray[0] = COMMUNICATION_FAILURE;

                ei_vBoot_I2cWrite(EEPROM_24LC256_ADDRESS,FLASH_PROG_ERROR_ADDRESS,1,a_ui_I2cTxDataArray);
                EEPROM_WRITE_CYCLE_DELAY;

//              asm("    ESTOP0"); // FOR TESTING ONLY
                ei_vDisplayFlashFlt();
                for(;;);
            }
        }
    }
    // Toggle pin, service external watchdog etc
    MyCallbackCounter++;
}

//##################### Checksum from RS485 #######################################################################
// Function Name: ei_vBoot_ReceiveChecksum
// Return Type: void
// Arguments:   void
// Description: The checksum of the flashed program is received from the flashing GUI over RS485.
//  Exactly 4 hexadecimal characters must be received starting from the most signifcant nibble to the
//  least significant nibble.
//#################################################################################################################
static void ei_vBoot_ReceiveChecksum()
{
    // Wait till 4 charcters of checksum received
    while(SciaRegs.SCIFFRX.bit.RXFFST != 4);    // TIMEOUT IS FAILURE 1

    // Most significant nibble to the least significant nibble is shifted one by oe into the variable on the left.
    ul_ChecksumReceived = ei_uiConvertCharacterToNibble(SciaRegs.SCIRXBUF.all);
    ul_ChecksumReceived = (ul_ChecksumReceived << 4) | ei_uiConvertCharacterToNibble(SciaRegs.SCIRXBUF.all);
    ul_ChecksumReceived = (ul_ChecksumReceived << 4) | ei_uiConvertCharacterToNibble(SciaRegs.SCIRXBUF.all);
    ul_ChecksumReceived = (ul_ChecksumReceived << 4) | ei_uiConvertCharacterToNibble(SciaRegs.SCIRXBUF.all);
}

//##################### Character to Nibble #######################################################################
// Function Name: ei_uiConvertCharacterToNibble
// Return Type: Uint32
// Arguments:   int8 c_ChecksumCharacter
// Description: Input a character between 0-9 and A-F (Uppercase only) to this function and it returns appropriate
//              hexadecimal nibble.
//#################################################################################################################
static Uint16 ei_uiConvertCharacterToNibble(Int8 c_ChecksumCharacter)
{
    if(CHARACTER_LIES_BETWEEN_0_AND_9)
    {
        return c_ChecksumCharacter - ASCII_FOR_0;
    }
    else if(CHARACTER_LIES_BETWEEN_A_AND_F)
    {
        return c_ChecksumCharacter - ASCII_FOR_A + 0xA;
    }
    else
    {
        return 0;
    }
}

//##################### FLASH_CHECKSUM ############################################################################
// Function Name: ei_iChecksumFlash
// Return Type: Uint32
// Arguments:   void
// Description: Returns checksum of the whole flash memory 
//              Must be executed from outside flash/OTP
//#################################################################################################################
static Uint32 ei_iChecksumFlash()
{
    Uint32   j;
    Uint16    sum = 0;
    Uint16  *mem;

    mem = (Uint16*)DEST_ADDR;
    for (j = 0; j <= FLASH_RANGE; j++)
        sum += *mem++;

    return sum;
}

//##################### I2C_Write #################################################################################
// Function Name: ei_vBoot_I2cWrite
// Return Type: Uint16
// Arguments:   Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Write_Array[]
// Description: I2C Write Driver. Pass Slave Address, Write location, No of databytes and the array with data. Returns
// I2C operation status (SUCCESS or FAILURE). This is a boot I2C_Write Function. The _WAIT_UNTIL defs are defined differently
// than that of the I2C_Write used in the main code. Here we just wait and come out. No bit set in fault register. 
//#################################################################################################################
static Uint16 ei_vBoot_I2cWrite(Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Write_Array[])
{
    Uint16 i;

    i = 0;

    I2caRegs.I2CSAR = Slave_address;
    I2C_WAIT_UNTIL(I2caRegs.I2CMDR.bit.STP != 0)

    //  Start bit, write mode, Higher 16 address bits, Master, Repeat mode.
     I2caRegs.I2CMDR.bit.TRX = 1;

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

//##################### I2C_Write #################################################################################
// Function Name: ei_vBoot_I2cRead
// Return Type: Uint16
// Arguments:   Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Write_Array[]
// Description: I2C Read Driver. Pass Slave Address, Write location, No of databytes and the array with data. Returns
// I2C operation status (SUCCESS or FAILURE). This is a boot I2C_Read Function. The _WAIT_UNTIL defs are defined differently
// than that of the I2C_Write used in the main code. Here we just wait and come out. No bit set in fault register. 
//#################################################################################################################
static Uint16 ei_vBoot_I2CRead(Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Read_Array[])
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

//##################### Boot Mode Display #########################################################################
// Function Name: ei_vDisplayIndicationDuringBootMode
// Return Type: void 
// Arguments:   void
// Description: This routine is specially called during booting process. It does some basic initialisation of Display ICs.
//              When the firmware is being updated from RS485, the display will indicate a special sequence. All digits
//              are OFF but all decimal points and LEDs are ON.
//#################################################################################################################
static void ei_vDisplayIndicationDuringBootMode()
{
    Uint16 Nibble_Counter = 0;
//  Uint16 Nibble_Array[4];

    /*Nibble_Array[3] = DISPLAY_BLANK;
    Nibble_Array[2] = DISPLAY_BLANK;
    Nibble_Array[1] = DISPLAY_BLANK;
    Nibble_Array[0] = DISPLAY_BLANK;*/

    a_ui_I2cTxDataArray[0] = 0x00 | NORMAL_OPERATION | GLOBAL_CLEAR_MAX;
    ei_vBoot_I2cWrite(MAX6959A_ADDRESS, CONFIGURATION_REGISTER_ADDRESS, 1, a_ui_I2cTxDataArray);
    ei_vBoot_I2cWrite(MAX6958B_ADDRESS, CONFIGURATION_REGISTER_ADDRESS, 1, a_ui_I2cTxDataArray);

    a_ui_I2cTxDataArray[0] = ENABLE_DIG0_TO_DIG3 ;
    ei_vBoot_I2cWrite(MAX6959A_ADDRESS, SCAN_LIMIT_REGISTER_ADDRESS, 1, a_ui_I2cTxDataArray);
    ei_vBoot_I2cWrite(MAX6958B_ADDRESS, SCAN_LIMIT_REGISTER_ADDRESS, 1, a_ui_I2cTxDataArray);

    a_ui_I2cTxDataArray[0] = LEVEL_SELECTOR ;
    ei_vBoot_I2cWrite(MAX6959A_ADDRESS, INTENSITY_REGISTER_ADDRESS, 1, a_ui_I2cTxDataArray);
    ei_vBoot_I2cWrite(MAX6958B_ADDRESS, INTENSITY_REGISTER_ADDRESS, 1, a_ui_I2cTxDataArray);

    //  Convert the Character to be displayed to appropriate value for Digit Register and put it on the I2C Bus
    Nibble_Counter = 0;
    while(Nibble_Counter < NUMBER_OF_DIGITS_DISPLAY)
    {
//          a_ui_I2cTxDataArray[Nibble_Counter] = ei_uiDisplayDigitMapping(Nibble_Array[Nibble_Counter]);
        a_ui_I2cTxDataArray[Nibble_Counter] = DIGIT_REGISTER_MAPPING_FOR_DISPLAY_BLANK;
        Nibble_Counter++;
    }

    // All digits blank.
    ei_vBoot_I2cWrite(MAX6959A_ADDRESS, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, a_ui_I2cTxDataArray);
    ei_vBoot_I2cWrite(MAX6958B_ADDRESS, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, a_ui_I2cTxDataArray);

    a_ui_I2cTxDataArray[0] = 0xFF;
    // All digits decimal point and all indicator LEDs Lit
    ei_vBoot_I2cWrite(MAX6959A_ADDRESS, DIGIT_DISPLAY_DECIMAL_POINT_AND_SEGMENT_LED_ADDRESS, 1, a_ui_I2cTxDataArray);
    ei_vBoot_I2cWrite(MAX6958B_ADDRESS, DIGIT_DISPLAY_DECIMAL_POINT_AND_SEGMENT_LED_ADDRESS, 1, a_ui_I2cTxDataArray);
}

//##################### Flash Flt #################################################################################
// Function Name: ei_vDisplayFlashFlt
// Return Type: void 
// Arguments:   void
// Description: Display "FLSH" " FLT" on the display
//              Must be run from ramfuncs
//#################################################################################################################
static void ei_vDisplayFlashFlt()
{
    a_ui_I2cTxDataArray[3] = DIGIT_REGISTER_MAPPING_FOR_F;
    a_ui_I2cTxDataArray[2] = DIGIT_REGISTER_MAPPING_FOR_L;
    a_ui_I2cTxDataArray[1] = DIGIT_REGISTER_MAPPING_FOR_5;
    a_ui_I2cTxDataArray[0] = DIGIT_REGISTER_MAPPING_FOR_H;
    ei_vBoot_I2cWrite(MAX6959A_ADDRESS, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, a_ui_I2cTxDataArray);

    a_ui_I2cTxDataArray[3] = DIGIT_REGISTER_MAPPING_FOR_DISPLAY_BLANK;
    a_ui_I2cTxDataArray[2] = DIGIT_REGISTER_MAPPING_FOR_F;
    a_ui_I2cTxDataArray[1] = DIGIT_REGISTER_MAPPING_FOR_L;
    a_ui_I2cTxDataArray[0] = DIGIT_REGISTER_MAPPING_FOR_T;
    ei_vBoot_I2cWrite(MAX6958B_ADDRESS, DIGIT_DISPLAY_BEGIN_ADDRESS, 4, a_ui_I2cTxDataArray);
}
