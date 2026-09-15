#include "storage_service.h"
#include "app_config.h"
#include "app_internal.h"
#include "health_service.h"
#include "uart_service.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <stddef.h>

#define RECORD_MAGIC 0x464C4F52UL
typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t length;
    SensorData_t sample;
    uint32_t crc32;
    uint32_t commit;
} FlashRecord_t;

static QueueHandle_t storage_queue;
static uint32_t write_address = APP_FLASH_HISTORY_ADDR;
static uint32_t record_count;

static uint32_t Crc32(const void *data, uint32_t length)
{
    const uint8_t *p = data; uint32_t crc = 0xFFFFFFFFUL;
    while (length--) { crc ^= *p++; for (uint32_t i = 0; i < 8U; i++) crc = (crc >> 1) ^ (0xEDB88320UL & (0U - (crc & 1U))); }
    return ~crc;
}

static bool AppendSample(const SensorData_t *sample)
{
    FlashRecord_t r = {RECORD_MAGIC, 1U, sizeof(FlashRecord_t), *sample, 0U, 0xFFFFFFFFUL};
    r.crc32 = Crc32(&r, offsetof(FlashRecord_t, crc32));

    /* Never split one record across erase sectors. */
    uint32_t sector_end = (write_address & ~(W25QXX_SECTOR_SIZE - 1U)) +
                          W25QXX_SECTOR_SIZE;
    if ((write_address + sizeof(r)) > sector_end) write_address = sector_end;
    if ((write_address + sizeof(r)) > APP_FLASH_TEST_ADDR)
        write_address = APP_FLASH_HISTORY_ADDR;

    if ((write_address % W25QXX_SECTOR_SIZE) == 0U) {
        if (W25QXX_EraseSector4K(App_Flash(), write_address) != W25QXX_OK) return false;
    }
    if (W25QXX_Write(App_Flash(), write_address, (uint8_t *)&r,
                     offsetof(FlashRecord_t, commit)) != W25QXX_OK) return false;
    uint32_t commit = 0x434F4D4DUL;
    if (W25QXX_Write(App_Flash(), write_address + offsetof(FlashRecord_t, commit),
                     (uint8_t *)&commit, sizeof(commit)) != W25QXX_OK) return false;
    write_address += sizeof(r); record_count++;
    return true;
}

static void StorageTask(void *argument)
{
    StorageRequest_t req;
    (void)argument;
    for (;;) {
        if (xQueueReceive(storage_queue, &req, pdMS_TO_TICKS(1000U)) == pdTRUE) {
            bool ok = true;
            switch (req.type) {
                case STORAGE_APPEND_SAMPLE: ok = AppendSample(&req.data.sample); break;
                case STORAGE_APPEND_ALARM: ok = AppendSample(&req.data.alarm.sample); break;
                case STORAGE_SAVE_CONFIG:
                    /* TODO: Config A/B copy with generation + CRC. */
                    UartService_Response("ACCEPTED CONFIG SAVE gen=%lu\r\n", (unsigned long)req.data.config.generation);
                    break;
                case STORAGE_READ_HISTORY:
                    /* TODO: Iterate valid committed records and stream in bounded chunks. */
                    UartService_Response("HISTORY count=%lu requested=%u\r\n",
                        (unsigned long)record_count, req.data.history_count);
                    break;
                case STORAGE_ERASE_HISTORY:
                    /* Deliberately not implemented: require an explicit confirmation command. */
                    ok = false; break;
                default: ok = false; break;
            }
            if (!ok) { App_StatisticsMutable()->storage_error_count++; UartService_Response("ERR STORAGE type=%u\r\n", req.type); }
        }
        HealthService_Heartbeat(HEALTH_STORAGE);
    }
}

bool StorageService_Init(void)
{
    storage_queue = xQueueCreate(APP_STORAGE_QUEUE_LENGTH, sizeof(StorageRequest_t));
    return storage_queue && (xTaskCreate(StorageTask, "Storage", APP_STORAGE_STACK_WORDS,
           NULL, APP_PRIO_STORAGE, NULL) == pdPASS);
}
bool StorageService_Request(const StorageRequest_t *r)
{
    if (!r || xQueueSend(storage_queue, r, 0U) != pdTRUE) {
        App_StatisticsMutable()->storage_drop_count++; return false;
    }
    return true;
}
uint32_t StorageService_GetRecordCount(void) { return record_count; }
