#include "w25qxx.h"
#include <stddef.h>

#define CMD_WRITE_ENABLE       0x06U
#define CMD_WRITE_DISABLE      0x04U
#define CMD_READ_STATUS_1      0x05U
#define CMD_READ_STATUS_2      0x35U
#define CMD_PAGE_PROGRAM       0x02U
#define CMD_READ_DATA          0x03U
#define CMD_SECTOR_ERASE_4K    0x20U
#define CMD_BLOCK_ERASE_32K    0x52U
#define CMD_BLOCK_ERASE_64K    0xD8U
#define CMD_CHIP_ERASE         0xC7U
#define CMD_POWER_DOWN         0xB9U
#define CMD_RELEASE_POWER_DOWN 0xABU
#define CMD_JEDEC_ID           0x9FU
#define STATUS_BUSY            0x01U
#define STATUS_WEL             0x02U

static void CS_Low(W25QXX_Handle *flash)
{
    HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_RESET);
}

static void CS_High(W25QXX_Handle *flash)
{
    HAL_GPIO_WritePin(flash->cs_port, flash->cs_pin, GPIO_PIN_SET);
}

static W25QXX_Result FromHAL(HAL_StatusTypeDef status)
{
    switch (status)
    {
        case HAL_OK: return W25QXX_OK;
        case HAL_BUSY: return W25QXX_BUSY;
        case HAL_TIMEOUT: return W25QXX_TIMEOUT;
        default: return W25QXX_ERROR;
    }
}

static bool Valid(const W25QXX_Handle *flash)
{
    return (flash != NULL) && (flash->spi != NULL) && (flash->cs_port != NULL);
}

static W25QXX_Result Tx(W25QXX_Handle *flash, const uint8_t *data, uint16_t length)
{
    return FromHAL(HAL_SPI_Transmit(flash->spi, (uint8_t *)data, length,
                                    flash->spi_timeout_ms));
}

static W25QXX_Result Rx(W25QXX_Handle *flash, uint8_t *data, uint16_t length)
{
    return FromHAL(HAL_SPI_Receive(flash->spi, data, length,
                                   flash->spi_timeout_ms));
}

static W25QXX_Result SimpleCommand(W25QXX_Handle *flash, uint8_t command)
{
    W25QXX_Result result;
    if (!Valid(flash)) return W25QXX_INVALID_ARGUMENT;
    CS_Low(flash);
    result = Tx(flash, &command, 1U);
    CS_High(flash);
    return result;
}

static bool RangeValid(const W25QXX_Handle *flash, uint32_t address, uint32_t length)
{
    if (length == 0U) return true;
    if ((flash->capacity_bytes == 0U) || (address >= flash->capacity_bytes)) return false;
    return length <= (flash->capacity_bytes - address);
}

static W25QXX_Result EraseAddress(W25QXX_Handle *flash, uint8_t command,
                                 uint32_t address, uint32_t alignment)
{
    uint8_t tx[4];
    uint8_t status;
    W25QXX_Result result;
    if (!Valid(flash) || ((address % alignment) != 0U) ||
        !RangeValid(flash, address, alignment)) return W25QXX_INVALID_ARGUMENT;
    result = W25QXX_WriteEnable(flash);
    if (result != W25QXX_OK) return result;
    result = W25QXX_ReadStatus1(flash, &status);
    if ((result != W25QXX_OK) || ((status & STATUS_WEL) == 0U)) return W25QXX_ERROR;

    tx[0] = command;
    tx[1] = (uint8_t)(address >> 16);
    tx[2] = (uint8_t)(address >> 8);
    tx[3] = (uint8_t)address;
    CS_Low(flash);
    result = Tx(flash, tx, sizeof(tx));
    CS_High(flash);
    return (result == W25QXX_OK)
        ? W25QXX_WaitReady(flash, flash->operation_timeout_ms) : result;
}

