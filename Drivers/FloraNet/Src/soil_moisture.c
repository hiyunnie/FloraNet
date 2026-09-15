#include "soil_moisture.h"
#include <stddef.h>

static SoilMoisture_Result SoilMoisture_FromHAL(HAL_StatusTypeDef status)
{
    switch (status)
    {
        case HAL_OK: return SOIL_MOISTURE_OK;
        case HAL_BUSY: return SOIL_MOISTURE_BUSY;
        case HAL_TIMEOUT: return SOIL_MOISTURE_TIMEOUT;
        default: return SOIL_MOISTURE_ERROR;
    }
}

void SoilMoisture_Init(SoilMoisture_Handle *sensor, ADC_HandleTypeDef *adc,
                       uint32_t adc_timeout_ms)
{
    if (sensor == NULL) return;
    sensor->adc = adc;
    sensor->digital_port = NULL;
    sensor->digital_pin = 0U;
    sensor->adc_timeout_ms = adc_timeout_ms;
    sensor->dry_raw = 0U;
    sensor->wet_raw = 0U;
    sensor->digital_wet_active_low = true;
    sensor->calibration_valid = false;
}

void SoilMoisture_SetDigitalInput(SoilMoisture_Handle *sensor,
                                 GPIO_TypeDef *port, uint16_t pin,
                                 bool wet_active_low)
{
    if (sensor == NULL) return;
    sensor->digital_port = port;
    sensor->digital_pin = pin;
    sensor->digital_wet_active_low = wet_active_low;
}

SoilMoisture_Result SoilMoisture_SetCalibration(SoilMoisture_Handle *sensor,
                                               uint16_t dry_raw,
                                               uint16_t wet_raw)
{
    if ((sensor == NULL) || (dry_raw == wet_raw))
        return SOIL_MOISTURE_INVALID_ARGUMENT;
    sensor->dry_raw = dry_raw;
    sensor->wet_raw = wet_raw;
    sensor->calibration_valid = true;
    return SOIL_MOISTURE_OK;
}

SoilMoisture_Result SoilMoisture_ReadRaw(SoilMoisture_Handle *sensor,
                                        uint16_t *raw)
{
    HAL_StatusTypeDef status;
    if ((sensor == NULL) || (sensor->adc == NULL) || (raw == NULL))
        return SOIL_MOISTURE_INVALID_ARGUMENT;

    status = HAL_ADC_Start(sensor->adc);
    if (status != HAL_OK) return SoilMoisture_FromHAL(status);
    status = HAL_ADC_PollForConversion(sensor->adc, sensor->adc_timeout_ms);
    if (status == HAL_OK) *raw = (uint16_t)HAL_ADC_GetValue(sensor->adc);
    (void)HAL_ADC_Stop(sensor->adc);
    return SoilMoisture_FromHAL(status);
}

SoilMoisture_Result SoilMoisture_ReadAverage(SoilMoisture_Handle *sensor,
                                            uint16_t sample_count,
                                            uint16_t *raw_average)
{
    uint32_t sum = 0U;
    uint16_t raw;
    uint16_t i;
    SoilMoisture_Result result;
    if ((sample_count == 0U) || (raw_average == NULL))
        return SOIL_MOISTURE_INVALID_ARGUMENT;

    for (i = 0U; i < sample_count; ++i)
    {
        result = SoilMoisture_ReadRaw(sensor, &raw);
        if (result != SOIL_MOISTURE_OK) return result;
        sum += raw;
    }
    *raw_average = (uint16_t)(sum / sample_count);
    return SOIL_MOISTURE_OK;
}

SoilMoisture_Result SoilMoisture_ReadPercent(SoilMoisture_Handle *sensor,
                                            uint16_t sample_count,
                                            float *percent,
                                            uint16_t *raw_average)
{
    uint16_t raw;
    float value;
    SoilMoisture_Result result;
    if ((sensor == NULL) || (percent == NULL))
        return SOIL_MOISTURE_INVALID_ARGUMENT;
    if (!sensor->calibration_valid) return SOIL_MOISTURE_NOT_CALIBRATED;

    result = SoilMoisture_ReadAverage(sensor, sample_count, &raw);
    if (result != SOIL_MOISTURE_OK) return result;

    value = ((float)((int32_t)raw - (int32_t)sensor->dry_raw) * 100.0f) /
            (float)((int32_t)sensor->wet_raw - (int32_t)sensor->dry_raw);
    if (value < 0.0f) value = 0.0f;
    if (value > 100.0f) value = 100.0f;
    *percent = value;
    if (raw_average != NULL) *raw_average = raw;
    return SOIL_MOISTURE_OK;
}

SoilMoisture_Result SoilMoisture_ReadDigitalWet(SoilMoisture_Handle *sensor,
                                               bool *is_wet)
{
    bool high;
    if ((sensor == NULL) || (sensor->digital_port == NULL) || (is_wet == NULL))
        return SOIL_MOISTURE_INVALID_ARGUMENT;
    high = (HAL_GPIO_ReadPin(sensor->digital_port, sensor->digital_pin) == GPIO_PIN_SET);
    *is_wet = sensor->digital_wet_active_low ? !high : high;
    return SOIL_MOISTURE_OK;
}
