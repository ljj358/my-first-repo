/*
 * pwm_ic.h
 *
 * created: 2022/3/2
 *  author:
 */

#ifndef _PWM_IC_H
#define _PWM_IC_H

#include <stdint.h>
#include <stddef.h>
void PCA_MG9XX_Init(float hz,uint8_t angle);
void IS31FL3193_Reg_Write(unsigned char addr,unsigned char data);
void Set_PWM(unsigned int brightpercent);
void pca_setpwm(uint8_t num, uint32_t on, uint32_t off);
#endif // _PWM_IC_H


