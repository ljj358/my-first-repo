/*
 * StepperMotor.h
 *
 * created: 2024/5/21
 *  author: 
 */

#ifndef _STEPPERMOTOR_H
#define _STEPPERMOTOR_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ls1b_gpio.h"
#include "rtthread.h"
void step_28byj48_control(unsigned char  step,unsigned char  dir);
void step_28byj48_angles(unsigned int  angles,unsigned char  dir);
#define SM_IN1(IN) gpio_write(40,IN)
#define SM_IN2(IN) gpio_write(41,IN)
#define SM_IN3(IN) gpio_write(39,IN)
#define SM_IN4(IN) gpio_write(44,IN)
void StepperMotor(void);
#ifdef __cplusplus
}
#endif

#endif // _STEPPERMOTOR_H

