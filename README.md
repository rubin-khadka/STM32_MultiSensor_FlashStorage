# STM32 Multi Sensor W25Q64 Flash Storage Project

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![STM32](https://img.shields.io/badge/STM32-F103C8T6-blue)
![CubeIDE](https://img.shields.io/badge/IDE-STM32CubeIDE-darkblue)

## Video Demonstrations

**Button 1 – Display Mode Switching**

https://github.com/user-attachments/assets/c837c958-dd8f-4fa7-b40e-5b68ad077332

Pressing Button 1 cycles through three LCD display modes: Temperature → Accelerometer → Gyroscope.

**Button 2 & 3 – Save & Retrieve Data**

https://github.com/user-attachments/assets/fe075a1f-e6e7-49b2-a4ce-5543424432ca

Button 2 saves sensor data to W25Q64 flash. Button 3 retrieves all stored data and sends via UART.

Both buttons have 1-Second Cooldown to prevent accidental multiple presses and allow flash time to save data. When entering a new sector, it is automatically erased in the background before saving.

**Data Persistence – After Reset**

https://github.com/user-attachments/assets/758fb621-f78c-4816-bed2-fe901e667270

Data persists in flash memory even after reset.

**UART Output Format**  

https://github.com/user-attachments/assets/6bae0c0d-5979-433f-b4e0-cab14ed7c274

HTerm output showing formatted table. Data can be directly copied and pasted into Excel for analysis.

## Table of Contents
- [Project Overview](#project-overview)
- [Task Scheduling](#task-scheduling)
- [Hardware](#hardware-components)
- [Drivers](#mpu-6050-imu-driver)
- [Data Logger](#data-logger-implementation)
- [Getting Started](#getting-started)
- [Resources](#related-projects)
- [Contact](#contact)

## Project Overview

The **STM32 Multi Sensor Flash Storage Project** is a comprehensive embedded systems project built around the STM32F103C8T6 (Bluepill) microcontroller. It demonstrates real-time sensor data acquisition, visualization on an LCD and UART, and reliable non-volatile storage using external SPI flash memory.

The system reads data from multiple sensors:
- **DS18B20**: Digital temperature sensor (1-Wire)
- **MPU6050**: 6-axis accelerometer, gyroscope, and temperature sensor (I2C)

Data is displayed on a **16x2 I2C LCD** and can be output via **UART** for PC monitoring. The entire system runs on a **precise 10ms control loop** that schedules all tasks at different frequencies for optimal performance.

### User Interface

Three ways to interact with the system:

1. **16x2 LCD Display**: Shows real-time sensor data in three selectable modes:
   - **Mode 1 (Temperature)**: Displays both DS18B20 and MPU6050 temperatures
   - **Mode 2 (Accelerometer)**: Shows X, Y, Z acceleration values
   - **Mode 3 (Gyroscope)**: Shows X, Y, Z gyroscope values

2. **Three Push Buttons**:
   - **Button 1**: Switch between display modes
   - **Button 2**: Save current sensor readings to flash memory
   - **Button 3**: Retrieve and export all stored data via UART

3. **UART Interface**: Export stored data in a formatted table for analysis on a PC

### Data Storage

At the heart of the system is the **W25Q64 64Mbit (8MB) SPI Flash memory**, featuring:

- **Circular Buffer Architecture**: When full, oldest data is automatically overwritten
- **Massive Capacity**: Stores up to **262,016 individual sensor readings** (each containing DS18B20 temperature, MPU6050 temperature, accelerometer, and gyroscope data)
- **Non-blocking Operations**: Sector erases happen in the background without freezing the system

> **Note**: All peripheral drivers are written in bare-metal register-level code for maximum efficiency and learning. The **SPI1** peripheral is the only exception, initialized using STM32CubeMX HAL as the bare-metal implementation was not functioning reliably.

## Task Scheduling

The system uses a **10ms timer-based control loop** with independent counters for each task. TIMER3 is configured to drive the main control loop, ensuring precise and non-blocking task execution.

### Task Frequencies

| Task | Frequency | Period | Execution |
|------|-----------|--------|-----------|
| **DS18B20 Read** | 1 Hz | 1 second | Every 100 loops |
| **MPU6050 Read** | 20 Hz | 50 ms | Every 5 loops |
| **LCD Update** | 10 Hz | 100 ms | Every 10 loops |
| **UART Output** | 10 Hz | 100 ms | Every 10 loops |
| **Data Logger Task** | 100 Hz | 10 ms | Every loop |
| **Button Status Check** | 100 Hz | 10 ms | Every loop |
| **Button Interrupts** | Event-driven | On press | EXTIx + TIM4 debounce |

### Timer Configuration

| Timer | Resolution | Purpose |
|-------|------------|---------|
| **DWT** | 1µs | DS18B20 1-Wire protocol precise timing |
| **TIMER2** | 1ms | System millisecond counter and delays |
| **TIMER3** | 0.1ms | **10ms control loop scheduler** (heartbeat) |
| **TIMER4** | 0.1ms | Button debouncing (50ms) |

🔗 [View DWT Driver (Microsecond Delay)](https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage/blob/main/Core/Src/dwt.c)  
🔗 [View TIMER2 Driver (Millisecond Counter)](https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage/blob/main/Core/Src/timer2.c)  
🔗 [View TIMER3 Driver (10ms Heartbeat)](https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage/blob/main/Core/Src/timer3.c)  
🔗 [View Button & TIMER4 Driver (Debounce)](https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage/blob/main/Core/Src/button.c)

> **Note:** DWT (Data Watchpoint and Trace) is a built-in peripheral in ARM Cortex-M3 cores that provides a cycle counter running at CPU frequency (72MHz). This gives ~13.9ns resolution, making it ideal for generating precise microsecond delays required by the DS18B20 1-Wire protocol. Unlike traditional timer-based delays, DWT does not occupy a dedicated timer peripheral and continues running in the background.

## Hardware Components

| Component | Quantity | Description |
|-----------|----------|-------------|
| **STM32F103C8T6** | 1 | "Blue Pill" development board with 72MHz Cortex-M3 |
| **W25Q64 Flash** | 1 | 64Mbit (8MB) SPI flash memory for data logging |
| **DS18B20** | 1 | Digital temperature sensor (1-Wire protocol, ±0.5°C accuracy) |
| **MPU6050** | 1 | 6-axis inertial measurement unit (accelerometer + gyroscope + temperature) |
| **LCD 16x2 with I2C** | 1 | Character display module with I2C backpack (PCF8574) |
| **Push Buttons** | 3 | Two-leg tactile switches for user input |
| **USB-to-Serial Converter** | 1 | CP2102 / CH340 / FTDI for UART communication and debugging |

### Pin Configuration

| Peripheral | Pin | Connection | Notes |
|------------|-----|------------|-------|
| **DS18B20** | PB0 | DQ | 1-Wire data (module has built-in pull-up) |
| | 5V | VCC | Power |
| | GND | GND | Common ground |
| **MPU6050** | PB10 | SCL | I2C2 clock (shared with LCD) |
| | PB11 | SDA | I2C2 data (shared with LCD) |
| | 5V | VCC | Power (module includes voltage regulator) |
| | GND | GND | Common ground |
| **LCD 16x2 I2C** | PB10 | SCL | I2C2 clock (shared with MPU6050) |
| | PB11 | SDA | I2C2 data (shared with MPU6050) |
| | 5V | VCC | Power |
| | GND | GND | Common ground |
| **W25Q64 Flash** | PB6 | CS | SPI1 chip select |
| | PB3 | CLK | SPI1 clock |
| | PB4 | MISO | SPI1 master in slave out |
| | PB5 | MOSI | SPI1 master out slave in |
| | 3.3V | VCC | Power (module may have 3.3V regulator) |
| | GND | GND | Common ground |
| **UART** | PA9 | TX to USB-Serial RX | 115200 baud, 8-N-1 |
| | PA10 | RX to USB-Serial TX | Optional for commands |
| **Button 1** | PA0 | Mode select | Input with internal pull-up |
| **Button 2** | PA1 | Save Data | Input with internal pull-up |
| **Button 3** | PA2 | Read Data | Input with internal pull-up |

The LCD display and MPU6050 share the same I2C bus (PB10/SCL, PB11/SDA) with different addresses:

| Device | I2C Address (7-bit) | 8-bit Write | 8-bit Read |
|--------|---------------------|-------------|------------|
| **MPU6050** | 0x68 | 0xD0 | 0xD1 |
| **LCD (PCF8574)** | 0x27 | 0x4E | 0x4F |

🔗 [View Custom Written I2C Driver Source Code](https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage/blob/main/Core/Src/i2c2.c) <br>
> **Note**: All peripherals are used as pre-built modules. The LCD module uses a PCF8574 I2C backpack. 

### Project Schematic 
---

<img width="1687" height="727" alt="Schematic_diagram" src="https://github.com/user-attachments/assets/b9e8124a-65e0-4f60-aa5b-d9ffb071a171" />

*Schematic designed using EasyEDA*

## MPU-6050 IMU Driver

The MPU6050 driver provides two types of data access:

1. **Raw Data**: 16-bit integer values directly from sensor registers
2. **Scaled Data**: Converted to physical units using sensitivity scale factors

### Key Features

| Parameter | Value |
|-----------|-------|
| **Interface** | I2C (shared bus with LCD) |
| **I2C Address** | 0x68 (7-bit) |
| **Accelerometer Range** | ±2g/±4g/±8g/±16g (configurable) |
| **Gyroscope Range** | ±250/±500/±1000/±2000 °/s (configurable) |
| **Data Output** | 16-bit raw values |

### Register Map

| Register | Address | Description |
|----------|---------|-------------|
| **WHO_AM_I** | 0x75 | Returns 0x68 for device identification |
| **PWR_MGMT_1** | 0x6B | Power management and device wake-up |
| **ACCEL_XOUT_H** | 0x3B | Accelerometer X-axis high byte |
| **GYRO_XOUT_H** | 0x43 | Gyroscope X-axis high byte |
| **TEMP_OUT_H** | 0x41 | Temperature sensor high byte |

### Implementation Details

- **Burst read** of all 14 data bytes in a single I2C transaction
- Data stored in global structure for access by other tasks
- Configurable measurement ranges via register writes
- Raw 16-bit values accessible for further processing

🔗 [View MPU6050 Driver Source Code](https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage/blob/main/Core/Src/mpu6050.c)

> **Note**: All data displayed on the LCD and stored in flash are **scaled values** converted to physical units (g for accelerometer, °/s for gyroscope, and °C for temperature). Raw 16-bit values are only used internally for calculations.

## DS18B20 Digital Temperature Sensor Driver

The DS18B20 is a 1-Wire digital temperature sensor providing 12-bit temperature measurements.

### Key Features

| Parameter | Value |
|-----------|-------|
| **Interface** | 1-Wire (single data line) |
| **GPIO Pin** | PB0 (open-drain with pull-up) |
| **Temperature Range** | -55°C to +125°C |
| **Accuracy** | ±0.5°C (from -10°C to +85°C) |
| **Resolution** | 12-bit (0.0625°C precision) |
| **Conversion Time** | 750ms (12-bit mode) |

### Commands Used

| Command | Byte | Description |
|---------|------|-------------|
| **SKIP_ROM** | 0xCC | Address single device (no ROM ID needed) |
| **CONVERT_T** | 0x44 | Start temperature conversion |
| **READ_SCRATCHPAD** | 0xBE | Read temperature registers |

### Implementation Details

- **1-Wire protocol** implemented with bit-banging using DWT microsecond delays
- **Open-drain I/O** with external pull-up (built into module)
- **Skip ROM** command used (single device on bus)
- **Temperature calculation**: `temp = raw_12bit * 0.0625°C`
- **Automatic conversion** started after each read

🔗 [View DS18B20 Driver Source Code](https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage/blob/main/Core/Src/ds18b20.c)

> **Note**: The sensor is read at 1Hz frequency to accommodate the 750ms conversion time. The driver automatically starts a new conversion after each read.

## W25Q64 SPI Flash Memory Driver

The W25Q64 is a 64Mbit (8MB) SPI flash memory used for non-volatile data storage.

### Key Features

| Parameter | Value |
|-----------|-------|
| **Interface** | SPI (4-wire: CS, CLK, MOSI, MISO) |
| **SPI Pins** | PB6(CS), PB3(CLK), PB4(MISO), PB5(MOSI) |
| **Capacity** | 64Mbit / 8MByte |
| **Page Size** | 256 bytes |
| **Sector Size** | 4KB (16 pages) |
| **Block Size** | 32KB/64KB (optional) |
| **JEDEC ID** | 0xEF 0x40 0x17 (Winbond W25Q64) |

### Commands Used

| Command | Byte | Description |
|---------|------|-------------|
| **READ_ID** | 0x9F | Read JEDEC ID for device identification |
| **READ_STATUS** | 0x05 | Read status register |
| **WRITE_ENABLE** | 0x06 | Enable write operations |
| **WRITE_DISABLE** | 0x04 | Disable write operations |
| **READ_DATA** | 0x03 | Read data from memory |
| **PAGE_PROGRAM** | 0x02 | Write data to page (max 256 bytes) |
| **SECTOR_ERASE** | 0x20 | Erase 4KB sector |
| **RESET_ENABLE** | 0x66 | Enable reset command |
| **RESET** | 0x99 | Reset device |

### Memory Organization

| Unit | Size | Contains |
|------|------|----------|
| **Byte** | 1 byte | Smallest addressable unit |
| **Page** | 256 bytes | Smallest programmable unit |
| **Sector** | 4KB (16 pages) | Smallest erasable unit |
| **Block (32K)** | 32KB (8 sectors) | Optional erase unit |
| **Block (64K)** | 64KB (16 sectors) | Optional erase unit |
| **Total** | 8MB (2048 sectors) | Full device capacity |

### Implementation Details

- **SPI communication** at up to 18MHz (prescaler 4)
- **Non-blocking erase** operations (sector erase takes ~400ms)
- **Page program** supports cross-page writes automatically
- **Write enable** required before any program/erase operation
- **Status register** polled to check busy state
- **JEDEC ID** verified at startup for device detection

### Storage Capacity with 32-byte Entries

| Unit | Entries |
|------|---------|
| **Page** | 8 entries (256/32) |
| **Sector** | 128 entries (4096/32) |
| **Full 8MB** | **262,144 entries** |

🔗 [View W25Q64 Driver Source Code](https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage/blob/main/Core/Src/w25q64.c)

> **Note**: The driver implements non-blocking sector erase. The erase command is sent and the function returns immediately; the main loop checks status register to verify completion after ~400ms.

## Data Logger Implementation

The data logger module manages storage of sensor readings in the W25Q64 flash memory using a circular buffer architecture.

### Log Entry Format (32 bytes)

| Field | Type | Bytes | Description |
|-------|------|-------|-------------|
| **DS18B20 Temp** | float | 4 | Temperature in °C |
| **MPU6050 Temp** | float | 4 | Temperature in °C |
| **Accel X** | float | 4 | X-axis acceleration (g) |
| **Accel Y** | float | 4 | Y-axis acceleration (g) |
| **Accel Z** | float | 4 | Z-axis acceleration (g) |
| **Gyro X** | float | 4 | X-axis rotation (°/s) |
| **Gyro Y** | float | 4 | Y-axis rotation (°/s) |
| **Gyro Z** | float | 4 | Z-axis rotation (°/s) |

### Memory Organization

| Unit | Size | Entries | Location |
|------|------|---------|----------|
| **Page** | 256 bytes | 8 entries | (sector × 16) + (entry/8) |
| **Sector** | 4KB | 128 entries | Sectors 1-100 |
| **Total** | 400KB | **12,800 entries** | 100 sectors × 128 entries |

- **Circular Buffer**: When full, oldest data automatically overwritten
- **Non-blocking Erase**: Sector erases happen in background (state machine)
- **Persistence**: Data survives power cycles and resets
- **Scan on Init**: Automatically finds existing data on startup

### Erase State Machine

| State | Description |
|-------|-------------|
| **ERASE_IDLE** | No erase in progress, ready to save |
| **ERASE_WAITING** | Erase started, waiting ~400ms for completion |
| **ERASE_VERIFY** | Checking status register to confirm erase done |

🔗 [View Data Logger Source Code](https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage/blob/main/Core/Src/data_logger.c)

> **Note**: The logger uses sectors 1-100 (configurable) for data storage, leaving sector 0 reserved for header configuration. Each sector is erased only when first accessed, spreading the erase overhead over time.

## Getting Started

### Prerequisites

| Component | Details |
|-----------|---------|
| **MCU** | STM32F103C8T6 "Blue Pill" development board |
| **Sensors** | DS18B20 (temperature) + MPU6050 (accelerometer/gyroscope) |
| **Flash Memory** | W25Q64 64Mbit SPI flash module |
| **Display** | 16x2 LCD with I2C backpack (PCF8574) |
| **Communication** | USB-to-Serial converter (CP2102/CH340/FTDI) |
| **Programmer** | ST-Link V2 |
| **IDE** | STM32CubeIDE v1.13.0 or later |

### Installation

1. Clone the repository
```bash
git clone https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage.git
```
2. Open this project in STM32CubeIDE:
   - `File` → `Open Projects from File System...`
   - Select the cloned directory
   - Click `Finish`

3. Build & Flash
    - Build: `Ctrl+B`
    - Debug: `F11`
    - Run: `F8` (Resume)

## Related Projects

- [STM32_MultiSensor_Datalogger](https://github.com/rubin-khadka/STM32_MultiSensor_Datalogger) - Similar project but with microSD card storage instead of SPI flash
- [STM32_DHT11_MPU6050_LCD](https://github.com/rubin-khadka/STM32_DHT11_MPU6050_LCD) - Sensor display project with DHT11 and MPU6050 (foundation for this project)

## Resources

- [STM32F103 Datasheet](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- [STM32F103 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0008-stm32f101xx-stm32f102xx-stm32f103xx-stm32f105xx-and-stm32f107xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [DS18B20 Temperature Sensor Datasheet](https://cdn.sparkfun.com/datasheets/Sensors/Temp/DS18B20.pdf)
- [MPU6050 Accelerometer/Gyroscope Datasheet](https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf)
- [PCF8574 I2C Backpack Datasheet](https://www.ti.com/lit/ds/symlink/pcf8574.pdf)
- [Winbond W25Q64 Flash Memory Datasheet](https://docs.rs-online.com/9bfc/0900766b81704060.pdf)

## Project Status
- **Status**: Complete
- **Version**: v1.0
- **Last Updated**: March 2026

## Contact
**Rubin Khadka Chhetri**  
📧 rubinkhadka84@gmail.com <br>
🐙 GitHub: https://github.com/rubin-khadka

