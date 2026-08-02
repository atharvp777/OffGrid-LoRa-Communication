# OffGrid LoRa Communication System

![STM32](https://img.shields.io/badge/MCU-STM32F103C8T6-blue?style=flat-square)
![LoRa](https://img.shields.io/badge/RF-SX1278%20433MHz-green?style=flat-square)
![Range](https://img.shields.io/badge/Range-up%20to%2015km-orange?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-lightgrey?style=flat-square)

A low-power, long-range wireless communication system built on STM32 Blue Pill (STM32F103C8T6) and the SX1278 LoRa module. Designed for emergency alerting, GPS tracking, and IoT sensing in environments where cellular and WiFi infrastructure is unavailable — disaster zones, remote terrain, off-grid installations.

---

## Use cases

- **Emergency alert system** — press a button, transmit an ALERT packet over kilometers with no network
- **Off-grid GPS tracker** — periodic GPS coordinate transmission via LoRa
- **Remote sensor network** — temperature, humidity, or other sensor data from areas without connectivity

---

## System architecture

```
TRANSMITTER SIDE                        RECEIVER SIDE

  [ Push Button ]                         [ OLED Display ]
       │                                       │
  [ STM32F103C8T6 ] ──SPI1──► [ SX1278 ] ══ 433 MHz ══► [ SX1278 ] ──SPI1──► [ STM32F103C8T6 ]
  (Blue Pill TX)                LoRa TX          air          LoRa RX            (Blue Pill RX)
       │                                                                               │
  [ Status LED ]                                                               [ Buzzer / Alert ]
```

---

## Hardware

| Component | Specification |
|---|---|
| Microcontroller | STM32F103C8T6 (Blue Pill), ARM Cortex-M3, 72 MHz |
| LoRa Module | SX1278, 433 MHz, up to +20 dBm |
| Display | SSD1306 OLED 128×64, I2C |
| Programmer | ST-Link V2 (SWD interface) |

### Pin connections

**TX side:**

| STM32 Pin | Connected to |
|---|---|
| PA0 | Push button (to GND, INPUT_PULLUP) |
| PA4 | LoRa NSS (SPI chip select) |
| PA5 | LoRa SCK (SPI1) |
| PA6 | LoRa MISO (SPI1) |
| PA7 | LoRa MOSI (SPI1) |
| PB0 | LoRa RESET |
| PB1 | LoRa DIO0 |
| PC13 | Status LED (active LOW) |

**RX side (same SPI wiring + additionally):**

| STM32 Pin | Connected to |
|---|---|
| PB6 | OLED SCL (I2C1) |
| PB7 | OLED SDA (I2C1) |
| PB9 | Buzzer / Alert LED (active HIGH) |

---

## LoRa configuration

| Parameter | Value | Rationale |
|---|---|---|
| Frequency | 433 MHz | ISM band, good penetration |
| Spreading Factor | SF7 | 5.5 kbps, ~2 km range |
| Bandwidth | 125 kHz | Standard, good noise rejection |
| Coding Rate | 4/5 | Minimal overhead |
| PA Output | +17 dBm | Safe long-range output via PA_BOOST |
| Sync Word | 0x12 | Private network (not LoRaWAN) |
| Preamble | 8 symbols | Standard |
| CRC | Enabled | Packet integrity check |

**Spreading factor trade-off:**

| SF | Data Rate | Range | Airtime (20 byte packet) |
|---|---|---|---|
| SF7 | 5.5 kbps | ~2 km | ~55 ms |
| SF9 | 1.4 kbps | ~6 km | ~180 ms |
| SF12 | 0.3 kbps | ~15 km | ~1480 ms |

---

## Project structure

```
OffGrid-LoRa-RX/
├── Core/
│   ├── Inc/
│   │   ├── lora.h               # Driver API and pin definitions
│   │   ├── sx1278_registers.h   # Full SX1278 register map
│   │   └── main.h               # CubeMX generated
│   └── Src/
│       ├── lora.c               # SPI driver: init, TX, RX, RSSI
│       └── main.c               # Application: RX loop, OLED output
├── Drivers/                     # STM32 HAL (CubeMX generated)
├── OffGrid-LoRa-RX.ioc          # STM32CubeMX project file
└── README.md
```

---

## How it works

### SPI communication with SX1278

The SX1278 uses SPI with a specific protocol — the MSB of the address byte determines read vs write:

```c
// Read: MSB = 0
txBuffer[0] = address & 0x7F;
txBuffer[1] = 0x00;  // dummy byte to generate clock
HAL_SPI_TransmitReceive(&hspi1, txBuffer, rxBuffer, 2, timeout);
return rxBuffer[1];  // rxBuffer[0] is garbage

// Write: MSB = 1
txBuffer[0] = address | 0x80;
txBuffer[1] = value;
HAL_SPI_Transmit(&hspi1, txBuffer, 2, timeout);
```

### Initialization sequence

Follows SX1278 datasheet §4.2.5.2:

1. Hardware reset (RESET pin low → high)
2. Verify chip version register = 0x12
3. Enter Sleep mode
4. Switch to LoRa mode (must be done in Sleep)
5. Set frequency (433 MHz)
6. Configure PA, modem, preamble, sync word, FIFO base addresses
7. Enter Standby — ready for TX or RX

### Transmit flow

```
Button press (PA0 LOW)
    → LoRa_Transmit("ALERT:001", 9)
        → Set FIFO pointer to base (0x00)
        → Write payload bytes to FIFO register
        → Switch to MODE_TX
        → Poll IRQ_TX_DONE flag
        → Clear flag, return to Standby
```

### Receive flow

```
LoRa_StartReceive()
    → MODE_RX_CONTINUOUS
    
loop:
    LoRa_PacketAvailable()
        → read REG_IRQ_FLAGS
        → check IRQ_RX_DONE_MASK
        → check CRC error
    if packet available:
        LoRa_Receive(buffer, maxLen)
        LoRa_GetRSSI()
        → trigger buzzer (PB9)
        → update OLED
```

---

## Getting started

### Prerequisites

- STM32CubeIDE (v1.14+)
- ST-Link V2 driver installed
- Python 3.x (for setup automation)

### Setup

1. Clone the repository:
```bash
git clone git@github.com:atharvp777/OffGrid-LoRa-Communication.git
cd OffGrid-LoRa-Communication
```

2. Open `OffGrid-LoRa-RX.ioc` in STM32CubeIDE

3. Build: `Project → Build All` (Ctrl+B)

4. Flash: `Run → Debug` → Resume (▶)

### First test — verify LoRa chip

Before full TX/RX test, verify SPI is working:

```c
uint8_t version = LoRa_ReadRegister(REG_VERSION);
// Should read 0x12 for SX1278
// If 0xFF → SPI wiring issue
// If 0x00 → NSS/CS not toggling
```

If `LoRa_Init()` returns `LORA_ERROR`, the LED will blink fast — check SPI wiring.

---

## Key technical concepts

**Why LoRa over WiFi/GSM/Bluetooth:**

| Technology | Range | Power | Infrastructure needed |
|---|---|---|---|
| Bluetooth | ~10 m | Low | None |
| WiFi | ~100 m | High | Router |
| GSM | Global | High | Cell tower |
| **LoRa** | **2–15 km** | **Very low** | **None** |

**Chirp Spread Spectrum (CSS):** LoRa modulates data by sweeping frequency up or down (chirps). This gives extreme noise immunity — the signal can be decoded even 20 dB below the noise floor. Higher spreading factors = more chirps per symbol = more range, less speed.

**Why SPI over I2C for LoRa:** SPI is full-duplex and faster (~4 Mbps here vs I2C's 100/400 kHz), critical for real-time wireless packet handling. I2C is used for the lower-bandwidth OLED display.

---

## Results

| Metric | Value |
|---|---|
| Frequency | 433 MHz |
| Spreading Factor | SF7 (configurable) |
| Estimated range (open field) | ~2 km at SF7, up to 15 km at SF12 |
| Packet size | 20–30 bytes per packet |
| TX airtime | ~55 ms (SF7) |
| Power (TX mode) | ~100 mA @ 3.3V |
| Power (RX mode) | ~11 mA @ 3.3V |

---

## Possible improvements

- Add GPS module (NEO-6M via UART) for location-tagged alerts
- Mesh network with multiple RX nodes
- LoRaWAN gateway integration (change sync word to 0x34)
- AES-128 payload encryption
- Low-power sleep between transmissions (STM32 STOP mode)
- Mobile app interface via LoRa → UART → BLE bridge

---

## Interview reference

One-line definition:
> "A low-power, long-range wireless communication system using LoRa to transmit emergency alerts and sensor data in off-grid environments where cellular or WiFi infrastructure is unavailable."

Key design decisions to explain:
- LoRa chosen for range + low power (no infrastructure needed)
- SPI used for LoRa (full-duplex, fast); I2C for OLED (simpler, lower bandwidth)
- Interrupt-capable GPIO for DIO0 (TX/RX done detection)
- Software NSS control (PA4) for precise SPI timing with SX1278
- Peripheral bus distribution: SPI1 on APB2 (72 MHz), I2C1 on APB1 (36 MHz)

---

## License

MIT License — see [LICENSE](LICENSE) for details.
