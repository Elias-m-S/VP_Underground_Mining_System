/******************************************************************************
 * @file ADCModule.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of the ADC Service Layer Module this includes the
 * initialization of the ADC and DMA peripherals according the needs of
 * the application
 *
 *
 *****************************************************************************/

/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"

#include "System.h"
#include "HardwareConfig.h"
#include "ADCModule.h"

#include <string.h>

/***** PRIVATE CONSTANTS *****************************************************/
static const int32_t MICROVOLTS_PER_DIGIT = 805;    //!< 805 µV / digit


/***** PRIVATE MACROS ********************************************************/
#define ADC_CHANNEL_COUNT       5                   //!< Total number of used ADC channels

#define IDX_ADC_INPUT0          0                   //!< Array index for ADC channel 0 (Pot 1) in global ADC value array
#define IDX_ADC_INPUT1          1                   //!< Array index for ADC channel 1 (Pot 2) in global ADC value array
#define IDX_ADC_TEMP            2                   //!< Array index for ADC channel 2 (internal Temp) in global ADC value array
#define IDX_ADC_VBAT            3                   //!< Array index for ADC channel 3 (VBat) in global ADC value array
#define IDX_ADC_VREF            4                   //!< Array index for ADC channel 4 (internal reference voltage) in global ADC value array


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/

static void adcInitializeDMA(void);


/***** PRIVATE VARIABLES *****************************************************/
static ADC_HandleTypeDef gADCHandle;                //!< Global handle for ADC peripheral
static DMA_HandleTypeDef gDMA_ADC_Handle;           //!< Global handle for DMA peripheral used for ADC data transfer

static uint32_t gADCValues[ADC_CHANNEL_COUNT];      //!< Global array for ADC values used by the DMA transfer


/***** PUBLIC FUNCTIONS ******************************************************/

