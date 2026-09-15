#ifndef SHT3X_H
#define SHT3X_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define SHT3X_ADDRESS_A                 0x44U
#define SHT3X_ADDRESS_B                 0x45U

#define SHT3X_STATUS_ALERT_PENDING      (1U << 15)
#define SHT3X_STATUS_HEATER_ON          (1U << 13)
#define SHT3X_STATUS_RH_ALERT           (1U << 11)
#define SHT3X_STATUS_T_ALERT            (1U << 10)
#define SHT3X_STATUS_RESET_DETECTED     (1U << 4)
#define SHT3X_STATUS_COMMAND_ERROR      (1U << 1)
#define SHT3X_STATUS_WRITE_CRC_ERROR    (1U << 0)

typedef enum
{
    SHT3X_OK = 0,
    SHT3X_ERROR,
    SHT3X_BUSY,
    SHT3X_TIMEOUT,
    SHT3X_CRC_ERROR,
    SHT3X_INVALID_ARGUMENT
} SHT3x_Result;

typedef enum
{
    SHT3X_REPEATABILITY_LOW = 0,
    SHT3X_REPEATABILITY_MEDIUM,
    SHT3X_REPEATABILITY_HIGH
} SHT3x_Repeatability;

typedef enum
{
    SHT3X_RATE_0_5_HZ = 0,
    SHT3X_RATE_1_HZ,
    SHT3X_RATE_2_HZ,
    SHT3X_RATE_4_HZ,
    SHT3X_RATE_10_HZ
} SHT3x_Rate;

typedef struct
{
    float temperature_c;
    float humidity_rh;
    uint16_t raw_temperature;
    uint16_t raw_humidity;
} SHT3x_Measurement;

typedef struct
{
    I2C_HandleTypeDef *i2c;
    uint16_t hal_address;
    uint32_t timeout_ms;
} SHT3x_Handle;

void SHT3x_Init(SHT3x_Handle *sensor,
                I2C_HandleTypeDef *i2c,
                uint8_t address_7bit,
                uint32_t timeout_ms);

SHT3x_Result SHT3x_Probe(SHT3x_Handle *sensor);
SHT3x_Result SHT3x_SendCommand(SHT3x_Handle *sensor, uint16_t command);

/* FreeRTOS-friendly non-blocking sequence:
 * 1. SHT3x_StartSingleShot()
 * 2. vTaskDelay(15/6/4 ms when clock_stretching == false)
 * 3. SHT3x_ReadMeasurement()
 */
SHT3x_Result SHT3x_StartSingleShot(SHT3x_Handle *sensor,
                                  SHT3x_Repeatability repeatability,
                                  bool clock_stretching);
SHT3x_Result SHT3x_ReadMeasurement(SHT3x_Handle *sensor,
                                  SHT3x_Measurement *measurement);

/* Convenience blocking API. HAL_Delay() is used while conversion is running. */
SHT3x_Result SHT3x_ReadSingleShotBlocking(SHT3x_Handle *sensor,
                                         SHT3x_Repeatability repeatability,
                                         bool clock_stretching,
                                         SHT3x_Measurement *measurement);

SHT3x_Result SHT3x_StartPeriodic(SHT3x_Handle *sensor,
                                SHT3x_Rate rate,
                                SHT3x_Repeatability repeatability);
SHT3x_Result SHT3x_FetchPeriodic(SHT3x_Handle *sensor,
                                SHT3x_Measurement *measurement);
SHT3x_Result SHT3x_StopPeriodic(SHT3x_Handle *sensor);

SHT3x_Result SHT3x_ReadStatus(SHT3x_Handle *sensor, uint16_t *status);
SHT3x_Result SHT3x_ClearStatus(SHT3x_Handle *sensor);
SHT3x_Result SHT3x_SoftReset(SHT3x_Handle *sensor);
SHT3x_Result SHT3x_SetHeater(SHT3x_Handle *sensor, bool enable);

uint8_t SHT3x_CalculateCRC(const uint8_t *data, uint32_t length);
uint32_t SHT3x_GetMeasurementDelayMs(SHT3x_Repeatability repeatability);

#ifdef __cplusplus
}
#endif

#endif /* SHT3X_H */
