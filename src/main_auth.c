/******************************************************************************
 * @file main_auth.c
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
 * Implements the top-level entry point for the Authenticator binary.
 *
 * Responsibilities:
 *   1. HAL initialisation (HAL_Init, SystemClock_Config)
 *   2. Peripheral initialisation – only the peripherals needed by the
 *      Authenticator (UART for key reception, LEDs for status indication)
 *   3. One-time initialisation of the Authenticator state machine
 *   4. Cyclic call to Authfunc_Update() from within the main loop
 *
 * The Authenticator does NOT use a scheduler. The state machine is driven
 * directly by the main loop and uses HAL_GetTick() for timeout handling.
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"
#include "System.h"

#include "Util/Global.h"
#include "Util/Log/LogOutput.h"

#include "UARTModule.h"
#include "LEDModule.h"

#include "Authfunc.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/
static int32_t initializePeripherals(void);


/***** PRIVATE VARIABLES *****************************************************/


/***** PUBLIC FUNCTIONS ******************************************************/

/**
 * @brief Main entry point of the Authenticator binary.
 *
 * Initialises hardware, then hands control to the Authenticator state
 * machine which runs in a tight loop until the Application is started
 * or a Failure state is entered.
 */
int main(void)
{
    /* --- 1. HAL and clock initialisation --------------------------------- */
    HAL_Init();
    SystemClock_Config();

    /* --- 2. Peripheral initialisation ------------------------------------ */
    int32_t result = initializePeripherals();
    if (result != ERROR_OK)
    {
        /* Peripheral init failed – cannot show anything useful, just hang */
        Error_Handler();
    }

    outputLogf("[AUTH] Authenticator started\r\n");

    /* --- 3. Authenticator state machine initialisation ------------------- */
    Authfunc_Init();

    /* --- 4. Main loop: drive the Authenticator state machine ------------- */
    while (1)
    {
        Authfunc_Update();
    }
}


/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief Initialises the peripherals required by the Authenticator.
 *
 * The Authenticator only needs:
 *   - UART  – to receive the 'A' trigger and the decryption key
 *   - LEDs  – to indicate status / timeouts / failure
 *
 * Peripherals like ADC, Timer, Display, Buttons or the Scheduler are
 * NOT initialised here – they belong to the Application binary.
 *
 * @return ERROR_OK if no error occurred, ERROR_GENERAL otherwise.
 */
static int32_t initializePeripherals(void)
{
    int32_t result = ERROR_OK;

    /* UART – 115200 baud, 8N1 */
    result = uartInitialize(115200);
    if (result != UART_ERR_OK)
    {
        return ERROR_GENERAL;
    }

    /* LEDs – status output */
    result = ledInitialize();
    if (result != LED_ERR_OK)
    {
        return ERROR_GENERAL;
    }

    return ERROR_OK;
}