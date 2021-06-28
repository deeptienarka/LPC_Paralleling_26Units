#include "F2837xS_device.h"
#include "F2837xS_Examples.h"
#include "F28x_Project.h"
#include "oi_globalPrototypes.h"
#include "oi_scheduler.h"
#include "oi_genStructureDefs.h"
#include "oi_externVariables.h"


/**
 * main.c
 */
int main(void)
{
    oi_vSystemInit();

    while (1)
    {

        if (Global_Flags.fg_Scheduled_General_Event_Flag == 1)
        {
            Global_Flags.fg_Scheduled_General_Event_Flag = 0;

            // Function pointer to Call the events
            if (Event_Array[ui_Event_Initiated].p_fn_EventDriver() == 0)
            {
                // Error Handler
            }
        }

    }

}
