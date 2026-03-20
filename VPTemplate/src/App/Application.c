/******************************************************************************
 * @file Application.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation file for main application (state machine)
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "Application.h"
#include "Util/Global.h"

/* HAL input */
#include "ButtonModule.h"

/* State table */
#include "Util/StateTable/StateTable.h"

/* Global data exchange (Service Layer) */
#include "Service/GlobalObjects.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/

/* Task cycle of Application (called from 50ms task) */
#define APP_CYCLE_MS                    50U

/* Button debounce requirement: 50 ms stable input corresponds to one application cycle. */
#define DEBOUNCE_STABLE_CYCLES          1U

/* Thresholds and durations (from requirements) */
#define GAS_WARN_PPM                    3000
#define GAS_WARN_TIME_MS                5000U
#define GAS_EMERG_PPM                   5000
#define GAS_EMERG_TIME_MS               3000U

#define WATER_WARN_CM                   250U
#define WATER_WARN_TIME_MS              10000U
#define WATER_EMERG_CM                  300U
#define WATER_EMERG_TIME_MS             5000U

/* Initialization stabilization time */
#define INIT_STABLE_TIME_MS             200U

/* Convert durations to "50ms ticks" (no float) */
#define MS_TO_APP_TICKS(ms)             ((uint16_t)(((ms) + (APP_CYCLE_MS - 1U)) / APP_CYCLE_MS))

#define GAS_WARN_TICKS                  MS_TO_APP_TICKS(GAS_WARN_TIME_MS)
#define GAS_EMERG_TICKS                 MS_TO_APP_TICKS(GAS_EMERG_TIME_MS)
#define WATER_WARN_TICKS                MS_TO_APP_TICKS(WATER_WARN_TIME_MS)
#define WATER_EMERG_TICKS               MS_TO_APP_TICKS(WATER_EMERG_TIME_MS)
#define INIT_STABLE_TICKS               MS_TO_APP_TICKS(INIT_STABLE_TIME_MS)

/***** PRIVATE TYPES *********************************************************/

typedef struct _DebounceButton
{
    Button_t button;

    Button_Status_t lastRaw;
    uint8_t stableCnt;
    Button_Status_t debounced;
    Button_Status_t lastDebounced;
} DebounceButton_t;

/***** PRIVATE PROTOTYPES ****************************************************/

static int32_t onStateInitialization(State_t* pState, int32_t eventID);
static int32_t onEntryPreOperational(State_t* pState, int32_t eventID);
static int32_t onStatePreOperational(State_t* pState, int32_t eventID);
static int32_t onEntryOperational(State_t* pState, int32_t eventID);
static int32_t onStateOperational(State_t* pState, int32_t eventID);
static int32_t onEntryEmergency(State_t* pState, int32_t eventID);
static int32_t onStateEmergency(State_t* pState, int32_t eventID);
static int32_t onEntryTestMode(State_t* pState, int32_t eventID);
static int32_t onStateTestMode(State_t* pState, int32_t eventID);
static int32_t onEntryFailure(State_t* pState, int32_t eventID);
static int32_t onStateFailure(State_t* pState, int32_t eventID);

static void appProcessInputsAndGenerateEvents(void);
static void appUpdateWarningLogic(bool enabled);
static bool appIsAnySensorDefect(void);

static void debounceInit(DebounceButton_t* pBtn, Button_t button);
static void debounceUpdate(DebounceButton_t* pBtn);
static bool debounceRisingEdge(DebounceButton_t* pBtn);

/***** PRIVATE VARIABLES *****************************************************/

//__attribute__((section(".signature"), used))
//int8_t signatur[] = "UMMS";
/**
 * @brief List of State for the State Machine
 *
 * This list only constructs the state objects for each possible state
 * in the state machine. There are no transistions or events defined
 *
 *
 *
 */

static State_t gStateList[] =
{
    {STATE_ID_INITIALIZATION,      0,                     onStateInitialization,   0, false},
    {STATE_ID_PREOPERATIONAL,      onEntryPreOperational, onStatePreOperational,   0, false},
    {STATE_ID_OPERATIONAL,         onEntryOperational,    onStateOperational,      0, false},
    {STATE_ID_EMERGENCY,           onEntryEmergency,      onStateEmergency,        0, false},
    {STATE_ID_TESTMODE,            onEntryTestMode,       onStateTestMode,         0, false},
    {STATE_ID_FAILURE,             onEntryFailure,        onStateFailure,          0, false}
};

