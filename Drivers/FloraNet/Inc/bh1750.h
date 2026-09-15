#ifndef BH1750_H
#define BH1750_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define BH1750_ADDRESS_LOW   0x23U
#define BH1750_ADDRESS_HIGH  0x5CU

typedef enum
{
    BH1750_OK = 0,
    BH1750_ERROR,
    BH1750_BUSY,
    BH1750_TIMEOUT,
    BH1750_INVALID_ARGUMENT
} BH1750_Result;

typedef enum
{
    BH1750_CONTINUOUS_HIGH_RES = 0x10U,
    BH1750_CONTINUOUS_HIGH_RES_2 = 0x11U,
    BH1750_CONTINUOUS_LOW_RES = 0x13U,
    BH1750_ONE_TIME_HIGH_RES = 0x20U,
    BH1750_ONE_TIME_HIGH_RES_2 = 0x21U,
    BH1750_ONE_TIME_LOW_RES = 0x23U
} BH1750_Mode;

typedef struct
{
    I2C_HandleTypeDef *i2c;
    uint16_t hal_address;
    uint32_t timeout_ms;
    uint8_t mtreg;
    BH1750_Mode mode;
} BH1750_Handle;

void BH1750_Init(BH1750_Handle *sensor, I2C_HandleTypeDef *i2c,
                 uint8_t address_7bit, uint32_t timeout_ms);
BH1750_Result BH1750_Probe(BH1750_Handle *sensor);
BH1750_Result BH1750_PowerOn(BH1750_Handle *sensor);
BH1750_Result BH1750_PowerDown(BH1750_Handle *sensor);
BH1750_Result BH1750_ResetData(BH1750_Handle *sensor);
BH1750_Result BH1750_SetMeasurementTime(BH1750_Handle *sensor, uint8_t mtreg);
BH1750_Result BH1750_StartMeasurement(BH1750_Handle *sensor, BH1750_Mode mode);
BH1750_Result BH1750_Read(BH1750_Handle *sensor, uint16_t *raw, float *lux);
BH1750_Result BH1750_ReadBlocking(BH1750_Handle *sensor, BH1750_Mode mode,
                                 uint16_t *raw, float *lux);
uint32_t BH1750_GetMeasurementDelayMs(const BH1750_Handle *sensor,
                                      BH1750_Mode mode);

#ifdef __cplusplus
}
#endif
#endif
