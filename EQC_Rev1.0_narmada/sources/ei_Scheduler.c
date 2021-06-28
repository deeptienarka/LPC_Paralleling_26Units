//#################################################################################################################
//# 							enArka Instruments proprietary
//# File: ei_Scheduler.c
//# Copyright (c) 2013 by enArka Instruments Pvt. Ltd.
//# All Rights Reserved
//#
//#################################################################################################################

//######################## PROJECT CHECKLIST ######################################################################
// Every time  a new project is created using piccolo 6x processor the following steps must be completed.
//
// 0. Other than proper device selection while creating new project, ensure that the proper runtime library is linked when 
//	  Piccolo 6x uses "rts2800ml_fpu32.lib"
// 1. Add the "includes" and  "sources" folder from "To be included in every project" in "PiccoloF2806xSupportFiles"
// 2. Right click on the project in the workspace -> Click on Build Properties -> Link the "includes" Folder in the
//    workspace in "Include Options"
// 3. Add the following libraries to the project in the workspace:
//	  1. IQmath_fpu32_v15c.lib
//	  2. IQmath_fpu32_v160.lib
//	  3. rts2800_fpu32_fast_supplement.lib
// 4. Right click on the project in the workspace -> Click on Build Properties -> Link all the .lib files added in
//	  "File Search Path" option.
// 5. Go to Build Properties -> CCS Build -> Link Order and add both "rts2800_fpu32.lib" and "rts2800_fpu32_fast_supplement.lib"
//	  Ensure that the fast_supplement library is higher in the order than rts2800_fpu32.lib. Ensure that 2 options (resolve backward references, Search in priority) is checked.
// 6. Select the appropriate target using TARGET in F2806x_Device.h. We are using DSP28_28062PZ
// 7. In F2806x_Examples.h, using #defines select DSP28_PLLCR, DSP28_DIVSEL, CPU_RATE at 16,2 and 11.111L respectively.
//	  (We need to operate at 90MHz)
// 8. Enable Crystal Clocking in F2806x_SysCtrl.c by calling function XtalOscSel() and commenting InstOsc1Sel()
// 9. Ensure that the clocks to all the used peripherals used in the project have been enabled and clocks for unused 
//	  peripherals have been disabled.
// 10. Ensure that if XCLKOUT is not required in the application then it is turned off. Here it is not used.
//	   If used in the application we need to enable the particular GPIO18 for XCLKOUT operation and set the XCLKOUT
//	   scaling in XCLK register.
// 11. Ensure correct value in LOSPCP. We need 22.5M on the LSPCLK. Write value 2 in the LOSPCP register to achieve that.
// 12. Ensure that code is linked in F28062.cmd.
// 13. Ensure that F2806x_Headers_nonBIOS.cmd is added in the project.
// 14. Ensure that the CCXML file has been generated correctly.
//     This project will use XDS100v2F28062.ccxml
//######################################### INCLUDE FILES #########################################################
#include "DSP28x_Project.h"
#include "enArka_Common_headers.h"
#include "GlobalVariables.h"
#include "Revision.h"
//#################################################################################################################
									/*------- Global Prototypes ----------*/
//Uint16 Event0();
//Uint16 Event1();
//Uint16 Event2();
//Uint16 Event3();
//Uint16 Event4();
//Uint16 Event5();
//Uint16 Event6();
//Uint16 Event7();
//Uint16 Event8();
//Uint16 Event9();

//################################# Application after Booting process #############################################
#pragma CODE_SECTION(main2,"Application_Code_main");
void main2(void)
{
	ei_vSystemInit();	// This function is located in SystemInitFlashDSPBrd.c

	while(1)
	{
		if(Global_Flags.fg_Scheduled_Event_Flag == TRUE)
		{
			Global_Flags.fg_Scheduled_Event_Flag = FALSE;

			// Function pointer to Call the events
			if(Event_Array[ui_Event_Initiated].p_fn_EventDriver() == FAILURE)
			{
				// Error Handler
			}
		}
		else
		{
			// Background task manager goes here
		}
	}
}

//----------------------------------Event Drivers------------------------------------------------//
Uint16 Event0()
{
	asm("	nop");
	return SUCCESS;
}
//------------------------------------------------------------------------------------------------//
Uint16 Event1()
{
	ei_vCheckFaults();
	ei_vState();
	return SUCCESS;
}
//------------------------------------------------------------------------------------------------//
Uint16 Event2()
{
	ei_vDisplayOperation();
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
Uint16 Event3()
{
	if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)	// If Product calibrated
	{
		if(ANY_FAULT_OTHER_THAN_OUT_FAULT == NONE)
		{
			ei_vVoltageEncoderOperation();
			ei_vCurrentEncoderOperation();
		}

		// No fault checking for setting references. It is important that the below function be called
		// only in this event. The Encoder variables will contain the latest value only after the above 2 functions are called.

		// In case of a fault (Non OUT Fault) though, we see that only the above 2 functions will not be called
		ei_vSetReferences();
	}
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
Uint16 Event4()
{
	if(Product_Info.ui_Calibration_Status != FACTORY_DEFAULT)	// If Product calibrated
	{
		ei_vKeyProcessing();
	}
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
Uint16 Event5()
{
//	ei_vAdjustOvp();
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
Uint16 Event6()
{
	ei_vExternalAdcOperation();
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
Uint16 Event7()
{
	ei_vSciCommEvent();
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
Uint16 Event8()
{
	ei_vCanCommEvent();
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
Uint16 Event9()
{
//	ei_vTemperatureSenseEvent();
	return TRUE;
}
