//###########################################################################
//
// FILE:   F2806x_GlobalPrototypes.h
//
// TITLE:  Global prototypes for F2806x Examples
//
//###########################################################################
// $TI Release: F2806x C/C++ Header Files and Peripheral Examples V130 $
// $Release Date: November 30, 2011 $
//###########################################################################

#ifndef F2806x_GLOBALPROTOTYPES_H
#define F2806x_GLOBALPROTOTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*---- shared global function prototypes -----------------------------------*/
extern void AdcOffsetSelfCal(void);
extern void AdcChanSelect(Uint16 ch_no);
extern Uint16 AdcConversion (void);

extern void InitPieCtrl(void);
extern void InitPieVectTable(void);
extern void InitSysCtrl(void);
extern void InitTzGpio(void);
extern void InitXIntrupt(void);
extern void InitPll(Uint16 pllcr, Uint16 clkindiv);
extern void InitPll2(Uint16 clksrc, Uint16 pllmult, Uint16 clkdiv);
extern void InitPeripheralClocks(void);
extern void EnableInterrupts(void);
extern void DSP28x_usDelay(Uint32 Count);

#define KickDog ServiceDog     // For compatiblity with previous versions
extern void ServiceDog(void);
extern void DisableDog(void);

extern Uint16 CsmUnlock(void);
extern void IntOsc1Sel (void);
extern void IntOsc2Sel (void);
extern void XtalOscSel (void);
extern void ExtOscSel (void);

extern int16 GetTemperatureC(int16 sensorSample); // returns temp in deg. C
extern int16 GetTemperatureK(int16 sensorSample); // returns temp in deg. K
extern void Osc1Comp(int16 sensorSample);
extern void Osc2Comp(int16 sensorSample);


// DSP28_DBGIER.asm
extern void SetDBGIER(Uint16 dbgier);

//                 CAUTION
// This function MUST be executed out of RAM. Executing it
// out of OTP/Flash will yield unpredictable results
extern void InitFlash(void);

//---------------------------------------------------------------------------
// External symbols created by the linker cmd file
// DSP28 examples will use these to relocate code from one LOAD location
// in Flash to a different RUN location in internal
// RAM
extern Uint16 RamfuncsLoadStart;
extern Uint16 RamfuncsLoadEnd;
extern Uint16 RamfuncsRunStart;
extern Uint16 RamfuncsLoadSize;

// IQMATH
extern Uint16 IQmath_loadstart;
extern Uint16 IQmath_loadend;
extern Uint16 IQmath_runstart;

// Function for copying sections from Run address to load address
extern void MemCopy(Uint16 *SourceAddr, Uint16* SourceEndAddr, Uint16* DestAddr);

//--------------------- User Defined  Global Function Prototypes --------------------------------------------------
		// Initialisation
extern void main2();
extern void ei_vSystemInit();
extern void ei_vPinMapping();
extern void ei_vInitPeripherals();
extern void ei_vInitState();
extern void ei_vRetrieveLastSettings();
extern void ei_vDeviceSpecificCanMailboxInit();

		// Interrupts
extern interrupt void ei_vEPWM6_underflow_ISR();
extern interrupt void ei_vSciaFifoRX_ISR();
extern interrupt void ei_vCpu_Timer0_ISR();

		// Scheduler Drivers
//extern Uint16 (*ei_uiEventDriver)();
extern Uint16 Event0();
extern Uint16 Event1();
extern Uint16 Event2();
extern Uint16 Event3();
extern Uint16 Event4();
extern Uint16 Event5();
extern Uint16 Event6();
extern Uint16 Event7();
extern Uint16 Event8();
extern Uint16 Event9();
//extern Uint16 Event10();

		// State
extern void ei_vCheckFaults();
extern void ei_vState();
extern void ei_vClearPsuStatusStructure();
extern void ei_vResetPsuToSafeState();

		// Ramfuncs for Booting
extern void ei_vDspBootMode();
extern void MyCallbackFunction();

		// Communication drivers
// I2C Drivers
extern Uint16 ei_uiI2CWrite(Uint16 Slave_address, Uint16 Start_address, Uint16 No_of_databytes, Uint16 Write_Array[]);
extern Uint16 ei_uiI2CRead(Uint16 Slave_address, Uint16 Start_address,Uint16 No_of_databytes, Uint16 Read_Array[]);

// SPI Drivers
extern Uint16 ei_uiSpiWrite(Uint32 ui_Device, Uint16 ui_NoOfDatabytes, unsigned char a_c_DataArray[]);
extern Uint16 ei_uiSpiRead(Uint16 *p_ui_Result);

		// AD7705 Drivers
extern void ei_vAD7705SoftwareReset();
extern Uint16 ei_vInitializeAD7705();
extern void ei_vSocAD7705(Uint16 Channel_no);
extern Uint16 ei_vReadAD7705Result(Uint16 Channel_no);
extern void ei_vAD7705LPF(Uint16 Channel_no);
extern void ei_vSwitchSpiClkPolarity(Uint16 Device_Type);
extern void ei_vExternalAdcOperation();

		// EEPROM Operations
extern void ei_vWriteLastSettingsEEPROM(Uint16 Cal_Mode);
extern void ei_vWriteOnTimeToArray(Uint16 *I2CArray);

		// Encoder and References
extern void ei_vSetReferences();
extern void ei_vSetOvpReference();
extern void ei_vSetVoltageReference();
extern void ei_vSetCurrentReference();
extern void ei_vAdjustOvp();
extern void ei_vVoltageEncoderOperation();
extern void ei_vCurrentEncoderOperation();

		// ADCApp.c
extern void ei_vInitAdc();

		// Display and Keyscan
extern void ei_vMax625_X_Y_I2C_initialization(Uint16 MAX_Init_Device_address);
extern void ei_vDisplayOperation();
extern void ei_vKeyProcessing();

		// Temperature Operations
extern void ei_vTempSensorInitialization();
extern void ei_vReadTemperature();
extern void ei_vTemperatureSenseEvent();

		// Remote Operations
extern void ei_vSciCommEvent();
extern void ei_vCanCommEvent();
extern void ei_vRetrieveLastSettings();
extern void ei_vSetVoltageCurrentForCalib(Uint16 Parameter_To_Be_Set);
extern void ei_vCheckAndSetModelNo();
extern void ei_vRecallLastPSUSettings();

		// String operations
extern void Strlower(Int8 *String);
extern void ei_vEnqueueFaultRegsInErrorQueue(Uint16 Fault_Flag, const Int8 *ErrorString);
extern void ei_vEnqueue_Non_Fault_Based_DDEs_In_ErrorQueue(const Int8 *ErrorString);
extern void ei_vEnqueueOutputMessageInTransmitBuffer(Int8 *MessageToBePushed);
extern void ei_vUartFifoTx();

		// TEST Code
extern void TEST_I2CWrite();

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif   // - end of F2806x_GLOBALPROTOTYPES_H

//===========================================================================
// End of file.
//===========================================================================
