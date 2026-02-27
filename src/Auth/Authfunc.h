/******************************************************************************
 * @file Authfunc.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header file for the Authenticator state machine
 *
 * Implements the Authenticator state machine according to the UMMS
 * System Overview specification.
 *
 *****************************************************************************/
#ifndef AUTHFUNC_H_
#define AUTHFUNC_H_

#include <stdint.h>

/***** CONSTANTS *************************************************************/

/** Maximum length of the decryption key (without terminator) */
#define AUTH_KEY_MAX_LEN            8U

/** Timeout in ms before expecting 'A' trigger character (15 s) */
#define AUTH_TIMEOUT_INIT_MS        15000U

/** First key-receive timeout in ms -> turn on LED D1 (10 s after 'A') */
#define AUTH_TIMEOUT_KEY_STAGE1_MS  10000U

/** Second key-receive timeout in ms -> flash LED D1 (30 s after 'A') */
#define AUTH_TIMEOUT_KEY_STAGE2_MS  30000U

/** Final key-receive timeout in ms -> switch to Failure (45 s after 'A') */
#define AUTH_TIMEOUT_KEY_STAGE3_MS  45000U

/** LED flash period in ms (used for stage-2 LED D2 flashing) */
#define AUTH_LED_FLASH_PERIOD_MS    250U

/***** TYPES *****************************************************************/

/**
 * @brief States of the Authenticator state machine (switch-case approach)
 *
 * States follow the UMMS System Overview specification.
 */
typedef enum
{
    AUTH_STATE_BOOTUP,              /**< HW initialisation and preparation   */
    AUTH_STATE_PREPARE_APPLICATION, /**< Wait for 'A', receive key, decrypt  */
    AUTH_STATE_START_APPLICATION,   /**< Copy .auth, decrypt, call verify()  */
    AUTH_STATE_FAILURE              /**< Unrecoverable error – only reset     */
} AuthState_t;

/**
 * @brief Buffer type for the decryption key
 */
typedef struct
{
    uint8_t  data[AUTH_KEY_MAX_LEN]; /**< Raw key bytes                      */
    uint8_t  length;                 /**< Number of bytes received so far     */
    uint8_t  receivingKey;           /**< Flag: 'A' was received, reading key */
} AuthKey_t;

/***** FUNCTION PROTOTYPES ***************************************************/

/**
 * @brief Initialises the authenticator state machine.
 *
 * Must be called once before the main loop.
 */
void Authfunc_Init(void);

/**
 * @brief Cyclic update function – drives the authenticator state machine.
 *
 * Must be called repeatedly from the main loop (no fixed period required;
 * the function uses HAL_GetTick() internally for timeout handling).
 */
void Authfunc_Update(void);

/**
 * @brief Returns the current state of the authenticator state machine.
 *
 * @return Current AuthState_t value.
 */
AuthState_t Authfunc_GetState(void);

/**
 * @brief verify() – placed in the .auth section in RAM.
 *
 * Checks the UMMS signature bytes of the Application binary and, if
 * correct, calls the Application StartHandler via a function pointer.
 *
 * The Linker places this function into the .auth section so that the
 * Authenticator can copy it from Flash to RAM, optionally decrypt it and
 * then execute it from RAM.
 */
void __attribute__((section(".auth"))) auth_verify(void);

#endif /* AUTHFUNC_H_ */