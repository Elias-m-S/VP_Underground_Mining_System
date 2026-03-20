/******************************************************************************
 * @file StateTable.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of a generic state table
 *
 *
 *****************************************************************************/

/***** INCLUDES **************************************************************/
#include "StateTable.h"


/***** PRIVATE PROTOTYPES ****************************************************/
static bool stateTableFindState(StateTable_t* pStateTable, int32_t stateID, State_t** pFoundState);

static bool stateTableQueuePop(StateTable_t* pStateTable, int32_t* pEvent);
static int32_t stateTableQueuePush(StateTable_t* pStateTable, int32_t event);


/***** PUBLIC FUNCTIONS ******************************************************/

int32_t stateTableInitialize(StateTable_t* pStateTable,
                            StateTableEntry_t* pTableEntries,
                            int32_t entryCount,
                            int32_t initStateID)
{
    // Check for valid pointer
    if (pStateTable == 0 || pTableEntries == 0)
        return STATETBL_ERR_INVALID_PTR;

    // Initialize the State Table
    pStateTable->pTableEntries          = pTableEntries;
    pStateTable->stateTableEntryCount   = entryCount;

    // Initialize queue
    pStateTable->pendingEvent = STT_NONE_EVENT;
    pStateTable->eventHead = 0U;
    pStateTable->eventTail = 0U;
    pStateTable->eventCount = 0U;

    // Initialize the dynamic entry data
    for (int32_t i=0; i<pStateTable->stateTableEntryCount; i++)
    {
        int32_t fromStateID = pStateTable->pTableEntries[i].stateIDFrom;
        int32_t toStateID   = pStateTable->pTableEntries[i].stateIDTo;

        // Find the state in the state list
        State_t* pStateFrom = 0;
        stateTableFindState(pStateTable, fromStateID, &pStateFrom);

        State_t* pStateTo = 0;
        stateTableFindState(pStateTable, toStateID, &pStateTo);

        pStateTable->pTableEntries[i].pFromStateRef = pStateFrom;
        pStateTable->pTableEntries[i].pToStateRef   = pStateTo;
    }

    pStateTable->currentStateID         = initStateID;
    pStateTable->previousStateID        = STT_UNKNOWN_STATE;

    stateTableFindState(pStateTable, pStateTable->currentStateID, &(pStateTable->pCurrentStateRef));

    return STATETBL_ERR_OK;
}


int32_t stateTableRunCyclic(StateTable_t* pStateTable)
{
    int32_t result = STATETBL_ERR_EVENT_UNHANDLED;

    if (pStateTable == 0)
        return STATETBL_ERR_INVALID_PTR;

    // Pop next event (if any)
    int32_t currentEvent = STT_NONE_EVENT;
    (void)stateTableQueuePop(pStateTable, &currentEvent);

    // Check for new Event
    if (currentEvent != STT_NONE_EVENT)
    {
        // If there is an event, dispatch it
        for (int32_t i=0; i<pStateTable->stateTableEntryCount; i++)
        {
            StateTableEntry_t* pEntry = &(pStateTable->pTableEntries[i]);

            // Find entry for current state + event
            if (pEntry->stateIDFrom == pStateTable->currentStateID && pEntry->eventID == currentEvent)
            {
                bool transitionAllowed = true;

                // Guard check
                if (pEntry->pGuard != 0)
                {
                    transitionAllowed = pEntry->pGuard(pEntry, currentEvent);
                }

                if (transitionAllowed == true)
                {
                    // Call the onExit function for the current state
                    if (pEntry->pFromStateRef != 0)
                    {
                        if (pEntry->pFromStateRef->pOnExit != 0)
                        {
                            pEntry->pFromStateRef->pOnExit(pEntry->pFromStateRef, currentEvent);
                        }

                        // Reset the OnEntry flag
                        pEntry->pFromStateRef->onEntryCalled = false;
                    }

                    // Perform transition
                    pStateTable->previousStateID    = pStateTable->currentStateID;
                    pStateTable->currentStateID     = pEntry->stateIDTo;
                    pStateTable->pCurrentStateRef   = pEntry->pToStateRef;

                    // OnEntry will be called in normal cycle
                    if (pStateTable->pCurrentStateRef != 0)
                    {
                        pStateTable->pCurrentStateRef->onEntryCalled = false;
                    }

                    result = STATETBL_ERR_OK;
                    break;
                }
            }
        }

        return result;
    }

    // No event: normal cyclic state execution
    if (pStateTable->pCurrentStateRef != 0)
    {
        State_t *pCurrentState = pStateTable->pCurrentStateRef;

        // Check for onEntry call
        if (pCurrentState->pOnEntry != 0 && pCurrentState->onEntryCalled == false)
        {
            pCurrentState->pOnEntry(pCurrentState, currentEvent);
            pCurrentState->onEntryCalled = true;
        }

        // Call cyclic function
        if (pCurrentState->pOnState != 0)
        {
            pCurrentState->pOnState(pCurrentState, currentEvent);
        }
    }

    return STATETBL_ERR_OK;
}


int32_t stateTableSendEvent(StateTable_t* pStateTable, int32_t event)
{
    // Check for valid pointer
    if (pStateTable == 0)
        return STATETBL_ERR_INVALID_PTR;

    // Push into queue (do not lose events)
    return stateTableQueuePush(pStateTable, event);
}


/***** PRIVATE FUNCTIONS *****************************************************/

static int32_t stateTableQueuePush(StateTable_t* pStateTable, int32_t event)
{
    if (pStateTable->eventCount >= STT_EVENT_QUEUE_SIZE)
    {
        return STATETBL_ERR_EVENT_PENDING;
    }

    pStateTable->eventQueue[pStateTable->eventTail] = event;
    pStateTable->eventTail++;

    if (pStateTable->eventTail >= STT_EVENT_QUEUE_SIZE)
        pStateTable->eventTail = 0U;

    pStateTable->eventCount++;

    return STATETBL_ERR_OK;
}

static bool stateTableQueuePop(StateTable_t* pStateTable, int32_t* pEvent)
{
    if ((pStateTable == 0) || (pEvent == 0))
        return false;

    if (pStateTable->eventCount == 0U)
    {
        *pEvent = STT_NONE_EVENT;
        return false;
    }

    *pEvent = pStateTable->eventQueue[pStateTable->eventHead];
    pStateTable->eventHead++;

    if (pStateTable->eventHead >= STT_EVENT_QUEUE_SIZE)
        pStateTable->eventHead = 0U;

    pStateTable->eventCount--;

    return true;
}

/**
 * @brief Searches for a state in the state list with the provided state ID
 */
static bool stateTableFindState(StateTable_t* pStateTable, int32_t stateID, State_t** pFoundState)
{
    bool foundState = false;
    *pFoundState = 0;

    for (int32_t i=0; i<pStateTable->stateCount; i++)
    {
        State_t* pState = &(pStateTable->pStateList[i]);
        if (pState->stateID == stateID)
        {
            *pFoundState = pState;
            foundState = true;
            break;
        }
    }

    return foundState;
}
