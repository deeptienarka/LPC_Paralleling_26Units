//#################################################################################################################
//#                             enArka Instruments proprietary
//# File: ei_InitPeripherals.c
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.
//# All Rights Reserved
//#
//#################################################################################################################

//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
#include "ExternalVariables.h"
//#################################################################################################################
                                    /*--------Global Prototypes---------*/
//void ei_vInitPeripherals();
//void ei_vDeviceSpecificCanMailboxInit();
//#################################################################################################################
                                    /*-------Local Prototypes----------*/
static void ei_vInitEQep1();
static void ei_vInitEQep2();
static void ei_vInitI2C();
static void ei_vInitSpia();
static void ei_vInitEPwm2();
static void ei_vInitEPwm3();
static void ei_vInitEPwm4();
static void ei_vInitEPwm5();
static void ei_vInitEPwm8();
static void ei_vInitEPwm6();
static void ei_vInitScia();
static void ei_vInitEcana();
static void ei_vInitTimer0();
//#################################################################################################################
void ei_vInitPeripherals()
{
//  EQep
    ei_vInitEQep1();
    ei_vInitEQep2();

//  I2C
    ei_vInitI2C();

//  SPI
    ei_vInitSpia();

//  ADC
    ei_vInitAdc(); // This Function is defined in ADCApp.c

//  EPwm.
    EALLOW;         // Some registers in EPwm are EALLOW protected.
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    ei_vInitEPwm2();
    ei_vInitEPwm3();
    ei_vInitEPwm4();
    ei_vInitEPwm5();
    ei_vInitEPwm8();
    ei_vInitEPwm6(); //Scheduler only. No pwms generated
    SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;

//  SCI
    ei_vInitScia();

//  CAN
    ei_vInitEcana();

//  Timer 0
    ei_vInitTimer0();
}

//##################### Initialise EQEP1 ##########################################################################
// Function Name: ei_vInitEQep1
// Return Type: void
// Arguments:   void
// Description: Initialises EQep1 for voltage encoder applications.
//#################################################################################################################
static void ei_vInitEQep1()
{
    EQep1Regs.QDECCTL.all = 0x0000; // Clear all
    EQep1Regs.QDECCTL.bit.QSRC = QUADRATURE_COUNT_MODE;
    EQep1Regs.QDECCTL.bit.SOEN = POSITION_CONTROL_SYNC_OUTPUT_DISABLE;
    EQep1Regs.QDECCTL.bit.XCR = RESOLUTION_2X;

    EQep1Regs.QEPCTL.all = 0x0000; // Clear all
    EQep1Regs.QEPCTL.bit.PCRM = RESET_ON_MAX_POSITION;
    EQep1Regs.QPOSINIT = 0; // Put the value of POSINIT Reg here
    EQep1Regs.QEPCTL.bit.SWI = 1;
    EQep1Regs.QEPCTL.bit.QPEN = ENABLE_COUNTER;


    EQep1Regs.QPOSCTL.all = 0x0000; // Disable Position Compare Unit

    EQep1Regs.QCAPCTL.all = 0x0000; // Disable Qep Capture Unit

    EQep1Regs.QPOSMAX = QEP_MAX_OVP_COUNT + QEP_SAFETY_NET;

    EQep1Regs.QEINT.all = 0x0000; //Disable Interrupts

    EQep1Regs.QCLR.all = 0x0FFF;  //Clears Flag
}

//##################### Initialise EQEP2 ##########################################################################
// Function Name: ei_vInitEQep2
// Return Type: void
// Arguments:   void
// Description: Initialises EQep2 for current encoder applications.
//#################################################################################################################
static void ei_vInitEQep2()
{
    EQep2Regs.QDECCTL.all = 0x0000; // Clear all
    EQep2Regs.QDECCTL.bit.QSRC = QUADRATURE_COUNT_MODE;
    EQep2Regs.QDECCTL.bit.SOEN = POSITION_CONTROL_SYNC_OUTPUT_DISABLE;
    EQep2Regs.QDECCTL.bit.XCR = RESOLUTION_2X;

    EQep2Regs.QEPCTL.all = 0x0000; // Clear all
    EQep2Regs.QEPCTL.bit.PCRM = RESET_ON_MAX_POSITION;
    EQep2Regs.QPOSINIT = 0; // Put the value of POSINIT Reg here
    EQep2Regs.QEPCTL.bit.SWI = 1;
    EQep2Regs.QEPCTL.bit.QPEN = ENABLE_COUNTER;


    EQep2Regs.QPOSCTL.all = 0x0000; // Disable Position Compare Unit

    EQep2Regs.QCAPCTL.all = 0x0000; // Disable Qep Capture Unit

    //EQep2Regs.QPOSINIT = COUNT_CORRESPONDING_TO_2P3V;

    EQep2Regs.QPOSMAX = QEP_FULL_ROTATIONS_COUNT + QEP_SAFETY_NET;

    EQep2Regs.QEINT.all = 0x0000; //Disable Interrupts

    EQep2Regs.QCLR.all = 0x0FFF;  //Clears Flag
}

