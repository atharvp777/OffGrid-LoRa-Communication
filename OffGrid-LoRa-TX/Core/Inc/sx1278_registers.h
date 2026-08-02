#ifndef INC_SX1278_REGISTERS_H_
#define INC_SX1278_REGISTERS_H_

/* =========================================================
 * sx1278_registers.h
 * SX1278 LoRa Register Map
 * Ref: SX1276/77/78/79 Datasheet Rev 7
 * ========================================================= */

#define REG_FIFO                    0x00
#define REG_OP_MODE                 0x01
#define REG_FR_MSB                  0x06
#define REG_FR_MID                  0x07
#define REG_FR_LSB                  0x08
#define REG_PA_CONFIG               0x09
#define REG_OCP                     0x0B
#define REG_LNA                     0x0C
#define REG_FIFO_ADDR_PTR           0x0D
#define REG_FIFO_TX_BASE_ADDR       0x0E
#define REG_FIFO_RX_BASE_ADDR       0x0F
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_IRQ_FLAGS_MASK          0x11
#define REG_IRQ_FLAGS               0x12
#define REG_RX_NB_BYTES             0x13
#define REG_PKT_SNR_VALUE           0x19
#define REG_PKT_RSSI_VALUE          0x1A
#define REG_MODEM_CONFIG_1          0x1D
#define REG_MODEM_CONFIG_2          0x1E
#define REG_PREAMBLE_MSB            0x20
#define REG_PREAMBLE_LSB            0x21
#define REG_PAYLOAD_LENGTH          0x22
#define REG_MODEM_CONFIG_3          0x26
#define REG_SYNC_WORD               0x39
#define REG_DIO_MAPPING_1           0x40
#define REG_VERSION                 0x42

#define MODE_LONG_RANGE_MODE        0x80
#define MODE_SLEEP                  0x00
#define MODE_STDBY                  0x01
#define MODE_TX                     0x03
#define MODE_RX_CONTINUOUS          0x05

#define IRQ_TX_DONE_MASK            0x08
#define IRQ_RX_DONE_MASK            0x40
#define IRQ_PAYLOAD_CRC_ERROR_MASK  0x20

#define PA_OUTPUT_17dBm             0x8F
#define BW_125KHZ                   0x70
#define CR_4_5                      0x02
#define HEADER_EXPLICIT             0x00
#define SF7                         0x70
#define TX_SINGLE_PACKET            0x00
#define CRC_ENABLE                  0x04
#define AGC_AUTO_ON                 0x04
#define LORA_SYNC_WORD_PRIVATE      0x12
#define SX1278_VERSION              0x12

#endif /* INC_SX1278_REGISTERS_H_ */
