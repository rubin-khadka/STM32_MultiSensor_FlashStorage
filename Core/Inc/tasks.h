/*
 * tasks.h
 *
 *  Created on: Feb 22, 2026
 *      Author: Rubin Khadka
 */

#ifndef TASKS_H_
#define TASKS_H_

// Function prototypes
// Task Functions
void Task_MPU6050_Read(void);
void Task_LCD_Update(void);
void Task_UART_Output(void);
void Task_DS18B20_Read(void);

// Feedback Functions
void Feedback_Show(const char *line1, const char *line2, uint16_t duration_ms);
void Task_Feedback_Update(void);
void Feedback_Clear(void);
uint8_t Feedback_IsActive(void);

#endif /* TASKS_H_ */
