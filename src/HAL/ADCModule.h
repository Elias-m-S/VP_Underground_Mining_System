/******************************************************************************
 * @file ADCModule.h
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Header File for the ADC Service Layer Module
 *
 *
 *****************************************************************************/
#ifndef _ADC_MODULE_H
#define _ADC_MODULE_H

/***** INCLUDES **************************************************************/
#include <stdint.h>

/***** CONSTANTS *************************************************************/


/***** MACROS ****************************************************************/
#define ADC_ERR_OK                  0               //!< No error occured
#define ADC_ERR_INIT_FAILURE        -1              //!< Error during ADC initialization

/***** TYPES *****************************************************************/

/**
 * @brief Enumeration for used ADC channels
 *
 */
typedef enum _ADC_Channel_
{
    ADC_INPUT0,             //!< ADC Channel 0 used for Pot 1 (Speed Sensor)
    ADC_INPUT1,             //!< ADC Channel 1 used for Pot 2 (Flow Rate Sensor)
    ADC_TEMP,               //!< ADC Channel 2 used for internal Temperature Sensor
    ADC_VBAT,               //!< ADC Channel 3 used for internal VBat voltage
    ADC_VREF                //!< ADC Channel 4 used for internal reference voltage
} ADC_Channel_t;


/***** PROTOTYPES ************************************************************/

/**
 * @brief Initialize the ADC peripheral block
 *
 * @return Returns ADC_ERR_OK if no error occured
 */
int32_t adcInitialize();

/**
 * @brief Reads an ADC channel by returning the global ADC value read via
 * interrupt and DMA and converts it to millivolt
 *
 * @param adcChannel Channel to read
 *
 * @return Returns value of ADC channel in microvolt [µV]
 */
int32_t adcReadChannel(ADC_Channel_t adcChannel);

/**
 * @brief Reads an ADC channel by returning the global ADC value read via
 * interrupt and DMA
 *
 * @param adcChannel Channel to read
 *
 * @return Returns value of ADC channel in digits
 */
int32_t adcReadChannelRaw(ADC_Channel_t adcChannel);


#endif