//##################### Initialise I2C ############################################################################
// Function Name: ei_vInitI2C
// Return Type: void
// Arguments:   void
// Description: Initialises the I2C peripheral. For Baud Rate setting related information
//              see I2CApp.c
//#################################################################################################################
static void ei_vInitI2C(void)
{
    I2caRegs.I2CSAR = 0x00;                     //Dummy
    I2caRegs.I2CMDR.bit.IRS = I2C_RESET;        // Hold in reset
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

//##################### Initialise SPI ############################################################################
// Function Name: ei_vInitSpia
// Return Type: void
// Arguments:   void
// Description: Initialises the SPI peripheral.
//#################################################################################################################
static void ei_vInitSpia()
{
    SpiaRegs.SPICCR.bit.SPISWRESET = 0;     // Hold in reset
    SpiaRegs.SPICCR.bit.CLKPOLARITY = 1;    // Clock high when idle. AD7705 must be operated in this mode.
    SpiaRegs.SPICCR.bit.SPILBK  = 0;        // Not in loopback mode
    SpiaRegs.SPICCR.bit.SPICHAR = 7;        // 7 corresponds to 8 bit character

    SpiaRegs.SPICTL.bit.CLK_PHASE = 0;      // Data output on falling edge and input on rising edge
    SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1;   // Master mode
    SpiaRegs.SPICTL.bit.OVERRUNINTENA = 0;  // Disable interrupt
    SpiaRegs.SPICTL.bit.SPIINTENA = 0;      // Disable interrupt
    SpiaRegs.SPICTL.bit.TALK = 1;           // Enable communication
//  SpiaRegs.SPIFFRX.all = 0x001F;
//  SpiaRegs.SPIFFTX.all = 0x0000;

    SpiaRegs.SPIBRR = 44;                   // Spi Clk = 22.5M/45 = 500kbps

    SpiaRegs.SPICCR.bit.SPISWRESET = 1;     // Spi Relinquished from reset
}

//##################### Initialise EPWM ###########################################################################
// Function Name: ei_vInitEPwm2
// Return Type: void
// Arguments:   void
// Description: Initialises the EPwm2 peripheral.
//#################################################################################################################
static void ei_vInitEPwm2()
{
   EPwm2Regs.ETSEL.bit.INTEN = DISABLE_INT;         // Disable EPWM2_INT
   
   EPwm2Regs.TBCTL.bit.CLKDIV = CLKDIV_1;
   EPwm2Regs.TBCTL.bit.HSPCLKDIV = DIV_1 ;
   EPwm2Regs.TBCTL.bit.SYNCOSEL = DISABLE_PHASE_SYNC;
   EPwm2Regs.TBCTL.bit.PRDLD = LOAD_IN_SHADOW_REG;
   EPwm2Regs.TBCTL.bit.PHSEN = DISABLE_PHASE;
   EPwm2Regs.TBCTL.bit.CTRMODE = UP_DOWN_MODE;
   
   EPwm2Regs.TBPRD = EPWM2_PERIOD_VALUE;             // Set period for ePWM2
   
   EPwm2Regs.CMPCTL.bit.SHDWBMODE = LOAD_IN_SHADOW_REG ;
   EPwm2Regs.CMPCTL.bit.SHDWAMODE = LOAD_IN_SHADOW_REG ;
   EPwm2Regs.CMPCTL.bit.LOADAMODE = LOAD_DURING_UNDERFLOW ; 
   EPwm2Regs.CMPCTL.bit.LOADBMODE = LOAD_DURING_UNDERFLOW ;
      
   EPwm2Regs.CMPA.half.CMPA = 0;
   EPwm2Regs.CMPA.half.CMPAHR = 0;
   
// Active Low
   EPwm2Regs.AQCTLA.bit.CAD = PULL_HIGH;
   EPwm2Regs.AQCTLA.bit.CAU = PULL_LOW;
  
   EPwm2Regs.ETSEL.bit.INTSEL = DISABLE_SELECTON;           // Counter = 0 event for epwm2_int
   EPwm2Regs.ETPS.bit.INTPRD = DISABLE_INT_PERIOD;         // Generate pulse on 1st event
   EPwm2Regs.ETCLR.bit.INT = CLEAR_INTERRUPT;
}

//##################### Initialize EPWM ###########################################################################
// Function Name: ei_vInitEPwm3
// Return Type: void
// Arguments:   void
// Description: Initializes the EPwm3 peripheral.
//#################################################################################################################
static void ei_vInitEPwm3()
{
    EPwm3Regs.ETSEL.bit.INTEN = DISABLE_INT;         // Disable EPWM3_INT
   
    EPwm3Regs.TBCTL.bit.CLKDIV = CLKDIV_1;
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = DIV_1 ;
    EPwm3Regs.TBCTL.bit.SYNCOSEL = DISABLE_PHASE_SYNC;
    EPwm3Regs.TBCTL.bit.PRDLD = LOAD_IN_SHADOW_REG;
    EPwm3Regs.TBCTL.bit.PHSEN = DISABLE_PHASE;
    EPwm3Regs.TBCTL.bit.CTRMODE = UP_DOWN_MODE;

//  EPwm3Regs.TBPRD = EPWM3_PERIOD_VALUE;             // Set period for ePWM3
    EPwm3Regs.TBPRD = ADC7705_MCLK_PRD;               // Set period for ePWM3

    EPwm3Regs.CMPCTL.bit.SHDWBMODE = LOAD_IN_SHADOW_REG ;
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = LOAD_IN_SHADOW_REG ;
    EPwm3Regs.CMPCTL.bit.LOADAMODE = LOAD_DURING_UNDERFLOW ;
    EPwm3Regs.CMPCTL.bit.LOADBMODE = LOAD_DURING_UNDERFLOW ;

    EPwm3Regs.CMPA.half.CMPA = ADC7705_MCLK_PRD/2;
    EPwm3Regs.CMPA.half.CMPAHR = 0;

//  Active low configuration.
    EPwm3Regs.AQCTLA.bit.CAD = PULL_HIGH;
    EPwm3Regs.AQCTLA.bit.CAU = PULL_LOW;

    EPwm3Regs.ETSEL.bit.INTSEL = DISABLE_SELECTON;        // Counter = 0 event for epwm3_int
    EPwm3Regs.ETPS.bit.INTPRD = DISABLE_INT_PERIOD;         // Generate pulse on 1st event
    EPwm3Regs.ETCLR.bit.INT = CLEAR_INTERRUPT;
}

//##################### Initialise EPWM ###########################################################################
// Function Name: ei_vInitEPwm4
// Return Type: void
// Arguments:   void
// Description: Initialises the EPwm4 peripheral.
//#################################################################################################################
static void ei_vInitEPwm4()
{
    EPwm4Regs.ETSEL.bit.INTEN = DISABLE_INT;         // Disable EPWM4_INT
   
    EPwm4Regs.TBCTL.bit.CLKDIV = CLKDIV_1;
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = DIV_1 ;
    EPwm4Regs.TBCTL.bit.SYNCOSEL = PHASE_SYNC_AT_CTR_ZERO;
    EPwm4Regs.TBCTL.bit.PRDLD = LOAD_IN_SHADOW_REG;
    EPwm4Regs.TBCTL.bit.PHSEN = DISABLE_PHASE;
    EPwm4Regs.TBCTL.bit.CTRMODE = UP_DOWN_MODE;

    EPwm4Regs.TBPRD = EPWM4_PERIOD_VALUE;             // Set period for ePWM4

    EPwm4Regs.CMPCTL.bit.SHDWBMODE = LOAD_IN_SHADOW_REG ;
    EPwm4Regs.CMPCTL.bit.SHDWAMODE = LOAD_IN_SHADOW_REG ;
    EPwm4Regs.CMPCTL.bit.LOADAMODE = LOAD_DURING_UNDERFLOW ;
    EPwm4Regs.CMPCTL.bit.LOADBMODE = LOAD_DURING_UNDERFLOW ;

    EPwm4Regs.CMPA.half.CMPA = DEADBAND_UC3825;             // PWM High time is the deadband in hardware
    EPwm4Regs.CMPA.half.CMPAHR = 0;

//  Active low configuration.
    EPwm4Regs.AQCTLA.bit.CAD = PULL_HIGH;
    EPwm4Regs.AQCTLA.bit.CAU = PULL_LOW;

    EPwm4Regs.ETSEL.bit.INTSEL = DISABLE_SELECTON;        // Counter = 0 event for epwm4_int
    EPwm4Regs.ETPS.bit.INTPRD = DISABLE_INT_PERIOD;         // Generate pulse on 1st event
    EPwm4Regs.ETCLR.bit.INT = CLEAR_INTERRUPT;
}

//##################### Initialize EPWM ###########################################################################
// Function Name: ei_vInitEPwm5
// Return Type: void
// Arguments:   void
// Description: Initializes the EPwm5 peripheral.
//#################################################################################################################
static void ei_vInitEPwm5()
{
    EPwm5Regs.ETSEL.bit.INTEN = DISABLE_INT;         // Disable EPWM5_INT
   
    EPwm5Regs.TBCTL.bit.CLKDIV = CLKDIV_1;
    EPwm5Regs.TBCTL.bit.HSPCLKDIV = DIV_1 ;
    EPwm5Regs.TBCTL.bit.SYNCOSEL = DISABLE_PHASE_SYNC;
    EPwm5Regs.TBCTL.bit.PRDLD = LOAD_IN_SHADOW_REG;
    EPwm5Regs.TBCTL.bit.PHSEN = ENABLE_PHASE;
    EPwm5Regs.TBCTL.bit.CTRMODE = UP_DOWN_MODE;

    EPwm5Regs.TBPRD = EPWM5_PERIOD_VALUE;             // Set period for ePWM5

    EPwm5Regs.TBPHS.half.TBPHS = EPWM5_PERIOD_VALUE;

    EPwm5Regs.CMPCTL.bit.SHDWBMODE = LOAD_IN_SHADOW_REG ;
    EPwm5Regs.CMPCTL.bit.SHDWAMODE = LOAD_IN_SHADOW_REG ;
    EPwm5Regs.CMPCTL.bit.LOADAMODE = LOAD_DURING_UNDERFLOW ;
    EPwm5Regs.CMPCTL.bit.LOADBMODE = LOAD_DURING_UNDERFLOW ;

    EPwm5Regs.CMPA.half.CMPA = DEADBAND_UC3825;         // PWM High time is the deadband in hardware
    EPwm5Regs.CMPA.half.CMPAHR = 0;

//  Active low configuration.
    EPwm5Regs.AQCTLA.bit.CAD = PULL_HIGH;
    EPwm5Regs.AQCTLA.bit.CAU = PULL_LOW;

    EPwm5Regs.ETSEL.bit.INTSEL = DISABLE_SELECTON;        // Counter = 0 event for epwm5_int
    EPwm5Regs.ETPS.bit.INTPRD = DISABLE_INT_PERIOD;         // Generate pulse on 1st event
    EPwm5Regs.ETCLR.bit.INT = CLEAR_INTERRUPT;
}

//##################### Initialize EPWM ###########################################################################
// Function Name: ei_vInitEPwm8
// Return Type: void
// Arguments:   void
// Description: Initializes the EPwm8 peripheral.
//#################################################################################################################
static void ei_vInitEPwm8()
{
    EPwm8Regs.ETSEL.bit.INTEN = DISABLE_INT;         // Disable EPWM8_INT
   
    EPwm8Regs.TBCTL.bit.CLKDIV = CLKDIV_1;
    EPwm8Regs.TBCTL.bit.HSPCLKDIV = DIV_1 ;
    EPwm8Regs.TBCTL.bit.SYNCOSEL = DISABLE_PHASE_SYNC;
    EPwm8Regs.TBCTL.bit.PRDLD = LOAD_IN_SHADOW_REG;
    EPwm8Regs.TBCTL.bit.PHSEN = DISABLE_PHASE;
    EPwm8Regs.TBCTL.bit.CTRMODE = UP_DOWN_MODE;

    EPwm8Regs.TBPRD = EPWM8_PERIOD_VALUE;             // Set period for ePWM8

    EPwm8Regs.CMPCTL.bit.SHDWBMODE = LOAD_IN_SHADOW_REG ;
    EPwm8Regs.CMPCTL.bit.SHDWAMODE = LOAD_IN_SHADOW_REG ;
    EPwm8Regs.CMPCTL.bit.LOADAMODE = LOAD_DURING_UNDERFLOW ;
    EPwm8Regs.CMPCTL.bit.LOADBMODE = LOAD_DURING_UNDERFLOW ;

    EPwm8Regs.CMPA.half.CMPA = EPWM8_PERIOD_VALUE;
    EPwm8Regs.CMPA.half.CMPAHR = 0;

//  Active low configuration.
    EPwm8Regs.AQCTLA.bit.CAD = PULL_HIGH;
    EPwm8Regs.AQCTLA.bit.CAU = PULL_LOW;

    EPwm8Regs.ETSEL.bit.INTSEL = DISABLE_SELECTON;        // Counter = 0 event for epwm8_int
    EPwm8Regs.ETPS.bit.INTPRD = DISABLE_INT_PERIOD;         // Generate pulse on 1st event
    EPwm8Regs.ETCLR.bit.INT = CLEAR_INTERRUPT;
}

//##################### Initialize EPWM ###########################################################################
// Function Name: ei_vInitEPwm6
// Return Type: void
// Arguments:   void
// Description: Initializes the EPwm6 peripheral. This acts as the scheduler for the whole project.
//#################################################################################################################
// Scheduler time: 1ms
static void ei_vInitEPwm6()
{
   EPwm6Regs.ETSEL.bit.INTEN = DISABLE_INT;             // Disable EPWM6_INT
   
   EPwm6Regs.TBCTL.bit.CLKDIV = CLKDIV_1;
   EPwm6Regs.TBCTL.bit.HSPCLKDIV = DIV_1 ;
   EPwm6Regs.TBCTL.bit.SYNCOSEL = DISABLE_PHASE_SYNC;
   EPwm6Regs.TBCTL.bit.PRDLD = LOAD_IN_SHADOW_REG;
   EPwm6Regs.TBCTL.bit.PHSEN = DISABLE_PHASE;
   EPwm6Regs.TBCTL.bit.CTRMODE = UP_DOWN_MODE;
   
   EPwm6Regs.TBPRD = EPWM6_PERIOD_VALUE;                // Set period for ePWM6

   EPwm6Regs.ETCLR.bit.INT = CLEAR_INTERRUPT;
   EPwm6Regs.ETSEL.bit.INTEN = ENABLE_INT;              // Enable EPWM6_INT
   EPwm6Regs.ETSEL.bit.INTSEL = INTERRUPT_UNDERFLOW;    // Counter = 0 event for epwm6_int
   EPwm6Regs.ETPS.bit.INTPRD = INTERRUPT_FIRST_EVENT;   // Generate pulse on 1st event
}

//##################### Initialise Scia ##########################################################################
// Function Name: ei_vInitScia
// Return Type: void
// Arguments:   void
// Description: Initialises the Scia peripheral. Interrupt based.
//#################################################################################################################
static void ei_vInitScia()
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

    SciaRegs.SCIFFRX.bit.RXFFOVRCLR = 1; // Clear Overflow Flag//0x405F;
    SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1; // Clear Interrupt Flag
    SciaRegs.SCIFFRX.bit.RXFIFORESET = HOLD_IN_RESET;
    SciaRegs.SCIFFRX.bit.RXFFIENA = ENABLE_INT;
//      SciaRegs.SCIFFRX.bit.RXFFIENA = DISABLE_INT;

    SciaRegs.SCIFFRX.bit.RXFFIL = SINGLE_CHARACTER_FOR_UART_INTERRUPT;
    

//  Auto Baud Disable and no FIFO TX delay.
    SciaRegs.SCIFFCT.all = 0x0000;
   
//  Note: Clocks were turned on to the SCIA peripheral in
//  the InitSysCtrl() function

    SciaRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
                                   // No parity,8 char bits,
                                   // asynch mode, idle-line protocol
    SciaRegs.SCICTL1.all =0x0003;  // enable TX, RX, internal SCICLK, Hold SCI in Reset
                                   // Disable RX ERR, SLEEP, TXWAKE
    SciaRegs.SCICTL2.all =0x0000;  // Rx/BKDT interrupt  and TX(non FIFO mode) interrupt disable

    /*// 1199.87 baud @LSPCLK = 22.5MHz. (1199.87 = 22.5M/{2344*8})
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
    SciaRegs.SCILBAUD    =0x0024;       //                  SCILBAUD = 0x24*/

    // 19263.69 baud @LSPCLK = 22.5MHz. (19263.69 = 22.5M/{296*8})
    SciaRegs.SCIHBAUD    =0x0000;       // 145 = 0x0091 =>  SCIHBAUD = 0x00
    SciaRegs.SCILBAUD    =0x0091;       //                  SCILBAUD = 0x91

    SciaRegs.SCIFFTX.bit.TXFIFOXRESET = 1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET = 1;
    SciaRegs.SCICTL1.all =0x0023;  // Relinquish SCI from Reset
}

