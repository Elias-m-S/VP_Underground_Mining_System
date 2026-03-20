#ifndef _SENSOR_HANDLER_H_
#define _SENSOR_HANDLER_H_

#include <stdint.h>
#include <stdbool.h>

void sensorHandlerInitialize(void);
void sensorHandlerCycle(void);

/* Optional: Application direct request */
bool sensorHandlerIsSensorDefect(void);

#endif /* _SENSOR_HANDLER_H_ */
