#ifndef APP_H
#define APP_H

#include "app_types.h"
#include "stm32f4xx_hal.h"
#include <stdbool.h>

typedef struct {
    I2C_HandleTypeDef *i2c;
    SPI_HandleTypeDef *spi;
    ADC_HandleTypeDef *adc;
    UART_HandleTypeDef *uart;
    GPIO_TypeDef *flash_cs_port;
    uint16_t flash_cs_pin;
} AppHardware_t;

bool App_Init(const AppHardware_t *hardware);
bool App_GetLatest(SensorData_t *sample);
bool App_GetConfig(AppConfig_t *config);
bool App_UpdateConfig(const AppConfig_t *config);
void App_GetStatistics(AppStatistics_t *statistics);

/* Call from HAL_UART_RxCpltCallback(). */
void App_UartRxCompleteISR(UART_HandleTypeDef *huart);

#endif
