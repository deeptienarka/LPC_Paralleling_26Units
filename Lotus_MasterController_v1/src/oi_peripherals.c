/*
 * oi_peripherals.c
 *
 *  Created on: Jan 15, 2021
 *      Author: enArka
 */

#include <oi_can.h>
#include "F2837xS_device.h"
#include "F2837xS_Examples.h"
#include "oi_initPeripherals.h"
#include "oi_globalPrototypes.h"
#include "oi_externVariables.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_can.h"
#include "driverlib/can.h"
//
// Initialize EPWM1 configuration - ADC Start of Conversion
//
void oi_initEPwm()
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0; // This bit reset to 0 stops all PWM clocks.
    oi_initEPwm1();                                   // Initialization of EPWMs

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; // This bit set to 1 turns ON all pwm clocks at the same time
    EDIS;
}



//
// InitEPwm2Example - Initialize EPWM2 configuration - ADC Start of Conversion
//
void oi_initEPwm1()
{
    //
    // Setup TBCLK
    //
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up/down and down
    EPwm1Regs.TBPRD = EPWM1_TIMER_TBPRD;           // Set timer period
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;        // Disable phase loading
    EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;            // Phase is 0
    EPwm1Regs.TBCTR = 0x0000;                      // Clear counter
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;       // Clock ratio to SYSCLKOUT
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    //
    // Setup shadow register load on ZERO
    //
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO_PRD;
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO_PRD;

    //
    // Set Actions
    //
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;         // Set PWM2A on period
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;       // Clear PWM2A on event B, down
                                             // count

    EPwm1Regs.AQCTLB.bit.CBU = AQ_CLEAR;       // Clear PWM2A on period
    EPwm1Regs.AQCTLB.bit.CBD = AQ_SET;         // Set PWM2A on event A, up

    EPwm1Regs.CMPA.bit.CMPA = 0;

    // Interrupt where we will change the Compare Values
    //
    EPwm1Regs.ETSEL.bit.INTSEL = ET_CTR_PRD;    // Select INT on Zero event
    EPwm1Regs.ETSEL.bit.INTEN = 1;               // Enable INT
    EPwm1Regs.ETPS.bit.INTPRD = ET_1ST;          // Generate INT on 3rd event

}


void oi_CAN_init(void)
{

// /***************PSU Message IDs***************/

    // CAN TX  message ID's
    CAN_setupMessageObject(CANA_BASE, MAILBOX_1, 0x11100004, CAN_MSG_FRAME_EXT,
                           CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                           CAN_MSG_OBJ_NO_FLAGS,
                           MSG_DATA_LENGTH);

    // CAN RX  message ID's


    CAN_setupMessageObject(CANA_BASE, MAILBOX_5, 0x111B0962, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER | CAN_MSG_OBJ_USE_EXT_FILTER,  //
                             MSG_DATA_LENGTH_RX);

    CAN_setupMessageObject(CANA_BASE, MAILBOX_6, 0x111B096C, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER | CAN_MSG_OBJ_USE_EXT_FILTER,
                             7);

    CAN_setupMessageObject(CANA_BASE, MAILBOX_7, 0x111B0961, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER | CAN_MSG_OBJ_USE_EXT_FILTER,
                             MSG_DATA_LENGTH_RX);


    /*************** Voltage Sensing Card Message IDs ***************/

    // CAN RX  message ID's

    CAN_setupMessageObject(CANA_BASE, MAILBOX_12, 0x14110805, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER|CAN_MSG_OBJ_USE_EXT_FILTER,
                             MSG_DATA_LENGTH_RX);

    CAN_setupMessageObject(CANA_BASE, MAILBOX_13, 0x1411080A, CAN_MSG_FRAME_EXT,
                           CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_RX_INT_ENABLE | CAN_MSG_OBJ_USE_ID_FILTER | CAN_MSG_OBJ_USE_EXT_FILTER,
                           MSG_DATA_LENGTH);

    /************** I/O Card Message IDs *****************************/

    // CAN RX Message IDs

    CAN_setupMessageObject(CANA_BASE, MAILBOX_16, 0x14210121, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER|CAN_MSG_OBJ_USE_EXT_FILTER,  // Response of Analog Inputs Block 1
                             MSG_DATA_LENGTH_RX);

    CAN_setupMessageObject(CANA_BASE, MAILBOX_17, 0x14210122, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER|CAN_MSG_OBJ_USE_EXT_FILTER,  // Response of Analog Inputs Block 2
                             MSG_DATA_LENGTH_RX);

    CAN_setupMessageObject(CANA_BASE, MAILBOX_18, 0x14210131, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER|CAN_MSG_OBJ_USE_EXT_FILTER,  // Response of Thermocouple values
                             MSG_DATA_LENGTH_RX);

    CAN_setupMessageObject(CANA_BASE, MAILBOX_19, 0x14210111, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER|CAN_MSG_OBJ_USE_EXT_FILTER,  // Response of General I/O Parameters
                             MSG_DATA_LENGTH_RX);

    CAN_startModule(CANA_BASE);


    // Start CAN B module operations

    // CAN TX  message ID's
    CAN_setupMessageObject(CANB_BASE, MAILBOX_1, 14160111, CAN_MSG_FRAME_EXT,
                           CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                           CAN_MSG_OBJ_NO_FLAGS,
                           MSG_DATA_LENGTH_TX_CANB);

    CAN_setupMessageObject(CANB_BASE, MAILBOX_2, 0x14160111, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER|CAN_MSG_OBJ_USE_EXT_FILTER,  // Response of Analog Inputs Block 1
                             MSG_DATA_LENGTH_RX);

    CAN_setupMessageObject(CANB_BASE, MAILBOX_3, 0x14160120, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER|CAN_MSG_OBJ_USE_EXT_FILTER,  // Response of Analog Inputs Block 2
                             MSG_DATA_LENGTH_RX);

    CAN_setupMessageObject(CANB_BASE, MAILBOX_4, 0x14160121, CAN_MSG_FRAME_EXT,
                             CAN_MSG_OBJ_TYPE_RX, 0x1FFF00FF, CAN_MSG_OBJ_USE_ID_FILTER|CAN_MSG_OBJ_USE_EXT_FILTER,  // Response of Thermocouple values
                             MSG_DATA_LENGTH_RX);

    CAN_setupMessageObject(CANB_BASE, MAILBOX_6, 0x14160155, CAN_MSG_FRAME_EXT,
                           CAN_MSG_OBJ_TYPE_TX, 0x1FFFFFFF,
                           CAN_MSG_OBJ_NO_FLAGS,
                           8);

    CAN_startModule(CANB_BASE);

}






