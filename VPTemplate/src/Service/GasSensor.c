#include "GasSensor.h"

/* Initialize gas sensor structure */
int32_t GasSensorInitialize(GasSensor* pSensor)
{
    /* Check for invalid pointer */
    if (pSensor == 0)
        return SENSOR_INVALID_PTR;

    /* Initialize sensor voltage with default value */
    pSensor->sensorVoltage_uV = 0U;
    return SENSOR_OK;
}

/* Update the raw sensor voltage value (in microvolts) */
int32_t GasSensorSetSensorVoltage_uV(GasSensor* pSensor, uint32_t sensorVoltage_uV)
{
    /* Check for invalid pointer */
    if (pSensor == 0)
        return SENSOR_INVALID_PTR;

    /* Store latest sensor voltage reading */
    pSensor->sensorVoltage_uV = sensorVoltage_uV;
    return SENSOR_OK;
}

/* Check whether the given sensor voltage indicates a defect */
bool GasSensorIsDefectVoltage(uint32_t sensorVoltage_uV)
{
    /* Voltage below allowed sensor range */
    if (sensorVoltage_uV < GAS_UV_MIN)
        return true;

    /* Voltage above allowed sensor range */
    if (sensorVoltage_uV > GAS_UV_MAX)
        return true;

    return false;
}

/* Convert sensor voltage to gas concentration in ppm */
int32_t GasSensorGetPpm(GasSensor* pSensor)
{
    /* Check for invalid pointer */
    if (pSensor == 0)
        return SENSOR_INVALID_PTR;

    uint32_t uV = pSensor->sensorVoltage_uV;

    /* Detect sensor defect based on voltage range */
    if (GasSensorIsDefectVoltage(uV))
        return SENSOR_DEFECT;

    /* Linear conversion: ppm = (uV * 1000) / 204082
       Implemented using integer arithmetic (no floating point allowed) */
    uint32_t ppm_u32 = (uV * 1000U) / GAS_UV_PER_PPM_X1000;
    int32_t ppm = (int32_t)ppm_u32;

    /* Check if calculated ppm value is within valid measurement range */
    if (ppm < GAS_PPM_MIN || ppm > GAS_PPM_MAX)
        return SENSOR_INVALID_VALUE;

    return ppm;
}
