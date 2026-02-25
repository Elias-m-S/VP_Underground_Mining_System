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

    // Initialize the System Clock
    SystemClock_Config();

    // Initialize Peripherals
    initializePeripherals();

    // Initialize Scheduler
    schedInitialize(&gScheduler);

    int globalCounter = 0;
    uint8_t left = 0;

    while (1)
    {
        // Read to buttons
        Button_Status_t but1 = buttonGetButtonStatus(BTN_SW1);
        Button_Status_t but2 = buttonGetButtonStatus(BTN_SW2);
        Button_Status_t but3 = buttonGetButtonStatus(BTN_B1);

        // Read the POT1 input from ADC
        int adcValue = adcReadChannel(ADC_INPUT0);

        // If SW1 is pressed, print some debug message on the terminal
        if (but1 == BUTTON_PRESSED)
        {
            // Toggle all LEDs to the their functionality (Toggle frequency depends on HAL_Delay at end of loop)
            ledToggleLED(LED0);
            HAL_Delay(25);
            ledToggleLED(LED1);
            HAL_Delay(25);
            ledToggleLED(LED2);
            HAL_Delay(25);
            ledToggleLED(LED3);
            HAL_Delay(25);
            ledToggleLED(LED4);
            HAL_Delay(25);
        }

        // If SW2 is pressed, print the ADC digit value on the terminal
        if (but2 == BUTTON_PRESSED)
        {
        	HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_PIN, GPIO_PIN_RESET);
        }
        else
        {
        	HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_PIN, GPIO_PIN_SET);
        }

        if (but3 == BUTTON_PRESSED)
        {
        	outputLogf("ADC Val: %d\n\r", adcValue);
        }

        globalCounter++;
        if (globalCounter > 99)
        {
            globalCounter = 0;
        }

        if (left == 1)
        {
            displayShowDigit(LEFT_DISPLAY, (globalCounter / 10));
        }
        else
        {
            displayShowDigit(RIGHT_DISPLAY, (globalCounter % 10));
        }

        left = !left;

        // Remove this HAL_Delay as soon as there is a Scheduler used
        HAL_Delay(25);
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
