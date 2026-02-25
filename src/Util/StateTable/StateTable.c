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


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/
static bool stateTableFindState(StateTable_t* pStateTable, int32_t stateID, State_t** pFoundState);


/***** PRIVATE VARIABLES *****************************************************/


/***** PUBLIC FUNCTIONS ******************************************************/


int32_t stateTableInitialize(StateTable_t* pStateTable, StateTableEntry_t* pTableEntries, int32_t entryCount, int32_t initStateID)
{
    // Check for valid pointer
    if (pStateTable == 0 || pTableEntries == 0)
        return STATETBL_ERR_INVALID_PTR;

    // Initialize the State Table
    pStateTable->pTableEntries          = pTableEntries;
    pStateTable->stateTableEntryCount   = entryCount;

    // Initialize the dynamic entry data
    for (int32_t i=0; i<pStateTable->stateTableEntryCount; i++)
    {
        int32_t fromStateID = pStateTable->pTableEntries[i].stateIDFrom;
        int32_t toStateID = pStateTable->pTableEntries[i].stateIDTo;

        // Find the state in the state list
        State_t* pStateFrom = 0;
        stateTableFindState(pStateTable, fromStateID, &pStateFrom);

        State_t* pStateTo = 0;
        stateTableFindState(pStateTable, toStateID, &pStateTo);

        pStateTable->pTableEntries[i].pFromStateRef = pStateFrom;
        pStateTable->pTableEntries[i].pToStateRef = pStateTo;
    }

    pStateTable->currentStateID         = initStateID;
    pStateTable->previousStateID        = STT_UNKNOWN_STATE;

    stateTableFindState(pStateTable, pStateTable->currentStateID, &(pStateTable->pCurrentStateRef));

    return STATETBL_ERR_OK;
}


int32_t stateTableRunCyclic(StateTable_t* pStateTable)
{
    int32_t result = STATETBL_ERR_EVENT_UNHANDLED;

    // Get the current pending event and reset it in the table
    // to indicate that the event has been processed
    int32_t currentEvent        = pStateTable->pendingEvent;
    pStateTable->pendingEvent   = STT_NONE_EVENT;

    // Check for new Event
    if (currentEvent != STT_NONE_EVENT)
    {
        // If there is an even, lets dispatch it
        for (int32_t i=0; i<pStateTable->stateTableEntryCount; i++)
        {
            StateTableEntry_t* pEntry = &(pStateTable->pTableEntries[i]);
            // Iterate through the state table and try to find the entry for the state/event combination
            if (pEntry->stateIDFrom == pStateTable->currentStateID && pEntry->eventID == currentEvent)
            {
                bool transitionAllowed = true;

                // We found an entry with the actual state and event combination
                if (pEntry->pGuard != 0)
                {
                    // Check if the transition is allowed
                    transitionAllowed = pEntry->pGuard(pEntry, currentEvent);
                }

                if (transitionAllowed == true)
                {
                    // Call the onExit function for the current state
                    if (pEntry->pFromStateRef != 0)
                    {
                        if (pEntry->pFromStateRef->pOnExit != 0)
                        {
                            // Call OnExit
                            pEntry->pFromStateRef->pOnExit(pEntry->pFromStateRef, currentEvent);
                        }

                        // Reset the OnEntry flag
                        pEntry->pFromStateRef->onEntryCalled = false;
                    }

                    // Perform the transition
                    pStateTable->previousStateID    = pStateTable->currentStateID;
                    pStateTable->currentStateID     = pEntry->stateIDTo;
                    pStateTable->pCurrentStateRef   = pEntry->pToStateRef;

                    // On Entry will be called in the normal cycle

                    result = STATETBL_ERR_OK;
                    break;
                }
            }
        }
    }
    else
    {
        // No new event, then we check whether we need to call an Onentry function and continue with the normal
        // cyclic state function
        if (pStateTable->pCurrentStateRef != 0)
        {
            State_t *pCurrentState = pStateTable->pCurrentStateRef;

            // Check for onEntry call
            if (pCurrentState->pOnEntry != 0 && pCurrentState->onEntryCalled == false)
            {
                pCurrentState->pOnEntry(pCurrentState, currentEvent);
                pCurrentState->onEntryCalled = true;
            }

            // Now call the cyclic function
            if (pCurrentState->pOnState != 0)
            {
                pCurrentState->pOnState(pCurrentState, currentEvent);
            }
        }
    }

    return result;
}

int32_t stateTableSendEvent(StateTable_t* pStateTable, int32_t event)
{
    // Check for valid pointer
    if (pStateTable == 0 )
        return STATETBL_ERR_INVALID_PTR;

    // Check if there is still an event pending. If so, we do not
    // overwrite the old event but return an error
    if (pStateTable->pendingEvent != STT_NONE_EVENT)
    {
        return STATETBL_ERR_EVENT_PENDING;
    }
    else
    {
        // If no pending event is waiting, we set the new event
        pStateTable->pendingEvent = event;
    }

    return STATETBL_ERR_OK;
}


/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief Searches for a state in the state list with the provided state ID
 *
 * @param pStateTable   Pointer to the state table to use
 * @param stateID       State ID to search for
 * @param pFoundState   Poitner to the found state entry
 * @return true         If the state with the state ID was found
 * @return false        If the state was not found
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
            // We found the state entry for the requested ID
            *pFoundState = pState;
            foundState = true;
            break;
        }
    }

    return foundState;
}
