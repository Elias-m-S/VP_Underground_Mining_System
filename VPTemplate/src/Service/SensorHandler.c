/******************************************************************************
 * @file SensorHandler.c
 *
 * @brief Service module to read/process sensor values and update GlobalObjects.
 *
 *****************************************************************************/

/***** INCLUDES **************************************************************/
#include "Service/SensorHandler.h"

#include "HAL/ADCModule.h"
#include "HAL/UARTModule.h"

#include "Service/GlobalObjects.h"
#include "Util/Filter/Filter.h"

/***** PRIVATE MACROS ********************************************************/

/* EMA filter constants (bigger => smoother, slower) */
#define EMA_K_POT1              (8U)
#define EMA_K_POT2              (8U)

/* EMA implementation via Filter module:
 * y = y + (x - y)/k  is equivalent to alpha = 1/k
 * Use scaling factor = k, alpha = 1
 */
#define EMA_SCALING_POT1        ((int32_t)EMA_K_POT1)
#define EMA_ALPHA_POT1          (1)
#define EMA_SCALING_POT2        ((int32_t)EMA_K_POT2)
#define EMA_ALPHA_POT2          (1)

/* Allowed percentage deviation between both gas channels */
#define GAS_INCONSIST_PCT       (30U)

/* Water sensor */
#define WATER_FRAME_SIZE        (8U)
#define WATER_TIMEOUT_TICKS     (150U)   /* 150 * 10ms = 1500ms */
#define WATER_RESERVED_L        (0xDEU)
#define WATER_RESERVED_H        (0xC0U)

/* Water conversion: 0.5V..2.5V -> 50..1000 cm */
#define WATER_UV_MIN            (500000U)
#define WATER_UV_MAX            (2500000U)
#define WATER_CM_MIN            (50U)
#define WATER_CM_MAX            (1000U)
#define WATER_TOTAL_CM_RANGE    (950U)
#define WATER_TOTAL_UV_RANGE    (2000000U)

/* Gas conversion according to specification:
 * 0.5V .. 2.5V  ->  200 ppm .. 10000 ppm
 * <0.5V or >2.5V -> sensor defect
 */
#define GAS_UV_MIN              (500000U)
#define GAS_UV_MAX              (2500000U)
#define GAS_PPM_MIN             (200)
#define GAS_PPM_MAX             (10000)
#define GAS_TOTAL_PPM_RANGE     (9800U)
#define GAS_TOTAL_UV_RANGE      (2000000U)

/***** PRIVATE VARIABLES *****************************************************/

/* Filtered gas sensor voltages in microvolts */
static uint32_t s_pot1_uV_filt;
static uint32_t s_pot2_uV_filt;

/* EMA filter instances for both gas channels */
static EMAFilterData_t s_pot1Filter;
static EMAFilterData_t s_pot2Filter;

/* Sliding buffer for cyclic UART water sensor frame reception */
static uint8_t  s_uartFrame[WATER_FRAME_SIZE];
static uint8_t  s_uartFillLevel;

/* Used to verify correct packet counter increment including rollover */
static uint8_t  s_lastPacketCounter;
static uint8_t  s_hasLastCounter;

/* Counts missing UART data packets in 10 ms cycles */
static uint8_t  s_noDataTicks;

/* Overall sensor defect flag used by upper layers */
static bool s_sensorDefect;

/***** PRIVATE FUNCTIONS *****************************************************/

/* Calculate checksum as two's complement of the byte sum */
static uint8_t checksum_calc(const uint8_t* pData, uint8_t lenWithoutChecksum)
{
    uint16_t sum = 0U;
    uint8_t i = 0U;

    for (i = 0U; i < lenWithoutChecksum; i++)
    {
        sum = (uint16_t)(sum + pData[i]);
    }

    return (uint8_t)(((sum & 0xFFU) ^ 0xFFU) + 0x01U);
}

/* Check whether the current packet counter matches the expected next value */
static bool packetCounterExpected(uint8_t prev, uint8_t current)
{
    uint8_t expected = (uint8_t)(prev + 1U);
    return (current == expected);
}

/* Convert valid water sensor voltage to water level in cm */
static uint16_t water_uV_to_cm(uint32_t uV)
{
    uint32_t delta;
    uint32_t cm;

    /* Clamp values below sensor range to minimum measurement */
    if (uV < WATER_UV_MIN)
    {
        return WATER_CM_MIN;
    }

    /* Clamp values above sensor range to maximum measurement */
    if (uV > WATER_UV_MAX)
    {
        return WATER_CM_MAX;
    }

    delta = uV - WATER_UV_MIN;
    cm = (uint32_t)WATER_CM_MIN
       + (delta * (uint32_t)WATER_TOTAL_CM_RANGE) / (uint32_t)WATER_TOTAL_UV_RANGE;

    if (cm < WATER_CM_MIN)
    {
        cm = WATER_CM_MIN;
    }

    if (cm > WATER_CM_MAX)
    {
        cm = WATER_CM_MAX;
    }

    return (uint16_t)cm;
}