void W25QXX_InitHandle(W25QXX_Handle *flash, SPI_HandleTypeDef *spi,
                      GPIO_TypeDef *cs_port, uint16_t cs_pin,
                      uint32_t spi_timeout_ms, uint32_t operation_timeout_ms)
{
    if (flash == NULL) return;
    flash->spi = spi;
    flash->cs_port = cs_port;
    flash->cs_pin = cs_pin;
    flash->spi_timeout_ms = spi_timeout_ms;
    flash->operation_timeout_ms = operation_timeout_ms;
    flash->manufacturer_id = 0U;
    flash->memory_type = 0U;
    flash->capacity_id = 0U;
    flash->capacity_bytes = 0U;
    if (cs_port != NULL) CS_High(flash);
}

W25QXX_Result W25QXX_ReadJEDECID(W25QXX_Handle *flash, uint8_t id[3])
{
    uint8_t command = CMD_JEDEC_ID;
    W25QXX_Result result;
    if (!Valid(flash) || (id == NULL)) return W25QXX_INVALID_ARGUMENT;
    CS_Low(flash);
    result = Tx(flash, &command, 1U);
    if (result == W25QXX_OK) result = Rx(flash, id, 3U);
    CS_High(flash);
    return result;
}

W25QXX_Result W25QXX_Probe(W25QXX_Handle *flash)
{
    uint8_t id[3];
    W25QXX_Result result = W25QXX_ReadJEDECID(flash, id);
    if (result != W25QXX_OK) return result;
    if (id[0] != 0xEFU) return W25QXX_ID_ERROR;
    if ((id[2] < 0x14U) || (id[2] > 0x1FU)) return W25QXX_ID_ERROR;
    flash->manufacturer_id = id[0];
    flash->memory_type = id[1];
    flash->capacity_id = id[2];
    flash->capacity_bytes = (uint32_t)1UL << id[2];
    return W25QXX_OK;
}

static W25QXX_Result ReadStatus(W25QXX_Handle *flash, uint8_t command,
                               uint8_t *status)
{
    W25QXX_Result result;
    if (!Valid(flash) || (status == NULL)) return W25QXX_INVALID_ARGUMENT;
    CS_Low(flash);
    result = Tx(flash, &command, 1U);
    if (result == W25QXX_OK) result = Rx(flash, status, 1U);
    CS_High(flash);
    return result;
}

W25QXX_Result W25QXX_ReadStatus1(W25QXX_Handle *flash, uint8_t *status)
{
    return ReadStatus(flash, CMD_READ_STATUS_1, status);
}

W25QXX_Result W25QXX_ReadStatus2(W25QXX_Handle *flash, uint8_t *status)
{
    return ReadStatus(flash, CMD_READ_STATUS_2, status);
}

W25QXX_Result W25QXX_WaitReady(W25QXX_Handle *flash, uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint8_t status;
    W25QXX_Result result;
    do
    {
        result = W25QXX_ReadStatus1(flash, &status);
        if (result != W25QXX_OK) return result;
        if ((status & STATUS_BUSY) == 0U) return W25QXX_OK;
    } while ((HAL_GetTick() - start) < timeout_ms);
    return W25QXX_TIMEOUT;
}

W25QXX_Result W25QXX_WriteEnable(W25QXX_Handle *flash)
{
    return SimpleCommand(flash, CMD_WRITE_ENABLE);
}

W25QXX_Result W25QXX_WriteDisable(W25QXX_Handle *flash)
{
    return SimpleCommand(flash, CMD_WRITE_DISABLE);
}

