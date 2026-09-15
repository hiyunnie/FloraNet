#ifndef UART_SERVICE_H
#define UART_SERVICE_H

#include <stdbool.h>

bool UartService_Init(void);
bool UartService_Response(const char *format, ...);
bool UartService_Log(const char *format, ...);

#endif
