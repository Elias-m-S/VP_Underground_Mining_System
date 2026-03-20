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
#define UART_RX_BUFFER_SIZE     (128U)

/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/
static void uartPushRxByte(uint8_t byte);


/***** PRIVATE VARIABLES *****************************************************/
static UART_HandleTypeDef gUARTHandle;     //!< Global handle for UART
static volatile uint8_t  s_uartRxBuffer[UART_RX_BUFFER_SIZE];
static volatile uint16_t s_uartRxHead;
static volatile uint16_t s_uartRxTail;

/***** PUBLIC FUNCTIONS ******************************************************/

int32_t uartInitialize(uint32_t baudrate)
{
    int32_t result = UART_ERR_OK;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    s_uartRxHead = 0U;
    s_uartRxTail = 0U;

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
    PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
        Error_Handler();
        return UART_ERR_INIT_FAILURE;
    }

    /* LPUART1 clock enable */
    __HAL_RCC_LPUART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**LPUART1 GPIO Configuration
     * PA2 ------> LPUART1_TX
     * PA3 ------> LPUART1_RX
     */
    GPIO_InitStruct.Pin = USART_TX_PIN | USART_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF12_LPUART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    gUARTHandle.Instance = LPUART1;
    gUARTHandle.Init.BaudRate = baudrate;
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
        return UART_ERR_INIT_FAILURE;
    }

    if (HAL_UARTEx_SetTxFifoThreshold(&gUARTHandle, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
        return UART_ERR_INIT_FAILURE;
    }

    if (HAL_UARTEx_SetRxFifoThreshold(&gUARTHandle, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        Error_Handler();
        return UART_ERR_INIT_FAILURE;
    }

    if (HAL_UARTEx_DisableFifoMode(&gUARTHandle) != HAL_OK)
    {
        Error_Handler();
        return UART_ERR_INIT_FAILURE;
    }

    /* Enable LPUART1 interrupt in NVIC */
    HAL_NVIC_SetPriority(LPUART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LPUART1_IRQn);

    /* Clear possible old error flags */
    gUARTHandle.Instance->ICR =
          USART_ICR_ORECF
        | USART_ICR_FECF
        | USART_ICR_NECF
        | USART_ICR_PECF;

    /* Enable RX-not-empty interrupt and error interrupt directly via registers */
    gUARTHandle.Instance->CR1 |= USART_CR1_RXNEIE_RXFNEIE;
    gUARTHandle.Instance->CR3 |= USART_CR3_EIE;

    return result;
}

int32_t uartSendData(uint8_t* pDataBuffer, int32_t bufferLength)
{
    int32_t result = UART_ERR_OK;
    HAL_StatusTypeDef halStatus;

    if ((pDataBuffer == 0) || (bufferLength <= 0))
    {
        return UART_ERR_TRANSMIT;
    }

    halStatus = HAL_UART_Transmit(&gUARTHandle, pDataBuffer, (uint16_t)bufferLength, HAL_MAX_DELAY);

    if (halStatus != HAL_OK)
    {
        result = UART_ERR_TRANSMIT;
    }

    return result;
}

int32_t uartReceiveData(uint8_t* pDataBuffer, int32_t bufferLength)
{
    int32_t i;

    if ((pDataBuffer == 0) || (bufferLength <= 0))
    {
        return UART_ERR_RECEIVE;
    }

    for (i = 0; i < bufferLength; i++)
    {
        if (s_uartRxHead == s_uartRxTail)
        {
            return UART_ERR_RECEIVE;
        }

        pDataBuffer[i] = s_uartRxBuffer[s_uartRxTail];
        s_uartRxTail = (uint16_t)((s_uartRxTail + 1U) % UART_RX_BUFFER_SIZE);
    }

    return UART_ERR_OK;
}

int32_t uartHasData(int8_t* pHasData)
{
    if (pHasData == 0)
    {
        return UART_ERR_RECEIVE;
    }

    if (s_uartRxHead != s_uartRxTail)
    {
        *pHasData = 1;
    }
    else
    {
        *pHasData = 0;
    }

    return UART_ERR_OK;
}

/**
 * @brief LPUART1 interrupt handler
 */
void LPUART1_IRQHandler(void)
{
    /* Read all available RX bytes from hardware into software ring buffer */
    while ((gUARTHandle.Instance->ISR & USART_ISR_RXNE_RXFNE) != 0U)
    {
        uint8_t rxByte = (uint8_t)(gUARTHandle.Instance->RDR & 0xFFU);
        uartPushRxByte(rxByte);
    }

    /* Clear UART error flags if present */
    if ((gUARTHandle.Instance->ISR & USART_ISR_ORE) != 0U)
    {
        gUARTHandle.Instance->ICR = USART_ICR_ORECF;
    }

    if ((gUARTHandle.Instance->ISR & USART_ISR_FE) != 0U)
    {
        gUARTHandle.Instance->ICR = USART_ICR_FECF;
    }

    if ((gUARTHandle.Instance->ISR & USART_ISR_NE) != 0U)
    {
        gUARTHandle.Instance->ICR = USART_ICR_NECF;
    }

    if ((gUARTHandle.Instance->ISR & USART_ISR_PE) != 0U)
    {
        gUARTHandle.Instance->ICR = USART_ICR_PECF;
    }
}

/***** PRIVATE FUNCTIONS *****************************************************/

static void uartPushRxByte(uint8_t byte)
{
    uint16_t nextHead = (uint16_t)((s_uartRxHead + 1U) % UART_RX_BUFFER_SIZE);

    if (nextHead == s_uartRxTail)
    {
        /* Buffer full: drop oldest byte */
        s_uartRxTail = (uint16_t)((s_uartRxTail + 1U) % UART_RX_BUFFER_SIZE);
    }

    s_uartRxBuffer[s_uartRxHead] = byte;
    s_uartRxHead = nextHead;
}
