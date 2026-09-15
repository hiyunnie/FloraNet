#ifndef W25QXX_H
#define W25QXX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

#define W25QXX_PAGE_SIZE       256U
#define W25QXX_SECTOR_SIZE     4096U
#define W25QXX_BLOCK32_SIZE    32768U
#define W25QXX_BLOCK64_SIZE    65536U

typedef enum
{
    W25QXX_OK = 0,
    W25QXX_ERROR,
    W25QXX_BUSY,
    W25QXX_TIMEOUT,
    W25QXX_INVALID_ARGUMENT,
    W25QXX_OUT_OF_RANGE,
    W25QXX_ID_ERROR
} W25QXX_Result;

typedef struct
{
    SPI_HandleTypeDef *spi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    uint32_t spi_timeout_ms;
    uint32_t operation_timeout_ms;
    uint8_t manufacturer_id;
    uint8_t memory_type;
    uint8_t capacity_id;
    uint32_t capacity_bytes;
} W25QXX_Handle;

void W25QXX_InitHandle(W25QXX_Handle *flash, SPI_HandleTypeDef *spi,
                      GPIO_TypeDef *cs_port, uint16_t cs_pin,
                      uint32_t spi_timeout_ms, uint32_t operation_timeout_ms);
W25QXX_Result W25QXX_Probe(W25QXX_Handle *flash);
W25QXX_Result W25QXX_ReadJEDECID(W25QXX_Handle *flash, uint8_t id[3]);
W25QXX_Result W25QXX_ReadStatus1(W25QXX_Handle *flash, uint8_t *status);
W25QXX_Result W25QXX_ReadStatus2(W25QXX_Handle *flash, uint8_t *status);
W25QXX_Result W25QXX_WaitReady(W25QXX_Handle *flash, uint32_t timeout_ms);
W25QXX_Result W25QXX_WriteEnable(W25QXX_Handle *flash);
W25QXX_Result W25QXX_WriteDisable(W25QXX_Handle *flash);
W25QXX_Result W25QXX_Read(W25QXX_Handle *flash, uint32_t address,
                         uint8_t *data, uint32_t length);
W25QXX_Result W25QXX_Write(W25QXX_Handle *flash, uint32_t address,
                          const uint8_t *data, uint32_t length);
W25QXX_Result W25QXX_EraseSector4K(W25QXX_Handle *flash, uint32_t address);
W25QXX_Result W25QXX_EraseBlock32K(W25QXX_Handle *flash, uint32_t address);
W25QXX_Result W25QXX_EraseBlock64K(W25QXX_Handle *flash, uint32_t address);
W25QXX_Result W25QXX_EraseChip(W25QXX_Handle *flash);
W25QXX_Result W25QXX_PowerDown(W25QXX_Handle *flash);
W25QXX_Result W25QXX_ReleasePowerDown(W25QXX_Handle *flash);

#ifdef __cplusplus
}
#endif
#endif
