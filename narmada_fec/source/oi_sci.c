/*
 * oi_sci.c
 *
 *  Created on: 14-Dec-2020
 *      Author: niles
 */


#include "F2837xS_device.h"
#include "F2837xS_Examples.h"
#include "oi_initPeripherals.h"
#include "oi_globalPrototypes.h"
#include "oi_genStructureDefs.h"
#include "device.h"
#include "driverlib.h"
#include "oi_externVariables.h"

void oi_sciEvent(void)
{

    EALLOW;

    onStatus = fec_state.healthStatus;

    State_timers.timer1++;

    if (State_timers.timer1 > 10)
    {

        if (sci_cnt == 10)
        {
            ScibRegs.SCITXBUF.bit.TXDT = 0xFF;
        }
        else if (sci_cnt == 20)
        {
            ScibRegs.SCITXBUF.bit.TXDT = faults_l.all;
        }
        else if (sci_cnt == 30)
        {
            ScibRegs.SCITXBUF.bit.TXDT = faults_h.all;
        }
        else if (sci_cnt == 40)
        {
            ScibRegs.SCITXBUF.bit.TXDT = onStatus;

            sci_cnt = 0;
        }

        sci_cnt++;

        State_timers.timer1 = 0;
    }

    // Receiving Section of SCI.

    receivedChar = ScibRegs.SCIRXBUF.bit.SAR;

    if(receivedChar == 0xAA)
    {
        onCommand = 1;
    }
    else if(receivedChar == 0x55)
    {
        onCommand = 0;
    }

    EDIS;
}


void oi_sciInitGpio()
{
    EALLOW;
    //
    // Configuration for the SCI Rx pin.
    //
    GPIO_setMasterCore(DEVICE_GPIO_PIN_SCIRXDA, GPIO_CORE_CPU1);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCIRXDA);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCIRXDA, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCIRXDA, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCIRXDA, GPIO_QUAL_ASYNC);

    //
    // Configuration for the SCI Tx pin.
    //
    GPIO_setMasterCore(DEVICE_GPIO_PIN_SCITXDA, GPIO_CORE_CPU1);
    GPIO_setPinConfig(DEVICE_GPIO_CFG_SCITXDA);
    GPIO_setDirectionMode(DEVICE_GPIO_PIN_SCITXDA, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(DEVICE_GPIO_PIN_SCITXDA, GPIO_PIN_TYPE_STD);
    GPIO_setQualificationMode(DEVICE_GPIO_PIN_SCITXDA, GPIO_QUAL_ASYNC);

    EDIS;
}

int intToStr1(int x, unsigned char str[], int d)
{
    int i = 0;
    while (x)
    {
        if (x < 1)
        {
            str[i] = '0';
            i = i + 1;
            x = x / 10;
        }
        else
        {
            str[i] = x % 10 + '0';
            i = i + 1;
            x = x / 10;
        }
    }

    while (i < d)
    {
        str[i++] = '0';
    }
    reverse(str, i);
    str[i] = '\0';

    return i;
}

void reverse(unsigned char *str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

