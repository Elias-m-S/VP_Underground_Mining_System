/******************************************************************************
 * @file StackMonitor.h
 *
 * @brief Stack monitor module for stack usage and corruption detection.
 *
 *****************************************************************************/
#ifndef _STACK_MONITOR_H_
#define _STACK_MONITOR_H_

/***** INCLUDES **************************************************************/
#include <stdint.h>
#include <stdbool.h>

/***** PROTOTYPES ************************************************************/

void stackMonitorInitialize(void);
void stackMonitorCheck(void);

bool stackMonitorIsCorrupted(void);
uint32_t stackMonitorGetFreeStackBytes(void);

#endif /* _STACK_MONITOR_H_ */
