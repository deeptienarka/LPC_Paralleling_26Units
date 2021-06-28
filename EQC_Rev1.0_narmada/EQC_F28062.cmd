/*
//###########################################################################
//
// FILE:    F28062.cmd
//
// TITLE:   Linker Command File For F28062 Device
//
//###########################################################################
// $TI Release: F2806x C/C++ Header Files and Peripheral Examples V135 $ 
// $Release Date: Sep 8, 2012 $ 
//###########################################################################
*/

/* ======================================================
// For Code Composer Studio V2.2 and later
// ---------------------------------------
// In addition to this memory linker command file,
// add the header linker command file directly to the project.
// The header linker command file is required to link the
// peripheral structures to the proper locations within
// the memory map.
//
// The header linker files are found in <base>\F2806x_headers\cmd
//
// For BIOS applications add:      F2806x_Headers_BIOS.cmd
// For nonBIOS applications add:   F2806x_Headers_nonBIOS.cmd
========================================================= */

/* ======================================================
// For Code Composer Studio prior to V2.2
// --------------------------------------
// 1) Use one of the following -l statements to include the
// header linker command file in the project. The header linker
// file is required to link the peripheral structures to the proper
// locations within the memory map                                    */

/* Uncomment this line to include file only for non-BIOS applications */
/* -l F2806x_Headers_nonBIOS.cmd */

/* Uncomment this line to include file only for BIOS applications */
/* -l F2806x_Headers_BIOS.cmd */

/* 2) In your project add the path to <base>\F2806x_headers\cmd to the
   library search path under project->build options, linker tab,
   library search path (-i).
/*========================================================= */

/* Define the memory block start/length for the F2806x
   PAGE 0 will be used to organize program sections
   PAGE 1 will be used to organize data sections

   Notes:
         Memory blocks on F28062 are uniform (ie same
         physical memory) in both PAGE 0 and PAGE 1.
         That is the same memory region should not be
         defined for both PAGE 0 and PAGE 1.
         Doing so will result in corruption of program
         and/or data.

         Contiguous SARAM memory blocks can be combined
         if required to create a larger memory block.
*/

MEMORY
{
PAGE 0 :   /* Program Memory */
           /* Memory (RAM/FLASH/OTP) blocks can be moved to PAGE1 for data allocation */
   RAML0       : origin = 0x008000, length = 0x000800     /* on-chip RAM block L0 */
   RAML1       : origin = 0x008800, length = 0x000400     /* on-chip RAM block L1 */
   OTP         : origin = 0x3D7800, length = 0x000400     /* on-chip OTP */

   FLASHD      : origin = 0x3E8000, length = 0x00DFE2, fill = 0xFFFF   /* on-chip FLASH */
   APP_BEGIN   : origin = 0x3F5FE2, length = 0x00001E, fill = 0xFFFF   /* on-chip FLASH */
   FLASHA      : origin = 0x3F6000, length = 0x001E80, fill = 0xFFFF   /* on-chip FLASH */

   PIE_VECT_INITIAL      : origin = 0x3F7E80, length = 0x000100, fill = 0xFFFF   /* on-chip FLASH */

   CSM_RSVD    : origin = 0x3F7F80, length = 0x000076     /* Part of FLASHA.  Program with all 0x0000 when CSM is in use. */
   BEGIN       : origin = 0x3F7FF6, length = 0x000002     /* Part of FLASHA.  Used for "boot to Flash" bootloader mode. */
   CSM_PWL_P0  : origin = 0x3F7FF8, length = 0x000008     /* Part of FLASHA.  CSM password locations in FLASHA */

   FPUTABLES   : origin = 0x3FD860, length = 0x0006A0	  /* FPU Tables in Boot ROM */
   IQTABLES    : origin = 0x3FDF00, length = 0x000B50     /* IQ Math Tables in Boot ROM */
   IQTABLES2   : origin = 0x3FEA50, length = 0x00008C     /* IQ Math Tables in Boot ROM */
   IQTABLES3   : origin = 0x3FEADC, length = 0x0000AA	  /* IQ Math Tables in Boot ROM */

   ROM         : origin = 0x3FF3B0, length = 0x000C10     /* Boot ROM */
   RESET       : origin = 0x3FFFC0, length = 0x000002     /* part of boot ROM  */
   VECTORS     : origin = 0x3FFFC2, length = 0x00003E     /* part of boot ROM  */

PAGE 1 :   /* Data Memory */
           /* Memory (RAM/FLASH/OTP) blocks can be moved to PAGE0 for program allocation */
           /* Registers remain on PAGE1                                                  */

   BOOT_RSVD   : origin = 0x000000, length = 0x000050     /* Part of M0, BOOT rom will use this for stack */
   RAMM0       : origin = 0x000050, length = 0x0003B0     /* on-chip RAM block M0 */
   RAMM1       : origin = 0x000400, length = 0x000400     /* on-chip RAM block M1 */
   RAML2       : origin = 0x008C00, length = 0x000900     /* on-chip RAM block L2 */
   BOOT_SYMBOLS : origin = 0x009500, length = 0x000100	  /* on-chip RAM block L3 */
   EBSS		   : origin = 0x009600, length = 0x000A00	  /* on-chip RAM block L3 */
   RAML4_0     : origin = 0x00A000, length = 0x001000     /* on-chip RAM block L4 */
   RAML4_1     : origin = 0x00B000, length = 0x001000     /* on-chip RAM block L4 */
   RAML5       : origin = 0x00C000, length = 0x002000     /* on-chip RAM block L5_0 */
   USB_RAM     : origin = 0x040000, length = 0x000800     /* USB RAM		  */
}

