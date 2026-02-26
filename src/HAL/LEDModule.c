/******************************************************************************
 * @file LEDModule.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of the LED module
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"

#include "HardwareConfig.h"
#include "LEDModule.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/


/***** PUBLIC FUNCTIONS ******************************************************/


int32_t ledInitialize()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, _7SEGA_PIN | _7SEGB_PIN | _7SEGC_PIN | _7SEGD_PIN | _7SEGE_PIN, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, _7SEG_COM_PIN | LED0_PIN | LED1_PIN | LED3_PIN | LED4_PIN, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, _7SEGF_PIN | _7SEGG_PIN | LED2_PIN, GPIO_PIN_RESET);


    GPIO_InitStruct.Pin 	= _7SEGA_PIN | _7SEGB_PIN | _7SEGC_PIN | _7SEGD_PIN | _7SEGE_PIN;
	GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull 	= GPIO_NOPULL;
	GPIO_InitStruct.Speed	 = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin 	= _7SEG_COM_PIN | LED0_PIN | LED1_PIN | LED3_PIN | BEEP_PIN | LED4_PIN;
	GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull 	= GPIO_NOPULL;
	GPIO_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : PCPin PCPin PCPin */
	GPIO_InitStruct.Pin 	= _7SEGF_PIN | _7SEGG_PIN | LED2_PIN;
	GPIO_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull 	= GPIO_NOPULL;
	GPIO_InitStruct.Speed	= GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	HAL_GPIO_WritePin(BEEP_GPIO_PORT, BEEP_PIN, GPIO_PIN_SET);

    //HAL_GPIO_WritePin(GPIOA, LED4_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LED0_PIN | LED1_PIN | LED3_PIN | LED4_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, LED2_PIN, GPIO_PIN_RESET);

    return LED_ERR_OK;
}

void ledToggleLED(LED_t led)
{
    switch(led)
    {
        case LED0:
            HAL_GPIO_TogglePin(LED0_GPIO_PORT, LED0_PIN);
            break;

        case LED1:
            HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
            break;

        case LED2:
            HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
            break;

        case LED3:
            HAL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_PIN);
            break;

        case LED4:
            HAL_GPIO_TogglePin(LED4_GPIO_PORT, LED4_PIN);
            break;

        default:
            break;
    }
}

void ledSetLED(LED_t led, LED_Status_t ledStatus)
{
    switch(led)
    {
        case LED0:
            HAL_GPIO_WritePin(LED0_GPIO_PORT, LED0_PIN, ledStatus);
            break;

        case LED1:
            HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, ledStatus);
            break;

        case LED2:
            HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, ledStatus);
            break;

        case LED3:
            HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, ledStatus);
            break;

        case LED4:
            HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_PIN, ledStatus);
            break;

        default:
            break;
    }
}

/***** PRIVATE FUNCTIONS *****************************************************/
