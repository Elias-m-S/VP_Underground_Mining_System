#ifndef AUTHFUNC_H_
#define AUTHFUNC_H_

#include <stdint.h>

/***** CONSTANTS *************************************************************/
#define AUTH_KEY_MAX_LEN            8U
#define AUTH_TIMEOUT_INIT_MS        15000U
#define AUTH_TIMEOUT_KEY_STAGE1_MS  10000U
#define AUTH_TIMEOUT_KEY_STAGE2_MS  30000U
#define AUTH_TIMEOUT_KEY_STAGE3_MS  45000U
#define AUTH_LED_FLASH_PERIOD_MS    250U

/***** ENUMERATIONS **********************************************************/
typedef enum {
    AUTH_STATE_BOOTUP,
    AUTH_STATE_PREPARE_APP,
    AUTH_STATE_START_APP,
    AUTH_STATE_FAILURE
} Auth_State_t;

/***** PROTOTYPES ************************************************************/
// HEAD State Machine
void Auth_Process(void);

// Helper for Sate machine
void Auth_HandleLEDs(Auth_State_t state, uint32_t elapsedTime);
void Auth_CopyandDecrypt(void);

#endif /* AUTHFUNC_H_ */
