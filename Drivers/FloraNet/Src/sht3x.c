#include "sht3x.h"
#include <stddef.h>

#define SHT3X_CMD_FETCH_DATA             0xE000U
#define SHT3X_CMD_BREAK                  0x3093U
#define SHT3X_CMD_SOFT_RESET             0x30A2U
#define SHT3X_CMD_HEATER_ENABLE          0x306DU
#define SHT3X_CMD_HEATER_DISABLE         0x3066U
#define SHT3X_CMD_READ_STATUS            0xF32DU
#define SHT3X_CMD_CLEAR_STATUS           0x3041U

#define SHT3X_RX_MEASUREMENT_SIZE        6U
#define SHT3X_RX_STATUS_SIZE             3U

static SHT3x_Result SHT3x_FromHAL(HAL_StatusTypeDef status)
{
    switch (status)
    {
        case HAL_OK:      return SHT3X_OK;
        case HAL_BUSY:    return SHT3X_BUSY;
        case HAL_TIMEOUT: return SHT3X_TIMEOUT;
        default:          return SHT3X_ERROR;
    }
}

static bool SHT3x_IsValid(const SHT3x_Handle *sensor)
{
    return (sensor != NULL) && (sensor->i2c != NULL);
}

static SHT3x_Result SHT3x_Receive(SHT3x_Handle *sensor,
                                 uint8_t *data,
                                 uint16_t length)
{
    if (!SHT3x_IsValid(sensor) || (data == NULL) || (length == 0U))
    {
        return SHT3X_INVALID_ARGUMENT;
    }

    return SHT3x_FromHAL(HAL_I2C_Master_Receive(sensor->i2c,
                                                sensor->hal_address,
                                                data,
                                                length,
                                                sensor->timeout_ms));
}

void SHT3x_Init(SHT3x_Handle *sensor,
                I2C_HandleTypeDef *i2c,
                uint8_t address_7bit,
                uint32_t timeout_ms)
{
    if (sensor == NULL)
    {
        return;
    }

    sensor->i2c = i2c;
    sensor->hal_address = (uint16_t)((uint16_t)address_7bit << 1);
    sensor->timeout_ms = timeout_ms;
}

SHT3x_Result SHT3x_Probe(SHT3x_Handle *sensor)
{
    if (!SHT3x_IsValid(sensor))
    {
        return SHT3X_INVALID_ARGUMENT;
    }

    return SHT3x_FromHAL(HAL_I2C_IsDeviceReady(sensor->i2c,
                                                sensor->hal_address,
                                                3U,
                                                sensor->timeout_ms));
}

SHT3x_Result SHT3x_SendCommand(SHT3x_Handle *sensor, uint16_t command)
{
    uint8_t tx[2];

    if (!SHT3x_IsValid(sensor))
    {
        return SHT3X_INVALID_ARGUMENT;
    }

    tx[0] = (uint8_t)(command >> 8);
    tx[1] = (uint8_t)(command & 0xFFU);

    return SHT3x_FromHAL(HAL_I2C_Master_Transmit(sensor->i2c,
                                                 sensor->hal_address,
                                                 tx,
                                                 sizeof(tx),
                                                 sensor->timeout_ms));
}

uint32_t SHT3x_GetMeasurementDelayMs(SHT3x_Repeatability repeatability)
{
    switch (repeatability)
    {
        case SHT3X_REPEATABILITY_LOW:   return 4U;
        case SHT3X_REPEATABILITY_MEDIUM:return 6U;
        case SHT3X_REPEATABILITY_HIGH:  return 15U;
        default:                        return 0U;
    }
}

SHT3x_Result SHT3x_StartSingleShot(SHT3x_Handle *sensor,
                                  SHT3x_Repeatability repeatability,
                                  bool clock_stretching)
{
    static const uint16_t no_stretch_commands[] = {0x2416U, 0x240BU, 0x2400U};
    static const uint16_t stretch_commands[] = {0x2C10U, 0x2C0DU, 0x2C06U};

    if (repeatability > SHT3X_REPEATABILITY_HIGH)
    {
        return SHT3X_INVALID_ARGUMENT;
    }

    return SHT3x_SendCommand(sensor,
        clock_stretching ? stretch_commands[repeatability]
                         : no_stretch_commands[repeatability]);
}

uint8_t SHT3x_CalculateCRC(const uint8_t *data, uint32_t length)
{
    uint8_t crc = 0xFFU;
    uint32_t i;
    uint8_t bit;

    if (data == NULL)
    {
        return 0U;
    }

    for (i = 0U; i < length; ++i)
    {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; ++bit)
        {
            crc = ((crc & 0x80U) != 0U)
                ? (uint8_t)((crc << 1) ^ 0x31U)
                : (uint8_t)(crc << 1);
        }
    }

    return crc;
}

