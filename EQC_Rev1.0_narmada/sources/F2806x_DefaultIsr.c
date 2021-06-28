//###########################################################################
//
// FILE:	F2806x_DefaultIsr.c
//
// TITLE:	F2806x Device Default Interrupt Service Routines.
//
// This file contains shell ISR routines for the 2803x PIE vector table.
// Typically these shell ISR routines can be used to populate the entire PIE
// vector table during device debug.  In this manner if an interrupt is taken
// during firmware development, there will always be an ISR to catch it.
//
// As develpment progresses, these ISR rotuines can be eliminated and replaced
// with the user's own ISR routines for each interrupt.  Since these shell ISRs
// include infinite loops they will typically not be included as-is in the final
// production firmware.
//
//###########################################################################
// $TI Release: F2806x C/C++ Header Files and Peripheral Examples V130 $
// $Release Date: November 30, 2011 $
//###########################################################################

#include "F2806x_Device.h"     // F2806x Headerfile Include File
#include "F2806x_Examples.h"   // F2806x Examples Include File

// Connected to INT13 of CPU (use MINT13 mask):
// ISR can be used by the user.
#pragma CODE_SECTION(INT13_ISR,"Critical_Section");
__interrupt void INT13_ISR(void)     // INT13 or CPU-Timer1
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(INT14_ISR,"Critical_Section");
__interrupt void INT14_ISR(void)     // INT14 or CPU-Timer2
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(DATALOG_ISR,"Critical_Section");
__interrupt void DATALOG_ISR(void)   // Datalogging interrupt
{
   // Insert ISR Code here

   // Next two lines for debug only to halt the processor here
   // Remove after inserting ISR Code
  __asm ("      ESTOP0");
   for(;;);
}

#pragma CODE_SECTION(RTOSINT_ISR,"Critical_Section");
__interrupt void RTOSINT_ISR(void)   // RTOS interrupt
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(EMUINT_ISR,"Critical_Section");
__interrupt void EMUINT_ISR(void)    // Emulation interrupt
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(NMI_ISR,"Critical_Section");
__interrupt void NMI_ISR(void)       // Non-maskable interrupt
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(ILLEGAL_ISR,"Critical_Section");
__interrupt void ILLEGAL_ISR(void)   // Illegal operation TRAP
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm("          ESTOP0");
  for(;;);

}

#pragma CODE_SECTION(USER1_ISR,"Critical_Section");
__interrupt void USER1_ISR(void)     // User Defined trap 1
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

#pragma CODE_SECTION(USER2_ISR,"Critical_Section");
__interrupt void USER2_ISR(void)     // User Defined trap 2
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