/* Check whether the gas sensor voltage is outside the valid physical range */
static bool gas_is_defect_uV(uint32_t uV)
{
    if (uV < GAS_UV_MIN)
    {
        return true;
    }

    if (uV > GAS_UV_MAX)
    {
        return true;
    }

    return false;
}

/* Convert valid gas sensor voltage to ppm using integer arithmetic only */
static int32_t gas_uV_to_ppm(uint32_t uV)
{
    uint32_t delta_uV;
    uint64_t ppm;

    /* Clamp lower boundary to minimum measurable ppm */
    if (uV <= GAS_UV_MIN)
    {
        return GAS_PPM_MIN;
    }

    /* Clamp upper boundary to maximum measurable ppm */
    if (uV >= GAS_UV_MAX)
    {
        return GAS_PPM_MAX;
    }

    delta_uV = uV - GAS_UV_MIN;

    ppm = (uint64_t)GAS_PPM_MIN
        + (((uint64_t)delta_uV * (uint64_t)GAS_TOTAL_PPM_RANGE) / (uint64_t)GAS_TOTAL_UV_RANGE);

    if (ppm > (uint64_t)GAS_PPM_MAX)
    {
        ppm = (uint64_t)GAS_PPM_MAX;
    }

    return (int32_t)ppm;
}

/* Update local and global overall sensor defect status */
static void setDefect(bool defect)
{
    s_sensorDefect = defect;
    globalSetSensorFailure(defect);
}

/* Validate complete UART water frame and extract payload values */
static bool waterFrameIsValid(const uint8_t* pFrame, uint32_t* pValueUv, uint8_t* pCounter)
{
    uint32_t value_uV;
    uint8_t counter;
    uint8_t resL;
    uint8_t resH;
    uint8_t chk;

    counter = pFrame[0];

    /* Sensor value is encoded as 32-bit little-endian voltage in microvolts */
    value_uV =
        ((uint32_t)pFrame[1]) |
        ((uint32_t)pFrame[2] << 8) |
        ((uint32_t)pFrame[3] << 16) |
        ((uint32_t)pFrame[4] << 24);

    resL = pFrame[5];
    resH = pFrame[6];
    chk  = pFrame[7];

    /* Check fixed reserved marker bytes */
    if ((resL != WATER_RESERVED_L) || (resH != WATER_RESERVED_H))
    {
        return false;
    }

    /* Check frame checksum */
    if (checksum_calc(pFrame, 7U) != chk)
    {
        return false;
    }

    /* Check whether sensor voltage is within valid range */
    if ((value_uV < WATER_UV_MIN) || (value_uV > WATER_UV_MAX))
    {
        return false;
    }

    *pValueUv = value_uV;
    *pCounter = counter;
    return true;
}

/* Shift one received UART byte into the frame buffer */
static void waterShiftInByte(uint8_t b)
{
    uint8_t i;

    if (s_uartFillLevel < WATER_FRAME_SIZE)
    {
        s_uartFrame[s_uartFillLevel] = b;
        s_uartFillLevel++;
    }
    else
    {
        /* Keep the newest 8 bytes by shifting the window */
        for (i = 0U; i < (WATER_FRAME_SIZE - 1U); i++)
        {
            s_uartFrame[i] = s_uartFrame[i + 1U];
        }

        s_uartFrame[WATER_FRAME_SIZE - 1U] = b;
    }
}

/***** PUBLIC FUNCTIONS ******************************************************/

/* Initialize filters, UART frame tracking and exported sensor status values */
void sensorHandlerInitialize(void)
{
    s_pot1_uV_filt = 0U;
    s_pot2_uV_filt = 0U;

    (void)filterInitEMA(&s_pot1Filter, EMA_SCALING_POT1, EMA_ALPHA_POT1, true);
    (void)filterInitEMA(&s_pot2Filter, EMA_SCALING_POT2, EMA_ALPHA_POT2, true);

    s_uartFillLevel = 0U;
    s_lastPacketCounter = 0U;
    s_hasLastCounter = 0U;
    s_noDataTicks = 0U;

    s_sensorDefect = false;

    globalSetGasPpm(0);
    globalSetWaterLevelCm(0);
    globalSetGasDefect(false);
    globalSetWaterDefect(false);
    globalSetInconsistency(false);
    globalSetSensorFailure(false);
}

/* Return overall sensor defect state */
bool sensorHandlerIsSensorDefect(void)
{
    return s_sensorDefect;
}

