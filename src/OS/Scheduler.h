/******************************************************************************
 * @file Scheduler.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header File for cooperative scheduler module
 *
 *
 *****************************************************************************/
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_


/***** INCLUDES **************************************************************/
#include <stdint.h>

/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/
#define SCHED_ERR_OK                0           //!< No error occured (Scheduler)
#define SCHED_ERR_INVALID_PTR       -1          //!< Invalid pointer (Scheduler)

/***** TYPES *****************************************************************/

/**
 * @brief Function pointer for reading the current HAL Tick timer
 *
 * This provides the possibility to decouple the scheduler from a
 * specific HAL implementation, but still makes it possible to measure
 * the time inside the scheduler.
 *
 */
typedef uint32_t (*GetHALTick)(void);

/**
 * @brief Function pointer for cyclic function for the scheduler
 *
 */
typedef void (*CyclicFunction)(void);

/**
 * @brief Struct definition which holds the HAL tick
 * time stamps for the different tasks
 *
 */
typedef struct _Scheduler
{
    GetHALTick pGetHALTick;             //!< Function pointer for callback to read current HAL tick counter

    uint32_t halTick_1ms;               //!< Timestamp for last execution of 1ms task
    CyclicFunction pTask_1ms;           //!< Function pointer to 1ms cyclic task function

    uint32_t halTick_10ms;              //!< Timestamp for last execution of 10ms task
    CyclicFunction pTask_10ms;          //!< Function pointer to 10ms cyclic task function

    uint32_t halTick_100ms;             //!< Timestamp for last execution of 100ms task
    CyclicFunction pTask_100ms;         //!< Function pointer to 100ms cyclic task function

    uint32_t halTick_250ms;             //!< Timestamp for last execution of 250ms task
    CyclicFunction pTask_250ms;         //!< Function pointer to 250ms cyclic task function

    uint32_t halTick_1000ms;            //!< Timestamp for last execution of 1000ms task
    CyclicFunction pTask_1000ms;        //!< Function pointer to 1000ms cyclic task function
} Scheduler;


/***** PROTOTYPES ************************************************************/


/**
 * @brief Initializes the Scheduler component
 * Initializes the internal values for the timestamps.
 *
 * @remark: This function doesn't initialize the function
 * pointers in the Scheduler struct!
 *
 * @param pScheduler Pointer to scheduler struct
 *
 * @return SCHED_ERR_OK if no error occured
 */
int32_t schedInitialize(Scheduler* pScheduler);

/**
 * @brief Cyclic function for the scheduler
 * This function should be called in the super loop of the system
 * Hereby the scheduler takes care of the different time slots for
 * the tasks
 *
 * @param pScheduler Pointer to scheduler struct
 *
 * @return SCHED_ERR_OK if no error occured
 */
int32_t schedCycle(Scheduler* pScheduler);

#endif
