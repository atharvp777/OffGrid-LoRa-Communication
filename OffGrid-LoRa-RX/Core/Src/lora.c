/* =========================================================
 * lora.c - LoRa (SX1278) Driver
 * Project: OffGrid LoRa Communication System
 * Config : 433 MHz | SF7 | BW 125kHz | CR 4/5 | +17dBm
 * ========================================================= */

#include "lora.h"
#include "sx1278_registers.h"
#include "spi.h"
#include "gpio.h"
#include <string.h>

static void LoRa_Select(void)
{
    HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_RESET);
}

static void LoRa_Unselect(void)
{
    HAL_GPIO_WritePin(LORA_NSS_GPIO_Port, LORA_NSS_Pin, GPIO_PIN_SET);
}

static void LoRa_Reset(void)
{
    HAL_GPIO_WritePin(LORA_RESET_GPIO_Port, LORA_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(LORA_RESET_GPIO_Port, LORA_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

uint8_t LoRa_ReadRegister(uint8_t address)
{
    uint8_t txBuffer[2] = { address & 0x7F, 0x00 };
    uint8_t rxBuffer[2] = { 0 };
    LoRa_Select();
    if (HAL_SPI_TransmitReceive(&hspi1, txBuffer, rxBuffer, 2, LORA_SPI_TIMEOUT) != HAL_OK)
    {
        LoRa_Unselect();
        return 0xFF;
    }
    LoRa_Unselect();
    return rxBuffer[1];
}

void LoRa_WriteRegister(uint8_t address, uint8_t value)
{
    uint8_t txBuffer[2] = { address | 0x80, value };
    LoRa_Select();
    HAL_SPI_Transmit(&hspi1, txBuffer, 2, LORA_SPI_TIMEOUT);
    LoRa_Unselect();
}

static void LoRa_SetFrequency(uint32_t freq_hz)
{
    uint64_t frf = ((uint64_t)freq_hz << 19) / 32000000UL;
    LoRa_WriteRegister(REG_FR_MSB, (uint8_t)(frf >> 16));
    LoRa_WriteRegister(REG_FR_MID, (uint8_t)(frf >>  8));
    LoRa_WriteRegister(REG_FR_LSB, (uint8_t)(frf >>  0));
}

LoRa_Status LoRa_Init(void)
{
    LoRa_Reset();

    if (LoRa_ReadRegister(REG_VERSION) != SX1278_VERSION)
        return LORA_ERROR;

    LoRa_WriteRegister(REG_OP_MODE, MODE_SLEEP);
    HAL_Delay(10);
    LoRa_WriteRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_SLEEP);
    HAL_Delay(10);

    LoRa_SetFrequency(433000000UL);
    LoRa_WriteRegister(REG_PA_CONFIG,         PA_OUTPUT_17dBm);
    LoRa_WriteRegister(REG_MODEM_CONFIG_1,    BW_125KHZ | CR_4_5 | HEADER_EXPLICIT);
    LoRa_WriteRegister(REG_MODEM_CONFIG_2,    SF7 | TX_SINGLE_PACKET | CRC_ENABLE);
    LoRa_WriteRegister(REG_MODEM_CONFIG_3,    AGC_AUTO_ON);
    LoRa_WriteRegister(REG_PREAMBLE_MSB,      0x00);
    LoRa_WriteRegister(REG_PREAMBLE_LSB,      0x08);
    LoRa_WriteRegister(REG_SYNC_WORD,         LORA_SYNC_WORD_PRIVATE);
    LoRa_WriteRegister(REG_FIFO_TX_BASE_ADDR, 0x00);
    LoRa_WriteRegister(REG_FIFO_RX_BASE_ADDR, 0x00);
    LoRa_WriteRegister(REG_OP_MODE, MODE_LONG_RANGE_MODE | MODE_STDBY);
    HAL_Delay(10);

    return LORA_OK;
}

LoRa_Status LoRa_Transmit(uint8_t *data, uint8_t length)
{
    if (length > LORA_MAX_PACKET_SIZE) length = LORA_MAX_PACKET_SIZE;

    LoRa_WriteRegister(REG_OP_MODE,        MODE_LONG_RANGE_MODE | MODE_STDBY);
    LoRa_WriteRegister(REG_FIFO_ADDR_PTR,  0x00);
    LoRa_WriteRegister(REG_PAYLOAD_LENGTH, length);

    for (uint8_t i = 0; i < length; i++)
        LoRa_WriteRegister(REG_FIFO, data[i]);

    LoRa_WriteRegister(REG_IRQ_FLAGS, 0xFF);
    LoRa_WriteRegister(REG_OP_MODE,   MODE_LONG_RANGE_MODE | MODE_TX);

    uint32_t start = HAL_GetTick();
    while (!(LoRa_ReadRegister(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK))
    {
        if ((HAL_GetTick() - start) > 3000) return LORA_TIMEOUT;
    }
    LoRa_WriteRegister(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);
    return LORA_OK;
}

void LoRa_StartReceive(void)
{
    LoRa_WriteRegister(REG_FIFO_ADDR_PTR, 0x00);
    LoRa_WriteRegister(REG_IRQ_FLAGS,     0xFF);
    LoRa_WriteRegister(REG_OP_MODE,       MODE_LONG_RANGE_MODE | MODE_RX_CONTINUOUS);
}

uint8_t LoRa_PacketAvailable(void)
{
    uint8_t irq = LoRa_ReadRegister(REG_IRQ_FLAGS);
    if (irq & IRQ_RX_DONE_MASK)
    {
        LoRa_WriteRegister(REG_IRQ_FLAGS, IRQ_RX_DONE_MASK);
        if (irq & IRQ_PAYLOAD_CRC_ERROR_MASK)
        {
            LoRa_WriteRegister(REG_IRQ_FLAGS, IRQ_PAYLOAD_CRC_ERROR_MASK);
            return 0;
        }
        return 1;
    }
    return 0;
}

uint8_t LoRa_Receive(uint8_t *buffer, uint8_t maxLen)
{
    uint8_t packetLen   = LoRa_ReadRegister(REG_RX_NB_BYTES);
    uint8_t currentAddr = LoRa_ReadRegister(REG_FIFO_RX_CURRENT_ADDR);
    LoRa_WriteRegister(REG_FIFO_ADDR_PTR, currentAddr);
    if (packetLen > maxLen) packetLen = maxLen;
    for (uint8_t i = 0; i < packetLen; i++)
        buffer[i] = LoRa_ReadRegister(REG_FIFO);
    return packetLen;
}

int16_t LoRa_GetRSSI(void)
{
    return (int16_t)(-157 + LoRa_ReadRegister(REG_PKT_RSSI_VALUE));
}
