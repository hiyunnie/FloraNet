#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define APP_SENSOR_QUEUE_LENGTH       4U
#define APP_STORAGE_QUEUE_LENGTH      12U
#define APP_UART_HIGH_QUEUE_LENGTH    8U
#define APP_UART_LOW_QUEUE_LENGTH     16U
#define APP_UART_MESSAGE_MAX          192U
#define APP_COMMAND_MAX               96U

#define APP_SENSOR_STACK_WORDS        512U
#define APP_ALARM_STACK_WORDS         384U
#define APP_COMMAND_STACK_WORDS       512U
#define APP_STORAGE_STACK_WORDS       512U
#define APP_UART_STACK_WORDS          512U
#define APP_HEALTH_STACK_WORDS        384U

#define APP_PRIO_ALARM                5U
#define APP_PRIO_SENSOR               4U
#define APP_PRIO_COMMAND              4U
#define APP_PRIO_STORAGE              3U
#define APP_PRIO_HEALTH               2U
#define APP_PRIO_UART                 1U

#define APP_SENSOR_DEADLINE_MS        250U
#define APP_COMMAND_DEADLINE_MS       100U
#define APP_ALARM_DEADLINE_MS         20U
#define APP_HEALTH_PERIOD_MS          5000U

#define APP_FLASH_CONFIG_A_ADDR       0x000000UL
#define APP_FLASH_CONFIG_B_ADDR       0x001000UL
#define APP_FLASH_HISTORY_ADDR        0x010000UL
#define APP_FLASH_TEST_ADDR           0x7FF000UL

#endif
