//###########################################################################
//
// FILE:    Revision.h
//
// TITLE:   Revision Control for the entire Project
//
//###########################################################################
#ifndef REVISION_H_
#define REVISION_H_

// Project Start Date: 05-04-2013
// Project Start Revision: "EQC Rev:1U:1.0"
Int8 const * const SOFTWARE_VERSION = "EQC Rev:1.6";	// Please update the revision number in SOFTWARE_VERSION_IN_CAN_FORMAT macro located in eCan_AppDefs.h
Int8 const * const BOOT_KEY = "EQC REV:1.X\r";
Int8 const * const IEEE_LAN_SOFTWARE_REVISION = "EQC Rev:0.0";
//###########################################################################
//
// Ver  | dd-mmm-yyyy |  Who  | Description of changes
// =====|=============|=======|==============================================
//  1.0 | 12-04-2013  | V.S   | Project Started
//		|			  |		  | 1. Pin Mapping completed
//		|			  |		  | 2. Peripheral Initialisation completed. CAN and Uart peripheral assignment statement commented out.
//		|			  |		  | 3. Interrupt Initialisation completed.
//		|			  |		  | 4. Scheduler and 1 minute ISR defined and an empty 100ms Event1 created and verified.
//		|			  |		  | 5. Local operation of the PSU completed and all the necessary code for enabling that completed
//		|			  |		  |    and tested.
//______|_____________|_______|______________________________________________
//	1.1	| 15-05-2103  |	V.S	  | 1. Ovp Ref set to 100% when Initialising the PWM peripheral.
//		|			  |		  | 2. #if defs used to distinguish between PSU and EQC.
//		|			  |		  | 3. #if defs used to distinguish between PSU and EQC.
//		|			  |		  | 4. Changes from the Latest AUM Revision for OVP/UVL limits incorporated and #defs added.
//		|			  |		  | 5. Read Model Specs, Retrieve Last settings and Write Last settings implemented.
//______|_____________|_______|______________________________________________
// 1.2	| 28-5-213    | V.S   | 1. Partial modification of the linker (no new sections created but only FLASH Memory
//		|			  |		  | 2. Changes made to F2806x_Cla_typedefs.h and F2806x_Device.h typedefs to accommodate char data type typedefs. Refer to git for exact changes.
//		|			  |		  | 3. High time for EPWM4 and 5 changed to 2.51uS from 1uS
//		|			  |		  | 4. SCPI brought in completely and key commands tested.
//______|_____________|_______|______________________________________________
// 1.3	| 31-05-2013  | V.S   | 1. Bugfix: PSU now cannot be brought from LOCAL LOCKOUT to LOCAL. the "if" condition while processing the REM key has been changed.
//		|			  |		  | 2. Boot Code completed.
//______|_____________|_______|______________________________________________
// 1.4	| 26-06-2013  | V.S   | 1. Input Qualification QUALPRD value specified for the GPBCTRL for all sets of GPIO.
// 		| 			  |		  | 2. CAN completed.
// 		| 			  |		  | 3. Revision query in CAN implemented. Additional variables to control that added. Whenever we make a revision update
// 		| 			  |		  |    the corresponding CAN variable (SOFTWARE_VERSION_IN_CAN_FORMAT) must also be changed with the SCPI string.
// 		| 			  |		  | 4. Bug in Date_of_Can_Calibration query removed. Earlier it used to reply in the Register Query message Id.
//______|_____________|_______|______________________________________________
// 1.5	| 10-11-2013  | V.S   | 1. Pins changed in hardware/Software: CS_ADC and OTP_FAULT swapped (Now: CS_ADC: GPIO10 and OTP_FAULT: GPIO57)
// 		| 			  |		  | 2. The above change resulted in a change in SPI Drivers. Now the first argument passed is the value that controls the Gpio register straight away.
// 		| 			  |		  | 3. ei_AdcApp.c added and ADC configuration done. A new ADC based temperature sensor added in hardware.
// 		| 			  |		  | 4. ei_OvpAdjust function and event added. But for now the function has been commented out inside the driver for sake of accuracy
// 		| 			  |		  | 5. I2C based Temperature sensor removed. Thus all I2C temperature sensor related functions removed.
// 		| 			  |		  | 6. ReadTemperature to be modified from I2C based code to ADC based code.
// 		| 			  |		  | 7. SCPI query of Rated/Specified parameters displays 3 digits after decimal point for a clean rounding and avoiding IQmath inaccuracies.
// 		| 			  |		  | 8. OVP Bug: Full scale OVP used to go to 2.5V. This was because "PWM_COUNTS_CORRESPONDING_TO_OVP_RATED_VALUE" was not changed according to new TBPRD value. It
// 		| 			  |		  |    has been solved now.
// 		| 			  |		  | 9. Bugfix: When output switched on from CAN/RS485 PSU always goes to NON_LATCHED_REMOTE. This bug has now been solved by making changes in
//		|             | 	  |    ei_vState() in ARMED_POWER_STATE
//		|             | 	  | 10. TRANS_ENABLE pin set to 0 initially while assigning GPIOs. Earlier it was high and went low only after the dat got transmitted completely the first time.
//		|             | 	  |     This can cause communication problems when new devices are added on the bus.
//		|             | 	  |
//		|             | 	  |    ::::BOOT CODE CHANGE::::
//		|             | 	  | 1. Writing Boot command received in ei_vDspBootMode function removed. Instead this operation is performed in the appcode itself on
//		|             | 	  |    receiving the BOOT COMMAND. This is because any time an error in reading the EEPROM while booting happens,it gets overwritten by
//		|             | 	  |    by BOOT COMMAND. This makes debugging difficult.
//		|             | 	  | 2. Erroneous reads from FLASH_PROG_STATUS while booting are copied to a new location in EEPROM at FLASH_PROG_STATUS_ILLEGAL_READ_ADDRESS
//		|             | 	  |    Anything other than 0xFF, 0xAA and 0xCC constitute erroneous reads.
//		|             | 	  | 3. Any error that can occur during Firmware Flashing through RS485 is copied to a new location called FLASH_PROG_ERROR_ADDRESS.
//		|             | 	  |	4. Delay function added in boot code before reading EEPROM.
// 		| 			  |		  | 5. errno.obj and strtok.obj added to CustomBoot.lib and they are now linked in specific location
// 		| 			  |		  | 6. The .ebss of CustomBoot.lib is also linked in its own separate section along with boot symbols
// 		| 			  |		  | 7. TRANSENABLE is now toggled in Boot Code also.
//		|             | 	  | 8. IQMath Taken out from FLASHA and linked in FLASHD. This is because IQ functions are used only in application code. Hence
//		|             | 	  |    they must be linked only in Application space. Linking them in resident program section can cause the resident program checksum
//		|             | 	  |    to change under your nose. (This is because any new IQfunctions added/removed in code will change this section). Also the IQ math
//		|             | 	  |    is now loaded in FLASHD but run from RAMM1. This will increase speed of execution. NEEDS TO BE VERIFIED.
// 		| 			  |		  | 9. RAML3 split into BOOT_SYMBOLS (size: 0x100) and EBSS sections (size: 0xA00).
// 		| 			  |		  | 10. .esysmem linked along with .ebss in EBSS section
// 		| 			  |		  | 11. .stack changed to 0x800 (earlier 0x300) and linked in RAML2
// 		| 			  |		  |    :::: NEW BOOT CODE Checksum: 0xF378  ::::
// 		| 			  |		  |
// 		| 			  |		  | 9. A Custom Model (450V-22.3A) created with Model no:254
// 		| 			  |		  | 10. Max overall rated current for EQC product line changed to 700A (MAX_RATED_CURRENT_FOR_EQC_MODEL_TIMES_10 macro).
// 		| 			  |		  | 11. Added a new custom model 450V-23A. (Model Number assigned is 254)
// 		| 			  |		  | 12. DISABLE_UART_TRANSMISSION line uncommented in ei_vUartFifoTx() because we want want to Disable Transenable once a message
// 		| 			  |		  |     has been completely transmitted over RS485. (This maintains data integrity on the RS485 bus. Only one driver must be enabled
// 		| 			  |		  |     at a time in the bus.)
// 		| 			  |		  | 13. Checksum_Operation_Result = 1 Statement removed from ReadModelSpecs(). This was a piece of test code that was not removed from final code.
//		| 			  | 	  |    Leaving this statement uncommented will cause the code to miss EEPROM corruption errors.
// 		| 			  |		  |
//		|             | 	  |    ::::BOOT CODE CHANGE::::
// 		| 			  |		  | 1. errno.obj and strtok.obj removed from CustomBoot.lib This is because theese rts functions are generated by application code and must not
// 		| 			  |		  |    reside in resident program section.
// 		| 			  |		  | 2. A new section called Cinit_App created. This section contains the .cinit for rts2800ml_fpu32.lib . This section is linked in FLASHD along-
// 		| 			  |		  |    with application code. The .cinit section for the resident program is stored in FLASHA only as before.
// 		| 			  |		  |
// 		| 			  |		  |    :::: NEW BOOT CODE Checksum: 0x8811  ::::
// 		| 			  |		  |
// 		| 			  |		  | 14. The bug that caused a flickering effect on display whenever AUT/SAF or UFP/LFP toggles was solved by modifying the if condition for
// 		| 			  |		  |		not displaying the OUT fault in ei_vDisplayFault function.
// 		| 			  |		  | 15. Bugs during calibration solved:
// 		| 			  |		  | 			MAX_RATED_VOLTAGE_FOR_EQC_MODEL_TIMES_10 changed to MAX_RATED_VOLTAGE_FOR_EQC_MODEL_TIMES_100 alongwith value
// 		| 			  |		  |				MAX_RATED_CURRENT_FOR_EQC_MODEL_TIMES_10 changed to MAX_RATED_CURRENT_FOR_EQC_MODEL_TIMES_100 alongwith value
// 		| 			  |		  |
// 		| 			  |		  | 16. Major bug during calibration identified and solved: Ovp_Pwm_Gain was calculated in ei_vCheckAndSetModelNo() using PWM_COUNTS_CORRESPONDING_TO_RATED_VALUE
//		| 			  | 	  |     Instead it should be done using PWM_COUNTS_CORRESPONDING_TO_OVP_RATED_VALUE. The earlier bug will lead to OVP fault whenever we Set VOLTAGE_HIGH during calibration
// 		| 			  |		  |		because it results in Ovp_Pwm_Gain to a lower value than it needs to be.
//______|_____________|_______|______________________________________________
// 1.6	| In Progress | V.S   | 1. Added the 400V_25A model and made necessary changes to the code. The following functions were changed:
// 		| 			  |		  |	   1. ei_vReadModelSpecs()
// 		| 			  |		  |	   2. ei_vCheckAndSetModelNo()
// 		| 			  |		  |

#endif /*REVISION_H_*/
