/******************************************************************************
 * @file TimerModule.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header file for the Timer Module
 *
 *
 *****************************************************************************/


#ifndef _TIMER_MODULE_H_
#define _TIMER_MODULE_H_

/***** INCLUDES **************************************************************/
#include <stdint.h>

/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/
#define TIMER_ERR_OK                  0         //!< No error occured
#define TIMER_ERR_INIT_FAILURE        -1        //!< Error during timer initialization


/***** TYPES *****************************************************************/


/***** PROTOTYPES ************************************************************/

/**
 * @brief Initializes the Timer Module
 *
 * @return Returns TIMER_ERR_OK if no error occured, otherwiese TIMER_ERR_INIT_FAILURE
 */
int32_t timerInitialize();

#endif
