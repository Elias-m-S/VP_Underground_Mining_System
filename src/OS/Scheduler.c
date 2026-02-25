/******************************************************************************
 * @file Scheduler.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of the cooperative scheduler with a  pre-defined set
 * of cyclic "task slots"
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "Scheduler.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/
#define HAL_TICK_VALUE_1MS      1       //!< Number of HAL Ticks used for 1ms Tasks
#define HAL_TICK_VALUE_10MS     10      //!< Number of HAL Ticks used for 10ms Tasks
#define HAL_TICK_VALUE_100MS    100     //!< Number of HAL Ticks used for 100ms Tasks
#define HAL_TICK_VALUE_250MS    250     //!< Number of HAL Ticks used for 250ms Tasks
#define HAL_TICK_VALUE_1000MS   1000    //!< Number of HAL Ticks used for 1000ms Tasks


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/


/***** PUBLIC FUNCTIONS ******************************************************/


int32_t schedInitialize(Scheduler* pScheduler)
{

    return SCHED_ERR_OK;
}


int32_t schedCycle(Scheduler* pScheduler)
{


    return SCHED_ERR_OK;
}


/***** PRIVATE FUNCTIONS *****************************************************/