//##################### Initialise ECana ##########################################################################
// Function Name: ei_vInitEcana
// Return Type: void
// Arguments:   void
// Description: Initialises the ECana peripheral.
//#################################################################################################################
static void ei_vInitEcana()
{
    EALLOW;

//  Enable RX Function
    ECanaShadow.CANRIOC.all = ECanaRegs.CANRIOC.all;
    ECanaShadow.CANRIOC.bit.RXFUNC = 1;
    ECanaRegs.CANRIOC.all = ECanaShadow.CANRIOC.all;

//  Enable TX Function
    ECanaShadow.CANTIOC.all = ECanaRegs.CANTIOC.all;
    ECanaShadow.CANTIOC.bit.TXFUNC = 1;
    ECanaRegs.CANTIOC.all = ECanaShadow.CANTIOC.all;

//  Disable all Mailboxes
    ECanaShadow.CANME.all = 0;
    ECanaRegs.CANME.all = ECanaShadow.CANME.all;

//  Set CCR  = 1 to request access to CANBTC
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 1;
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

    do
    {
        ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    }while(ECanaShadow.CANES.bit.CCE != 1); // Wait till write Access to CANBTC is granted

//  Set the Bit Time Configurations
    ECanaShadow.CANBTC.all = ECanaRegs.CANBTC.all;

    // Required settings. For Complete information on Baud rate setting refer to ECanApp.c
    // 1. 500kbps Baudrate with Bit Time = 18
    // 2. => BRP = 5
    // 3. 80% Sampling Required
    // 4. TSEG1 = 14; TSEG2 = 3
    // 5. SJW = 2

    ECanaShadow.CANBTC.bit.BRPREG = 4;      // BRPreg = 0 not allowed
    ECanaShadow.CANBTC.bit.SAM = 1; // 3 Sample Points
    ECanaShadow.CANBTC.bit.TSEG1REG = 13;
    ECanaShadow.CANBTC.bit.TSEG2REG = 2;
    ECanaShadow.CANBTC.bit.SJWREG = 1;

    ECanaRegs.CANBTC.all = ECanaShadow.CANBTC.all;

//  Program CANMC
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;

    ECanaShadow.CANMC.bit.SCB = 1; // eCAN Mode
    ECanaShadow.CANMC.bit.DBO = 0; // Data is transmitted or received least significant byte first
    ECanaShadow.CANMC.bit.ABO = 1; // Automatic Bus On
    ECanaShadow.CANMC.bit.STM = 0; // Normal mode. Not in self test mode

    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

//  Tell CAN module that Configuration is complete.
    ECanaShadow.CANMC.all = ECanaRegs.CANMC.all;
    ECanaShadow.CANMC.bit.CCR = 0;              // Configuration Complete
    ECanaRegs.CANMC.all = ECanaShadow.CANMC.all;

//  Write to CANOPC registers to Disable Receive Mailboxes from being overwritten before the message is read.
    ECanaShadow.CANOPC.all = 0x00000000;
    ECanaShadow.CANOPC.bit.OPC0 = 1;
    ECanaRegs.CANOPC.all = ECanaShadow.CANOPC.all;

//  Clear all the MailBox message control registers.
    ECanaMboxes.MBOX0.MSGCTRL.all = 0;
    ECanaMboxes.MBOX1.MSGCTRL.all = 0;
    ECanaMboxes.MBOX2.MSGCTRL.all = 0;
    ECanaMboxes.MBOX3.MSGCTRL.all = 0;
    ECanaMboxes.MBOX4.MSGCTRL.all = 0;
    ECanaMboxes.MBOX5.MSGCTRL.all = 0;
    ECanaMboxes.MBOX6.MSGCTRL.all = 0;
    ECanaMboxes.MBOX7.MSGCTRL.all = 0;
    ECanaMboxes.MBOX8.MSGCTRL.all = 0;
    ECanaMboxes.MBOX9.MSGCTRL.all = 0;
    ECanaMboxes.MBOX10.MSGCTRL.all = 0;
    ECanaMboxes.MBOX11.MSGCTRL.all = 0;
    ECanaMboxes.MBOX12.MSGCTRL.all = 0;
    ECanaMboxes.MBOX13.MSGCTRL.all = 0;
    ECanaMboxes.MBOX14.MSGCTRL.all = 0;
    ECanaMboxes.MBOX15.MSGCTRL.all = 0;
    ECanaMboxes.MBOX16.MSGCTRL.all = 0;
    ECanaMboxes.MBOX17.MSGCTRL.all = 0;
    ECanaMboxes.MBOX18.MSGCTRL.all = 0;
    ECanaMboxes.MBOX19.MSGCTRL.all = 0;
    ECanaMboxes.MBOX20.MSGCTRL.all = 0;
    ECanaMboxes.MBOX21.MSGCTRL.all = 0;
    ECanaMboxes.MBOX22.MSGCTRL.all = 0;
    ECanaMboxes.MBOX23.MSGCTRL.all = 0;
    ECanaMboxes.MBOX24.MSGCTRL.all = 0;
    ECanaMboxes.MBOX25.MSGCTRL.all = 0;
    ECanaMboxes.MBOX26.MSGCTRL.all = 0;
    ECanaMboxes.MBOX27.MSGCTRL.all = 0;
    ECanaMboxes.MBOX28.MSGCTRL.all = 0;
    ECanaMboxes.MBOX29.MSGCTRL.all = 0;
    ECanaMboxes.MBOX30.MSGCTRL.all = 0;
    ECanaMboxes.MBOX31.MSGCTRL.all = 0;

//  Clear all the MailBox message IDE registers.
    ECanaMboxes.MBOX0.MSGID.all = 0;
    ECanaMboxes.MBOX1.MSGID.all = 0;
    ECanaMboxes.MBOX2.MSGID.all = 0;
    ECanaMboxes.MBOX3.MSGID.all = 0;
    ECanaMboxes.MBOX4.MSGID.all = 0;
    ECanaMboxes.MBOX5.MSGID.all = 0;
    ECanaMboxes.MBOX6.MSGID.all = 0;
    ECanaMboxes.MBOX7.MSGID.all = 0;
    ECanaMboxes.MBOX8.MSGID.all = 0;
    ECanaMboxes.MBOX9.MSGID.all = 0;
    ECanaMboxes.MBOX10.MSGID.all = 0;
    ECanaMboxes.MBOX11.MSGID.all = 0;
    ECanaMboxes.MBOX12.MSGID.all = 0;
    ECanaMboxes.MBOX13.MSGID.all = 0;
    ECanaMboxes.MBOX14.MSGID.all = 0;
    ECanaMboxes.MBOX15.MSGID.all = 0;
    ECanaMboxes.MBOX16.MSGID.all = 0;
    ECanaMboxes.MBOX17.MSGID.all = 0;
    ECanaMboxes.MBOX18.MSGID.all = 0;
    ECanaMboxes.MBOX19.MSGID.all = 0;
    ECanaMboxes.MBOX20.MSGID.all = 0;
    ECanaMboxes.MBOX21.MSGID.all = 0;
    ECanaMboxes.MBOX22.MSGID.all = 0;
    ECanaMboxes.MBOX23.MSGID.all = 0;
    ECanaMboxes.MBOX24.MSGID.all = 0;
    ECanaMboxes.MBOX25.MSGID.all = 0;
    ECanaMboxes.MBOX26.MSGID.all = 0;
    ECanaMboxes.MBOX27.MSGID.all = 0;
    ECanaMboxes.MBOX28.MSGID.all = 0;
    ECanaMboxes.MBOX29.MSGID.all = 0;
    ECanaMboxes.MBOX30.MSGID.all = 0;
    ECanaMboxes.MBOX31.MSGID.all = 0;

    do
    {
        ECanaShadow.CANES.all = ECanaRegs.CANES.all;
    }while(ECanaShadow.CANES.bit.CCE != 0); // Wait till write CAN returns to normal mode after end of configuration

    ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
    ECanaShadow.CANME.all = ECanaRegs.CANME.all;

    // Generic CAN Configuration for All 3U Psu's
//  CONFIGURE MAILBOX 15 for Global enArka Messages
    ECanaMboxes.MBOX15.MSGID.all = MBOX15_BASE_MSG_ID;
    ECanaMboxes.MBOX15.MSGID.bit.IDE = EXTENDED_IDENTIFIER;
    ECanaMboxes.MBOX15.MSGID.bit.AME = 1;                       // Allow AME

    ECanaMboxes.MBOX15.MSGCTRL.bit.RTR = 0; // Remote Handling disabled

    //Program the Acceptance mask to allow message with all IDs
    ECanaLAMRegs.LAM15.all = MBOX15_MSG_ID_MASK;
    ECanaLAMRegs.LAM15.bit.LAMI = NO_MASKING_FOR_IDE;       // LAMI is set to 0 to allow frames specified in the IDE bit

    //Set Direction and Enable MBOX
    ECanaShadow.CANMD.bit.MD15 = RECEIVE_MAILBOX;
    ECanaShadow.CANME.bit.ME15 = 1; // Enable Mailbox

//  CONFIGURE MAILBOX 14 for Global PSU Messages
    ECanaMboxes.MBOX14.MSGID.all = MBOX14_BASE_MSG_ID;
    ECanaMboxes.MBOX14.MSGID.bit.IDE = EXTENDED_IDENTIFIER;
    ECanaMboxes.MBOX14.MSGID.bit.AME = 1;                       // Allow AME

    ECanaMboxes.MBOX14.MSGCTRL.bit.RTR = 0; // Remote Handling disabled

    //Program the Acceptance mask to allow message with all IDs
    ECanaLAMRegs.LAM14.all = MBOX14_MSG_ID_MASK;
    ECanaLAMRegs.LAM14.bit.LAMI = NO_MASKING_FOR_IDE;       // LAMI is set to 0 to allow frames specified in the IDE bit

    //Set Direction and Enable MBOX
    ECanaShadow.CANMD.bit.MD14 = RECEIVE_MAILBOX;
    ECanaShadow.CANME.bit.ME14 = 1; // Enable Mailbox

//  CONFIGURE MAILBOX 13 for Global PSU 3U Messages
    ECanaMboxes.MBOX13.MSGID.all = MBOX13_BASE_MSG_ID;
    ECanaMboxes.MBOX13.MSGID.bit.IDE = EXTENDED_IDENTIFIER;
    ECanaMboxes.MBOX13.MSGID.bit.AME = 1;                       // Allow AME

    ECanaMboxes.MBOX13.MSGCTRL.bit.RTR = 0; // Remote Handling disabled

    //Program the Acceptance mask to allow message with all IDs
    ECanaLAMRegs.LAM13.all = MBOX13_MSG_ID_MASK;
    ECanaLAMRegs.LAM13.bit.LAMI = NO_MASKING_FOR_IDE;       // LAMI is set to 0 to allow frames specified in the IDE bit

    //Set Direction and Enable MBOX
    ECanaShadow.CANMD.bit.MD13 = RECEIVE_MAILBOX;
    ECanaShadow.CANME.bit.ME13 = 1; // Enable Mailbox

    ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
    ECanaRegs.CANME.all = ECanaShadow.CANME.all;

    // No Can interrupts used in the project. Enable interrupts if required later
    ECanaRegs.CANGAM.all = 0x00000000;
    ECanaRegs.CANGIM.all = 0x00000000;
    ECanaRegs.CANMIL.all = 0x00000000;
    ECanaRegs.CANMIM.all = 0x00000000;
    ECanaRegs.CANGIF0.all = 0xFFFFFFFF;
    ECanaRegs.CANGIF1.all = 0xFFFFFFFF;

    EDIS;
}

