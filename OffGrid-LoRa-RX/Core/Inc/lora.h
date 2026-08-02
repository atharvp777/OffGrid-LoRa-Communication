#ifndef INC_LORA_H_
#define INC_LORA_H_

/* =========================================================
 * lora.h - LoRa (SX1278) Driver Header
 * Project: OffGrid LoRa Communication System
 * MCU: STM32F103C8T6
 * ========================================================= */

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define LORA_NSS_GPIO_Port      GPIOA
#define LORA_NSS_Pin            GPIO_PIN_4
#define LORA_RESET_GPIO_Port    GPIOB
#define LORA_RESET_Pin          GPIO_PIN_0
#define LORA_DIO0_GPIO_Port     GPIOB
#define LORA_DIO0_Pin           GPIO_PIN_1

#define LORA_SPI_TIMEOUT        100
#define LORA_MAX_PACKET_SIZE    64

typedef enum {
    LORA_OK      = 0,
    LORA_ERROR   = 1,
    LORA_TIMEOUT = 2
} LoRa_Status;

LoRa_Status LoRa_Init(void);
LoRa_Status LoRa_Transmit(uint8_t *data, uint8_t length);
void        LoRa_StartReceive(void);
uint8_t     LoRa_PacketAvailable(void);
uint8_t     LoRa_Receive(uint8_t *buffer, uint8_t maxLen);
int16_t     LoRa_GetRSSI(void);
uint8_t     LoRa_ReadRegister(uint8_t address);
void        LoRa_WriteRegister(uint8_t address, uint8_t value);

#endif /* INC_LORA_H_ */
