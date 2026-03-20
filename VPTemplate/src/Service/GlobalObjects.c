/******************************************************************************
 * @file GlobalObjects.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 ******************************************************************************
 *
 * @brief Implementation file for global objects used across different modules
 *
 *****************************************************************************/

/***** INCLUDES **************************************************************/
#include "Service/GlobalObjects.h"

/***** PRIVATE VARIABLES *****************************************************/
/* Global application state used by the main state machine */
static AppState_t s_appState;

/* Struct containing the current processed sensor values */
static SensorValues_t  s_sensorValues;

/* Struct containing the current sensor status flags (defects, inconsistencies) */
static SensorStatus_t  s_sensorStatus;

/* Struct controlling all logical output indicators (LEDs, display behaviour) */
static OutputControl_t s_outputControl;

/* Minimal global "sensor failure" flag (used by SensorHandler) */
static bool s_sensorFailure;

/***** PUBLIC FUNCTIONS ******************************************************/

/* Initialize all global objects with default startup values */
void globalObjectsInitialize(void)
{
    /* Initial system state after application start */
    s_appState = APP_STATE_INITIALIZATION;

    /* Reset sensor values */
    s_sensorValues.gasPpm = 0;
    s_sensorValues.waterLevelPercent = 0;
    s_sensorValues.waterLevelCm = 0;

    /* Reset sensor status flags */
    s_sensorStatus.gasDefect = false;
    s_sensorStatus.waterDefect = false;
    s_sensorStatus.inconsistency = false;

    /* Reset output control flags */
    s_outputControl.showWaterOnDisplay = false;
    s_outputControl.blinkWarning = false;
    s_outputControl.blinkEmergency = false;
    s_outputControl.failureAllOn = false;

    s_sensorFailure = false;
}

/* Set current application state */
void globalSetAppState(AppState_t state)
{
    s_appState = state;
}

/* Get current application state */
AppState_t globalGetAppState(void)
{
    return s_appState;
}

/* Store calculated gas concentration in ppm */
void globalSetGasPpm(int32_t gasPpm)
{
    s_sensorValues.gasPpm = gasPpm;
}

/* Retrieve current gas concentration */
int32_t globalGetGasPpm(void)
{
    return s_sensorValues.gasPpm;
}

/* Set water level in percent (value is clamped to 100%) */
void globalSetWaterLevelPercent(uint8_t percent)
{
    if (percent > 100U)
        percent = 100U;

    s_sensorValues.waterLevelPercent = percent;
}

/* Get current water level percentage */
uint8_t globalGetWaterLevelPercent(void)
{
    return s_sensorValues.waterLevelPercent;
}

/* Store water level in centimeters */
void globalSetWaterLevelCm(uint16_t cm)
{
    s_sensorValues.waterLevelCm = cm;
}

/* Retrieve water level in centimeters */
uint16_t globalGetWaterLevelCm(void)
{
    return s_sensorValues.waterLevelCm;
}

/* Update complete sensor status structure */
void globalSetSensorStatus(SensorStatus_t status)
{
    s_sensorStatus = status;
}

/* Retrieve current sensor status information */
SensorStatus_t globalGetSensorStatus(void)
{
    return s_sensorStatus;
}

/* Set gas sensor defect flag */
void globalSetGasDefect(bool defect)
{
    s_sensorStatus.gasDefect = defect;
}

/* Get gas sensor defect flag */
bool globalGetGasDefect(void)
{
    return s_sensorStatus.gasDefect;
}

/* Set water sensor defect flag */
void globalSetWaterDefect(bool defect)
{
    s_sensorStatus.waterDefect = defect;
}

/* Get water sensor defect flag */
bool globalGetWaterDefect(void)
{
    return s_sensorStatus.waterDefect;
}

/* Set inconsistency flag between redundant sensors */
void globalSetInconsistency(bool inconsistency)
{
    s_sensorStatus.inconsistency = inconsistency;
}

/* Get inconsistency status */
bool globalGetInconsistency(void)
{
    return s_sensorStatus.inconsistency;
}

/* Minimal global "sensor failure" flag (used by SensorHandler) */
void globalSetSensorFailure(bool failure)
{
    s_sensorFailure = failure;
}

/* Get global sensor failure status */
bool globalGetSensorFailure(void)
{
    return s_sensorFailure;
}

/* Update complete output control structure */
void globalSetOutputControl(OutputControl_t ctrl)
{
    s_outputControl = ctrl;
}

/* Retrieve output control structure */
OutputControl_t globalGetOutputControl(void)
{
    return s_outputControl;
}

/* Enable or disable display of water level */
void globalSetShowWaterOnDisplay(bool enable)
{
    s_outputControl.showWaterOnDisplay = enable;
}

/* Get display enable state */
bool globalGetShowWaterOnDisplay(void)
{
    return s_outputControl.showWaterOnDisplay;
}

/* Enable or disable warning LED blinking */
void globalSetWarningBlink(bool enable)
{
    s_outputControl.blinkWarning = enable;
}

/* Get warning blink status */
bool globalGetWarningBlink(void)
{
    return s_outputControl.blinkWarning;
}

/* Enable or disable emergency LED blinking */
void globalSetEmergencyBlink(bool enable)
{
    s_outputControl.blinkEmergency = enable;
}

/* Get emergency blink status */
bool globalGetEmergencyBlink(void)
{
    return s_outputControl.blinkEmergency;
}

/* Enable or disable failure indicator state */
void globalSetFailureAllOn(bool enable)
{
    s_outputControl.failureAllOn = enable;
}

/* Get failure indicator state */
bool globalGetFailureAllOn(void)
{
    return s_outputControl.failureAllOn;
}