#pragma CODE_SECTION(USER3_ISR,"Critical_Section");
__interrupt void USER3_ISR(void)     // User Defined trap 3
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(USER4_ISR,"Critical_Section");
__interrupt void USER4_ISR(void)     // User Defined trap 4
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(USER5_ISR,"Critical_Section");
__interrupt void USER5_ISR(void)     // User Defined trap 5
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(USER6_ISR,"Critical_Section");
__interrupt void USER6_ISR(void)     // User Defined trap 6
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(USER7_ISR,"Critical_Section");
__interrupt void USER7_ISR(void)     // User Defined trap 7
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(USER8_ISR,"Critical_Section");
__interrupt void USER8_ISR(void)     // User Defined trap 8
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(USER9_ISR,"Critical_Section");
__interrupt void USER9_ISR(void)     // User Defined trap 9
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(USER10_ISR,"Critical_Section");
__interrupt void USER10_ISR(void)    // User Defined trap 10
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(USER11_ISR,"Critical_Section");
__interrupt void USER11_ISR(void)    // User Defined trap 11
{
  // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(USER12_ISR,"Critical_Section");
__interrupt void USER12_ISR(void)     // User Defined trap 12
{
 // Insert ISR Code here

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// -----------------------------------------------------------
// PIE Group 1 - MUXed into CPU INT1
// -----------------------------------------------------------
// INT1.1
#pragma CODE_SECTION(ADCINT1_ISR,"Critical_Section");
__interrupt void ADCINT1_ISR(void)   // ADC  (Can also be ISR for INT10.1 when enabled)
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code

 __asm ("      ESTOP0");
  for(;;);

}

// INT1.2
#pragma CODE_SECTION(ADCINT2_ISR,"Critical_Section");
__interrupt void ADCINT2_ISR(void)  // ADC  (Can also be ISR for INT10.2 when enabled)
{

  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code

 __asm("	  ESTOP0");
  for(;;);

}

// INT1.3 - Reserved

// INT1.4
#pragma CODE_SECTION(XINT1_ISR,"Critical_Section");
__interrupt void  XINT1_ISR(void)
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT1.5
#pragma CODE_SECTION(XINT2_ISR,"Critical_Section");
__interrupt void  XINT2_ISR(void)
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT1.6
#pragma CODE_SECTION(ADCINT9_ISR,"Critical_Section");
__interrupt void  ADCINT9_ISR(void)
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT1.7
#pragma CODE_SECTION(TINT0_ISR,"Critical_Section");
__interrupt void  TINT0_ISR(void)      // CPU-Timer 0
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT1.8
#pragma CODE_SECTION(WAKEINT_ISR,"Critical_Section");
__interrupt void  WAKEINT_ISR(void)    // WD, LOW Power
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// -----------------------------------------------------------
// PIE Group 2 - MUXed into CPU INT2
// -----------------------------------------------------------

// INT2.1
#pragma CODE_SECTION(EPWM1_TZINT_ISR,"Critical_Section");
__interrupt void EPWM1_TZINT_ISR(void)    // EPWM Trip Zone-1
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT2.2
#pragma CODE_SECTION(EPWM2_TZINT_ISR,"Critical_Section");
__interrupt void EPWM2_TZINT_ISR(void)    // EPWM Trip Zone-2
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT2.3
#pragma CODE_SECTION(EPWM3_TZINT_ISR,"Critical_Section");
__interrupt void EPWM3_TZINT_ISR(void)    // EPWM Trip Zone-3
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT2.4
#pragma CODE_SECTION(EPWM4_TZINT_ISR,"Critical_Section");
__interrupt void EPWM4_TZINT_ISR(void)    // EPWM Trip Zone-4
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT2.5
#pragma CODE_SECTION(EPWM5_TZINT_ISR,"Critical_Section");
__interrupt void EPWM5_TZINT_ISR(void)    // EPWM Trip Zone-5
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT2.6
#pragma CODE_SECTION(EPWM6_TZINT_ISR,"Critical_Section");
__interrupt void EPWM6_TZINT_ISR(void)    // EPWM Trip Zone-6
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT2.7
#pragma CODE_SECTION(EPWM7_TZINT_ISR,"Critical_Section");
__interrupt void EPWM7_TZINT_ISR(void)    // EPWM Trip Zone-7
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT2.8
#pragma CODE_SECTION(EPWM8_TZINT_ISR,"Critical_Section");
__interrupt void EPWM8_TZINT_ISR(void)    // EPWM Trip Zone-8
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP2;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// -----------------------------------------------------------
// PIE Group 3 - MUXed into CPU INT3
// -----------------------------------------------------------

// INT 3.1
#pragma CODE_SECTION(EPWM1_INT_ISR,"Critical_Section");
__interrupt void EPWM1_INT_ISR(void)     // EPWM-1
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT3.2
#pragma CODE_SECTION(EPWM2_INT_ISR,"Critical_Section");
__interrupt void EPWM2_INT_ISR(void)     // EPWM-2
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT3.3
#pragma CODE_SECTION(EPWM3_INT_ISR,"Critical_Section");
__interrupt void EPWM3_INT_ISR(void)    // EPWM-3
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT3.4
#pragma CODE_SECTION(EPWM4_INT_ISR,"Critical_Section");
__interrupt void EPWM4_INT_ISR(void)    // EPWM-4
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT3.5
#pragma CODE_SECTION(EPWM5_INT_ISR,"Critical_Section");
__interrupt void EPWM5_INT_ISR(void)    // EPWM-5
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT3.6
#pragma CODE_SECTION(EPWM6_INT_ISR,"Critical_Section");
__interrupt void EPWM6_INT_ISR(void)    // EPWM-6
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT3.7
#pragma CODE_SECTION(EPWM7_INT_ISR,"Critical_Section");
__interrupt void EPWM7_INT_ISR(void)    // EPWM-7
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT3.8
#pragma CODE_SECTION(EPWM8_INT_ISR,"Critical_Section");
__interrupt void EPWM8_INT_ISR(void)    // EPWM-8
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// -----------------------------------------------------------
// PIE Group 4 - MUXed into CPU INT4
// -----------------------------------------------------------

// INT 4.1
#pragma CODE_SECTION(ECAP1_INT_ISR,"Critical_Section");
__interrupt void ECAP1_INT_ISR(void)    // ECAP-1
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT4.2
#pragma CODE_SECTION(ECAP2_INT_ISR,"Critical_Section");
__interrupt void ECAP2_INT_ISR(void)    // ECAP-2
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT4.3
#pragma CODE_SECTION(ECAP3_INT_ISR,"Critical_Section");
__interrupt void ECAP3_INT_ISR(void)    // ECAP-3
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT4.4 - Reserved
// INT4.5 - Reserved
// INT4.6 - Reserved

// INT4.7
#pragma CODE_SECTION(HRCAP1_INT_ISR,"Critical_Section");
__interrupt void HRCAP1_INT_ISR(void)    // HRCAP-1
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT4.8
#pragma CODE_SECTION(HRCAP2_INT_ISR,"Critical_Section");
__interrupt void HRCAP2_INT_ISR(void)    // HRCAP-2
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP4;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// -----------------------------------------------------------
// PIE Group 5 - MUXed into CPU INT5
// -----------------------------------------------------------

// INT 5.1
#pragma CODE_SECTION(EQEP1_INT_ISR,"Critical_Section");
__interrupt void EQEP1_INT_ISR(void)    // EQEP-1
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT5.2
#pragma CODE_SECTION(EQEP2_INT_ISR,"Critical_Section");
__interrupt void EQEP2_INT_ISR(void)    // EQEP-2
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT5.3 - Reserved

// INT5.4
#pragma CODE_SECTION(HRCAP3_INT_ISR,"Critical_Section");
__interrupt void HRCAP3_INT_ISR(void)    // HRCAP-3
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT5.5
#pragma CODE_SECTION(HRCAP4_INT_ISR,"Critical_Section");
__interrupt void HRCAP4_INT_ISR(void)    // HRCAP-4
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT5.6 - Reserved
// INT5.7 - Reserved

// INT5.8
#pragma CODE_SECTION(USB0_INT_ISR,"Critical_Section");
__interrupt void USB0_INT_ISR(void)    // USB-0
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP5;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// -----------------------------------------------------------
// PIE Group 6 - MUXed into CPU INT6
// -----------------------------------------------------------

// INT6.1
#pragma CODE_SECTION(SPIRXINTA_ISR,"Critical_Section");
__interrupt void SPIRXINTA_ISR(void)    // SPI-A
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP6;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT6.2
#pragma CODE_SECTION(SPITXINTA_ISR,"Critical_Section");
__interrupt void SPITXINTA_ISR(void)     // SPI-A
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP6;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT6.3
#pragma CODE_SECTION(SPIRXINTB_ISR,"Critical_Section");
__interrupt void SPIRXINTB_ISR(void)    // SPI-B
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP6;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT6.4
#pragma CODE_SECTION(SPITXINTB_ISR,"Critical_Section");
__interrupt void SPITXINTB_ISR(void)     // SPI-B
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP6;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT6.5
#pragma CODE_SECTION(MRINTA_ISR,"Critical_Section");
__interrupt void MRINTA_ISR(void)     // McBSP-A
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP6;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT6.6
#pragma CODE_SECTION(MXINTA_ISR,"Critical_Section");
__interrupt void MXINTA_ISR(void)     // McBSP-A
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP6;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT6.7 - Reserved
// INT6.8 - Reserved

// -----------------------------------------------------------
// PIE Group 7 - MUXed into CPU INT7
// -----------------------------------------------------------

// INT7.1
#pragma CODE_SECTION(DINTCH1_ISR,"Critical_Section");
__interrupt void DINTCH1_ISR(void)     // DMA Channel 1
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP7;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT7.2
#pragma CODE_SECTION(DINTCH2_ISR,"Critical_Section");
__interrupt void DINTCH2_ISR(void)     // DMA Channel 2
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP7;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT7.3
#pragma CODE_SECTION(DINTCH3_ISR,"Critical_Section");
__interrupt void DINTCH3_ISR(void)     // DMA Channel 3
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP7;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT7.4
#pragma CODE_SECTION(DINTCH4_ISR,"Critical_Section");
__interrupt void DINTCH4_ISR(void)     // DMA Channel 4
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP7;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT7.5
#pragma CODE_SECTION(DINTCH5_ISR,"Critical_Section");
__interrupt void DINTCH5_ISR(void)     // DMA Channel 5
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP7;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT7.6
#pragma CODE_SECTION(DINTCH6_ISR,"Critical_Section");
__interrupt void DINTCH6_ISR(void)     // DMA Channel 6
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP7;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT7.7 - Reserved
// INT7.8 - Reserved

// -----------------------------------------------------------
// PIE Group 8 - MUXed into CPU INT8
// -----------------------------------------------------------

// INT8.1
#pragma CODE_SECTION(I2CINT1A_ISR,"Critical_Section");
__interrupt void I2CINT1A_ISR(void)     // I2C-A
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP8;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT8.2
#pragma CODE_SECTION(I2CINT2A_ISR,"Critical_Section");
__interrupt void I2CINT2A_ISR(void)     // I2C-A
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP8;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT8.3 - Reserved
// INT8.4 - Reserved
// INT8.5 - Reserved
// INT8.6 - Reserved
// INT8.7 - Reserved
// INT8.8 - Reserved

// -----------------------------------------------------------
// PIE Group 9 - MUXed into CPU INT9
// -----------------------------------------------------------

// INT9.1
#pragma CODE_SECTION(SCIRXINTA_ISR,"Critical_Section");
__interrupt void SCIRXINTA_ISR(void)     // SCI-A
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT9.2
#pragma CODE_SECTION(SCITXINTA_ISR,"Critical_Section");
__interrupt void SCITXINTA_ISR(void)     // SCI-A
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT9.3
#pragma CODE_SECTION(SCIRXINTB_ISR,"Critical_Section");
__interrupt void SCIRXINTB_ISR(void)     // SCI-B
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT9.4
#pragma CODE_SECTION(SCITXINTB_ISR,"Critical_Section");
__interrupt void SCITXINTB_ISR(void)     // SCI-B
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT9.5
#pragma CODE_SECTION(ECAN0INTA_ISR,"Critical_Section");
__interrupt void ECAN0INTA_ISR(void)     // ECAN-A
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT9.6
#pragma CODE_SECTION(ECAN1INTA_ISR,"Critical_Section");
__interrupt void ECAN1INTA_ISR(void)     // ECAN-A
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT9.7 - Reserved
// INT9.8 - Reserved

// -----------------------------------------------------------
// PIE Group 10 - MUXed into CPU INT10
// -----------------------------------------------------------

// INT10.1 - Reserved or ADCINT1_ISR
// INT10.2 - Reserved or ADCINT2_ISR

// INT10.3
#pragma CODE_SECTION(ADCINT3_ISR,"Critical_Section");
__interrupt void ADCINT3_ISR(void)    // ADC
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT10.4
#pragma CODE_SECTION(ADCINT4_ISR,"Critical_Section");
__interrupt void ADCINT4_ISR(void)    // ADC
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT10.5
#pragma CODE_SECTION(ADCINT5_ISR,"Critical_Section");
__interrupt void ADCINT5_ISR(void)    // ADC
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT10.6
#pragma CODE_SECTION(ADCINT6_ISR,"Critical_Section");
__interrupt void ADCINT6_ISR(void)    // ADC
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT10.7
#pragma CODE_SECTION(ADCINT7_ISR,"Critical_Section");
__interrupt void ADCINT7_ISR(void)    // ADC
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT10.8
#pragma CODE_SECTION(ADCINT8_ISR,"Critical_Section");
__interrupt void ADCINT8_ISR(void)    // ADC
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP10;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}


