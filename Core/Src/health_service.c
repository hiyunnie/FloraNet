#include "health_service.h"
#include "app_config.h"
#include "app_internal.h"
#include "uart_service.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

#define REQUIRED_BITS (HEALTH_SENSOR | HEALTH_ALARM | HEALTH_STORAGE | HEALTH_COMMAND | HEALTH_UART_TX)
static EventGroupHandle_t heartbeats;

void HealthService_Heartbeat(HealthBit_t bit)
{
    if (heartbeats != NULL) (void)xEventGroupSetBits(heartbeats, (EventBits_t)bit);
}

static void HealthTask(void *argument)
{
    (void)argument;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(APP_HEALTH_PERIOD_MS));
        EventBits_t bits = xEventGroupGetBits(heartbeats);
        xEventGroupClearBits(heartbeats, REQUIRED_BITS);
        if ((bits & REQUIRED_BITS) == REQUIRED_BITS) {
            /* TODO: HAL_IWDG_Refresh(&hiwdg); only here. */
            UartService_Log("[HEALTH] OK bits=%02lX\r\n", (unsigned long)bits);
        } else {
            UartService_Response("[HEALTH] DEGRADED missing=%02lX\r\n",
                (unsigned long)(REQUIRED_BITS & ~bits));
        }
    }
}

bool HealthService_Init(void)
{
    heartbeats = xEventGroupCreate();
    return heartbeats && (xTaskCreate(HealthTask, "Health", APP_HEALTH_STACK_WORDS,
          NULL, APP_PRIO_HEALTH, NULL) == pdPASS);
}
