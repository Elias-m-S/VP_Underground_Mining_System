/******************************************************************************
 * @file TimerModule.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of the Timer Module to trigger ADC conversion
 * based on DMA
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"

#include "System.h"
#include "HardwareConfig.h"
#include "TimerModule.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/
static TIM_HandleTypeDef gTimer3Handle;         //! Global handle for Timer 3 (TIM3) peripheral


/***** PUBLIC FUNCTIONS ******************************************************/

int32_t timerInitialize()
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    /* Initialize the Timer to get a 10ms cycle
     * 128 MHz Peripheral Clock ==> divided by Prescaler ==> 128e6 / 1280 = 100000
     * Timer Frequency of 100.000 = 100kHz to count to 1000 (0-999) and then generate interrupt
     * ==> 100kHz / 1000 = 100Hz ==> 10ms
    */
    gTimer3Handle.Instance                  = TIM3;
    gTimer3Handle.Init.Prescaler            = 1280;
    gTimer3Handle.Init.CounterMode          = TIM_COUNTERMODE_UP;
    gTimer3Handle.Init.Period               = 999;
    gTimer3Handle.Init.ClockDivision        = TIM_CLOCKDIVISION_DIV1;
    gTimer3Handle.Init.AutoReloadPreload    = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(&gTimer3Handle) != HAL_OK)
    {
        return TIMER_ERR_INIT_FAILURE;
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

    if (HAL_TIM_ConfigClockSource(&gTimer3Handle, &sClockSourceConfig) != HAL_OK)
    {
        return TIMER_ERR_INIT_FAILURE;
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(&gTimer3Handle, &sMasterConfig) != HAL_OK)
    {
        return TIMER_ERR_INIT_FAILURE;
    }

    HAL_TIM_Base_Start_IT(&gTimer3Handle);

    return TIMER_ERR_OK;
}

/**
* @brief TIM_Base MSP Initialization
* This function configures the hardware resources used in this example
*
* @param htim_base: TIM_Base handle pointer
*
* @remark: this HAL_TIM_Base_MspInit function is called automatically by the
* STM32 HAL library
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
    if(htim_base->Instance==TIM3)
    {
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();

        /* TIM3 interrupt Init */
        HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    }
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&gTimer3Handle);
}


/***** PRIVATE FUNCTIONS *****************************************************/
