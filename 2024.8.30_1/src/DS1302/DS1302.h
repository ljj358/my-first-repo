/*
 * DS1302.h
 *
 * created: 2024/5/24
 *  author: 
 */

#ifndef _DS1302_H
#define _DS1302_H

#ifdef __cplusplus
extern "C" {
#endif



extern unsigned char DS1302_Time[];//��������ʱ�������
void  DS1302_Init();//������ʼ������
void  DS1302_WriteBety(unsigned char command,unsigned char Data);//�������ֽ�д�뺯��
unsigned char   DS1302_ReadBety(unsigned char command);//�������ֽڶ�������
void DS1302_SetTime();//���������ڲ�ʱ�亯��
void DS1302_ReadTime();//������ȡ�ڲ�ʱ�亯��

#ifdef __cplusplus
}
#endif

#endif // _DS1302_H

