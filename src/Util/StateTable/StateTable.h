/******************************************************************************
 * @file StateTable.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief file for a generic state table implementation
 *
 *
 *****************************************************************************/
#ifndef _STATE_TABLE_H_
#define _STATE_TABLE_H_

/***** INCLUDES **************************************************************/
#include <stdint.h>
#include <stdbool.h>


/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/
#define STATETBL_ERR_OK                     0       //!< No error occured
#define STATETBL_ERR_INVALID_PTR            -1      //!< Invalid pointer (null pointer)
#define STATETBL_ERR_INVALID_STATE_ID       -2      //!< Invalid state ID found
#define STATETBL_ERR_INVALID_EVENT_ID       -3      //!< Invalid event ID found
#define STATETBL_ERR_EVENT_PENDING          -4      //!< New event sent but still an event is pending
#define STATETBL_ERR_EVENT_UNHANDLED        -5      //!< Event couldn't be handled

#define STT_INVALID_STATE                   -1      //!< Invalid state
#define STT_INITIAL_STATE                   0       //!< Initial state for startup of State Machine
#define STT_UNKNOWN_STATE                   1       //!< Unknown state ID

#define STT_NONE_EVENT                      0       //!< ID for "No Event"


/***** TYPES *****************************************************************/
// Forward Declaration for StateEntry
typedef struct _State State_t;
typedef struct _StateTableEntry StateTableEntry_t;

/**
 * @brief Function pointer for state function (state, on entry, on exit)
 *
 */
typedef int32_t (*StateFunction)(State_t* pState, int32_t eventID);

/**
 * @brief Function pointer for the transition guards to check whether a
 * transistion is allowed or not
 *
 */
typedef bool (*TransitionGuardFunction)(StateTableEntry_t* pEntry, int32_t eventID);

/**
 * @brief Struct to represent a state in the state machine
 *
 */
typedef struct _State
{
    // Static fields used for state configuration
    int32_t stateID;                        //!< ID of the state
    StateFunction pOnEntry;                 //!< Function pointer for the on entry function of the state
    StateFunction pOnState;                 //!< Function Pointer for the state function
    StateFunction pOnExit;                  //!< Function pointer for the on exit function of the state

    // Dynamic fields used during runtime
    bool onEntryCalled;                     //!< Flag to indicate whethter the onEntry function has been called
} State_t;

/**
 * @brief Struct to represent an entry in the state table
 *
 */
typedef struct _StateTableEntry
{
    // Static fileds used for state table configuration
    int32_t stateIDFrom;                    //!< ID of the state the transition starts from
    int32_t stateIDTo;                      //!< ID of the state the transition will go to
    int32_t eventID;                        //!< Event which triggers the transition

    TransitionGuardFunction pGuard;         //!< Function pointer for a transition guard function

    // Dynamic files used during runtime
    State_t* pFromStateRef;                 //!< Pointer to the "from state object"
    State_t* pToStateRef;                   //!< Poitner to the "to state object"
} StateTableEntry_t;

/**
 * @brief Struct which represents the state table respectivly the
 * complete state machine including current and previous state
 *
 */
typedef struct _StateTable
{
    int32_t stateTableEntryCount;           //!< Number of entries in the state table
    StateTableEntry_t* pTableEntries;       //!< Array of state table entries

    int32_t stateCount;                     //!< Number of total states
    State_t* pStateList;                    //!< List of all states

    int32_t currentStateID;                 //!< ID of the current state
    int32_t previousStateID;                //!< ID of the previous state

    State_t *pCurrentStateRef;              //!< Pointer to the current state object

    int32_t pendingEvent;                   //!< Current pending event
} StateTable_t;


/***** PROTOTYPES ************************************************************/

/**
 * @brief Initializes the state table instance with the table entries and needed configuration parameter
 *
 * @param pStateTable       Pointer to the state table instance
 * @param pTableEntries     Pointer to the list of transitions
 * @param entryCount        Number of entries in the transition list
 * @param initStateID       State ID for the initial state
 *
 * @return Returns STATETBL_ERR_OK if no error occured
 */
int32_t stateTableInitialize(StateTable_t* pStateTable, StateTableEntry_t* pTableEntries, int32_t entryCount, int32_t initStateID);

/**
 * @brief Cyclic run function for the state machine. This function performs either the
 * state transitions if an event is pending or it calles the state function if such a
 * function is provided for the current state
 *
 * @param pStateTable   Pointer to the state machine instance
 *
 * @return Returns STATETBL_ERR_OK if no error occured
 */
int32_t stateTableRunCyclic(StateTable_t* pStateTable);

/**
 * @brief Sends an event to the state machine instance which is processed in the next
 * state machien cycle
 *
 * @param pStateTable   Pointer to the state machine instance
 * @param event         Event ID to send to the state machine
 *
 * @return Returns STATETBL_ERR_OK if no error occured
 */
int32_t stateTableSendEvent(StateTable_t* pStateTable, int32_t event);

#endif
