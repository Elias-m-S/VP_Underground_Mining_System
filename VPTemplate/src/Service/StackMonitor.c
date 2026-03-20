/******************************************************************************
 * @file StackMonitor.c
 *
 * @brief Stack monitor module for stack usage and corruption detection.
 *
 *****************************************************************************/

/***** INCLUDES **************************************************************/
#include "StackMonitor.h"

/***** PRIVATE MACROS ********************************************************/
#define STACK_PATTERN_WORD      (0xA5A5A5A5UL)
#define STACK_GUARD_WORDS       (4U)

/***** PRIVATE VARIABLES *****************************************************/
static bool s_stackCorrupted = false;
static uint32_t s_freeStackBytes = 0U;

/***** LINKER SYMBOLS ********************************************************/
extern uint32_t _stack_start;
extern uint32_t _stack_end;

/***** PUBLIC FUNCTIONS ******************************************************/

void stackMonitorInitialize(void)
{
    s_stackCorrupted = false;
    s_freeStackBytes = 0U;

    stackMonitorCheck();
}

void stackMonitorCheck(void)
{
    volatile uint32_t* pStart = (volatile uint32_t*)&_stack_start;
    volatile uint32_t* pEnd   = (volatile uint32_t*)&_stack_end;

    uint32_t totalWords = (uint32_t)(pEnd - pStart);
    uint32_t guardWords = STACK_GUARD_WORDS;
    uint32_t freeWords = 0U;
    uint32_t i;

    if (guardWords > totalWords)
    {
        guardWords = totalWords;
    }

    s_stackCorrupted = false;

    /* Check lowest stack words for overflow/corruption.
     * Stack grows downwards from _stack_end to _stack_start.
     * Therefore, overflow will hit _stack_start first.
     */
    for (i = 0U; i < guardWords; i++)
    {
        if (pStart[i] != STACK_PATTERN_WORD)
        {
            s_stackCorrupted = true;
            break;
        }
    }

    /* Determine remaining untouched stack bytes.
     * This is effectively the stack high-water mark.
     */
    while ((freeWords < totalWords) && (pStart[freeWords] == STACK_PATTERN_WORD))
    {
        freeWords++;
    }

    s_freeStackBytes = freeWords * (uint32_t)sizeof(uint32_t);
}

bool stackMonitorIsCorrupted(void)
{
    return s_stackCorrupted;
}

uint32_t stackMonitorGetFreeStackBytes(void)
{
    return s_freeStackBytes;
}