SHT3x_Result SHT3x_ReadMeasurement(SHT3x_Handle *sensor,
                                  SHT3x_Measurement *measurement)
{
    uint8_t rx[SHT3X_RX_MEASUREMENT_SIZE];
    SHT3x_Result result;

    if (measurement == NULL)
    {
        return SHT3X_INVALID_ARGUMENT;
    }

    result = SHT3x_Receive(sensor, rx, sizeof(rx));
    if (result != SHT3X_OK)
    {
        return result;
    }

    if ((SHT3x_CalculateCRC(&rx[0], 2U) != rx[2]) ||
        (SHT3x_CalculateCRC(&rx[3], 2U) != rx[5]))
    {
        return SHT3X_CRC_ERROR;
    }

    measurement->raw_temperature = (uint16_t)(((uint16_t)rx[0] << 8) | rx[1]);
    measurement->raw_humidity = (uint16_t)(((uint16_t)rx[3] << 8) | rx[4]);
    measurement->temperature_c = -45.0f +
        (175.0f * (float)measurement->raw_temperature / 65535.0f);
    measurement->humidity_rh =
        100.0f * (float)measurement->raw_humidity / 65535.0f;

    return SHT3X_OK;
}

SHT3x_Result SHT3x_ReadSingleShotBlocking(SHT3x_Handle *sensor,
                                         SHT3x_Repeatability repeatability,
                                         bool clock_stretching,
                                         SHT3x_Measurement *measurement)
{
    SHT3x_Result result = SHT3x_StartSingleShot(sensor,
                                                repeatability,
                                                clock_stretching);
    if (result != SHT3X_OK)
    {
        return result;
    }

    if (!clock_stretching)
    {
        HAL_Delay(SHT3x_GetMeasurementDelayMs(repeatability));
    }

    return SHT3x_ReadMeasurement(sensor, measurement);
}

SHT3x_Result SHT3x_StartPeriodic(SHT3x_Handle *sensor,
                                SHT3x_Rate rate,
                                SHT3x_Repeatability repeatability)
{
    static const uint16_t commands[5][3] =
    {
        {0x202FU, 0x2024U, 0x2032U},
        {0x212DU, 0x2126U, 0x2130U},
        {0x222BU, 0x2220U, 0x2236U},
        {0x2329U, 0x2322U, 0x2334U},
        {0x272AU, 0x2721U, 0x2737U}
    };

    if ((rate > SHT3X_RATE_10_HZ) ||
        (repeatability > SHT3X_REPEATABILITY_HIGH))
    {
        return SHT3X_INVALID_ARGUMENT;
    }

    return SHT3x_SendCommand(sensor, commands[rate][repeatability]);
}

SHT3x_Result SHT3x_FetchPeriodic(SHT3x_Handle *sensor,
                                SHT3x_Measurement *measurement)
{
    SHT3x_Result result = SHT3x_SendCommand(sensor, SHT3X_CMD_FETCH_DATA);
    if (result != SHT3X_OK)
    {
        return result;
    }

    return SHT3x_ReadMeasurement(sensor, measurement);
}

SHT3x_Result SHT3x_StopPeriodic(SHT3x_Handle *sensor)
{
    SHT3x_Result result = SHT3x_SendCommand(sensor, SHT3X_CMD_BREAK);
    if (result == SHT3X_OK)
    {
        HAL_Delay(15U);
    }
    return result;
}

SHT3x_Result SHT3x_ReadStatus(SHT3x_Handle *sensor, uint16_t *status)
{
    uint8_t rx[SHT3X_RX_STATUS_SIZE];
    SHT3x_Result result;

    if (status == NULL)
    {
        return SHT3X_INVALID_ARGUMENT;
    }

    result = SHT3x_SendCommand(sensor, SHT3X_CMD_READ_STATUS);
    if (result != SHT3X_OK)
    {
        return result;
    }

    result = SHT3x_Receive(sensor, rx, sizeof(rx));
    if (result != SHT3X_OK)
    {
        return result;
    }

    if (SHT3x_CalculateCRC(rx, 2U) != rx[2])
    {
        return SHT3X_CRC_ERROR;
    }

    *status = (uint16_t)(((uint16_t)rx[0] << 8) | rx[1]);
    return SHT3X_OK;
}

SHT3x_Result SHT3x_ClearStatus(SHT3x_Handle *sensor)
{
    return SHT3x_SendCommand(sensor, SHT3X_CMD_CLEAR_STATUS);
}

SHT3x_Result SHT3x_SoftReset(SHT3x_Handle *sensor)
{
    SHT3x_Result result = SHT3x_SendCommand(sensor, SHT3X_CMD_SOFT_RESET);
    if (result == SHT3X_OK)
    {
        HAL_Delay(2U);
    }
    return result;
}

SHT3x_Result SHT3x_SetHeater(SHT3x_Handle *sensor, bool enable)
{
    return SHT3x_SendCommand(sensor,
                             enable ? SHT3X_CMD_HEATER_ENABLE
                                    : SHT3X_CMD_HEATER_DISABLE);
}