// -----------------------------------------------------------
// PIE Group 11 - MUXed into CPU INT11
// -----------------------------------------------------------

// INT11.1
#pragma CODE_SECTION(CLA1_INT1_ISR,"Critical_Section");
__interrupt void CLA1_INT1_ISR(void)   // MCLA
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP11;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code

 __asm ("      ESTOP0");
  for(;;);

}

// INT11.2
#pragma CODE_SECTION(CLA1_INT2_ISR,"Critical_Section");
__interrupt void CLA1_INT2_ISR(void)  // MCLA
{

  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP11;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code

 __asm("	  ESTOP0");
  for(;;);

}

// INT11.3
#pragma CODE_SECTION(CLA1_INT3_ISR,"Critical_Section");
__interrupt void CLA1_INT3_ISR(void)    // MCLA
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP11;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT11.4
#pragma CODE_SECTION(CLA1_INT4_ISR,"Critical_Section");
__interrupt void CLA1_INT4_ISR(void)    // MCLA
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP11;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT11.5
#pragma CODE_SECTION(CLA1_INT5_ISR,"Critical_Section");
__interrupt void CLA1_INT5_ISR(void)    // MCLA
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP11;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT11.6
#pragma CODE_SECTION(CLA1_INT6_ISR,"Critical_Section");
__interrupt void CLA1_INT6_ISR(void)    // MCLA
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP11;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT11.7
#pragma CODE_SECTION(CLA1_INT7_ISR,"Critical_Section");
__interrupt void CLA1_INT7_ISR(void)    // MCLA
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP11;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// INT11.8
#pragma CODE_SECTION(CLA1_INT8_ISR,"Critical_Section");
__interrupt void CLA1_INT8_ISR(void)    // MCLA
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP11;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);
}

