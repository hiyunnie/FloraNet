#ifndef ALARM_SERVICE_H
#define ALARM_SERVICE_H

#include "app_types.h"
#include <stdbool.h>

bool AlarmService_Init(void);
bool AlarmService_Publish(const SensorData_t *sample);
uint32_t AlarmService_GetActive(void);
void AlarmService_Acknowledge(void);

#endif