int32_t adcInitialize()
{
    /* Initialize DMA block for use with ADC */
    adcInitializeDMA();

    memset(gADCValues, 0, ADC_CHANNEL_COUNT * sizeof(uint32_t));

    /**
     * Common config
     */
    ADC_MultiModeTypeDef multimode = {0};
    ADC_ChannelConfTypeDef sConfig = {0};

    gADCHandle.Instance 					= ADC1;
    gADCHandle.Init.ClockPrescaler 			= ADC_CLOCK_SYNC_PCLK_DIV4;
    gADCHandle.Init.Resolution 				= ADC_RESOLUTION_12B;
    gADCHandle.Init.DataAlign 				= ADC_DATAALIGN_RIGHT;
    gADCHandle.Init.GainCompensation 		= 0;
    gADCHandle.Init.ScanConvMode 			= ADC_SCAN_ENABLE;
    gADCHandle.Init.EOCSelection 			= ADC_EOC_SINGLE_CONV;
    gADCHandle.Init.LowPowerAutoWait 		= DISABLE;
    gADCHandle.Init.ContinuousConvMode 		= DISABLE;
    gADCHandle.Init.NbrOfConversion 		= 5;
    gADCHandle.Init.DiscontinuousConvMode 	= DISABLE;
    gADCHandle.Init.ExternalTrigConv 		= ADC_EXTERNALTRIG_T3_TRGO;
    gADCHandle.Init.ExternalTrigConvEdge 	= ADC_EXTERNALTRIGCONVEDGE_RISING;
    gADCHandle.Init.DMAContinuousRequests 	= ENABLE;
    gADCHandle.Init.Overrun 				= ADC_OVR_DATA_PRESERVED;
    gADCHandle.Init.OversamplingMode 		= DISABLE;

    if (HAL_ADC_Init(&gADCHandle) != HAL_OK)
    {
    	Error_Handler();
    }

	/** Configure the ADC multi-mode
	*/
	multimode.Mode = ADC_MODE_INDEPENDENT;
	if (HAL_ADCEx_MultiModeConfigChannel(&gADCHandle, &multimode) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Regular Channel
	*/
	sConfig.Channel 		= ADC_CHANNEL_1;
	sConfig.Rank 			= ADC_REGULAR_RANK_1;
	sConfig.SamplingTime 	= ADC_SAMPLETIME_92CYCLES_5;
	sConfig.SingleDiff 		= ADC_SINGLE_ENDED;
	sConfig.OffsetNumber 	= ADC_OFFSET_NONE;
	sConfig.Offset 			= 0;
	if (HAL_ADC_ConfigChannel(&gADCHandle, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Regular Channel
	*/
	sConfig.Channel 		= ADC_CHANNEL_2;
	sConfig.Rank 			= ADC_REGULAR_RANK_2;
	if (HAL_ADC_ConfigChannel(&gADCHandle, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Regular Channel
	*/
	sConfig.Channel 		= ADC_CHANNEL_TEMPSENSOR_ADC1;
	sConfig.Rank 			= ADC_REGULAR_RANK_3;
	if (HAL_ADC_ConfigChannel(&gADCHandle, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Regular Channel
	*/
	sConfig.Channel 		= ADC_CHANNEL_VBAT;
	sConfig.Rank 			= ADC_REGULAR_RANK_4;
	if (HAL_ADC_ConfigChannel(&gADCHandle, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure Regular Channel
	*/
	sConfig.Channel 		= ADC_CHANNEL_VREFINT;
	sConfig.Rank 			= ADC_REGULAR_RANK_5;
	if (HAL_ADC_ConfigChannel(&gADCHandle, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}

	/* Calibrate the ADC */
    HAL_ADCEx_Calibration_Start(&gADCHandle, ADC_SINGLE_ENDED);

    // Start ADC in DMA mode
    // This assumes, that DMA peripheral has been already configured
    HAL_ADC_Start_DMA(&gADCHandle, gADCValues, ADC_CHANNEL_COUNT);

	return ADC_ERR_OK;
}

/**
* @brief ADC MSP Initialization
*
* This function configures the hardware resources used for ADC including
* GPIO pins for ADC input
*
* @param hadc: ADC handle pointer
*
* @remark: this HAL_ADC_MspInit function is called automatically by the
* STM32 HAL library
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  if(adcHandle->Instance==ADC1)
  {
	/** Initializes the peripherals clocks
	*/
	PeriphClkInit.PeriphClockSelection 	= RCC_PERIPHCLK_ADC12;
	PeriphClkInit.Adc12ClockSelection 	= RCC_ADC12CLKSOURCE_SYSCLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
	  Error_Handler();
	}

	/* ADC1 clock enable */
	__HAL_RCC_ADC12_CLK_ENABLE();

	__HAL_RCC_GPIOA_CLK_ENABLE();
	/**ADC1 GPIO Configuration
	PA0     ------> ADC1_IN1
	PA1     ------> ADC1_IN2
	*/
	//GPIO_InitStruct.Pin 	= POT1_PIN | POT2_PIN;
	GPIO_InitStruct.Pin 	= POT1_PIN;
	GPIO_InitStruct.Mode 	= GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull 	= GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* ADC1 DMA Init */
	/* ADC1 Init */
	gDMA_ADC_Handle.Instance 					= DMA1_Channel1;
	gDMA_ADC_Handle.Init.Request 				= DMA_REQUEST_ADC1;
	gDMA_ADC_Handle.Init.Direction 				= DMA_PERIPH_TO_MEMORY;
	gDMA_ADC_Handle.Init.PeriphInc 				= DMA_PINC_DISABLE;
	gDMA_ADC_Handle.Init.MemInc 				= DMA_MINC_ENABLE;
	gDMA_ADC_Handle.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_WORD;
	gDMA_ADC_Handle.Init.MemDataAlignment 		= DMA_MDATAALIGN_WORD;
	gDMA_ADC_Handle.Init.Mode 					= DMA_CIRCULAR;
	gDMA_ADC_Handle.Init.Priority 				= DMA_PRIORITY_LOW;

	if (HAL_DMA_Init(&gDMA_ADC_Handle) != HAL_OK)
	{
	  Error_Handler();
	}

	__HAL_LINKDMA(adcHandle, DMA_Handle, gDMA_ADC_Handle);

	/* ADC1 interrupt Init */
	HAL_NVIC_SetPriority(ADC1_2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
  }
}

int32_t adcReadChannelRaw(ADC_Channel_t adcChannel)
{
    int32_t adcValue = 0;

    switch(adcChannel)
    {
        case ADC_INPUT0:
            adcValue = gADCValues[IDX_ADC_INPUT0];
            break;

        case ADC_INPUT1:
            adcValue = gADCValues[IDX_ADC_INPUT1];
            break;

        case ADC_TEMP:
            adcValue = gADCValues[IDX_ADC_TEMP];
            break;

        case ADC_VBAT:
            adcValue = gADCValues[IDX_ADC_VBAT];
            break;

        case ADC_VREF:
            adcValue = gADCValues[IDX_ADC_VREF];
            break;
    }

    return adcValue;
}

int32_t adcReadChannel(ADC_Channel_t adcChannel)
{
    int32_t adcRawValue = adcReadChannelRaw(adcChannel);
    int32_t adcMicroVoltValue = adcRawValue * MICROVOLTS_PER_DIGIT;

    return adcMicroVoltValue;
}



/***** PRIVATE FUNCTIONS *****************************************************/

/**
 * @brief Initializes the DMA peripheral (DMA1) for use with the ADC block
 *
 */
static void adcInitializeDMA(void)
{
    /* DMA controller clock enable */
	__HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&gDMA_ADC_Handle);
}

/**
  * @brief This function handles ADC1 global interrupt.
  */
void ADC1_2_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&gADCHandle);
}


