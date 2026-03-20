/******************************************************************************
 * @file LEDModule.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header file for the LED module
 *
 *
 *****************************************************************************/
#ifndef _LED_MODULE_H_
#define _LED_MODULE_H_

/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"
#include <stdint.h>


/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/
#define LED_ERR_OK      0               //!< No error occured


/***** TYPES *****************************************************************/

/**
 * @brief Enumeration of available LEDs and their usage
 *
 */
typedef enum _LED_t
{
    LED0,                               //!< LED0
    LED1,                               //!< LED1
    LED2,                               //!< LED2
    LED3,                               //!< LED3
    LED4                                //!< LED4
} LED_t;

/**
 * @brief Enumeration of possible LED status
 *
 */
typedef enum _LED_Status_t
{
    LED_ON  = GPIO_PIN_SET,             //!< Value to turn a LED on
    LED_OFF = GPIO_PIN_RESET            //!< Value to turn a LED off
} LED_Status_t;


/***** PROTOTYPES ************************************************************/

/**
 * @brief Initialize the GPIOs for the LED outouts
 *
 * @return Returns LED_ERR_OK if no error occured
 */
int32_t ledInitialize();

/**
 * @brief Toggles the provided LED
 *
 * @param led LED to toggle
 */
void ledToggleLED(LED_t led);

/**
 * @brief Allows to set (turn on) or reset (turn off) a LED
 *
 * @param led LED which should be changed
 * @param ledStatus New status of the LED
 */
void ledSetLED(LED_t led, LED_Status_t ledStatus);

#endif
