#ifndef _Stepper_motor_H
#define _Stepper_motor_H

#include <stdint.h>
#include <stddef.h>


void set_frequency(float freq);//…Ë÷√PWM∆µ¬ 
void set_Duty_cycle(uint8_t num, uint32_t on, uint32_t off);
void Stepper_motor_Init(float hz,int angle);
void Stepper_motor_open(void);
#endif 