/**
 * @brief Definition of the transistion table of the state machine. Each row
 * contains FROM_STATE_ID, TO_STATE_ID, EVENT_ID, Function Pointer Guard Function
 *
 * The last two members of a transistion row are only the initialization of dynamic
 * members used durin runtim
 */
static StateTableEntry_t gStateTableEntries[] =
{
    /* Init -> PreOp */
    {STATE_ID_INITIALIZATION,   STATE_ID_PREOPERATIONAL, EVT_ID_INIT_OK,              0, 0, 0},

    /* Sensor defect -> Failure
     * Note:
     * In PreOperational no sensor validity checks shall trigger Failure.
     */
    {STATE_ID_INITIALIZATION,   STATE_ID_FAILURE,        EVT_ID_SENSOR_DEFECT,        0, 0, 0},
    {STATE_ID_OPERATIONAL,      STATE_ID_FAILURE,        EVT_ID_SENSOR_DEFECT,        0, 0, 0},
    {STATE_ID_EMERGENCY,        STATE_ID_FAILURE,        EVT_ID_SENSOR_DEFECT,        0, 0, 0},
    {STATE_ID_TESTMODE,         STATE_ID_FAILURE,        EVT_ID_SENSOR_DEFECT,        0, 0, 0},

    /* Stack corruption -> Failure from every non-failure state */
    {STATE_ID_INITIALIZATION,   STATE_ID_FAILURE,        EVT_ID_STACK_CORRUPTION,     0, 0, 0},
    {STATE_ID_PREOPERATIONAL,   STATE_ID_FAILURE,        EVT_ID_STACK_CORRUPTION,     0, 0, 0},
    {STATE_ID_OPERATIONAL,      STATE_ID_FAILURE,        EVT_ID_STACK_CORRUPTION,     0, 0, 0},
    {STATE_ID_EMERGENCY,        STATE_ID_FAILURE,        EVT_ID_STACK_CORRUPTION,     0, 0, 0},
    {STATE_ID_TESTMODE,         STATE_ID_FAILURE,        EVT_ID_STACK_CORRUPTION,     0, 0, 0},

    /* PreOp <-> Operational via SW1 */
    {STATE_ID_PREOPERATIONAL,   STATE_ID_OPERATIONAL,    EVT_ID_SW1_TOGGLE,           0, 0, 0},
    {STATE_ID_OPERATIONAL,      STATE_ID_PREOPERATIONAL, EVT_ID_SW1_TOGGLE,           0, 0, 0},

    /* Operational -> Emergency (threshold reached) */
    {STATE_ID_OPERATIONAL,      STATE_ID_EMERGENCY,      EVT_ID_EMERGENCY_TRIGGER,    0, 0, 0},

    /* Emergency -> Operational via B1 */
    {STATE_ID_EMERGENCY,        STATE_ID_OPERATIONAL,    EVT_ID_B1_RESET_ALARM,       0, 0, 0},

    /* TestMode toggle via SW2: enter from PreOp/Operational, exit back to PreOp */
    {STATE_ID_PREOPERATIONAL,   STATE_ID_TESTMODE,       EVT_ID_SW2_TOGGLE_TEST,      0, 0, 0},
    {STATE_ID_OPERATIONAL,      STATE_ID_TESTMODE,       EVT_ID_SW2_TOGGLE_TEST,      0, 0, 0},
    {STATE_ID_TESTMODE,         STATE_ID_PREOPERATIONAL, EVT_ID_SW2_TOGGLE_TEST,      0, 0, 0},
};

/**
 * @brief Global State Table instance
 *
 */
static StateTable_t gStateTable;

/* Debounce buttons (50ms) */
static DebounceButton_t s_btnSW1;
static DebounceButton_t s_btnSW2;
static DebounceButton_t s_btnB1;

/* Warning / threshold timers (50ms ticks) */
static uint16_t s_gasWarnCnt;
static uint16_t s_gasEmergCnt;
static uint16_t s_waterWarnCnt;
static uint16_t s_waterEmergCnt;

static uint8_t s_defectEventLatched;

/* Initialization stabilization counters */
static uint8_t s_initGasOkCnt;
static uint8_t s_initGasDefectCnt;


/***** PUBLIC FUNCTIONS ******************************************************/

