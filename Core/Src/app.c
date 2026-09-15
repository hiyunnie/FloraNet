#include "app.h"
#include "app_internal.h"
#include "alarm_service.h"
#include "command_service.h"
#include "health_service.h"
#include "sensor_service.h"
#include "storage_service.h"
#include "uart_service.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <string.h>

static SHT3x_Handle sht3x;
static BH1750_Handle bh1750;
static SoilMoisture_Handle soil;
static W25QXX_Handle flash;
static UART_HandleTypeDef *uart;
static SemaphoreHandle_t data_mutex;
static SemaphoreHandle_t config_mutex;
static SensorData_t latest;
static AppConfig_t config = {
    .temperature_high_x100 = 3500,
    .temperature_low_x100 = 500,
    .humidity_high_x100 = 8500,
    .humidity_low_x100 = 2000,
    .light_low_x100 = 1000,
    .soil_low_percent = 30,
    .hysteresis_x100 = 500,
    .sample_period_ms = 1000,
    .store_period_ms = 10000,
    .generation = 1
};
static AppStatistics_t statistics;

bool App_Init(const AppHardware_t *hw)
{
    if ((hw == NULL) || (hw->i2c == NULL) || (hw->spi == NULL) ||
        (hw->adc == NULL) || (hw->uart == NULL) ||
        (hw->flash_cs_port == NULL)) return false;

    data_mutex = xSemaphoreCreateMutex();
    config_mutex = xSemaphoreCreateMutex();
    if ((data_mutex == NULL) || (config_mutex == NULL)) return false;

    uart = hw->uart;
    SHT3x_Init(&sht3x, hw->i2c, SHT3X_ADDRESS_A, 100U);
    BH1750_Init(&bh1750, hw->i2c, BH1750_ADDRESS_LOW, 100U);
    SoilMoisture_Init(&soil, hw->adc, 20U);
    (void)SoilMoisture_SetCalibration(&soil, 4095U, 1400U);
    W25QXX_InitHandle(&flash, hw->spi, hw->flash_cs_port,
                     hw->flash_cs_pin, 100U, 3000U);

    if (!UartService_Init()) return false;
    if (SHT3x_Probe(&sht3x) != SHT3X_OK) UartService_Log("[INIT] SHT3x unavailable\r\n");
    if (BH1750_Probe(&bh1750) != BH1750_OK) UartService_Log("[INIT] BH1750 unavailable\r\n");
    if (W25QXX_Probe(&flash) != W25QXX_OK) UartService_Log("[INIT] W25Q64 unavailable\r\n");

    if (!HealthService_Init()) return false;
    if (!StorageService_Init()) return false;
    if (!AlarmService_Init()) return false;
    if (!CommandService_Init()) return false;
    if (!SensorService_Init()) return false;
    return true;
}

bool App_GetLatest(SensorData_t *out)
{
    if ((out == NULL) || (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10)) != pdTRUE)) return false;
    *out = latest;
    xSemaphoreGive(data_mutex);
    return true;
}

void App_SetLatest(const SensorData_t *sample)
{
    if ((sample != NULL) && (xSemaphoreTake(data_mutex, pdMS_TO_TICKS(10)) == pdTRUE)) {
        latest = *sample;
        xSemaphoreGive(data_mutex);
    }
}

bool App_GetConfig(AppConfig_t *out)
{
    if ((out == NULL) || (xSemaphoreTake(config_mutex, pdMS_TO_TICKS(10)) != pdTRUE)) return false;
    *out = config;
    xSemaphoreGive(config_mutex);
    return true;
}

bool App_UpdateConfig(const AppConfig_t *value)
{
    if ((value == NULL) || (value->sample_period_ms < 250U) ||
        (value->store_period_ms < value->sample_period_ms)) return false;
    if (xSemaphoreTake(config_mutex, pdMS_TO_TICKS(20)) != pdTRUE) return false;
    config = *value;
    config.generation++;
    xSemaphoreGive(config_mutex);
    return true;
}

void App_GetStatistics(AppStatistics_t *out) { if (out != NULL) *out = statistics; }
AppStatistics_t *App_StatisticsMutable(void) { return &statistics; }
SHT3x_Handle *App_SHT3x(void) { return &sht3x; }
BH1750_Handle *App_BH1750(void) { return &bh1750; }
SoilMoisture_Handle *App_Soil(void) { return &soil; }
W25QXX_Handle *App_Flash(void) { return &flash; }
UART_HandleTypeDef *App_Uart(void) { return uart; }
void App_UartRxCompleteISR(UART_HandleTypeDef *huart) { CommandService_RxCompleteISR(huart); }
