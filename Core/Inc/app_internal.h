#ifndef APP_INTERNAL_H
#define APP_INTERNAL_H

#include "app.h"
#include "sht3x.h"
#include "bh1750.h"
#include "soil_moisture.h"
#include "w25qxx.h"

SHT3x_Handle *App_SHT3x(void);
BH1750_Handle *App_BH1750(void);
SoilMoisture_Handle *App_Soil(void);
W25QXX_Handle *App_Flash(void);
UART_HandleTypeDef *App_Uart(void);
void App_SetLatest(const SensorData_t *sample);
AppStatistics_t *App_StatisticsMutable(void);

#endif
