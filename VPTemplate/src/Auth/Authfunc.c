/***** INCLUDES **************************************************************/
#include "Auth/Authfunc.h"
#include "Auth/AuthVerify.h"

#include "stm32g4xx_hal.h"

#include "UARTModule.h"
#include "LEDModule.h"
#include "Util/Log/LogOutput.h"

#include <string.h>

/***** LINKER SYMBOLS ********************************************************/
extern uint32_t _sloadauth;   /* Load address of .auth in FLASH              */
extern uint32_t _sauth;       /* Run address (start) of .auth in RAM         */
extern uint32_t _eauth;       /* Run address (end)   of .auth in RAM         */

/***** TYPES *****************************************************************/
typedef void (*VerifyFunc_t)(void);

/***** PRIVATE VARIABLES *****************************************************/
static Auth_State_t gState         = AUTH_STATE_BOOTUP;
static uint32_t     gStartTick     = 0U;   /* Set when entering PREPARE_APP  */
static uint8_t      gWaitingForKey = 0U;   /* 1 = 'A' received, collecting   */

/* Key buffer – also used by Auth_DecryptSection                              */
static uint8_t      gKeyBuf[AUTH_KEY_MAX_LEN + 1U];
static uint8_t      gKeyLen        = 0U;

/***** PUBLIC FUNCTIONS ******************************************************/

void Auth_CopyandDecrypt(void)
{
    uint8_t *dst = (uint8_t *)&_sauth;
    uint8_t *src = (uint8_t *)&_sloadauth;

    /* Byte length of the .auth section derived from linker symbols */
    size_t authLen = (size_t)((uint8_t *)&_eauth - (uint8_t *)&_sauth);

    /* --- 1. Copy .auth section from FLASH load address to RAM run address -- */
    memcpy(dst, src, authLen);

    //outputLog("[AUTH] .auth section copied to RAM\r\n");

    for(size_t i = 0; i < authLen; i++)
	{
		dst[i] ^= gKeyBuf[i % gKeyLen];
	}

    __DSB();
    __ISB();

}

void Auth_HandleLEDs(Auth_State_t state, uint32_t elapsedTime)
{
	 if (state == AUTH_STATE_FAILURE)
	    {
	        ledSetLED(LED4, LED_ON);
	        ledSetLED(LED0, LED_OFF);
	        ledSetLED(LED1, LED_OFF);
	        ledSetLED(LED2, LED_OFF);
	        ledSetLED(LED3, LED_OFF);
	        return;
	    }

	    /* D0: authenticator active */
	    ledSetLED(LED0, LED_ON);

	    /* D1: on after 10 s, flashing after 30 s */
	    if (elapsedTime >= AUTH_TIMEOUT_KEY_STAGE2_MS)
	    {
	        /* 30 s reached → D1 flashing */
	        uint32_t phase = (elapsedTime / AUTH_LED_FLASH_PERIOD_MS) % 2U;
	        ledSetLED(LED1, (phase == 0U) ? LED_ON : LED_OFF);
	    }
	    else if (elapsedTime >= AUTH_TIMEOUT_KEY_STAGE1_MS)
	    {
	        /* 10 s reached → D1 on */
	        ledSetLED(LED1, LED_ON);
	    }
	    else
	    {
	        ledSetLED(LED1, LED_OFF);
	    }
}

void Auth_Process(void)
{
    switch (gState)
    {
        case AUTH_STATE_BOOTUP:
        {
            outputLog("[AUTH] Authenticator started - waiting for key\r\n");

            /* Record start time for the key-receive timeout */
            gStartTick     = HAL_GetTick();
            gWaitingForKey = 0U;
            gKeyLen        = 0U;
            memset(gKeyBuf, 0, sizeof(gKeyBuf));

            gState = AUTH_STATE_PREPARE_APP;
            break;
        }

        case AUTH_STATE_PREPARE_APP:
        {
            uint32_t elapsed = HAL_GetTick() - gStartTick;

            /* Update LEDs every call */
            Auth_HandleLEDs(gState, elapsed);

            /* Hard 45 s total timeout */
            if (elapsed >= AUTH_TIMEOUT_KEY_STAGE3_MS)
            {
                outputLog("[AUTH] Total timeout (45s) - FAILURE\r\n");
                gState = AUTH_STATE_FAILURE;
                break;
            }

            /* 15 s initial timeout waiting for 'A' */
            if (!gWaitingForKey && (elapsed >= AUTH_TIMEOUT_INIT_MS))
            {
                outputLog("[AUTH] Initial timeout (15s) - FAILURE\r\n");
                gState = AUTH_STATE_FAILURE;
                break;
            }

            /* Poll UART */
            int8_t hasData = 0;
            uartHasData(&hasData);
            if (!hasData)
            {
                break;
            }

            uint8_t byte = 0U;
            uartReceiveData(&byte, 1);

            if (!gWaitingForKey)
            {
                /* Wait for the 'A' trigger character */
                if (byte == 'A')
                {
                    gWaitingForKey = 1U;
                    outputLog("[AUTH] 'A' received - send key followed by \\n\r\n");
                }
            }
            else
            {
                /* Accept '\n' (Linux) and '\r' (Enter on most terminals) */
                if (byte == '\n' || byte == '\r')
                {
                    gKeyBuf[gKeyLen] = '\0';

                    if (gKeyLen == 0U)
                    {
                        outputLog("[AUTH] Empty key - FAILURE\r\n");
                        gState = AUTH_STATE_FAILURE;
                    }
                    else
                    {
                        outputLog("[AUTH] Key received - preparing application\r\n");

                        /* Copy .auth section from FLASH to RAM, then decrypt */
                        Auth_CopyandDecrypt();
                        gState = AUTH_STATE_START_APP;
                    }
                }
                else if (gKeyLen < AUTH_KEY_MAX_LEN)
                {
                    gKeyBuf[gKeyLen++] = byte;
                }
                else
                {
                    outputLog("[AUTH] Key too long - FAILURE\r\n");
                    gState = AUTH_STATE_FAILURE;
                }
            }
            break;
        }

        /* ------------------------------------------------------------------ */
        case AUTH_STATE_START_APP:
        {
            /* Turn off authenticator LEDs */
            ledSetLED(LED0, LED_OFF);
            ledSetLED(LED1, LED_OFF);
            ledSetLED(LED2, LED_OFF);
            ledSetLED(LED4, LED_OFF);

            verify();
            /* verify() only returns if signature check failed */
            outputLog("[AUTH] verify() returned - FAILURE\r\n");
            gState = AUTH_STATE_FAILURE;
            break;
        }

        /* ------------------------------------------------------------------ */
        case AUTH_STATE_FAILURE:
        default:
        {
            /* LED D4 on – updated once per call via HandleLEDs in FAILURE    */
            Auth_HandleLEDs(AUTH_STATE_FAILURE, AUTH_TIMEOUT_KEY_STAGE3_MS);
            break;
        }
    }
}