// -----------------------------------------------------------
// PIE Group 12 - MUXed into CPU INT12
// -----------------------------------------------------------

// INT12.1
#pragma CODE_SECTION(XINT3_ISR,"Critical_Section");
__interrupt void XINT3_ISR(void)  // External interrupt 3
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT12.2 - Reserved
// INT12.3 - Reserved
// INT12.4 - Reserved
// INT12.5 - Reserved
// INT12.6 - Reserved

// INT12.7
#pragma CODE_SECTION(LVF_ISR,"Critical_Section");
__interrupt void LVF_ISR(void)  // Latched overflow
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

// INT12.8
#pragma CODE_SECTION(LUF_ISR,"Critical_Section");
__interrupt void LUF_ISR(void)  // Latched underflow
{
  // Insert ISR Code here

  // To receive more interrupts from this PIE group, acknowledge this interrupt
  // PieCtrlRegs.PIEACK.all = PIEACK_GROUP12;

  // Next two lines for debug only to halt the processor here
  // Remove after inserting ISR Code
 __asm ("      ESTOP0");
  for(;;);

}

//---------------------------------------------------------------------------
// Catch All Default ISRs:
//
#pragma CODE_SECTION(EMPTY_ISR,"Critical_Section");
__interrupt void EMPTY_ISR(void)  // Empty ISR - only does a return.
{

}

#pragma CODE_SECTION(PIE_RESERVED,"Critical_Section");
__interrupt void PIE_RESERVED(void)  // Reserved space.  For test.
{
 __asm ("      ESTOP0");
  for(;;);
}

#pragma CODE_SECTION(rsvd_ISR,"Critical_Section");
__interrupt void rsvd_ISR(void)      // For test
{
 __asm ("      ESTOP0");
  for(;;);
}

//===========================================================================
// End of file.
//===========================================================================

