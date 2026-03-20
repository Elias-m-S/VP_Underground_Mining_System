/***** INCLUDES **************************************************************/
#include <stdint.h>
#include <string.h>
#include "stdbool.h"
#include "stm32g4xx_hal.h"
#include "Util/Log/LogOutput.h"

/***** CONSTANTS *************************************************************/
/* Application .signature section address (see App.ld) */
#define APP_SIGNATURE_ADDR          0x08010000U
/* Expected signature: "UMMS" in little-endian 32-bit */

/* Reset handler (StartHandler) is at offset +4 in the vector table */
#define APP_STARTUP_HANDLER_ADDR    0x08010204U

/* Typedef for the application entry function pointer */
typedef void (*AppEntry_t)(void);

static const char expectedSignature[] = "UMMS";

/***** PUBLIC FUNCTIONS ******************************************************/

/**
 * @brief  Single .auth-section function: verifies the application signature
 *         and, if correct, tears down the Authenticator and jumps into the
 *         Application's StartHandler().
 **/

__attribute__((section(".auth")))

void verify(void)
{

	 /* --- 1. Check application signature --------------------------------------- */
	if(memcmp(expectedSignature, (const char*)APP_SIGNATURE_ADDR, 4u) == 0)
	{

    /* --- 2. Disable all interrupts ---------------------------------------- */
    __disable_irq();

    /* --- 3. Read StartHandler address and jump ----------------------------- */
    uint32_t * startUpPtr = (uint32_t*)(APP_STARTUP_HANDLER_ADDR);
    AppEntry_t app = (AppEntry_t)*(startUpPtr);
    app();
	}

    /* Should never be reached */
    while (1) { }
}