/*//Configure Interrupts. Interrupts required for Mailbox 1 in the interrupt level 1.
    ECanaShadow.CANMIL.all = ECanaRegs.CANMIL.all;
    ECanaShadow.CANMIL.all = 0xFFFFFFFF;  // All interrupts at Level 1
    ECanaRegs.CANMIL.all = ECanaShadow.CANMIL.all;

    ECanaShadow.CANMIM.all = ECanaRegs.CANMIM.all;
    ECanaShadow.CANMIM.all = 0x00000000;  // Clear all
    ECanaShadow.CANMIM.bit.MIM1 = 1;  // Enable MBOX1 Interrup1
    ECanaRegs.CANMIM.all = ECanaShadow.CANMIM.all;

    ECanaShadow.CANGIM.all = ECanaRegs.CANGIM.all;
    ECanaShadow.CANGIM.all = 0x00000000;  // Clear all.
    ECanaShadow.CANGIM.bit.I0EN = 1;  // Enable Interrupt level 0
    ECanaShadow.CANGIM.bit.I1EN = 1;  // Enable Interrupt level 1
//  NOTE: Global interrupts should always be enabled. Enable them later.
    ECanaShadow.CANGIM.bit.GIL = 0;   // Global Interrupts mapped to level 0
    ECanaRegs.CANGIM.all = ECanaShadow.CANGIM.all;*/