/* Allocate sections to memory blocks.
   Note:
         codestart user defined section in DSP28_CodeStartBranch.asm used to redirect code
                   execution when booting to flash
         ramfuncs  user defined section to store functions that will be copied from Flash into RAM
*/


SECTIONS
{

   /* Allocate program areas: */
   .cinit              : > FLASHA,     PAGE = 0
   .pinit              : > FLASHA,     PAGE = 0
   .text               : > FLASHD,     PAGE = 0
   codestart           : > BEGIN,      PAGE = 0
   ramfuncs            : LOAD = FLASHA,
                         RUN = RAML0,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
						 LOAD_SIZE(_RamfuncsLoadSize),
                         PAGE = 0

   csmpasswds          : > CSM_PWL_P0, PAGE = 0
   csm_rsvd            : > CSM_RSVD,   PAGE = 0

   /* Allocate uninitalized data sections: */
   .stack              : > RAML2,      PAGE = 1
   .ebss               : > EBSS,	   PAGE = 1
   .esysmem            : > EBSS,      PAGE = 1

   /* Initalized sections to go in Flash */
   /* For SDFlash to program these, they must be allocated to page 0 */
   .econst             : > FLASHD,     PAGE = 0
   .switch             : > FLASHD,     PAGE = 0

   /* Allocate IQ math areas: */
   IQmath            : LOAD = FLASHD,   PAGE = 0
                       RUN = RAMM1,     PAGE = 1
                       LOAD_START(_IQmath_loadstart),
                       LOAD_END(_IQmath_loadend),
                       RUN_START(_IQmath_runstart)
   IQmathTables        : > IQTABLES,   PAGE = 0, TYPE = NOLOAD
   
   /* Allocate FPU math areas: */
   FPUmathTables       : > FPUTABLES,  PAGE = 0, TYPE = NOLOAD
   
   Cinit_App:
   {
   		-lrts2800_fpu32.lib(.cinit)
   }	LOAD = FLASHD, PAGE = 0

   DMARAML5	           : > RAML5,      PAGE = 1

   /* Boot Section */
   BlockTransferBuffer1: > RAML4_0	   	PAGE = 1
   BlockTransferBuffer2: > RAML4_1	   	PAGE = 1
   Boot_Symbols	   	   : > BOOT_SYMBOLS	PAGE = 1
   Boot:
   {
   		-lCustomBoot.lib(.text)
   }	LOAD = FLASHA, PAGE = 0
   BootLib_Symbols:
   {
		-lCustomBoot.lib(.ebss)
   }	LOAD = BOOT_SYMBOLS, PAGE = 1

   Critical_Section	   : > FLASHA	   PAGE = 0
   Pie_Vector_Initial  : > PIE_VECT_INITIAL	   PAGE = 0

   /*App Section*/
   Application_Code_main   : > APP_BEGIN   PAGE = 0

  /* Uncomment the section below if calling the IQNexp() or IQexp()
      functions from the IQMath.lib library in order to utilize the
      relevant IQ Math table in Boot ROM (This saves space and Boot ROM
      is 1 wait-state). If this section is not uncommented, IQmathTables2
      will be loaded into other memory (SARAM, Flash, etc.) and will take
      up space, but 0 wait-state is possible.
   */
   /*
   IQmathTables2    : > IQTABLES2, PAGE = 0, TYPE = NOLOAD
   {

              IQmath.lib<IQNexpTable.obj> (IQmathTablesRam)

   }
   */
    /* Uncomment the section below if calling the IQNasin() or IQasin()
       functions from the IQMath.lib library in order to utilize the
       relevant IQ Math table in Boot ROM (This saves space and Boot ROM
       is 1 wait-state). If this section is not uncommented, IQmathTables2
       will be loaded into other memory (SARAM, Flash, etc.) and will take
       up space, but 0 wait-state is possible.
    */
    /*
    IQmathTables3    : > IQTABLES3, PAGE = 0, TYPE = NOLOAD
    {

               IQmath.lib<IQNasinTable.obj> (IQmathTablesRam)

    }
    */

   /* .reset is a standard section used by the compiler.  It contains the */
   /* the address of the start of _c_int00 for C Code.   /*
   /* When using the boot ROM this section and the CPU vector */
   /* table is not needed.  Thus the default type is set here to  */
   /* DSECT  */
   .reset              : > RESET,      PAGE = 0, TYPE = DSECT
   vectors             : > VECTORS,    PAGE = 0, TYPE = DSECT

}

/*
//===========================================================================
// End of file.
//===========================================================================
*/


