#ifndef HEALTH_SERVICE_H
#define HEALTH_SERVICE_H

#include "app_types.h"
#include <stdbool.h>

bool HealthService_Init(void);
void HealthService_Heartbeat(HealthBit_t bit);

#endif
