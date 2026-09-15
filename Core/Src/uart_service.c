#include "uart_service.h"
#include "app_config.h"
#include "app_internal.h"
#include "health_service.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

typedef struct { uint16_t length; char text[APP_UART_MESSAGE_MAX]; } UartMessage_t;
static QueueHandle_t high_queue, low_queue;

static bool Enqueue(QueueHandle_t queue, const char *format, va_list args)
{
    UartMessage_t msg;
    int n = vsnprintf(msg.text, sizeof(msg.text), format, args);
    if (n < 0) return false;
    msg.length = (uint16_t)((n >= (int)sizeof(msg.text)) ? sizeof(msg.text) - 1U : n);
    if (xQueueSend(queue, &msg, 0U) != pdTRUE) {
        App_StatisticsMutable()->uart_drop_count++;
        return false;
    }
    return true;
}

bool UartService_Response(const char *format, ...)
{
    va_list a; va_start(a, format); bool ok = Enqueue(high_queue, format, a); va_end(a); return ok;
}
bool UartService_Log(const char *format, ...)
{
    va_list a; va_start(a, format); bool ok = Enqueue(low_queue, format, a); va_end(a); return ok;
}

static void UartTask(void *argument)
{
    UartMessage_t msg;
    (void)argument;
    for (;;) {
        BaseType_t got = xQueueReceive(high_queue, &msg, 0U);
        if (got != pdTRUE) got = xQueueReceive(low_queue, &msg, pdMS_TO_TICKS(500U));
        if (got == pdTRUE)
            (void)HAL_UART_Transmit(App_Uart(), (uint8_t *)msg.text, msg.length, 1000U);
        HealthService_Heartbeat(HEALTH_UART_TX);
    }
}

bool UartService_Init(void)
{
    high_queue = xQueueCreate(APP_UART_HIGH_QUEUE_LENGTH, sizeof(UartMessage_t));
    low_queue = xQueueCreate(APP_UART_LOW_QUEUE_LENGTH, sizeof(UartMessage_t));
    return high_queue && low_queue &&
        (xTaskCreate(UartTask, "UartTx", APP_UART_STACK_WORDS, NULL,
                     APP_PRIO_UART, NULL) == pdPASS);
}
