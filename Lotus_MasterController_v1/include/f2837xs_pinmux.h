//*****************************************************************************
//
//  f2837xs_pinmux.h - Created using TI Pinmux 4.0.1543  on 1/6/2021 at 7:09:15 PM.
//
//*****************************************************************************
//
// Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the
//   distribution.
//
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************
//
//  These values will provide the functionality requested when written into
//  the registers for which the #defines are named.  For example, using the
//  C2000Ware device support header files, use the defines like in this
//  sample function:
//
//  void samplePinMuxFxn(void)
//  {
//      EALLOW;
//      //
//      // Write generated values to mux registers
//      //
//      GpioCtrlRegs.GPAMUX1.all  = GPAMUX1_VALUE;
//      GpioCtrlRegs.GPAMUX2.all  = GPAMUX2_VALUE;
//      GpioCtrlRegs.GPBMUX1.all  = GPBMUX1_VALUE;
//        . . .
//      EDIS;
//  }
//
//*****************************************************************************

//
// Port A mux register values
//
// Pin 93 (GPIO4) to CANTXA (mode 6)
// Pin 13 (GPIO21) to CANRXB (mode 3)
// Pin 12 (GPIO20) to CANTXB (mode 3)
// Pin 91 (GPIO2) to EPWM2A (mode 1)
// Pin 92 (GPIO3) to EPWM2B (mode 1)
// Pin 100 (GPIO10) to EPWM6A (mode 1)
// Pin 1 (GPIO11) to EPWM6B (mode 1)
// Pin 6 (GPIO15) to SCIRXDB (mode 2)
// Pin 5 (GPIO14) to SCITXDB (mode 2)
// Pin 4 (GPIO13) to SCIRXDC (mode 6)
// Pin 3 (GPIO12) to SCITXDC (mode 6)
// Pin 7 (GPIO16) to SPISIMOA (mode 1)
// Pin 8 (GPIO17) to SPISOMIA (mode 1)
// Pin 9 (GPIO18) to SPICLKA (mode 1)
// Pin 11 (GPIO19) to SPISTEA (mode 1)
#define GPAMUX1_MASK		0xfff003f0
#define GPAMUX2_MASK		0x00000fff
#define GPAMUX1_VALUE		0xaa500250
#define GPAMUX2_VALUE		0x00000f55
#define GPAGMUX1_VALUE		0x05000100
#define GPAGMUX2_VALUE		0x00000000

//
// Port B mux register values
//
// Pin 57 (GPIO62) to CANRXA (mode 6)
// Pin 51 (GPIO41) to GPIO41 (mode 0)
// Pin 52 (GPIO58) to GPIO58 (mode 0)
// Pin 53 (GPIO59) to GPIO59 (mode 0)
// Pin 54 (GPIO60) to GPIO60 (mode 0)
// Pin 56 (GPIO61) to GPIO61 (mode 0)
// Pin 58 (GPIO63) to GPIO63 (mode 0)
// Pin 74 (GPIO43) to SCIRXDA (mode 15)
// Pin 73 (GPIO42) to SCITXDA (mode 15)
#define GPBMUX1_MASK		0x00fc0000
#define GPBMUX2_MASK		0xfff00000
#define GPBMUX1_VALUE		0x00f00000
#define GPBMUX2_VALUE		0x20000000
#define GPBGMUX1_VALUE		0x00f00000
#define GPBGMUX2_VALUE		0x10000000

//
// Port C mux register values
//
// Pin 59 (GPIO64) to GPIO64 (mode 0)
// Pin 60 (GPIO65) to GPIO65 (mode 0)
// Pin 61 (GPIO66) to GPIO66 (mode 0)
// Pin 75 (GPIO69) to GPIO69 (mode 0)
// Pin 76 (GPIO70) to GPIO70 (mode 0)
// Pin 77 (GPIO71) to GPIO71 (mode 0)
// Pin 80 (GPIO72) to GPIO72 (mode 0)
// Pin 81 (GPIO73) to GPIO73 (mode 0)
// Pin 82 (GPIO78) to GPIO78 (mode 0)
// Pin 85 (GPIO84) to GPIO84 (mode 0)
// Pin 86 (GPIO85) to GPIO85 (mode 0)
// Pin 87 (GPIO86) to GPIO86 (mode 0)
// Pin 88 (GPIO87) to GPIO87 (mode 0)
// Pin 98 (GPIO91) to SDAA (mode 6)
// Pin 99 (GPIO92) to SCLA (mode 6)
#define GPCMUX1_MASK		0x300ffc3f
#define GPCMUX2_MASK		0x03c0ff00
#define GPCMUX1_VALUE		0x00000000
#define GPCMUX2_VALUE		0x02800000
#define GPCGMUX1_VALUE		0x00000000
#define GPCGMUX2_VALUE		0x01400000

//
// Port D mux register values
//
// Pin 14 (GPIO99) to GPIO99 (mode 0)
#define GPDMUX1_MASK		0x000000c0
#define GPDMUX1_VALUE		0x00000000
#define GPDGMUX1_VALUE		0x00000000

//
// Port B analog mode register values
//
// Pin 74 (GPIO43) to SCIRXDA (mode 15)
// Pin 73 (GPIO42) to SCITXDA (mode 15)
#define GPBAMSEL_MASK		0x00000c00
#define GPBAMSEL_VALUE		0x00000000

//*****************************************************************************
//
// Function prototype for function to write values above into their
// corresponding registers. This function is found in f2837xs_pinmux.c. Its use
// is completely optional.
//
//*****************************************************************************
extern void GPIO_setPinMuxConfig(void);