W25QXX_Result W25QXX_Read(W25QXX_Handle *flash, uint32_t address,
                         uint8_t *data, uint32_t length)
{
    uint8_t command[4];
    W25QXX_Result result;
    if (!Valid(flash) || (data == NULL) || (length == 0U))
        return W25QXX_INVALID_ARGUMENT;
    if (!RangeValid(flash, address, length)) return W25QXX_OUT_OF_RANGE;
    command[0] = CMD_READ_DATA;
    command[1] = (uint8_t)(address >> 16);
    command[2] = (uint8_t)(address >> 8);
    command[3] = (uint8_t)address;
    CS_Low(flash);
    result = Tx(flash, command, sizeof(command));
    while ((result == W25QXX_OK) && (length > 0U))
    {
        uint16_t chunk = (length > 65535U) ? 65535U : (uint16_t)length;
        result = Rx(flash, data, chunk);
        data += chunk;
        length -= chunk;
    }
    CS_High(flash);
    return result;
}

W25QXX_Result W25QXX_Write(W25QXX_Handle *flash, uint32_t address,
                          const uint8_t *data, uint32_t length)
{
    uint8_t command[4];
    uint8_t status;
    W25QXX_Result result;
    if (!Valid(flash) || (data == NULL) || (length == 0U))
        return W25QXX_INVALID_ARGUMENT;
    if (!RangeValid(flash, address, length)) return W25QXX_OUT_OF_RANGE;

    while (length > 0U)
    {
        uint16_t page_space = (uint16_t)(W25QXX_PAGE_SIZE -
                                        (address % W25QXX_PAGE_SIZE));
        uint16_t chunk = (length < page_space) ? (uint16_t)length : page_space;
        result = W25QXX_WriteEnable(flash);
        if (result != W25QXX_OK) return result;
        result = W25QXX_ReadStatus1(flash, &status);
        if ((result != W25QXX_OK) || ((status & STATUS_WEL) == 0U))
            return W25QXX_ERROR;

        command[0] = CMD_PAGE_PROGRAM;
        command[1] = (uint8_t)(address >> 16);
        command[2] = (uint8_t)(address >> 8);
        command[3] = (uint8_t)address;
        CS_Low(flash);
        result = Tx(flash, command, sizeof(command));
        if (result == W25QXX_OK) result = Tx(flash, data, chunk);
        CS_High(flash);
        if (result != W25QXX_OK) return result;
        result = W25QXX_WaitReady(flash, flash->operation_timeout_ms);
        if (result != W25QXX_OK) return result;
        address += chunk;
        data += chunk;
        length -= chunk;
    }
    return W25QXX_OK;
}

W25QXX_Result W25QXX_EraseSector4K(W25QXX_Handle *flash, uint32_t address)
{
    return EraseAddress(flash, CMD_SECTOR_ERASE_4K, address, W25QXX_SECTOR_SIZE);
}

W25QXX_Result W25QXX_EraseBlock32K(W25QXX_Handle *flash, uint32_t address)
{
    return EraseAddress(flash, CMD_BLOCK_ERASE_32K, address, W25QXX_BLOCK32_SIZE);
}

W25QXX_Result W25QXX_EraseBlock64K(W25QXX_Handle *flash, uint32_t address)
{
    return EraseAddress(flash, CMD_BLOCK_ERASE_64K, address, W25QXX_BLOCK64_SIZE);
}

W25QXX_Result W25QXX_EraseChip(W25QXX_Handle *flash)
{
    W25QXX_Result result = W25QXX_WriteEnable(flash);
    if (result != W25QXX_OK) return result;
    result = SimpleCommand(flash, CMD_CHIP_ERASE);
    return (result == W25QXX_OK)
        ? W25QXX_WaitReady(flash, flash->operation_timeout_ms) : result;
}

W25QXX_Result W25QXX_PowerDown(W25QXX_Handle *flash)
{
    return SimpleCommand(flash, CMD_POWER_DOWN);
}

W25QXX_Result W25QXX_ReleasePowerDown(W25QXX_Handle *flash)
{
    W25QXX_Result result = SimpleCommand(flash, CMD_RELEASE_POWER_DOWN);
    if (result == W25QXX_OK) HAL_Delay(1U);
    return result;
}
