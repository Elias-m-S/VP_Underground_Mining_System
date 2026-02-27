/******************************************************************************
 * @file Authfunc.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of the Authenticator state machine.
 *
 * State machine overview (switch-case approach, as per UMMS spec):
 *
 *   Bootup
 *     └─► PrepareApplication   (HW init OK)
 *           ├─► Failure         (timeout: 'A' not received within 15 s, or
 *           │                   key not fully received within 45 s)
 *           └─► StartApplication (key received completely)
 *                 └─► (no return – application is started)
 *
 * The .auth section contains auth_verify() which is copied from Flash to
 * RAM, optionally XOR-decrypted and then executed from RAM. auth_verify()
 * reads the UMMS application signature and, if correct, jumps into the
 * Application via a function pointer to StartApp_Handler.
 *
 *****************************************************************************/

/***** INCLUDES **************************************************************/
#include "Authfunc.h"

#include "stm32g4xx_hal.h"

#include "UARTModule.h"
#include "LEDModule.h"
#include "Util/Log/LogOutput.h"

#include <string.h>

/***** PRIVATE CONSTANTS *****************************************************/

/**
 * @brief Flash address of the Application binary (fixed, as per memory map).
 *
 * Application starts at 0x08010000 (128 KiB after the Authenticator).
 * The StartApp_Handler is the very first function in the application vector
 * table (offset 4 = reset vector). In an ARM Cortex-M4 vector table the
 * reset vector is at offset 4. Thumb-mode addresses have bit 0 set.
 *
 * Adjust this address if the application vector table layout changes.
 */
#define APP_START_ADDRESS           0x08010000U
#define APP_RESET_VECTOR_OFFSET     0x00000004U

/**
 * @brief 'UMMS' encoded as a 4-byte little-endian word stored in the
 * .signature section of the Application binary (right after the vector
 * table / at the beginning of the application flash area).
 *
 * The verify() function checks this value before launching the app.
 */
#define APP_SIGNATURE_VALUE         0x534D4D55U  /* 'U','M','M','S' */

/**
 * @brief Address of the .signature section inside the application.
 * Adjust if the linker places the section at a different address.
 */
#define APP_SIGNATURE_ADDRESS       (APP_START_ADDRESS + 0x0200U)

/***** PRIVATE TYPES *********************************************************/

/** Function-pointer type for calling the Application StartApp_Handler */
typedef void (*AppStartFunc_t)(void);

/***** EXTERNAL LINKER SYMBOLS ***********************************************/
/*
 * These symbols are provided by Auth.ld and define the load address (in
 * Flash) and VMA address (in RAM) of the .auth section.
 */
extern uint32_t _sauth;       /**< Start of .auth section in RAM (VMA)       */
extern uint32_t _eauth;       /**< End   of .auth section in RAM (VMA)       */
extern uint32_t _sloadauth;   /**< Load address of .auth section in Flash    */

/***** PRIVATE VARIABLES *****************************************************/

/** Current state of the Authenticator state machine */
static AuthState_t gCurrentState;

/** Decryption key buffer */
static AuthKey_t   gKey;

/**
 * @brief Timestamp (HAL_GetTick) captured when entering PrepareApplication.
 *
 * Used to calculate elapsed time for all three timeout stages.
 */
static uint32_t gPrepareStartTick;

/**
 * @brief Timestamp captured when 'A' was received (key receive start).
 *
 * Used to measure the key-receive timeout relative to the 'A' character.
 */
static uint32_t gKeyStartTick;

/** Flag: stage-1 timeout LED already activated */
static uint8_t gStage1Active;
/** Flag: stage-2 timeout LED already activated */
static uint8_t gStage2Active;

/** Last HAL tick at which the stage-2 LED was toggled (for flashing) */
static uint32_t gLedFlashTick;

/***** PRIVATE FUNCTION PROTOTYPES *******************************************/
static void enterFailureState(void);
static void copyAuthSection(void);

#ifdef ENABLE_ENCRYPTION
static void decryptAuthSection(void);
#endif

/***** PUBLIC FUNCTIONS ******************************************************/

void Authfunc_Init(void)
{
    gCurrentState    = AUTH_STATE_BOOTUP;
    gStage1Active    = 0U;
    gStage2Active    = 0U;
    gLedFlashTick    = 0U;
    gPrepareStartTick = 0U;
    gKeyStartTick    = 0U;

    (void)memset(&gKey, 0, sizeof(gKey));

    /* LED D0 on – Authenticator active */
    ledSetLED(LED0, LED_ON);
    ledSetLED(LED1, LED_OFF);
    ledSetLED(LED2, LED_OFF);
    ledSetLED(LED3, LED_OFF);
    ledSetLED(LED4, LED_OFF);
}

