/******************************************************************************
 * @file LogOutput.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of Log debug outputs
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include <stdint.h>
#include <string.h>

#include "Util/Log/printf.h"

#include "stm32g4xx_hal.h"
#include "UARTModule.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/
#define MAX_OUTPUT_BUFFER         128


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/
static int internalFormattedOutput(const char* format, va_list va);


/***** PRIVATE VARIABLES *****************************************************/
/**
 * @brief Buffer used to store the strings create by internalFormattedOutput() calls
 * The buffer has a total size of MAX_OUTPUT_BUFFER. Therefore only MAX_OUTPUT_BUFFER - 1
 * chars can be stored.
 *
 */
static char gOutputBuffer[MAX_OUTPUT_BUFFER];


/***** PUBLIC FUNCTIONS ******************************************************/

void outputLog(const char* msg)
{
    // Send buffer to UART
    int32_t bufferLength = strlen(msg);
    uartSendData((uint8_t*)msg, bufferLength);
}


int outputLogf(const char* format, ...)
{
    int ret = 0;
    va_list va;
    va_start(va, format);

    ret = internalFormattedOutput(format, va);
    va_end(va);

    return ret;
}




/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief Formats a string according the format string spec and the arguments and
 * sends it to the UART
 *
 * @param format    Format string spec according printf()
 * @param va        Variable argument list
 *
 * @return Returns number of chars prepared for the string
 */
static int internalFormattedOutput(const char* format, va_list va)
{
    int ret = 0;
    ret = vsnprintf_(gOutputBuffer, MAX_OUTPUT_BUFFER, format, va);

    if (ret > 0 && ret <= MAX_OUTPUT_BUFFER)
    {
        uartSendData((uint8_t*)gOutputBuffer, ret);
    }

    return ret;
}

