/******************************************************************************
 * @file LEDHandler.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 ******************************************************************************
 *
 * @brief Implementation file for LED/Display handler
 *
 *****************************************************************************/

#include "Service/LEDHandler.h"

#include "HAL/LEDModule.h"
#include "HAL/DisplayModule.h"

#include "Service/GlobalObjects.h"

#define BLINK_TOGGLE_TICKS   (5U)   /* 5*50ms = 250ms */

static uint8_t s_blinkCnt;
static uint8_t s_blinkOn;

void ledHandlerInitialize(void)
{
    s_blinkCnt = 0U;
    s_blinkOn = 0U;

    /* Default outputs */
    ledSetLED(LED0, LED_OFF);
    ledSetLED(LED1, LED_OFF);
    ledSetLED(LED2, LED_OFF);
    ledSetLED(LED3, LED_OFF);
    ledSetLED(LED4, LED_OFF);

    (void)displayShowDigit(LEFT_DISPLAY, DIGIT_DASH);
    (void)displayShowDigit(RIGHT_DISPLAY, DIGIT_DASH);
}

static void updateBlink(void)
{
    s_blinkCnt++;
    if (s_blinkCnt >= BLINK_TOGGLE_TICKS)
    {
        s_blinkCnt = 0U;
        s_blinkOn = (uint8_t)(!s_blinkOn);
    }
}

void ledHandlerCycle(void)
{
    updateBlink();

    AppState_t st = globalGetAppState();

    bool gasDef  = globalGetGasDefect();
    bool watDef  = globalGetWaterDefect();
    bool incDef  = globalGetInconsistency();

    bool sensorFailure = (gasDef || watDef || incDef);

    /* D4: sensor failure */
    ledSetLED(LED4, sensorFailure ? LED_ON : LED_OFF);

    /* D3: test mode */
    ledSetLED(LED3, (st == APP_STATE_TESTMODE) ? LED_ON : LED_OFF);

    /* D2: system failure */
    ledSetLED(LED2, (st == APP_STATE_FAILURE) ? LED_ON : LED_OFF);

    /* D0: operational indicator */
    ledSetLED(LED0, (st == APP_STATE_OPERATIONAL) ? LED_ON : LED_OFF);

    /* D1: alarm indicator
     * - Emergency: flashing
     * - Warning: ON (warning is a flag, not a state)
     */
    if (st == APP_STATE_EMERGENCY)
    {
        ledSetLED(LED1, (s_blinkOn != 0U) ? LED_ON : LED_OFF);
    }
    else if (globalGetWarningBlink())
    {
        ledSetLED(LED1, LED_ON);
    }
    else
    {
        ledSetLED(LED1, LED_OFF);
    }

    /* Display
     * - Operational: show water level in cm (last 2 digits)
     * - else: "-"
     */
    if (st == APP_STATE_OPERATIONAL)
    {
        uint16_t cm = globalGetWaterLevelCm();

        uint8_t hundreds = (uint8_t)((cm / 100U) % 10U);
        uint8_t tens     = (uint8_t)((cm / 10U) % 10U);

        (void)displayShowDigit(LEFT_DISPLAY,  (int8_t)hundreds);
        (void)displayShowDigit(RIGHT_DISPLAY, (int8_t)tens);
    }
    else
    {
        (void)displayShowDigit(LEFT_DISPLAY, DIGIT_DASH);
        (void)displayShowDigit(RIGHT_DISPLAY, DIGIT_DASH);
    }
}
