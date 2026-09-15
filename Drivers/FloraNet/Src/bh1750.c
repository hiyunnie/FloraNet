#include "bh1750.h"
#include <stddef.h>

#define BH1750_CMD_POWER_DOWN  0x00U
#define BH1750_CMD_POWER_ON    0x01U
#define BH1750_CMD_RESET       0x07U
#define BH1750_MTREG_DEFAULT   69U
#define BH1750_MTREG_MIN       31U
#define BH1750_MTREG_MAX       254U

static BH1750_Result BH1750_FromHAL(HAL_StatusTypeDef status)
{
    switch (status)
    {
        case HAL_OK: return BH1750_OK;
        case HAL_BUSY: return BH1750_BUSY;
        case HAL_TIMEOUT: return BH1750_TIMEOUT;
        default: return BH1750_ERROR;
    }
}

static BH1750_Result BH1750_WriteCommand(BH1750_Handle *sensor, uint8_t command)
{
    if ((sensor == NULL) || (sensor->i2c == NULL))
    {
        return BH1750_INVALID_ARGUMENT;
    }
    return BH1750_FromHAL(HAL_I2C_Master_Transmit(sensor->i2c,
                                                  sensor->hal_address,
                                                  &command, 1U,
                                                  sensor->timeout_ms));
}

void BH1750_Init(BH1750_Handle *sensor, I2C_HandleTypeDef *i2c,
                 uint8_t address_7bit, uint32_t timeout_ms)
{
    if (sensor == NULL) return;
    sensor->i2c = i2c;
    sensor->hal_address = (uint16_t)((uint16_t)address_7bit << 1);
    sensor->timeout_ms = timeout_ms;
    sensor->mtreg = BH1750_MTREG_DEFAULT;
    sensor->mode = BH1750_ONE_TIME_HIGH_RES;
}

BH1750_Result BH1750_Probe(BH1750_Handle *sensor)
{
    if ((sensor == NULL) || (sensor->i2c == NULL)) return BH1750_INVALID_ARGUMENT;
    return BH1750_FromHAL(HAL_I2C_IsDeviceReady(sensor->i2c,
                                                sensor->hal_address,
                                                3U, sensor->timeout_ms));
}

BH1750_Result BH1750_PowerOn(BH1750_Handle *sensor)
{
    return BH1750_WriteCommand(sensor, BH1750_CMD_POWER_ON);
}

BH1750_Result BH1750_PowerDown(BH1750_Handle *sensor)
{
    return BH1750_WriteCommand(sensor, BH1750_CMD_POWER_DOWN);
}

BH1750_Result BH1750_ResetData(BH1750_Handle *sensor)
{
    BH1750_Result result = BH1750_PowerOn(sensor);
    return (result == BH1750_OK) ? BH1750_WriteCommand(sensor, BH1750_CMD_RESET)
                                : result;
}

BH1750_Result BH1750_SetMeasurementTime(BH1750_Handle *sensor, uint8_t mtreg)
{
    BH1750_Result result;
    uint8_t high;
    uint8_t low;
    if ((mtreg < BH1750_MTREG_MIN) || (mtreg > BH1750_MTREG_MAX))
        return BH1750_INVALID_ARGUMENT;

    high = (uint8_t)(0x40U | (mtreg >> 5));
    low = (uint8_t)(0x60U | (mtreg & 0x1FU));
    result = BH1750_WriteCommand(sensor, high);
    if (result != BH1750_OK) return result;
    result = BH1750_WriteCommand(sensor, low);
    if (result == BH1750_OK) sensor->mtreg = mtreg;
    return result;
}

BH1750_Result BH1750_StartMeasurement(BH1750_Handle *sensor, BH1750_Mode mode)
{
    switch (mode)
    {
        case BH1750_CONTINUOUS_HIGH_RES:
        case BH1750_CONTINUOUS_HIGH_RES_2:
        case BH1750_CONTINUOUS_LOW_RES:
        case BH1750_ONE_TIME_HIGH_RES:
        case BH1750_ONE_TIME_HIGH_RES_2:
        case BH1750_ONE_TIME_LOW_RES:
            break;
        default:
            return BH1750_INVALID_ARGUMENT;
    }
    BH1750_Result result = BH1750_WriteCommand(sensor, (uint8_t)mode);
    if (result == BH1750_OK) sensor->mode = mode;
    return result;
}

BH1750_Result BH1750_Read(BH1750_Handle *sensor, uint16_t *raw, float *lux)
{
    uint8_t rx[2];
    uint16_t value;
    float calculated;
    HAL_StatusTypeDef hal_status;
    if ((sensor == NULL) || (sensor->i2c == NULL) || (raw == NULL) || (lux == NULL))
        return BH1750_INVALID_ARGUMENT;

    hal_status = HAL_I2C_Master_Receive(sensor->i2c, sensor->hal_address,
                                        rx, sizeof(rx), sensor->timeout_ms);
    if (hal_status != HAL_OK) return BH1750_FromHAL(hal_status);

    value = (uint16_t)(((uint16_t)rx[0] << 8) | rx[1]);
    calculated = ((float)value / 1.2f) *
                 ((float)BH1750_MTREG_DEFAULT / (float)sensor->mtreg);
    if ((sensor->mode == BH1750_CONTINUOUS_HIGH_RES_2) ||
        (sensor->mode == BH1750_ONE_TIME_HIGH_RES_2))
        calculated /= 2.0f;

    *raw = value;
    *lux = calculated;
    return BH1750_OK;
}

uint32_t BH1750_GetMeasurementDelayMs(const BH1750_Handle *sensor,
                                      BH1750_Mode mode)
{
    uint32_t base;
    uint32_t mtreg;
    if (sensor == NULL) return 0U;
    base = ((mode == BH1750_CONTINUOUS_LOW_RES) ||
            (mode == BH1750_ONE_TIME_LOW_RES)) ? 24U : 180U;
    mtreg = sensor->mtreg;
    return ((base * mtreg) + (BH1750_MTREG_DEFAULT - 1U)) /
           BH1750_MTREG_DEFAULT;
}

BH1750_Result BH1750_ReadBlocking(BH1750_Handle *sensor, BH1750_Mode mode,
                                 uint16_t *raw, float *lux)
{
    BH1750_Result result = BH1750_StartMeasurement(sensor, mode);
    if (result != BH1750_OK) return result;
    HAL_Delay(BH1750_GetMeasurementDelayMs(sensor, mode));
    return BH1750_Read(sensor, raw, lux);
}
