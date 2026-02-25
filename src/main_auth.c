/******************************************************************************
 * @file main.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Main file for the VP Template Authenticator project
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

#include "GlobalObjects.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


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
    // Initialize the HAL
    HAL_Init();

    SystemClock_Config();

    // Initialize Peripherals
    initializePeripherals();

    // Initialize Scheduler
    schedInitialize(&gScheduler);

    while (1)
    {
        // Toggle all LEDs to the their functionality (Toggle frequency depends on HAL_Delay at end of loop)
        ledToggleLED(LED0);
        HAL_Delay(100);
        ledToggleLED(LED1);
        HAL_Delay(100);
        ledToggleLED(LED2);
        HAL_Delay(100);
        ledToggleLED(LED3);
        HAL_Delay(100);
        ledToggleLED(LED4);
        HAL_Delay(100);

        uint8_t buf[10];
        uartReceiveData(buf, 2);
        if (buf[0] == 'X' && buf[1] == '\r')
        {
            displayShowDigit(RIGHT_DISPLAY, DIGIT_DASH);
        }
        else
        {
            displayShowDigit(RIGHT_DISPLAY, DIGIT_OFF);
        }
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
