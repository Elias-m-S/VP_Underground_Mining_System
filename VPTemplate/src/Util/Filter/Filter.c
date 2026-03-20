/******************************************************************************
 * @file Filter.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation file for Filter library
 *
 *
 *****************************************************************************/

/***** INCLUDES **************************************************************/
#include "Filter.h"

/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/


/***** PUBLIC FUNCTIONS ******************************************************/

/* Initialize EMA filter parameters and optionally clear filter history */
int32_t filterInitEMA(EMAFilterData_t* pEMA, int32_t scalingFactor, int32_t alpha, bool resetFilter)
{
    /* Check for invalid output pointer */
    if (pEMA == 0)
    {
        return FILTER_ERR_INVALID_PTR;
    }

    /* Scaling factor must be greater than zero */
    if (scalingFactor <= 0)
    {
        return FILTER_ERR_INVALID_PARAM;
    }

    /* Alpha must be within valid EMA range */
    if ((alpha <= 0) || (alpha > scalingFactor))
    {
        return FILTER_ERR_INVALID_PARAM;
    }

    pEMA->scalingFactor = scalingFactor;
    pEMA->alpha = alpha;

    /* Optional reset to start with a clean filter state */
    if (resetFilter == true)
    {
        pEMA->firstValueAvailable = false;
        pEMA->previousValue = 0;
    }

    return FILTER_ERR_OK;
}

/* Reset EMA history so the next input becomes the new start value */
int32_t filterResetEMA(EMAFilterData_t* pEMA)
{
    /* Check for invalid pointer */
    if (pEMA == 0)
    {
        return FILTER_ERR_INVALID_PTR;
    }

    pEMA->firstValueAvailable = false;
    pEMA->previousValue = 0;

    return FILTER_ERR_OK;
}

/* Apply exponential moving average using integer arithmetic only */
int32_t filterEMA(EMAFilterData_t* pEMA, int32_t sensorValue)
{
    int64_t numerator;
    int32_t filteredValue;

    /* Fallback: return raw value if filter instance is invalid */
    if (pEMA == 0)
    {
        return sensorValue;
    }

    /* Fallback: return raw value if configuration is invalid */
    if (pEMA->scalingFactor <= 0)
    {
        return sensorValue;
    }

    /* Fallback: return raw value if alpha is out of valid range */
    if ((pEMA->alpha <= 0) || (pEMA->alpha > pEMA->scalingFactor))
    {
        return sensorValue;
    }

    /* First sample initializes the filter without smoothing */
    if (pEMA->firstValueAvailable == false)
    {
        pEMA->previousValue = sensorValue;
        pEMA->firstValueAvailable = true;
        return sensorValue;
    }

    /* Integer EMA:
     * y = (alpha * x + (scalingFactor - alpha) * previousValue) / scalingFactor
     */
    numerator =
        ((int64_t)pEMA->alpha * (int64_t)sensorValue) +
        ((int64_t)(pEMA->scalingFactor - pEMA->alpha) * (int64_t)pEMA->previousValue);

    filteredValue = (int32_t)(numerator / (int64_t)pEMA->scalingFactor);

    /* Store filtered value for next cycle */
    pEMA->previousValue = filteredValue;

    return filteredValue;
}
