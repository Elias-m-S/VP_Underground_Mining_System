/******************************************************************************
 * @file UARTModule.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of the UART Module
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"

#include "System.h"
#include "HardwareConfig.h"
#include "UARTModule.h"

/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/
static UART_HandleTypeDef gUARTHandle;     //!< Global handle for UART 2

/***** PUBLIC FUNCTIONS ******************************************************/


int32_t uartInitialize(uint32_t baudrate)
{
    int32_t result = UART_ERR_OK;

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
    }

    /* LPUART1 clock enable */
    __HAL_RCC_LPUART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**LPUART1 GPIO Configuration
     PA2     ------> LPUART1_TX
     PA3     ------> LPUART1_RX
     */
    GPIO_InitStruct.Pin = USART_TX_PIN | USART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF12_LPUART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /**USART2 GPIO Configuration
     PA2     ------> USART2_TX
     PA3     ------> USART2_RX
     */
    gUARTHandle.Instance = LPUART1;
    gUARTHandle.Init.BaudRate = 115200;
    gUARTHandle.Init.WordLength = UART_WORDLENGTH_8B;
    gUARTHandle.Init.StopBits = UART_STOPBITS_1;
    gUARTHandle.Init.Parity = UART_PARITY_NONE;
    gUARTHandle.Init.Mode = UART_MODE_TX_RX;
    gUARTHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    gUARTHandle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    gUARTHandle.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    gUARTHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_Init(&gUARTHandle) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_UARTEx_SetTxFifoThreshold(&gUARTHandle, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_UARTEx_SetRxFifoThreshold(&gUARTHandle, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_UARTEx_DisableFifoMode(&gUARTHandle) != HAL_OK)
    {
        Error_Handler();
    }

    return result;
}

int32_t uartSendData(uint8_t* pDataBuffer, int32_t bufferLength)
{
    int32_t result = UART_ERR_OK;

    HAL_StatusTypeDef halStatus = HAL_UART_Transmit(&gUARTHandle, pDataBuffer, bufferLength, HAL_MAX_DELAY);

    if (halStatus != HAL_OK )
    {
        result = UART_ERR_TRANSMIT;
    }

    return result;
}

int32_t uartReceiveData(uint8_t* pDataBuffer, int32_t bufferLength)
{
    int32_t result = UART_ERR_OK;

    HAL_StatusTypeDef halStatus = HAL_UART_Receive(&gUARTHandle, pDataBuffer, bufferLength, HAL_MAX_DELAY);

    if (halStatus != HAL_OK )
    {
        result = UART_ERR_RECEIVE;
    }

    return result;
}

int32_t uartHasData(int8_t* pHasData)
{
	int32_t result = UART_ERR_OK;

	if (__HAL_UART_GET_FLAG(&gUARTHandle, UART_FLAG_RXNE)==SET)
	{
		*pHasData = 1;
	}
	else
	{
		*pHasData = 0;
	}

	return result;
}

/***** PRIVATE FUNCTIONS *****************************************************/