void Authfunc_Update(void)
{
    uint32_t now = HAL_GetTick();

    switch (gCurrentState)
    {
        /* ------------------------------------------------------------------ */
        case AUTH_STATE_BOOTUP:
        /* ------------------------------------------------------------------ */
            /*
             * HW initialisation is handled in main() before Authfunc_Init()
             * is called. Therefore we transition immediately to
             * PrepareApplication and record the entry timestamp.
             */
            outputLogf("[AUTH] Bootup complete – entering PrepareApplication\r\n");
            gPrepareStartTick = HAL_GetTick();
            gCurrentState = AUTH_STATE_PREPARE_APPLICATION;
            break;

        /* ------------------------------------------------------------------ */
        case AUTH_STATE_PREPARE_APPLICATION:
        /* ------------------------------------------------------------------ */
        {
            uint32_t elapsed = now - gPrepareStartTick;

            /* ---- Phase A: waiting for the trigger character 'A' ---------- */
            if (!gKey.receivingKey)
            {
                /* Check for the 15-second trigger timeout */
                if (elapsed >= AUTH_TIMEOUT_INIT_MS)
                {
                    outputLogf("[AUTH] Timeout: 'A' not received within 15 s\r\n");
                    enterFailureState();
                    break;
                }

                /* Non-blocking single-byte peek */
                int8_t hasData = 0;
                uartHasData(&hasData);
                if (hasData)
                {
                    uint8_t ch = 0U;
                    uartReceiveData(&ch, 1);
                    if (ch == 'A')
                    {
                        outputLogf("[AUTH] Trigger 'A' received – waiting for key\r\n");
                        gKey.receivingKey = 1U;
                        gKey.length       = 0U;
                        gKeyStartTick     = HAL_GetTick();
                        gStage1Active     = 0U;
                        gStage2Active     = 0U;
                    }
                }
            }
            /* ---- Phase B: receiving the decryption key ------------------- */
            else
            {
                uint32_t keyElapsed = now - gKeyStartTick;

                /* Stage 1: 10 s → turn on LED D1 */
                if (!gStage1Active && (keyElapsed >= AUTH_TIMEOUT_KEY_STAGE1_MS))
                {
                    outputLogf("[AUTH] Stage-1 timeout (10 s) – LED D1 on\r\n");
                    ledSetLED(LED1, LED_ON);
                    gStage1Active = 1U;
                }

                /* Stage 2: 30 s → flash LED D2 */
                if (!gStage2Active && (keyElapsed >= AUTH_TIMEOUT_KEY_STAGE2_MS))
                {
                    outputLogf("[AUTH] Stage-2 timeout (30 s) – LED D2 flashing\r\n");
                    gLedFlashTick = now;
                    gStage2Active = 1U;
                }

                /* Flash LED D2 at AUTH_LED_FLASH_PERIOD_MS intervals */
                if (gStage2Active)
                {
                    if ((now - gLedFlashTick) >= AUTH_LED_FLASH_PERIOD_MS)
                    {
                        ledToggleLED(LED2);
                        gLedFlashTick = now;
                    }
                }

                /* Stage 3: 45 s → Failure */
                if (keyElapsed >= AUTH_TIMEOUT_KEY_STAGE3_MS)
                {
                    outputLogf("[AUTH] Stage-3 timeout (45 s) – switching to Failure\r\n");
                    enterFailureState();
                    break;
                }

                /* Try to read the next key byte (non-blocking) */
                int8_t hasData = 0;
                uartHasData(&hasData);
                if (hasData)
                {
                    uint8_t ch = 0U;
                    uartReceiveData(&ch, 1);

                    if (ch == '\n')
                    {
                        /* Key reception complete */
                        outputLogf("[AUTH] Key received (%u bytes) – proceeding\r\n",
                                   (unsigned)gKey.length);

                        /* Turn off timeout LEDs */
                        ledSetLED(LED1, LED_OFF);
                        ledSetLED(LED2, LED_OFF);

                        gCurrentState = AUTH_STATE_START_APPLICATION;
                    }
                    else if (gKey.length < AUTH_KEY_MAX_LEN)
                    {
                        gKey.data[gKey.length++] = ch;
                    }
                    /* Silently ignore bytes beyond AUTH_KEY_MAX_LEN */
                }
            }
            break;
        }

        /* ------------------------------------------------------------------ */
        case AUTH_STATE_START_APPLICATION:
        /* ------------------------------------------------------------------ */
            outputLogf("[AUTH] Copying .auth section to RAM\r\n");

            /* 1. Copy the .auth section from Flash to RAM */
            copyAuthSection();

#ifdef ENABLE_ENCRYPTION
            /* 2. Decrypt the .auth section in RAM using the received key */
            outputLogf("[AUTH] Decrypting .auth section\r\n");
            decryptAuthSection();
#endif

            /* 3. Call the verify() function that now lives in RAM.
             *    auth_verify() will check the application signature and,
             *    if correct, jump into the Application binary.
             *    This function does not return on success.            */
            outputLogf("[AUTH] Calling auth_verify()\r\n");
            auth_verify();

            /*
             * If we reach this point, verify() returned – which means the
             * signature check failed. Switch to Failure state.
             */
            outputLogf("[AUTH] verify() returned – signature mismatch\r\n");
            enterFailureState();
            break;

        /* ------------------------------------------------------------------ */
        case AUTH_STATE_FAILURE:
        /* ------------------------------------------------------------------ */
            /*
             * Stay in Failure state forever. A system reset is the only way
             * out, as per the UMMS specification.
             * LED D4 is kept on (set in enterFailureState).
             */
            break;

        /* ------------------------------------------------------------------ */
        default:
        /* ------------------------------------------------------------------ */
            enterFailureState();
            break;
    }
}

