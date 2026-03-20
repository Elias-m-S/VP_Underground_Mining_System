/******************************************************************************
 * @file AppTasks.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation File for the application tasks
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "AppTasks.h"

#include "Application.h"
#include "SensorHandler.h"
#include "LEDHandler.h"
#include "DisplayModule.h"
#include "StackMonitor.h"

/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/
static uint8_t s_stackFaultLatched = 0U;


/***** PUBLIC FUNCTIONS ******************************************************/

void taskApp1ms(void)
{
    /* Refresh 7-seg multiplexing */
    (void)displayCycle();
}

void taskApp10ms(void)
{
    /* Cycle the Sensor Handler (Input) */
    sensorHandlerCycle();
}

void taskApp50ms(void)
{
    /* Run the Application State Machine */
    applicationRun();

    /* Cycle the LED Handler (Output) */
    ledHandlerCycle();
}

void taskApp250ms(void)
{
    stackMonitorCheck();

    if (stackMonitorIsCorrupted())
    {
        if (s_stackFaultLatched == 0U)
        {
            if (applicationSendEvent(EVT_ID_STACK_CORRUPTION) >= 0)
            {
                s_stackFaultLatched = 1U;
            }
        }
    }
    else
    {
        s_stackFaultLatched = 0U;
    }
}

void taskAppInitialize(void)
{
    sensorHandlerInitialize();
    ledHandlerInitialize();
    applicationInitialize();
    stackMonitorInitialize();

    s_stackFaultLatched = 0U;
}

/***** PRIVATE FUNCTIONS *****************************************************/
