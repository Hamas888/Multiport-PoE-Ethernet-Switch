# Multiport PoE Switch with Custom Signaling

![Project Image](https://drive.google.com/uc?export=view&id=1J4BJ_7U2pBXm5Z4PWR-nIi7LH-2YpAVN) 

## Table of Contents
1. **[Overview](#overview)**  
2. **[Features](#features)**  
3. **[Hardware Design](#hardware-design)**  
   - [Key Components](#key-components)  
4. **[Firmware](#firmware)**  
5. **[Future Work](#future-work)**  
6. **[Licensing](#licensing)**

## Overview
This project is a high-performance, multi-port Power over Ethernet (PoE) switch capable of delivering IEEE 802.3bt/at PoE to up to 4 Powered Devices (PDs). It offers robust performance for embedded network applications, built around the STM32F4 MCU, KSZ9897 Ethernet switch, and TPS23881 PoE controller.

The device offers:
- **6 Ethernet ports**: 4 with 802.3bt/at PoE, 1 power input port (PoE PD), and 1 for standard Ethernet communication.
- **Custom LVDS signaling** on two pairs of a standard 4-pair Ethernet cable.
- **100Mbit Ethernet** with 802.3bt PoE capabilities.

The design integrates hardware (PCB) and firmware components to provide a complete PoE switch solution, enabling power delivery and traffic management through Ethernet.

## Features
- **4 RJ45 PoE Ports**: Provides up to 60W per port (802.3bt).
- **Ethernet Traffic Management**: Built on KSZ9897 for managed and efficient traffic flow.
- **PoE Power Management**: TPS23881 offers intelligent power allocation with over-current protection.
- **Custom Signaling**: Enables synchronized communication for connected devices.
- **Firmware Update**: Secure Ethernet-based firmware updates.
- **Power Input Flexibility**: External 48V DC source or 802.3bt PoE PD input.

## Hardware Design
The hardware design is compact and optimized for thermal management and EMC. The PCB is designed to fit in a bulkhead-mountable enclosure, supporting harsh environments and industrial applications.

### Key Components:
- **STM32F4**: Controls signaling and traffic.
- **KSZ9897**: Ethernet switch for managed 100Mbit Ethernet traffic.
- **TPS23881**: Manages PoE power delivery.
- **Custom LVDS Signaling**: LVDS drivers for synchronized data transfer.

## Firmware
The firmware includes:
- **PoE Power Management**: Handles PoE detection and power allocation.
- **Custom Traffic Management**: Optimizes the flow of Ethernet packets.
- **Fault Protection**: Detects over-current and protects devices.
- **Firmware Update via Ethernet**: Enables secure updates using a bootloader over the network.

<!-- ## Project Documentation

1. **Hardware**:
   - [Schematic](path-to-schematic.pdf)
   - [PCB Layout](path-to-PCB-layout.pdf)
   - [Bill of Materials (BOM)](path-to-bom.csv)
   - [Enclosure Design](path-to-enclosure-design.pdf)

2. **Firmware**:
   - [Source Code](path-to-firmware)
   - [Configuration Guide](path-to-guide)

---

## Testing & Validation
The hardware and firmware were rigorously tested for:
- **PoE power allocation** and over-current protection.
- **Ethernet throughput** across all ports.
- **Compliance with industry standards**: IEEE 802.3bt/at, EMC.
- **Reliability** under varying environmental conditions (temperature, humidity, etc.).

---

## How to Build & Use

### Hardware Assembly
1. Assemble the PCB using the provided schematic and BOM.
2. Mount the PCB in the provided enclosure.

### Flashing the Firmware
- Connect the STM32F4 via ST-Link and flash the firmware using [open-source toolchain].

### Using the PoE Switch
- Connect up to 4 Powered Devices (PDs) to the PoE ports.
- Configure custom signaling through the provided UI or CLI.
- Monitor and manage the device via Ethernet.

---
-->

## Future Work
- Support for 1Gbit Ethernet on non-PoE ports.
- Integration with cloud-based monitoring solutions.
- Extending power delivery beyond 60W per port for high-powered applications.

### Licensing
The header files provided in this repository are free to use under the following conditions:
- You may use and distribute these header files freely for non-commercial purposes.
- You are **not** allowed to reverse-engineer, modify, or distribute any part of the implementation that is not explicitly provided.

**Commercial Use**: 
- To obtain full access to the implementation files (`.cpp`), or to use this project in any commercial product, please contact me at [hamasaeed@gmail.com] to obtain a commercial license.