//##################### Device specific Can MboxInit ##############################################################
// Function Name: ei_vDeviceSpecificCanMailboxInit
// Return Type: void
// Arguments:   void
// Description: Initializes the device specific mailbox depending on the Can model no. stored in EEPROM and the
//              last PSU address stored in EEPROM. Called everytime PSU boots.
//#################################################################################################################
void ei_vDeviceSpecificCanMailboxInit()
{
    Uint16 a_ui_I2cRxDataArray[2];
    Uint32 ul_CanBaseIdForModelType;

    if((ei_uiI2CRead(EEPROM_24LC256_ADDRESS, CAN_MODEL_NO_ADDR, 2, a_ui_I2cRxDataArray)) == FAILURE)
    {
        return;
    }

    // To go inside this if, the PSU must have been already calibrated once from RS485.
    if(a_ui_I2cRxDataArray[0] != BLANK)
    {
        if(a_ui_I2cRxDataArray[0] == a_ui_I2cRxDataArray[1])    // EEPROM Integrity
        {
            ul_CanBaseIdForModelType = (Uint32)a_ui_I2cRxDataArray[0] << MODEL_TO_BASE_ID_LEFT_SHIFT_FACTOR;
            ECanaShadow.CANMD.all = ECanaRegs.CANMD.all;
            ECanaShadow.CANME.all = ECanaRegs.CANME.all;

            // Device specific CAN Configuration for All 1U Psu's

            // CONFIGURE MAILBOX 12 for Global Messages for this model
            ECanaMboxes.MBOX12.MSGID.all = CAN_BASE_ID_FOR_USER_SM_COM + CAN_BASE_ID_FOR_PRODUCT + CAN_BASE_ID_FOR_FAMILY + ul_CanBaseIdForModelType + CAN_BASE_ID_FOR_GLOBAL_MESSAGES;
            ECanaMboxes.MBOX12.MSGID.bit.IDE = EXTENDED_IDENTIFIER;
            ECanaMboxes.MBOX12.MSGID.bit.AME = 1;                       // Allow AME

            ECanaMboxes.MBOX12.MSGCTRL.bit.RTR = 0; // Remote Handling disabled

            //Program the Acceptance mask to allow message with all IDs
            ECanaLAMRegs.LAM12.all = MBOX12_MSG_ID_MASK;
            ECanaLAMRegs.LAM12.bit.LAMI = NO_MASKING_FOR_IDE;       // LAMI is set to 0 to allow frames specified in the IDE bit

            //Set Direction and Enable MBOX
            ECanaShadow.CANMD.bit.MD12 = RECEIVE_MAILBOX;
            ECanaShadow.CANME.bit.ME12 = 1; // Enable Mailbox

            //  CONFIGURE MAILBOX 11 for Calibration messages for this model
            ECanaMboxes.MBOX11.MSGID.all = CAN_BASE_ID_FOR_USER_SM_COM + CAN_BASE_ID_FOR_PRODUCT + CAN_BASE_ID_FOR_FAMILY + ul_CanBaseIdForModelType + CAN_BASE_ID_FOR_CALIBRATION_MESSAGES + ((Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
            ECanaMboxes.MBOX11.MSGID.bit.IDE = EXTENDED_IDENTIFIER;
            ECanaMboxes.MBOX11.MSGID.bit.AME = 1;                       // Allow AME

            ECanaMboxes.MBOX11.MSGCTRL.bit.RTR = 0; // Remote Handling disabled

            //Program the Acceptance mask to allow message with all IDs
            ECanaLAMRegs.LAM11.all = MBOX11_MSG_ID_MASK;
            ECanaLAMRegs.LAM11.bit.LAMI = NO_MASKING_FOR_IDE;       // LAMI is set to 0 to allow frames specified in the IDE bit

            //Set Direction and Enable MBOX
            ECanaShadow.CANMD.bit.MD11 = RECEIVE_MAILBOX;
            ECanaShadow.CANME.bit.ME11 = 1; // Enable Mailbox

            // CONFIGURE MAILBOX 10 for Output Control messages for this model
            ECanaMboxes.MBOX10.MSGID.all = CAN_BASE_ID_FOR_USER_SM_COM + CAN_BASE_ID_FOR_PRODUCT + CAN_BASE_ID_FOR_FAMILY + ul_CanBaseIdForModelType + CAN_BASE_ID_FOR_OUTPUT_CONTROL_MESSAGES + ((Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
            ECanaMboxes.MBOX10.MSGID.bit.IDE = EXTENDED_IDENTIFIER;
            ECanaMboxes.MBOX10.MSGID.bit.AME = 1;                       // Allow AME

            ECanaMboxes.MBOX10.MSGCTRL.bit.RTR = 0; // Remote Handling disabled

            //Program the Acceptance mask to allow message with all IDs
            ECanaLAMRegs.LAM10.all = MBOX10_MSG_ID_MASK;
            ECanaLAMRegs.LAM10.bit.LAMI = NO_MASKING_FOR_IDE;       // LAMI is set to 0 to allow frames specified in the IDE bit

            //Set Direction and Enable MBOX
            ECanaShadow.CANMD.bit.MD10 = RECEIVE_MAILBOX;
            ECanaShadow.CANME.bit.ME10 = 1; // Enable Mailbox

            // CONFIGURE MAILBOX 9 for State Control messages for this model
            ECanaMboxes.MBOX9.MSGID.all = CAN_BASE_ID_FOR_USER_SM_COM + CAN_BASE_ID_FOR_PRODUCT + CAN_BASE_ID_FOR_FAMILY + ul_CanBaseIdForModelType + CAN_BASE_ID_FOR_STATE_CONTROL_MESSAGES + ((Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
            ECanaMboxes.MBOX9.MSGID.bit.IDE = EXTENDED_IDENTIFIER;
            ECanaMboxes.MBOX9.MSGID.bit.AME = 1;                        // Allow AME

            ECanaMboxes.MBOX9.MSGCTRL.bit.RTR = 0;  // Remote Handling disabled

            //Program the Acceptance mask to allow message with all IDs
            ECanaLAMRegs.LAM9.all = MBOX9_MSG_ID_MASK;
            ECanaLAMRegs.LAM9.bit.LAMI = NO_MASKING_FOR_IDE;        // LAMI is set to 0 to allow frames specified in the IDE bit

            //Set Direction and Enable MBOX
            ECanaShadow.CANMD.bit.MD9 = RECEIVE_MAILBOX;
            ECanaShadow.CANME.bit.ME9 = 1; // Enable Mailbox

            // CONFIGURE MAILBOX 8 for Query Type messages messages for this model
            ECanaMboxes.MBOX8.MSGID.all = CAN_BASE_ID_FOR_USER_SM_COM + CAN_BASE_ID_FOR_PRODUCT + CAN_BASE_ID_FOR_FAMILY + ul_CanBaseIdForModelType + CAN_BASE_ID_FOR_QUERY_TYPE_MESSAGES + ((Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
            ECanaMboxes.MBOX8.MSGID.bit.IDE = EXTENDED_IDENTIFIER;
            ECanaMboxes.MBOX8.MSGID.bit.AME = 1;                        // Allow AME

            ECanaMboxes.MBOX8.MSGCTRL.bit.RTR = 0;  // Remote Handling disabled

            //Program the Acceptance mask to allow message with all IDs
            ECanaLAMRegs.LAM8.all = MBOX8_MSG_ID_MASK;
            ECanaLAMRegs.LAM8.bit.LAMI = NO_MASKING_FOR_IDE;        // LAMI is set to 0 to allow frames specified in the IDE bit

            //Set Direction and Enable MBOX
            ECanaShadow.CANMD.bit.MD8 = RECEIVE_MAILBOX;
            ECanaShadow.CANME.bit.ME8 = 1; // Enable Mailbox

            // CONFIGURE MAILBOX 31 to transmit General responses
            // Clear TRS.31
            ECanaShadow.CANTRR.all = ECanaRegs.CANTRR.all;
            ECanaShadow.CANTRR.bit.TRR31 = 1;  // This will clear TRS.0

            do
            {
                ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
            }while(ECanaShadow.CANTRS.bit.TRS31 != 0); // Wait till CANTRS.31 is 0

            ECanaMboxes.MBOX31.MSGID.all = CAN_BASE_ID_FOR_USER_SM_COM + CAN_BASE_ID_FOR_PRODUCT + CAN_BASE_ID_FOR_FAMILY + ul_CanBaseIdForModelType + CAN_BASE_ID_FOR_TRANSMIT_MBOX + CAN_BASE_ID_FOR_SRQ + ((Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
            ECanaMboxes.MBOX31.MSGID.bit.IDE = 1;
            ECanaMboxes.MBOX31.MSGCTRL.bit.DLC = 8;

            // Message in this mailbox is fixed
            ECanaMboxes.MBOX31.MDL.all = 0xFFFFFFFF;
            ECanaMboxes.MBOX31.MDH.all = 0xFFFFFFFF;

            ECanaShadow.CANMD.bit.MD31 = TRANSMIT_MAILBOX;  // Configured as transmit
            ECanaShadow.CANME.bit.ME31 = 1;                 // Enable Mailbox

            // CONFIGURE MAILBOX 16 to transmit General responses
            // Clear TRS.16
            ECanaShadow.CANTRR.all = ECanaRegs.CANTRR.all;
            ECanaShadow.CANTRR.bit.TRR16 = 1;  // This will clear TRS.0

            do
            {
                ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
            }while(ECanaShadow.CANTRS.bit.TRS16 != 0); // Wait till CANTRS.16 is 0

            ECanaMboxes.MBOX16.MSGID.all = CAN_BASE_ID_FOR_USER_SM_COM + CAN_BASE_ID_FOR_PRODUCT + CAN_BASE_ID_FOR_FAMILY + ul_CanBaseIdForModelType + CAN_BASE_ID_FOR_TRANSMIT_MBOX + CAN_BASE_ID_FOR_GENERAL_RESPONSES + ((Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
            ECanaMboxes.MBOX16.MSGID.bit.IDE = 1;
            ECanaMboxes.MBOX16.MSGCTRL.bit.DLC = 8;

            ECanaShadow.CANMD.bit.MD16 = TRANSMIT_MAILBOX;  // Configured as transmit
            ECanaShadow.CANME.bit.ME16 = 1;                 // Enable Mailbox

            // CONFIGURE MAILBOX 17 to transmit Responses to Queries
            // Clear TRS.17
            ECanaShadow.CANTRR.all = ECanaRegs.CANTRR.all;
            ECanaShadow.CANTRR.bit.TRR17 = 1;  // This will clear TRS.0

            do
            {
                ECanaShadow.CANTRS.all = ECanaRegs.CANTRS.all;
            }while(ECanaShadow.CANTRS.bit.TRS17 != 0); // Wait till CANTRS.16 is 0

            ECanaMboxes.MBOX17.MSGID.all = CAN_BASE_ID_FOR_USER_SM_COM + CAN_BASE_ID_FOR_PRODUCT + CAN_BASE_ID_FOR_FAMILY + ul_CanBaseIdForModelType + CAN_BASE_ID_FOR_TRANSMIT_MBOX + CAN_BASE_ID_FOR_RESPONSE_TO_QUERIES + ((Reference.ui_Set_Address) << ADDRESS_TO_BASE_ID_LEFT_SHIFT_FACTOR);
            ECanaMboxes.MBOX17.MSGID.bit.IDE = 1;
            ECanaMboxes.MBOX17.MSGCTRL.bit.DLC = 8;

            ECanaShadow.CANMD.bit.MD17 = TRANSMIT_MAILBOX;  // Configured as transmit
            ECanaShadow.CANME.bit.ME17 = 1;                 // Enable Mailbox

            ECanaRegs.CANMD.all = ECanaShadow.CANMD.all;
            ECanaRegs.CANME.all = ECanaShadow.CANME.all;
        }
    }
}

//##################### Initialise Timer0 ##########################################################################
// Function Name: ei_vInitTimer0
// Return Type: void
// Arguments:   void
// Description: Initialises the Timer 0 peripheral for Power On time measurement.
//#################################################################################################################
static void ei_vInitTimer0()
{
    // GENERAL TIMER INITIALISATION BEFORE GETTING INTO APPLICATION SPECIFIC INITIALIZATION
    // CPU Timer 0
    /*// Initialise address pointers to respective timer registers:
    CpuTimer0.RegsAddr = &CpuTimer0Regs;*/
    // Initialise timer period to maximum:
    CpuTimer0Regs.PRD.all  = 0xFFFFFFFF;
    // Initialise prescale counter to divide by 1 (SYSCLKOUT):
    CpuTimer0Regs.TPR.all  = 0;
    CpuTimer0Regs.TPRH.all = 0;
    // Make sure timer is stopped:
    CpuTimer0Regs.TCR.bit.TSS = 1;
    // Reload all counter register with period value:
    CpuTimer0Regs.TCR.bit.TRB = 1;

    // APPLICATION SPECIFIC INITIALIZATION
    Uint32  temp,CpuFreqInMHz,PeriodInUSec,TotalTimeInUSec;

    // Enter Specific Values desired
    CpuFreqInMHz = 90;          // We are Operating at 90MHz
    PeriodInUSec = 1000000;     // 1 Second in us
    TotalTimeInUSec = 60000000; // 60 seconds of total time desired

    // Calculation
    // Time period set in PRD Register

    /*PRD = CpuFreqInMHz * PeriodInUSec;*/

    // Total time given

    /*Total_Time = (PRD + (TPR+1))*Sysclockout;*/

    /*For 60 seconds:
    Set PRD to Countdown 1 second.
    Set TRP to 59;*/

    // Initialise timer period:
    temp = (long) (CpuFreqInMHz * PeriodInUSec);
    CpuTimer0Regs.PRD.all = temp;
    
    // Initialise Prescaler
    temp = (long)(TotalTimeInUSec/PeriodInUSec);
    CpuTimer0Regs.TPR.bit.TDDR = (temp&0x0000FFFF);     // Lower 16 bits
    CpuTimer0Regs.TPRH.bit.TDDRH = (temp>>16);  // Upper 16 bits

    // Initialise timer control register:
    CpuTimer0Regs.TCR.bit.TSS = 1;      // 1 = Stop timer, 0 = Start/Restart Timer
    CpuTimer0Regs.TCR.bit.TRB = 1;      // 1 = reload timer
    CpuTimer0Regs.TCR.bit.SOFT = 0;     // Timer Hard decrement
    CpuTimer0Regs.TCR.bit.FREE = 0;     
    CpuTimer0Regs.TCR.bit.TIE = 1;      // 0 = Disable/ 1 = Enable Timer Interrupt
}
