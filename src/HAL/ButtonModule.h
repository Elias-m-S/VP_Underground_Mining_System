/******************************************************************************
 * @file ButtonModule.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header file for the Button Module
 *
 *
 *****************************************************************************/
#ifndef _BUTTON_MODULE_H_
#define _BUTTON_MODULE_H_

/***** INCLUDES **************************************************************/


/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/
#define BUTTON_ERR_OK      0            //!< No error occured


/***** TYPES *****************************************************************/

/**
 * @brief Enumeration of available Buttons and their usage
 *
 */
typedef enum _Button_t
{
    BTN_B1,
    BTN_SW1,                        //!< Button (SW1)
    BTN_SW2,                        //!< Button (SW2)
} Button_t;

/**
 * @brief Enumeration for possible Button states
 *
 */
typedef enum _Button_Status_t
{
    BUTTON_PRESSED,                     //!< Button is pressed
    BUTTON_RELEASED                     //!< Button is released
} Button_Status_t;

/***** PROTOTYPES ************************************************************/

/**
 * @brief Initialize the GPIOs for the Button inputs
 *
 * @return Returns BUTTON_ERR_OK if no error occured
 */
int32_t buttonInitialize();

/**
 * @brief Reads the input status of the button
 *
 * @param button Button to read the status from
 *
 * @returns Returns the button status (pressed or released)
 *
 * @remark This function doesn't do any debouncing. It just reads
 * the pure GPIO state
 */
Button_Status_t buttonGetButtonStatus(Button_t button);


#endif
