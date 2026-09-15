#include "command_service.h"
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static QueueHandle_t rx_queue;
static uint8_t rx_byte;

static void PrintSensor(void)
{
    SensorData_t s;
    if (!App_GetLatest(&s)) { UartService_Response("ERR SENSOR UNAVAILABLE\r\n"); return; }
    UartService_Response("OK SEQ=%lu T=%d.%02d RH=%u.%02u LUX=%lu.%02lu SOIL=%u VALID=%08lX ERR=%08lX\r\n",
        (unsigned long)s.sequence, s.temperature_x100 / 100,
        s.temperature_x100 < 0 ? -(s.temperature_x100 % 100) : s.temperature_x100 % 100,
        s.humidity_x100 / 100, s.humidity_x100 % 100,
        (unsigned long)(s.light_x100 / 100), (unsigned long)(s.light_x100 % 100), s.soil_percent,
        (unsigned long)s.valid_flags, (unsigned long)s.error_flags);
}

static void Execute(char *cmd)
{
    AppConfig_t cfg;
    (void)App_GetConfig(&cfg);
    if (!strcmp(cmd, "GET SENSOR")) PrintSensor();
    else if (!strcmp(cmd, "GET ALARM")) UartService_Response("OK ALARM=%08lX TEMP_HIGH=%d SOIL_LOW=%u\r\n", (unsigned long)AlarmService_GetActive(), cfg.temperature_high_x100, cfg.soil_low_percent);
    else if (!strcmp(cmd, "GET STATUS")) {
        AppStatistics_t s; App_GetStatistics(&s);
        UartService_Response("OK RECORDS=%lu MISS=%lu SENSOR_ERR=%lu STORAGE_ERR=%lu DROP=%lu\r\n",
            (unsigned long)StorageService_GetRecordCount(), (unsigned long)s.deadline_miss_count,
            (unsigned long)s.sensor_error_count, (unsigned long)s.storage_error_count,
            (unsigned long)(s.storage_drop_count + s.uart_drop_count));
    } else if (!strncmp(cmd, "GET HISTORY ", 12)) {
        StorageRequest_t r = {.type = STORAGE_READ_HISTORY}; r.data.history_count = (uint16_t)strtoul(cmd + 12, NULL, 10);
        UartService_Response(StorageService_Request(&r) ? "ACCEPTED\r\n" : "ERR BUSY\r\n");
    } else if (!strncmp(cmd, "SET TEMP_HIGH ", 14)) {
        cfg.temperature_high_x100 = (int16_t)strtol(cmd + 14, NULL, 10);
        UartService_Response(App_UpdateConfig(&cfg) ? "OK\r\n" : "ERR VALUE\r\n");
    } else if (!strncmp(cmd, "SET SOIL_LOW ", 13)) {
        cfg.soil_low_percent = (uint8_t)strtoul(cmd + 13, NULL, 10);
        UartService_Response(App_UpdateConfig(&cfg) ? "OK\r\n" : "ERR VALUE\r\n");
    } else if (!strncmp(cmd, "SET SAMPLE_PERIOD ", 18)) {
        cfg.sample_period_ms = strtoul(cmd + 18, NULL, 10);
        UartService_Response(App_UpdateConfig(&cfg) ? "OK\r\n" : "ERR VALUE\r\n");
    } else if (!strncmp(cmd, "SET STORE_PERIOD ", 17)) {
        cfg.store_period_ms = strtoul(cmd + 17, NULL, 10);
        UartService_Response(App_UpdateConfig(&cfg) ? "OK\r\n" : "ERR VALUE\r\n");
    } else if (!strcmp(cmd, "SAVE CONFIG")) {
        StorageRequest_t r = {.type = STORAGE_SAVE_CONFIG}; r.data.config = cfg;
        UartService_Response(StorageService_Request(&r) ? "ACCEPTED\r\n" : "ERR BUSY\r\n");
    } else if (!strcmp(cmd, "ACK ALARM")) AlarmService_Acknowledge();
    else if (!strcmp(cmd, "FLASH INFO")) UartService_Response("OK JEDEC=%02X %02X %02X CAP=%lu\r\n", App_Flash()->manufacturer_id, App_Flash()->memory_type, App_Flash()->capacity_id, (unsigned long)App_Flash()->capacity_bytes);
    else if (!strcmp(cmd, "HELP")) UartService_Response("GET SENSOR|STATUS|ALARM|HISTORY n; SET TEMP_HIGH n|SOIL_LOW n|SAMPLE_PERIOD n|STORE_PERIOD n; SAVE CONFIG; ACK ALARM; FLASH INFO\r\n");
    else UartService_Response("ERR UNKNOWN COMMAND\r\n");
}

static void CommandTask(void *argument)
{
    char line[APP_COMMAND_MAX]; uint32_t used = 0U; uint8_t ch;
    (void)argument;
    (void)HAL_UART_Receive_IT(App_Uart(), &rx_byte, 1U);
    for (;;) {
        if (xQueueReceive(rx_queue, &ch, pdMS_TO_TICKS(1000U)) == pdTRUE) {
            if ((ch == '\r') || (ch == '\n')) {
                if (used) { line[used] = '\0'; Execute(line); used = 0U; }
            } else if (used < sizeof(line) - 1U) line[used++] = (char)ch;
            else { used = 0U; App_StatisticsMutable()->command_overflow_count++; UartService_Response("ERR COMMAND TOO LONG\r\n"); }
        }
        HealthService_Heartbeat(HEALTH_COMMAND);
    }
}

bool CommandService_Init(void)
{
    rx_queue = xQueueCreate(64U, sizeof(uint8_t));
    return rx_queue && (xTaskCreate(CommandTask, "Command", APP_COMMAND_STACK_WORDS,
          NULL, APP_PRIO_COMMAND, NULL) == pdPASS);
}
/*
bool CommandService_Init(void)
{
    printf("[INIT] Command free heap before queue=%lu\r\n",
           (unsigned long)xPortGetFreeHeapSize());

    rx_queue = xQueueCreate(64U, sizeof(uint8_t));

    if (rx_queue == NULL)
    {
        printf("[ERROR] Command RX queue creation failed, heap=%lu\r\n",
               (unsigned long)xPortGetFreeHeapSize());
        return false;
    }

    printf("[INIT] Command free heap before task=%lu\r\n",
           (unsigned long)xPortGetFreeHeapSize());

    BaseType_t result = xTaskCreate(
        CommandTask,
        "Command",
        APP_COMMAND_STACK_WORDS,
        NULL,
        APP_PRIO_COMMAND,
        NULL);

    if (result != pdPASS)
    {
        printf("[ERROR] CommandTask creation failed, heap=%lu\r\n",
               (unsigned long)xPortGetFreeHeapSize());
        return false;
    }

    printf("[INIT] CommandService OK, remaining heap=%lu\r\n",
           (unsigned long)xPortGetFreeHeapSize());

    return true;
}
*/
void CommandService_RxCompleteISR(UART_HandleTypeDef *huart)
{
    if (huart != App_Uart()) return;
    BaseType_t wake = pdFALSE;
    (void)xQueueSendFromISR(rx_queue, &rx_byte, &wake);
    (void)HAL_UART_Receive_IT(huart, &rx_byte, 1U);
    portYIELD_FROM_ISR(wake);
}
