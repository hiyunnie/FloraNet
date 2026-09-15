#ifndef COMMAND_SERVICE_H
#define COMMAND_SERVICE_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

bool CommandService_Init(void);
void CommandService_RxCompleteISR(UART_HandleTypeDef *huart);

#endif