int32_t applicationInitialize(void)
{
    /* Init button debounce */
    debounceInit(&s_btnSW1, BTN_SW1);
    debounceInit(&s_btnSW2, BTN_SW2);
    debounceInit(&s_btnB1,  BTN_B1);

    /* Reset counters */
    s_gasWarnCnt = 0U;
    s_gasEmergCnt = 0U;
    s_waterWarnCnt = 0U;
    s_waterEmergCnt = 0U;

    s_defectEventLatched = 0U;
    s_initGasOkCnt = 0U;
    s_initGasDefectCnt = 0U;

    /* Init State Table */
    gStateTable.pStateList = gStateList;
    gStateTable.stateCount = (int32_t)(sizeof(gStateList) / sizeof(State_t));

    return stateTableInitialize(
        &gStateTable,
        gStateTableEntries,
        (int32_t)(sizeof(gStateTableEntries) / sizeof(StateTableEntry_t)),
        STATE_ID_INITIALIZATION
    );
}

int32_t applicationRun(void)
{
    /* 1) Input processing + event generation (debounced 50ms) */
    appProcessInputsAndGenerateEvents();

    /* 2) Run table-based state machine */
    return stateTableRunCyclic(&gStateTable);
}

int32_t applicationSendEvent(int32_t eventID)
{
    return stateTableSendEvent(&gStateTable, eventID);
}


/***** PRIVATE FUNCTIONS *****************************************************/

/* ---------------- Debounce ---------------- */

static void debounceInit(DebounceButton_t* pBtn, Button_t button)
{
    pBtn->button = button;

    pBtn->lastRaw = buttonGetButtonStatus(button);
    pBtn->stableCnt = 0U;

    pBtn->debounced = pBtn->lastRaw;
    pBtn->lastDebounced = pBtn->debounced;
}

static void debounceUpdate(DebounceButton_t* pBtn)
{
    Button_Status_t raw = buttonGetButtonStatus(pBtn->button);

    if (raw == pBtn->lastRaw)
    {
        if (pBtn->stableCnt < 255U)
        {
            pBtn->stableCnt++;
        }
    }
    else
    {
        pBtn->stableCnt = 0U;
        pBtn->lastRaw = raw;
    }

    if (pBtn->stableCnt >= DEBOUNCE_STABLE_CYCLES)
    {
        pBtn->debounced = raw;
    }
}

static bool debounceRisingEdge(DebounceButton_t* pBtn)
{
    bool rising = (pBtn->lastDebounced == BUTTON_RELEASED) && (pBtn->debounced == BUTTON_PRESSED);
    pBtn->lastDebounced = pBtn->debounced;
    return rising;
}

/* ---------------- Common checks ---------------- */

static bool appIsAnySensorDefect(void)
{
    if (globalGetGasDefect()) return true;
    if (globalGetWaterDefect()) return true;
    if (globalGetInconsistency()) return true;

    return false;
}

static void appUpdateWarningLogic(bool enabled)
{
    globalSetWarningBlink(enabled);
}

/* ---------------- Input processing + event generation ---------------- */

static void appProcessInputsAndGenerateEvents(void)
{
    AppState_t appState = globalGetAppState();

    /* 1) Update debouncers first */
    debounceUpdate(&s_btnSW1);
    debounceUpdate(&s_btnSW2);
    debounceUpdate(&s_btnB1);

    /* 2) Buttons -> events */
    if (debounceRisingEdge(&s_btnSW1))
    {
        (void)applicationSendEvent(EVT_ID_SW1_TOGGLE);
    }

    if (debounceRisingEdge(&s_btnSW2))
    {
        (void)applicationSendEvent(EVT_ID_SW2_TOGGLE_TEST);
    }

    if (debounceRisingEdge(&s_btnB1))
    {
        (void)applicationSendEvent(EVT_ID_B1_RESET_ALARM);
    }

    /* 3) Global sensor defect check
     * In PreOperational no sensor validity checks shall trigger Failure.
     * Initialization handles its own initial sensor check in onStateInitialization().
     */
    if ((appState == APP_STATE_OPERATIONAL) ||
        (appState == APP_STATE_EMERGENCY) ||
        (appState == APP_STATE_TESTMODE))
    {
        if (appIsAnySensorDefect())
        {
            if (s_defectEventLatched == 0U)
            {
                if (applicationSendEvent(EVT_ID_SENSOR_DEFECT) >= 0)
                {
                    s_defectEventLatched = 1U;
                }
            }
        }
        else
        {
            s_defectEventLatched = 0U;
        }
    }
    else
    {
        s_defectEventLatched = 0U;
    }
}

/* ---------------- State functions ---------------- */

