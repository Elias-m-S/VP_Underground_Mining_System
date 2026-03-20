/******************************************************************************
 * @file Application.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header file for main application (state machine)
 *
 *
 *****************************************************************************/
#ifndef _APPLICATION_H_
#define _APPLICATION_H_

/***** INCLUDES **************************************************************/
#include <stdint.h>

/***** MACROS ****************************************************************/
/* State IDs (table based) */
#define STATE_ID_INITIALIZATION     10
#define STATE_ID_PREOPERATIONAL     20
#define STATE_ID_OPERATIONAL        30
#define STATE_ID_EMERGENCY          40
#define STATE_ID_TESTMODE           50
#define STATE_ID_FAILURE            60

/* Event IDs */
#define EVT_ID_NONE                 0
#define EVT_ID_INIT_OK              1
#define EVT_ID_SENSOR_DEFECT        2
#define EVT_ID_SW1_TOGGLE           3
#define EVT_ID_SW2_TOGGLE_TEST      4
#define EVT_ID_B1_RESET_ALARM       5
#define EVT_ID_EMERGENCY_TRIGGER    6
#define EVT_ID_STACK_CORRUPTION     7
/***** PROTOTYPES ************************************************************/

/* Keep original API names to avoid refactoring other files */
int32_t applicationInitialize(void);
int32_t applicationRun(void);
int32_t applicationSendEvent(int32_t eventID);

#endif /* _APPLICATION_H_ */
