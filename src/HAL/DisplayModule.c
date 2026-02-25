/******************************************************************************
 * @file DisplayModule.c
 *
 * @author Andreas Schmidt (a.v.schmidt81@googlemail.com)
 * @date   03.01.2026
 *
 * @copyright Copyright (c) 2026
 *
 ******************************************************************************
 *
 * @brief Implementation of the 7-Segement display module
 *
 *
 *****************************************************************************/


/***** INCLUDES **************************************************************/
#include "stm32g4xx_hal.h"

#include "System.h"
#include "HardwareConfig.h"
#include "DisplayModule.h"


/***** PRIVATE CONSTANTS *****************************************************/


/***** PRIVATE MACROS ********************************************************/


/***** PRIVATE TYPES *********************************************************/

/**
 * @brief Internal type to represent the activation of the 7-segments
 *
 * A bit field is used to demonstrate the usage of such a bit field
 * Other implementations might be more efficient
 */
typedef struct _SegmentEncodingEntry
{
    unsigned int segA : 1;      //!< Bit for activate/deactivate segment A
    unsigned int segB : 1;      //!< Bit for activate/deactivate segment B
    unsigned int segC : 1;      //!< Bit for activate/deactivate segment C
    unsigned int segD : 1;      //!< Bit for activate/deactivate segment D
    unsigned int segE : 1;      //!< Bit for activate/deactivate segment E
    unsigned int segF : 1;      //!< Bit for activate/deactivate segment F
    unsigned int segG : 1;      //!< Bit for activate/deactivate segment G
} SegmentEncodingEntry;


/***** PRIVATE PROTOTYPES ****************************************************/


/***** PRIVATE VARIABLES *****************************************************/

/**
 * @brief Encoding table for the specific LED segments on a single 7-Segment
 * display
 *
 */
static const SegmentEncodingEntry gSegmentEncodingTable[] =
{
//   a, b, c, d, e, f, g
    {1, 1, 1, 1, 1, 1, 0},  // 0
    {0, 1, 1, 0, 0, 0, 0},  // 1
    {1, 1, 0, 1, 1, 0, 1},  // 2
    {1, 1, 1, 1, 0, 0, 1},  // 3
    {0, 1, 1, 0, 0, 1, 1},  // 4
    {1, 0, 1, 1, 0, 1, 1},  // 5
    {1, 0, 1, 1, 1, 1, 1},  // 6
    {1, 1, 1, 0, 0, 0, 0},  // 7
    {1, 1, 1, 1, 1, 1, 1},  // 8
    {1, 1, 1, 0, 0, 1, 1},  // 9
    {1, 1, 1, 0, 1, 1, 1},  // A
    {0, 0, 1, 1, 1, 1, 1},  // B
    {1, 0, 0, 1, 1, 1, 0},  // C
    {0, 1, 1, 1, 1, 0, 1},  // D
    {1, 0, 0, 1, 1, 1, 1},  // E
    {1, 0, 0, 0, 1, 1, 1},  // F
    {0, 0, 0, 0, 0, 0, 1},  // - (DIGIT_DASH)
    {1, 1, 0, 0, 0, 1, 1},  // o (DIGIT_UPPER_O)
    {0, 0, 1, 1, 1, 0, 1},  // o (DIGIT_LOWER_O)
    {0, 0, 0, 0, 0, 0, 0},  // Off (DIGIT_OFF)
};

/***** PUBLIC FUNCTIONS ******************************************************/


int32_t displayInitialize()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, _7SEGF_PIN | _7SEGG_PIN, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOA, _7SEGA_PIN | _7SEGB_PIN | _7SEGC_PIN | _7SEGD_PIN  | _7SEGE_PIN, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOB, _7SEG_COM_PIN, GPIO_PIN_RESET);

    /*Configure GPIO pins : _7SEGF_Pin _7SEGG_Pin */
    GPIO_InitStruct.Pin = _7SEGF_PIN | _7SEGG_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pins : _7SEGA_Pin _7SEGB_Pin _7SEGC_Pin _7SEGD_Pin _7SEGE_Pin */
    GPIO_InitStruct.Pin = _7SEGA_PIN | _7SEGB_PIN | _7SEGC_PIN | _7SEGD_PIN | _7SEGE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /*Configure GPIO pins : _7SEG_COM_Pin */
    GPIO_InitStruct.Pin = _7SEG_COM_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    return DISPLAY_ERR_OK;
}

int32_t displayShowDigit(Display_t outputDisplay, int8_t digit)
{
    if (digit >= 0 && digit <= DIGIT_OFF)
    {
        // Get the entry out of the encoding table
        SegmentEncodingEntry entry = gSegmentEncodingTable[digit];

        // If the Floor Display is used, we need to invert all bits
        // due to the differnce between common anode and common kathode
        if (outputDisplay == LEFT_DISPLAY)
        {
            entry.segA = !entry.segA;
            entry.segB = !entry.segB;
            entry.segC = !entry.segC;
            entry.segD = !entry.segD;
            entry.segE = !entry.segE;
            entry.segF = !entry.segF;
            entry.segG = !entry.segG;

            /* Activate the Error Display */
            HAL_GPIO_WritePin(GPIOB, _7SEG_COM_PIN, GPIO_PIN_SET);
        }
        else
        {
            /* Activate the Floor Display */
            HAL_GPIO_WritePin(GPIOB, _7SEG_COM_PIN, GPIO_PIN_RESET);
        }

        // Write the single segments
        HAL_GPIO_WritePin(_7SEGA_GPIO_PORT, _7SEGA_PIN, entry.segA);
        HAL_GPIO_WritePin(_7SEGB_GPIO_PORT, _7SEGB_PIN, entry.segB);
        HAL_GPIO_WritePin(_7SEGC_GPIO_PORT, _7SEGC_PIN, entry.segC);
        HAL_GPIO_WritePin(_7SEGD_GPIO_PORT, _7SEGD_PIN, entry.segD);
        HAL_GPIO_WritePin(_7SEGE_GPIO_PORT, _7SEGE_PIN, entry.segE);
        HAL_GPIO_WritePin(_7SEGF_GPIO_PORT, _7SEGF_PIN, entry.segF);
        HAL_GPIO_WritePin(_7SEGG_GPIO_PORT, _7SEGG_PIN, entry.segG);
    }

    return DISPLAY_ERR_OK;
}

/***** PRIVATE FUNCTIONS *****************************************************/
