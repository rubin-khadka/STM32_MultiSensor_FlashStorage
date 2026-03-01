# STM32 MultiSensor W25Q64 FlashStorage Project

![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![STM32](https://img.shields.io/badge/STM32-F103C8T6-blue)
![CubeIDE](https://img.shields.io/badge/IDE-STM32CubeIDE-darkblue)

## Video Demonstrations

## Project Overview

The **STM32 MultiSensor Flash Storage Logger** is a comprehensive embedded systems project built around the STM32F103C8T6 (Bluepill) microcontroller. It demonstrates real-time sensor data acquisition, visualization on an LCD and UART, and reliable non-volatile storage using external SPI flash memory.

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

### Hardware Components

| Component | Quantity | Description |
|-----------|----------|-------------|
| **STM32F103C8T6** | 1 | "Blue Pill" development board with 72MHz Cortex-M3 |
| **W25Q64 Flash** | 1 | 64Mbit (8MB) SPI flash memory for data logging |
| **DS18B20** | 1 | Digital temperature sensor (1-Wire protocol, ±0.5°C accuracy) |
| **MPU6050** | 1 | 6-axis inertial measurement unit (accelerometer + gyroscope + temperature) |
| **LCD 16x2 with I2C** | 1 | Character display module with I2C backpack (PCF8574) |
| **Push Buttons** | 3 | Two-leg tactile switches for user input |
| **USB-to-Serial Converter** | 1 | CP2102 / CH340 / FTDI for UART communication and debugging |

## Pin Configuration

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
| **W25Q64 Flash** | PA4 | CS | SPI1 chip select |
| | PA5 | CLK | SPI1 clock |
| | PA6 | MISO | SPI1 master in slave out |
| | PA7 | MOSI | SPI1 master out slave in |
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
| **LCD Module** | 0x27 | 0x4E | 0x4F |

🔗 [View Custom Written I2C Driver Source Code](https://github.com/rubin-khadka/STM32_MultiSensor_FlashStorage/blob/main/Core/Src/i2c2.c) <br>
**Note**: All peripherals are used as pre-built modules. The LCD module uses a PCF8574 I2C backpack. 

### Project Schematic 

<img width="1687" height="727" alt="Schematic_diagram" src="https://github.com/user-attachments/assets/b9e8124a-65e0-4f60-aa5b-d9ffb071a171" />

*Schematic designed using EasyEDA*

## MPU-6050 IMU Driver

The MPU6050 is a 6-axis inertial measurement unit that combines a 3-axis accelerometer and a 3-axis gyroscope.

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