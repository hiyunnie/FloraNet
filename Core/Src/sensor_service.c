#include "sensor_service.h"
#include "app.h"
#include "app_config.h"
#include "app_internal.h"
#include "alarm_service.h"
#include "health_service.h"
#include "storage_service.h"
#include "uart_service.h"
#include "FreeRTOS.h"
#include "task.h"

static void SensorTask(void *argument)
{
    TickType_t last_wake = xTaskGetTickCount();
    uint32_t sequence = 0U, last_store_ms = 0U;
    (void)argument;

    for (;;) {
        TickType_t release = xTaskGetTickCount();
        AppConfig_t cfg;
        SensorData_t out = {0};
        SHT3x_Measurement sht;
        float lux = 0.0f, soil_percent = 0.0f;
        uint16_t light_raw = 0U;

        (void)App_GetConfig(&cfg);
        out.sequence = ++sequence;
        out.timestamp_ms = HAL_GetTick();

        SHT3x_Result sr = SHT3x_StartSingleShot(App_SHT3x(), SHT3X_REPEATABILITY_HIGH, false);
        BH1750_Result br = BH1750_StartMeasurement(App_BH1750(), BH1750_ONE_TIME_HIGH_RES);
        vTaskDelay(pdMS_TO_TICKS(180U));

        if ((sr == SHT3X_OK) && (SHT3x_ReadMeasurement(App_SHT3x(), &sht) == SHT3X_OK)) {
            out.temperature_x100 = (int16_t)(sht.temperature_c * 100.0f);
            out.humidity_x100 = (uint16_t)(sht.humidity_rh * 100.0f);
            out.valid_flags |= APP_VALID_SHT3X;
        } else out.error_flags |= APP_ERR_SHT3X;

        if ((br == BH1750_OK) && (BH1750_Read(App_BH1750(), &light_raw, &lux) == BH1750_OK)) {
            out.light_x100 = (uint32_t)(lux * 100.0f);
            out.valid_flags |= APP_VALID_BH1750;
        } else out.error_flags |= APP_ERR_BH1750;

        if (SoilMoisture_ReadPercent(App_Soil(), 8U, &soil_percent, &out.soil_raw) == SOIL_MOISTURE_OK) {
            if (soil_percent < 0.0f) soil_percent = 0.0f;
            if (soil_percent > 100.0f) soil_percent = 100.0f;
            out.soil_percent = (uint8_t)soil_percent;
            out.valid_flags |= APP_VALID_SOIL;
        } else out.error_flags |= APP_ERR_SOIL;

        App_SetLatest(&out);
        (void)AlarmService_Publish(&out);

        if ((out.timestamp_ms - last_store_ms) >= cfg.store_period_ms) {
            StorageRequest_t req = {.type = STORAGE_APPEND_SAMPLE};
            req.data.sample = out;
            (void)StorageService_Request(&req);
            last_store_ms = out.timestamp_ms;
        }

        UartService_Log("[DATA] seq=%lu T=%d.%02d RH=%u.%02u LUX=%lu.%02lu SOIL=%u%% err=%08lX\r\n",
            (unsigned long)out.sequence,
            out.temperature_x100 / 100, out.temperature_x100 < 0 ? -(out.temperature_x100 % 100) : out.temperature_x100 % 100,
            out.humidity_x100 / 100, out.humidity_x100 % 100,
            (unsigned long)(out.light_x100 / 100), (unsigned long)(out.light_x100 % 100),
            out.soil_percent, (unsigned long)out.error_flags);

        if ((xTaskGetTickCount() - release) > pdMS_TO_TICKS(APP_SENSOR_DEADLINE_MS))
            App_StatisticsMutable()->deadline_miss_count++;
        if (out.error_flags != 0U) App_StatisticsMutable()->sensor_error_count++;
        HealthService_Heartbeat(HEALTH_SENSOR);
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(cfg.sample_period_ms));
    }
}

bool SensorService_Init(void)
{
    return xTaskCreate(SensorTask, "Sensor", APP_SENSOR_STACK_WORDS, NULL,
                       APP_PRIO_SENSOR, NULL) == pdPASS;
}
