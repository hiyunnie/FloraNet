#include "alarm_service.h"
#include "app.h"
#include "app_config.h"
#include "app_internal.h"
#include "health_service.h"
#include "storage_service.h"
#include "uart_service.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

static QueueHandle_t alarm_queue;
static volatile uint32_t active_bits;

static uint32_t Evaluate(const SensorData_t *s, const AppConfig_t *c, uint32_t old)
{
    uint32_t now = old;
    if (s->valid_flags & APP_VALID_SHT3X) {
        if (s->temperature_x100 >= c->temperature_high_x100) now |= ALARM_TEMP_HIGH;
        else if (s->temperature_x100 <= c->temperature_high_x100 - (int32_t)c->hysteresis_x100) now &= ~ALARM_TEMP_HIGH;
        if (s->temperature_x100 <= c->temperature_low_x100) now |= ALARM_TEMP_LOW;
        else if (s->temperature_x100 >= c->temperature_low_x100 + (int32_t)c->hysteresis_x100) now &= ~ALARM_TEMP_LOW;
        if (s->humidity_x100 >= c->humidity_high_x100) now |= ALARM_RH_HIGH;
        else if (s->humidity_x100 + c->hysteresis_x100 <= c->humidity_high_x100) now &= ~ALARM_RH_HIGH;
        if (s->humidity_x100 <= c->humidity_low_x100) now |= ALARM_RH_LOW;
        else if (s->humidity_x100 >= c->humidity_low_x100 + c->hysteresis_x100) now &= ~ALARM_RH_LOW;
    }
    if (s->valid_flags & APP_VALID_BH1750) {
        if (s->light_x100 <= c->light_low_x100) now |= ALARM_LIGHT_LOW;
        else if (s->light_x100 >= c->light_low_x100 + c->hysteresis_x100) now &= ~ALARM_LIGHT_LOW;
    }
    if (s->valid_flags & APP_VALID_SOIL) {
        if (s->soil_percent <= c->soil_low_percent) now |= ALARM_SOIL_LOW;
        else if (s->soil_percent >= c->soil_low_percent + 5U) now &= ~ALARM_SOIL_LOW;
    }
    return now;
}

static void AlarmTask(void *argument)
{
    SensorData_t sample;
    (void)argument;
    for (;;) {
        if (xQueueReceive(alarm_queue, &sample, pdMS_TO_TICKS(1000U)) == pdTRUE) {
            TickType_t release = xTaskGetTickCount();
            AppConfig_t cfg;
            (void)App_GetConfig(&cfg);
            uint32_t old = active_bits;
            active_bits = Evaluate(&sample, &cfg, old);
            if (old != active_bits) {
                AlarmEvent_t event = {sample.timestamp_ms, active_bits, old ^ active_bits, sample};
                StorageRequest_t req = {.type = STORAGE_APPEND_ALARM};
                req.data.alarm = event;
                (void)StorageService_Request(&req);
                UartService_Response("[ALARM] active=%08lX changed=%08lX\r\n",
                    (unsigned long)event.active_bits, (unsigned long)event.changed_bits);
            }
            if ((xTaskGetTickCount() - release) > pdMS_TO_TICKS(APP_ALARM_DEADLINE_MS))
                App_StatisticsMutable()->deadline_miss_count++;
        }
        HealthService_Heartbeat(HEALTH_ALARM);
    }
}

bool AlarmService_Init(void)
{
    alarm_queue = xQueueCreate(APP_SENSOR_QUEUE_LENGTH, sizeof(SensorData_t));
    return alarm_queue && (xTaskCreate(AlarmTask, "Alarm", APP_ALARM_STACK_WORDS,
             NULL, APP_PRIO_ALARM, NULL) == pdPASS);
}
bool AlarmService_Publish(const SensorData_t *s) { return s && xQueueSend(alarm_queue, s, 0U) == pdTRUE; }
uint32_t AlarmService_GetActive(void) { return active_bits; }
void AlarmService_Acknowledge(void) { UartService_Response("OK ALARM ACK active=%08lX\r\n", (unsigned long)active_bits); }
