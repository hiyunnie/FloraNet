#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define APP_VALID_SHT3X       (1UL << 0)
#define APP_VALID_BH1750      (1UL << 1)
#define APP_VALID_SOIL        (1UL << 2)

#define APP_ERR_SHT3X         (1UL << 0)
#define APP_ERR_BH1750        (1UL << 1)
#define APP_ERR_SOIL          (1UL << 2)
#define APP_ERR_STORAGE       (1UL << 3)

typedef struct {
    uint32_t sequence;
    uint32_t timestamp_ms;
    int16_t temperature_x100;
    uint16_t humidity_x100;
    uint32_t light_x100;
    uint16_t soil_raw;
    uint8_t soil_percent;
    uint32_t valid_flags;
    uint32_t error_flags;
} SensorData_t;

typedef struct {
    int16_t temperature_high_x100;
    int16_t temperature_low_x100;
    uint16_t humidity_high_x100;
    uint16_t humidity_low_x100;
    uint32_t light_low_x100;
    uint8_t soil_low_percent;
    uint16_t hysteresis_x100;
    uint32_t sample_period_ms;
    uint32_t store_period_ms;
    uint32_t generation;
} AppConfig_t;

typedef enum {
    ALARM_NONE       = 0,
    ALARM_TEMP_HIGH  = 1UL << 0,
    ALARM_TEMP_LOW   = 1UL << 1,
    ALARM_RH_HIGH    = 1UL << 2,
    ALARM_RH_LOW     = 1UL << 3,
    ALARM_LIGHT_LOW  = 1UL << 4,
    ALARM_SOIL_LOW   = 1UL << 5
} AlarmBits_t;

typedef struct {
    uint32_t timestamp_ms;
    uint32_t active_bits;
    uint32_t changed_bits;
    SensorData_t sample;
} AlarmEvent_t;

typedef enum {
    STORAGE_APPEND_SAMPLE,
    STORAGE_APPEND_ALARM,
    STORAGE_SAVE_CONFIG,
    STORAGE_READ_HISTORY,
    STORAGE_ERASE_HISTORY
} StorageRequestType_t;

typedef struct {
    StorageRequestType_t type;
    union {
        SensorData_t sample;
        AlarmEvent_t alarm;
        AppConfig_t config;
        uint16_t history_count;
    } data;
} StorageRequest_t;

typedef enum {
    HEALTH_SENSOR  = 1UL << 0,
    HEALTH_ALARM   = 1UL << 1,
    HEALTH_STORAGE = 1UL << 2,
    HEALTH_COMMAND = 1UL << 3,
    HEALTH_UART_TX = 1UL << 4
} HealthBit_t;

typedef struct {
    uint32_t deadline_miss_count;
    uint32_t sensor_error_count;
    uint32_t storage_error_count;
    uint32_t storage_drop_count;
    uint32_t uart_drop_count;
    uint32_t command_overflow_count;
} AppStatistics_t;

#endif