void oi_initPeripherals()
{
    EALLOW;
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = DIVIDE_BY_TWO;             // Divide by 2 of PLLSYSCLK
    EDIS;
    oi_initGpio();                                                      // GPIO initialization to the PWMs
    oi_initEPwm();                                                      // Initialization of EPWMs
    //
    // Setup GPIO pin mux for CAN-A TX/RX and CAN-B TX/RX
    //
    GPIO_SetupPinMux(62, GPIO_MUX_CPU1, 6); //GPIO30 -  CANRXA
    GPIO_SetupPinOptions(62, GPIO_INPUT, GPIO_ASYNC);
    GPIO_SetupPinMux(63, GPIO_MUX_CPU1, 6); //GPIO31 - CANTXA
    GPIO_SetupPinOptions(63, GPIO_OUTPUT, GPIO_PUSHPULL);

    GPIO_SetupPinMux(73, GPIO_MUX_CPU1, 5); //GPIO73 -  CANRXB
    GPIO_SetupPinOptions(73, GPIO_INPUT, GPIO_ASYNC);
    GPIO_SetupPinMux(20, GPIO_MUX_CPU1, 3); //GPIO20 - CANTXB
    GPIO_SetupPinOptions(20, GPIO_OUTPUT, GPIO_PUSHPULL);
    //
       // Initialize the CAN controller
       //
       CAN_initModule(CANA_BASE);
       CAN_initModule(CANB_BASE);

    //
     // Set up the CAN bus bit rate to 500 kbps
     // Refer to the Driver Library User Guide for information on how to set
     // tighter timing control. Additionally, consult the device data sheet
     // for more information about the CAN module clocking.
     //
     CAN_setBitRate(CANA_BASE, 200000000, 500000, 20);
     CAN_setBitRate(CANB_BASE, 200000000, 500000, 20);


     CAN_disableTestMode(CANA_BASE);  // Disable Test Mode in CANA Module.
     CAN_disableTestMode(CANB_BASE);  // Disable Test Mode in CANA Module.


     //
     // Enable interrupts on the CAN peripheral.
     // Enables Interrupt line 0, Error & Status Change interrupts in CAN_CTL
     // register.
     //

     CAN_enableInterrupt(CANA_BASE, CAN_INT_IE0 | CAN_INT_ERROR | CAN_INT_STATUS);
     CAN_enableInterrupt(CANB_BASE, CAN_INT_IE0 | CAN_INT_ERROR | CAN_INT_STATUS);

}

