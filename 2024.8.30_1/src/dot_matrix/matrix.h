/*
 * matrix.h
 *
 * created: 2021/5/23
 *  author: 
 */

#ifndef _MATRIX_H
#define _MATRIX_H

#define RED   1
#define GREEN 2
#define BLUE  3

extern unsigned int Second;
void Dis_1_Roll(unsigned char *data, unsigned int len);
void Dis_Roll(unsigned char *data, unsigned int len);
void Matrix_init(void);
void LED_ALL_OFF(void);

void Dis_Char(unsigned char *data);
void Dis_Number(unsigned char *data,unsigned char *data1);



#endif // _MATRIX_H