AuthState_t Authfunc_GetState(void)
{
    return gCurrentState;
}

/***** .auth SECTION FUNCTION ************************************************/

/**
 * @brief Verify the Application signature and start the Application.
 *
 * This function is placed in the .auth section so it resides in RAM after
 * the Authenticator has copied (and optionally decrypted) that section.
 *
 * Steps:
 *   1. Read the 4-byte 'UMMS' signature from the Application flash area.
 *   2. If the signature matches, retrieve the reset-vector address from
 *      the Application vector table and call it via a function pointer.
 *   3. If the signature does not match, return (caller switches to Failure).
 *
 * @note The function must not use any function from the .text section that
 *       might not be available at call time, and must not rely on RAM data
 *       that has been overwritten. Keep it self-contained.
 */
void __attribute__((section(".auth"))) __attribute__((noinline)) auth_verify(void)
{
    /* Read the 4-byte UMMS signature from the application flash memory */
    const uint32_t* pSignature = (const uint32_t*)APP_SIGNATURE_ADDRESS;

    if (*pSignature != APP_SIGNATURE_VALUE)
    {
        /* Signature mismatch – return to caller which will enter Failure */
        return;
    }

    /*
     * Signature is valid. Retrieve the StartApp_Handler address from the
     * Application vector table (reset vector, offset 4).
     * The reset vector on Cortex-M4 contains a Thumb address (bit 0 set).
     */
    const uint32_t* pVectorTable   = (const uint32_t*)APP_START_ADDRESS;
    uint32_t        startHandlerAddr = pVectorTable[1]; /* index 1 = reset vector */

    /* Create a function pointer and call the Application entry point */
    AppStartFunc_t startApp = (AppStartFunc_t)(startHandlerAddr);
    startApp();

    /* Should never reach here */
    while (1)
    {
    }
}

/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief Transitions to the Failure state and configures the LEDs accordingly.
 *
 * Failure LED mapping (per UMMS spec):
 *   LED D0 – off  (Authenticator no longer "active")
 *   LED D1 – off
 *   LED D2 – off
 *   LED D3 – off
 *   LED D4 – on   (System Failure)
 */
static void enterFailureState(void)
{
    ledSetLED(LED0, LED_OFF);
    ledSetLED(LED1, LED_OFF);
    ledSetLED(LED2, LED_OFF);
    ledSetLED(LED3, LED_OFF);
    ledSetLED(LED4, LED_ON);

    gCurrentState = AUTH_STATE_FAILURE;
}

/**
 * @brief Copies the .auth section from Flash (load address) to RAM (VMA).
 *
 * Uses the linker-generated symbols _sauth, _eauth and _sloadauth.
 */
static void copyAuthSection(void)
{
    uint32_t* pDest  = &_sauth;
    uint32_t* pEnd   = &_eauth;
    uint32_t* pSrc   = &_sloadauth;

    while (pDest < pEnd)
    {
        *pDest++ = *pSrc++;
    }
}

#ifdef ENABLE_ENCRYPTION
/**
 * @brief Decrypts the .auth section in RAM using a simple XOR with the
 *        received key.
 *
 * The key is applied byte-by-byte, wrapping around when the end of the
 * key is reached (key length bytes cycling).
 *
 * Only compiled when ENABLE_ENCRYPTION is defined in the build system.
 */
static void decryptAuthSection(void)
{
    if (gKey.length == 0U)
    {
        return; /* Nothing to decrypt with an empty key */
    }

    uint8_t* pByte    = (uint8_t*)&_sauth;
    uint8_t* pBytEnd  = (uint8_t*)&_eauth;
    uint8_t  keyIdx   = 0U;

    while (pByte < pBytEnd)
    {
        *pByte ^= gKey.data[keyIdx];
        pByte++;
        keyIdx = (uint8_t)((keyIdx + 1U) % gKey.length);
    }
}
#endif /* ENABLE_ENCRYPTION */
