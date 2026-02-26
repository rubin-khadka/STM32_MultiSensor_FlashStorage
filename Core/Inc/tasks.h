/*
 * tasks.h
 *
 *  Created on: Feb 22, 2026
 *      Author: Rubin Khadka
 */

#ifndef TASKS_H_
#define TASKS_H_

// Task function prototypes
void Task_DHT11_Read(void);
void Task_MPU6050_Read(void);
void Task_LCD_Update(void);
void Task_UART_Output(void);
void Task_DS18B20_Read(void);

#endif /* TASKS_H_ */
