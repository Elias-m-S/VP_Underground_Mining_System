#ifndef _GAS_SENSOR_H_
#define _GAS_SENSOR_H_

#include <stdint.h>
#include <stdbool.h>

/* Return codes */
#define SENSOR_OK             ((int32_t)0)
#define SENSOR_INVALID_PTR    ((int32_t)-1)
#define SENSOR_INVALID_VALUE  ((int32_t)-2)
#define SENSOR_DEFECT         ((int32_t)-3)

/* Gas sensor physical limits (from spec) */
#define GAS_PPM_MIN           ((int32_t)200)
#define GAS_PPM_MAX           ((int32_t)10000)

/* Sensor output voltage limits (from spec): 0.5V .. 2.5V */
#define GAS_UV_MIN            ((uint32_t)500000U)
#define GAS_UV_MAX            ((uint32_t)2500000U)

/* Conversion: 1ppm = 204.082 µV  -> use integer math:
 * ppm = (uV * 1000) / 204082
 */
#define GAS_UV_PER_PPM_X1000  ((uint32_t)204082U)

typedef struct _GasSensor
{
    uint32_t sensorVoltage_uV;
} GasSensor;

int32_t GasSensorInitialize(GasSensor* pSensor);
int32_t GasSensorSetSensorVoltage_uV(GasSensor* pSensor, uint32_t sensorVoltage_uV);

/* Returns ppm on success (>=200..<=10000) or negative error code */
int32_t GasSensorGetPpm(GasSensor* pSensor);

/* True if sensor voltage indicates defect (<0.5V or >2.5V) */
bool GasSensorIsDefectVoltage(uint32_t sensorVoltage_uV);

#endif /* _GAS_SENSOR_H_ */
