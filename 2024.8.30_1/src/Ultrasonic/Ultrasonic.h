/*
 * Ultrasonic.h
 *
 * created: 2023/2/23
 *  author:
 */

#ifndef _ULTRASONIC_H
#define _ULTRASONIC_H

#include "ls1b_gpio.h"
#include "ls1x_pwm.h"
#include "bsp.h"
#include <stdbool.h>
#include "stdint.h"

void Ultrasonic_init(void);
unsigned int Ultrasonic_ranging(void);
unsigned int distance_ranging(void);
#endif // _ULTRASONIC_H