/* Periodic 10 ms sensor processing cycle */
void sensorHandlerCycle(void)
{
    int32_t pot1_raw_i = adcReadChannel(ADC_INPUT0);
    int32_t pot2_raw_i = adcReadChannel(ADC_INPUT1);

    /* ADC read error is handled as gas sensor defect */
    if ((pot1_raw_i < 0) || (pot2_raw_i < 0))
    {
        globalSetGasPpm(0);
        globalSetGasDefect(true);
        globalSetInconsistency(false);
    }
    else
    {
        uint32_t pot1_uV = (uint32_t)pot1_raw_i;
        uint32_t pot2_uV = (uint32_t)pot2_raw_i;

        /* Filter both gas sensor channels before evaluation */
        s_pot1_uV_filt = (uint32_t)filterEMA(&s_pot1Filter, (int32_t)pot1_uV);
        s_pot2_uV_filt = (uint32_t)filterEMA(&s_pot2Filter, (int32_t)pot2_uV);

        {
            bool gas1_defect = gas_is_defect_uV(s_pot1_uV_filt);
            bool gas2_defect = gas_is_defect_uV(s_pot2_uV_filt);

            /* Any invalid gas channel directly causes gas defect state */
            if (gas1_defect || gas2_defect)
            {
                globalSetGasPpm(0);
                globalSetGasDefect(true);
                globalSetInconsistency(false);
            }
            else
            {
                int32_t gas1_ppm = gas_uV_to_ppm(s_pot1_uV_filt);
                int32_t gas2_ppm = gas_uV_to_ppm(s_pot2_uV_filt);

                bool inconsistency = false;
                int32_t diff = (gas1_ppm > gas2_ppm) ? (gas1_ppm - gas2_ppm) : (gas2_ppm - gas1_ppm);
                int32_t maxv = (gas1_ppm > gas2_ppm) ? gas1_ppm : gas2_ppm;

                /* Relative deviation check between both redundant gas channels */
                if (maxv > 0)
                {
                    uint64_t diffPct = ((uint64_t)(uint32_t)diff * 100ULL) / (uint64_t)(uint32_t)maxv;

                    if (diffPct > (uint64_t)GAS_INCONSIST_PCT)
                    {
                        inconsistency = true;
                    }
                }

                globalSetGasDefect(false);
                globalSetInconsistency(inconsistency);

                if (!inconsistency)
                {
                    /* Use average ppm value only if both channels are consistent */
                    int32_t gasAvg = (gas1_ppm + gas2_ppm) / 2;
                    globalSetGasPpm(gasAvg);
                }
                else
                {
                    globalSetGasPpm(0);
                }
            }
        }
    }

    {
        AppState_t st = globalGetAppState();
        bool waterDefect = false;

        /* Water reception is only active in Operational state */
        if (st != APP_STATE_OPERATIONAL)
        {
            /* Reset UART-related state outside Operational mode */
            s_noDataTicks = 0U;
            s_uartFillLevel = 0U;
            s_hasLastCounter = 0U;

            globalSetWaterDefect(false);
        }
        else
        {
            int8_t hasData = 0;

            /* Timeout supervision in 10 ms base cycle */
            if (s_noDataTicks < 255U)
            {
                s_noDataTicks++;
            }

            (void)uartHasData(&hasData);

            while (hasData != 0)
            {
                uint8_t b;

                if (uartReceiveData(&b, 1) != UART_ERR_OK)
                {
                    break;
                }

                waterShiftInByte(b);

                if (s_uartFillLevel >= WATER_FRAME_SIZE)
                {
                    uint32_t value_uV;
                    uint8_t counter;

                    /* Evaluate current 8-byte receive window as possible frame */
                    if (waterFrameIsValid(s_uartFrame, &value_uV, &counter))
                    {
                        if (s_hasLastCounter)
                        {
                            /* Check packet continuity including 8-bit rollover */
                            if (!packetCounterExpected(s_lastPacketCounter, counter))
                            {
                                waterDefect = true;
                            }
                        }

                        s_lastPacketCounter = counter;
                        s_hasLastCounter = 1U;

                        {
                            uint16_t cm = water_uV_to_cm(value_uV);
                            globalSetWaterLevelCm(cm);
                        }

                        /* Valid frame resets timeout supervision */
                        s_noDataTicks = 0U;
                        waterDefect = false;
                    }
                }

                (void)uartHasData(&hasData);
            }

            /* Missing frames for too long are treated as water sensor defect */
            if (s_noDataTicks >= WATER_TIMEOUT_TICKS)
            {
                waterDefect = true;
            }

            globalSetWaterDefect(waterDefect);
        }

        /* Combine all sensor-related error sources into one global defect state */
        if (globalGetGasDefect() || globalGetInconsistency() || waterDefect)
        {
            setDefect(true);
        }
        else
        {
            setDefect(false);
        }
    }
}
