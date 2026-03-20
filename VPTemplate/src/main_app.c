/******************************************************************************
 * @file main_app.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Main file for the VP Template project
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"
#include "System.h"

#include "HardwareConfig.h"

#include "Util/Global.h"
#include "Util/Log/printf.h"
#include "Util/Log/LogOutput.h"

#include "UARTModule.h"
#include "ButtonModule.h"
#include "LEDModule.h"
#include "DisplayModule.h"
#include "ADCModule.h"
#include "TimerModule.h"
#include "Scheduler.h"
#include "AppTasks.h"

#include "stm32g4xx_hal.h"

#include "GlobalObjects.h"


/***** PRIVATE CONSTANTS *****************************************************/
const char signature[] __attribute__((section(".signature"), used)) = "UMMS";

/***** PRIVATE MACROS ********************************************************/
#define APP_VECTOR_BASE_ADDR         0x08010200U


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/
static int32_t initializePeripherals();


/***** PRIVATE VARIABLES *****************************************************/
static Scheduler gScheduler;            // Global Scheduler instance


/***** PUBLIC FUNCTIONS ******************************************************/


/**
 * @brief Main function of System
 */
int main(void)
{

	__enable_irq();
	__HAL_RCC_AHB1_FORCE_RESET();
	__HAL_RCC_AHB1_RELEASE_RESET();
	HAL_DeInit();


	SCB->VTOR = APP_VECTOR_BASE_ADDR;
	    __DSB();
	    __ISB();
	// Initialize the HAL (calls SystemInit internally - must come first)
    HAL_Init();


    // Initialize the System Clock
    SystemClock_Config();

    // Initialize Peripherals
    initializePeripherals();
    globalObjectsInitialize();

    // Initialize Scheduler
    schedInitialize(&gScheduler);

    gScheduler.pGetHALTick  = HAL_GetTick;
    gScheduler.pTask_1ms    = taskApp1ms;
    gScheduler.pTask_10ms   = taskApp10ms;
    gScheduler.pTask_50ms   = taskApp50ms;
    gScheduler.pTask_250ms  = taskApp250ms;

    // Initilize Tasks
    taskAppInitialize();

    while (1)
    {
    	schedCycle(&gScheduler);
    }
}

/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief Initializes the used peripherals like GPIO,
 * ADC, DMA and Timer Interrupts
 *
 * @return Returns ERROR_OK if no error occurred
 */
static int32_t initializePeripherals()
{
    // Initialize UART used for Debug-Outputs
    uartInitialize(115200);

    // Initialize GPIOs for LED and 7-Segment output
	ledInitialize();
    displayInitialize();

    // Initialize GPIOs for Buttons
    buttonInitialize();

    // Initialize Timer, DMA and ADC for sensor measurements
    timerInitialize();
    adcInitialize();

    return ERROR_OK;
}
