/******************************************************************************
 * @file SystemHandler.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of Exception-Handler (non-fault)
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"

/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/


/***** PUBLIC FUNCTIONS ******************************************************/


/**
 * @brief Default-Implementation of the NMI Handler
 *
 * According Programming Manual:
 * A NonMaskable Interrupt (NMI) can be signalled by a peripheral or
 * triggered by software. This is the highest priority exception other
 * than reset. It is permanently enabled and has a fixed priority of -2.
 * NMIs cannot be:
 *   • Masked or prevented from activation by any other exception
 *   • Preempted by any exception other than Reset.
 *
 */
void NMI_Handler(void)
{
}

/**
 * @brief Default-Implementation of SVC Handler
 *
 * According Programming Manual:
 * A supervisor call (SVC) is an exception that is triggered by the SVC
 * instruction. In an OS environment, applications can use SVC
 * instructions to access OS kernel functions and device drivers.
 *
 */
void SVC_Handler(void)
{
}

/**
 * @brief Default-Implementation of PendSV Handler
 *
 * According Programming Manual:
 * PendSV is an interrupt-driven request for system-level service. In an
 * OS environment, use PendSV for context switching when no other
 * exception is active.
 *
 */
void PendSV_Handler(void)
{
}

/**
 * @brief Default-Implementation of SysTick Handler
 *
 * This handler is called for every "tick" of the SysTick
 * timer. Currently, only the internal Tick-Counter for
 * the HAL is updated
 *
 * According Programming Manual:
 * A SysTick exception is an exception the system timer generates
 * when it reaches zero. Software can also generate a SysTick
 * exception. In an OS environment, the processor can use this
 * exception as system tick.
 *
 */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/**
 * @brief Default-Implementation of DebugMon Handler
 *
 * According Programming Manual:
 * ?
 *
 */
void DebugMon_Handler(void)
{
}


/***** PRIVATE FUNCTIONS *****************************************************/
