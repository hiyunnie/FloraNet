#ifndef SOIL_MOISTURE_H
#define SOIL_MOISTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    SOIL_MOISTURE_OK = 0,
    SOIL_MOISTURE_ERROR,
    SOIL_MOISTURE_BUSY,
    SOIL_MOISTURE_TIMEOUT,
    SOIL_MOISTURE_INVALID_ARGUMENT,
    SOIL_MOISTURE_NOT_CALIBRATED
} SoilMoisture_Result;

typedef struct
{
    ADC_HandleTypeDef *adc;
    GPIO_TypeDef *digital_port;
    uint16_t digital_pin;
    uint32_t adc_timeout_ms;
    uint16_t dry_raw;
    uint16_t wet_raw;
    bool digital_wet_active_low;
    bool calibration_valid;
} SoilMoisture_Handle;

void SoilMoisture_Init(SoilMoisture_Handle *sensor, ADC_HandleTypeDef *adc,
                       uint32_t adc_timeout_ms);
void SoilMoisture_SetDigitalInput(SoilMoisture_Handle *sensor,
                                 GPIO_TypeDef *port, uint16_t pin,
                                 bool wet_active_low);
SoilMoisture_Result SoilMoisture_SetCalibration(SoilMoisture_Handle *sensor,
                                               uint16_t dry_raw,
                                               uint16_t wet_raw);
SoilMoisture_Result SoilMoisture_ReadRaw(SoilMoisture_Handle *sensor,
                                        uint16_t *raw);
SoilMoisture_Result SoilMoisture_ReadAverage(SoilMoisture_Handle *sensor,
                                            uint16_t sample_count,
                                            uint16_t *raw_average);
SoilMoisture_Result SoilMoisture_ReadPercent(SoilMoisture_Handle *sensor,
                                            uint16_t sample_count,
                                            float *percent,
                                            uint16_t *raw_average);
SoilMoisture_Result SoilMoisture_ReadDigitalWet(SoilMoisture_Handle *sensor,
                                               bool *is_wet);

#ifdef __cplusplus
}
#endif
#endif
