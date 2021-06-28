/*
 * oi_DAC.c
 *
 *  Created on: Oct 15, 2020
 *      Author: enArka
 */
#include "F2837xS_device.h"
#include "F2837xS_Examples.h"
#include "oi_initPeripherals.h"
#include "oi_globalPrototypes.h"
#include "oi_genStructureDefs.h"
#include "oi_externVariables.h"

void oi_runDAC()
{
    EALLOW;
    DacaRegs.DACVALS.bit.DACVALS =  ((fec_state.theta_index_R)*1.0);//(fec_state.adc_buffer[VR_PHASE_SENSE_DSP].adc_result_regs);
    DacaRegs.DACVALA.bit.DACVALA = DacaRegs.DACVALS.bit.DACVALS;


    DacbRegs.DACVALS.bit.DACVALS = ((AC_inst_Values.V_R_PHASE)*1.0) + 2048;//(fec_state.adc_buffer[VR_PHASE_SENSE_DSP].adc_result_regs);
    DacbRegs.DACVALA.bit.DACVALA = DacbRegs.DACVALS.bit.DACVALS;

    EDIS;
}

