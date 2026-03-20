/******************************************************************************
 * @file GlobalObjects.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 ******************************************************************************
 *
 * @brief Header file for global objects used across different modules
 *
 *****************************************************************************/
#ifndef _GLOBAL_OBJECTS_H_
#define _GLOBAL_OBJECTS_H_

/***** INCLUDES **************************************************************/
#include <stdint.h>
#include <stdbool.h>

/***** TYPES *****************************************************************/

typedef enum _AppState_t
{
    APP_STATE_INITIALIZATION = 10,
    APP_STATE_PREOPERATIONAL = 20,
    APP_STATE_OPERATIONAL    = 30,
    APP_STATE_WARNING        = 40, /* optional */
    APP_STATE_EMERGENCY      = 50,
    APP_STATE_TESTMODE       = 60,
    APP_STATE_FAILURE        = 70
} AppState_t;

typedef struct _SensorStatus_t
{
    bool gasDefect;
    bool waterDefect;
    bool inconsistency;
} SensorStatus_t;

typedef struct _SensorValues_t
{
    int32_t  gasPpm;
    uint8_t  waterLevelPercent;
    uint16_t waterLevelCm;
} SensorValues_t;

typedef struct _OutputControl_t
{
    bool showWaterOnDisplay;
    bool blinkWarning;
    bool blinkEmergency;
    bool failureAllOn;
} OutputControl_t;

/***** PROTOTYPES ************************************************************/

/* Init */
void globalObjectsInitialize(void);

/* App state */
void globalSetAppState(AppState_t state);
AppState_t globalGetAppState(void);

/* Sensor values */
void globalSetGasPpm(int32_t gasPpm);
int32_t globalGetGasPpm(void);

void globalSetWaterLevelPercent(uint8_t percent);
uint8_t globalGetWaterLevelPercent(void);

void globalSetWaterLevelCm(uint16_t cm);
uint16_t globalGetWaterLevelCm(void);

/* Sensor status */
void globalSetSensorStatus(SensorStatus_t status);
SensorStatus_t globalGetSensorStatus(void);

void globalSetGasDefect(bool defect);
bool globalGetGasDefect(void);

void globalSetWaterDefect(bool defect);
bool globalGetWaterDefect(void);

void globalSetInconsistency(bool inconsistency);
bool globalGetInconsistency(void);

/* Minimal global "sensor failure" flag (used by SensorHandler) */
void globalSetSensorFailure(bool failure);
bool globalGetSensorFailure(void);

/* Output control */
void globalSetOutputControl(OutputControl_t ctrl);
OutputControl_t globalGetOutputControl(void);

void globalSetShowWaterOnDisplay(bool enable);
bool globalGetShowWaterOnDisplay(void);

void globalSetWarningBlink(bool enable);
bool globalGetWarningBlink(void);

void globalSetEmergencyBlink(bool enable);
bool globalGetEmergencyBlink(void);

void globalSetFailureAllOn(bool enable);
bool globalGetFailureAllOn(void);

#endif /* _GLOBAL_OBJECTS_H_ */
