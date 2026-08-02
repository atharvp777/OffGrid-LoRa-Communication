# OffGrid LoRa Emergency Communication System

## [? Live Simulation](https://atharvp777.github.io/OffGrid-LoRa-Communication/simulation.html)`n`n![STM32](https://img.shields.io/badge/MCU-STM32F103C8T6-blue?style=flat-square)
![LoRa](https://img.shields.io/badge/RF-SX1278%20433MHz-green?style=flat-square)
![Range](https://img.shields.io/badge/Range-up%20to%2010km-orange?style=flat-square)
![Cost](https://img.shields.io/badge/Cost-%E2%82%B92%2C558-lightgrey?style=flat-square)
![License](https://img.shields.io/badge/License-MIT-lightgrey?style=flat-square)

**Long-Range, Off-Grid Emergency Alert Messaging System Using LoRa Technology**

Academic mini-project — Modern College of Engineering, Pune (SPPU, 2025-26)
Authors: Abhijeet Dalvi · Atharv Pawar · Bhushan Todkar
Guide: Prof. S. L. Tatuskar · Dept. of E&TC

---

## Overview

A portable, battery-operated emergency communication system that transmits **HELP** and **SAFE** alerts with optional GPS coordinates over up to 10 km — with no SIM card, no WiFi, no internet. Built on STM32 Blue Pill + SX1278 LoRa at 433 MHz.

Designed for: disaster zones, trekking, forest operations, rural areas, search and rescue.

---

## System architecture

```
NODE 1 — TRANSMITTER                        NODE 2 — RECEIVER

[HELP Button PA0] ──┐                        ┌── [OLED SSD1306 I2C PB6/PB7]
[SAFE Button PB1] ──┤                        ├── [Buzzer PB9]
[GPS NEO-6M PA3]  ──┤── STM32 ──SPI──SX1278 ═══433MHz═══ SX1278──SPI──STM32 ──┤
[HELP LED PA8]    ──┤                                                           ├── [LED PC13]
[SAFE LED PA9]    ──┘                                                           └── [Buttons PA0/PA2]
```

---

## Hardware

| # | Component | Spec | Qty | Cost (₹) |
|---|---|---|---|---|
| 1 | STM32 Blue Pill | STM32F103C8T6, 72 MHz | 2 | 500 |
| 2 | LoRa SX1278 | 433 MHz, SPI, CSS | 2 | 900 |
| 3 | OLED SSD1306 | 128×64, I2C | 1 | 200 |
| 4 | GPS NEO-6M | UART, 9600 baud | 1 | 300 |
| 5 | Push buttons | Tactile switch | 4 | 40 |
| 6 | LEDs + 220Ω | 5mm LED | 4 | 28 |
| 7 | Li-ion battery | 3.7V rechargeable | 2 | 300 |
| 8 | AMS1117-3.3V | Voltage regulator | 2 | 40 |
| 9 | Wires + PCB | Jumper wires, breadboard | — | 250 |
| | **Total** | | | **₹2,558** |

---

## Pin connections

### Node 1 — Transmitter

| STM32 Pin | Connected to | Label |
|---|---|---|
| PA0 | HELP button → GND | HELP_BTN |
| PB1 | SAFE button → GND | SAFE_BTN |
| PA4 | LoRa NSS | LORA_NSS |
| PA5 | LoRa SCK (SPI1) | SPI1_SCK |
| PA6 | LoRa MISO (SPI1) | SPI1_MISO |
| PA7 | LoRa MOSI (SPI1) | SPI1_MOSI |
| PB0 | LoRa DIO0 | LORA_DIO0 |
| PA3 | GPS NEO-6M TX (UART2 RX) | GPS_RX |
| PA8 | HELP LED (220Ω → GND) | LED_HELP |
| PA9 | SAFE LED (220Ω → GND) | LED_SAFE |
| PC13 | Status LED (active LOW) | LED_STATUS |

### Node 2 — Receiver

| STM32 Pin | Connected to | Label |
|---|---|---|
| PA4–PA7, PB0 | LoRa SX1278 (SPI1) | same as TX |
| PB6 | OLED SCL (I2C1) | I2C1_SCL |
| PB7 | OLED SDA (I2C1) | I2C1_SDA |
| PB9 | Buzzer / Alert (220Ω) | BUZZER |
| PA0 | HELP button (test) | BTN_HELP |
| PA2 | SAFE button (test) | BTN_SAFE |
| PC13 | Status LED | LED_STATUS |

---

## LoRa configuration

| Parameter | Value |
|---|---|
| Frequency | 433 MHz (ISM band, Asia) |
| Spreading Factor | SF7 (configurable SF6–SF12) |
| Bandwidth | 125 kHz |
| Coding Rate | 4/5 |
| Output Power | +17 dBm (PA_BOOST) |
| Sync Word | 0x12 (private network) |
| Modulation | Chirp Spread Spectrum (CSS) |
| Range | 5–10 km (terrain dependent) |

---

## Project structure

```
OffGrid-LoRa-Communication/
├── OffGrid-LoRa-RX/            ← Receiver (Node 2)
│   └── Core/
│       ├── Inc/
│       │   ├── lora.h
│       │   └── sx1278_registers.h
│       └── Src/
│           ├── lora.c
│           └── main.c          ← OLED display, buzzer, RX loop
├── OffGrid-LoRa-TX/            ← Transmitter (Node 1)
│   └── Core/
│       ├── Inc/
│       │   ├── lora.h
│       │   └── sx1278_registers.h
│       └── Src/
│           ├── lora.c
│           └── main.c          ← HELP/SAFE buttons, GPS, TX logic
├── docs/
│   ├── IEEE_Research_Paper.pdf
│   └── Mini_Project_Report.pdf
├── README.md
└── .gitignore
```

---

## How it works

**TX flow:**
1. User presses HELP (PA0) or SAFE (PB1) button
2. 20 ms debounce, confirm press
3. Format packet: `"HELP:001"` or `"SAFE:001"`
4. Corresponding LED turns ON
5. `LoRa_Transmit()` → FIFO load → MODE_TX → poll TX_DONE
6. LED turns OFF, wait for button release

**RX flow:**
1. `LoRa_StartReceive()` → MODE_RX_CONTINUOUS
2. Poll `LoRa_PacketAvailable()` every 10 ms
3. On packet: read FIFO, get RSSI
4. Buzzer ON 500 ms
5. OLED updates: "Received: / HELP:001 / RSSI:-87dBm / Pkts:1"
6. Status LED toggles

---

## Results

- HELP and SAFE messages transmitted and displayed correctly on OLED
- Communication confirmed under NLOS conditions
- GPS NEO-6M NMEA data successfully appended to packets
- Power consumption low — suitable for 3.7V Li-ion battery operation
- Total cost: ₹2,558

---

## Documents

- [IEEE Research Paper](docs/IEEE_Research_Paper.pdf)
- [Mini Project Report](docs/Mini_Project_Report.pdf)

---

## Key concepts for interviews

**Why LoRa:** 2–10 km range, very low power, zero infrastructure needed. No SIM, no WiFi.

**CSS modulation:** chirps sweep frequency linearly over time. Signal decodable 20 dB below noise floor — extremely robust in rubble, forests, NLOS conditions.

**SPI protocol for SX1278:** STM32 is master, SX1278 is slave. Read: `addr & 0x7F` + dummy byte, data in rxBuffer[1]. Write: `addr | 0x80` + value.

**Why SPI for LoRa, I2C for OLED:** SPI is full-duplex, faster (4.5 Mbps here). I2C is simpler, 2-wire, sufficient for 100 kHz OLED updates.

**Peripheral buses:** SPI1 on APB2 (72 MHz); I2C1 on APB1 (36 MHz); UART2 on APB1 for GPS.

---

## Future scope

- Mesh network (multi-node relay)
- Mobile app via Bluetooth bridge
- AES-128 encryption
- Solar charging
- LoRaWAN gateway integration
- Real-time GPS map visualization

---

## References

1. Q. Zhou et al., "Design and Implementation of Open LoRa for IoT," IEEE Access, 2019.
2. S. Blesson, "LoRa Based Emergency Communication System," ISJEM, 2024.
3. D. Mahakal et al., "Cordless Communication System using LoRa Technology," IJERT, 2025.
4. E. S. Murugan et al., "Effective Communication System for Disaster Management Using LoRa," IJAEM, 2025.
5. G. Pasolini et al., "Smart City Pilot Projects Using LoRa," Sensors, 2018.

---

MIT License · [github.com/atharvp777/OffGrid-LoRa-Communication](https://github.com/atharvp777/OffGrid-LoRa-Communication)