static int32_t onStateInitialization(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_INITIALIZATION);

    /* In initialization only check gas sensor validity.
     * Require a stable result for several 50ms cycles to avoid
     * false failure decisions directly after reset.
     */
    if (globalGetGasDefect())
    {
        s_initGasOkCnt = 0U;

        if (s_initGasDefectCnt < 255U)
        {
            s_initGasDefectCnt++;
        }

        if (s_initGasDefectCnt >= INIT_STABLE_TICKS)
        {
            (void)applicationSendEvent(EVT_ID_SENSOR_DEFECT);
        }

        return ERROR_OK;
    }
    else
    {
        s_initGasDefectCnt = 0U;

        if (s_initGasOkCnt < 255U)
        {
            s_initGasOkCnt++;
        }

        if (s_initGasOkCnt >= INIT_STABLE_TICKS)
        {
            (void)applicationSendEvent(EVT_ID_INIT_OK);
        }

        return ERROR_OK;
    }
}

static int32_t onEntryPreOperational(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_PREOPERATIONAL);

    s_gasWarnCnt = 0U;
    s_gasEmergCnt = 0U;
    s_waterWarnCnt = 0U;
    s_waterEmergCnt = 0U;
    appUpdateWarningLogic(false);

    return ERROR_OK;
}

static int32_t onStatePreOperational(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_PREOPERATIONAL);
    appUpdateWarningLogic(false);

    return ERROR_OK;
}

static int32_t onEntryOperational(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_OPERATIONAL);

    s_gasWarnCnt = 0U;
    s_gasEmergCnt = 0U;
    s_waterWarnCnt = 0U;
    s_waterEmergCnt = 0U;
    appUpdateWarningLogic(false);

    return ERROR_OK;
}

static int32_t onStateOperational(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_OPERATIONAL);

    {
        int32_t gasPpm = globalGetGasPpm();
        uint16_t waterCm = globalGetWaterLevelCm();

        bool gasWarnCond = (gasPpm > GAS_WARN_PPM);
        bool gasEmergCond = (gasPpm > GAS_EMERG_PPM);

        bool waterWarnCond = (waterCm > WATER_WARN_CM);
        bool waterEmergCond = (waterCm > WATER_EMERG_CM);

        if (gasWarnCond)
        {
            if (s_gasWarnCnt < 0xFFFFU)
            {
                s_gasWarnCnt++;
            }
        }
        else
        {
            s_gasWarnCnt = 0U;
        }

        if (gasEmergCond)
        {
            if (s_gasEmergCnt < 0xFFFFU)
            {
                s_gasEmergCnt++;
            }
        }
        else
        {
            s_gasEmergCnt = 0U;
        }

        if (waterWarnCond)
        {
            if (s_waterWarnCnt < 0xFFFFU)
            {
                s_waterWarnCnt++;
            }
        }
        else
        {
            s_waterWarnCnt = 0U;
        }

        if (waterEmergCond)
        {
            if (s_waterEmergCnt < 0xFFFFU)
            {
                s_waterEmergCnt++;
            }
        }
        else
        {
            s_waterEmergCnt = 0U;
        }
    }

    if ((s_gasWarnCnt >= GAS_WARN_TICKS) || (s_waterWarnCnt >= WATER_WARN_TICKS))
    {
        appUpdateWarningLogic(true);
    }
    else
    {
        appUpdateWarningLogic(false);
    }

    if ((s_gasEmergCnt >= GAS_EMERG_TICKS) || (s_waterEmergCnt >= WATER_EMERG_TICKS))
    {
        (void)applicationSendEvent(EVT_ID_EMERGENCY_TRIGGER);
    }

    return ERROR_OK;
}

static int32_t onEntryEmergency(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_EMERGENCY);

    appUpdateWarningLogic(false);

    return ERROR_OK;
}

static int32_t onStateEmergency(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_EMERGENCY);

    return ERROR_OK;
}

static int32_t onEntryTestMode(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_TESTMODE);

    s_gasWarnCnt = 0U;
    s_gasEmergCnt = 0U;
    s_waterWarnCnt = 0U;
    s_waterEmergCnt = 0U;
    appUpdateWarningLogic(false);

    return ERROR_OK;
}

static int32_t onStateTestMode(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_TESTMODE);
    return ERROR_OK;
}

static int32_t onEntryFailure(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_FAILURE);

    appUpdateWarningLogic(false);

    return ERROR_OK;
}

static int32_t onStateFailure(State_t* pState, int32_t eventID)
{
    (void)pState;
    (void)eventID;

    globalSetAppState(APP_STATE_FAILURE);

    return ERROR_OK;
}
