/*
 * data_logger.h
 *
 *  Created on: Feb 28, 2026
 *      Author: Rubin Khadka
 */

#ifndef INC_DATA_LOGGER_H_
#define INC_DATA_LOGGER_H_

#include "stdint.h"

// Data structure for a single log entry (32 bytes)
typedef struct {
    float ds18b20_temp;       // 4 bytes
    float accel_x;            // 4 bytes
    float accel_y;            // 4 bytes
    float accel_z;            // 4 bytes
    float gyro_x;             // 4 bytes
    float gyro_y;             // 4 bytes
    float gyro_z;             // 4 bytes
    uint32_t timestamp;       // 4 bytes
} LogEntry_t;

// Public functions
void DataLogger_Init(void);

#endif /* INC_DATA_LOGGER_H_ */
