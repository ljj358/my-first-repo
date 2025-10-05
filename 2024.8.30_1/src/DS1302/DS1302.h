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



extern unsigned char DS1302_Time[];//声明设置时间的数组
void  DS1302_Init();//声明初始化函数
void  DS1302_WriteBety(unsigned char command,unsigned char Data);//声明单字节写入函数
unsigned char   DS1302_ReadBety(unsigned char command);//声明单字节读出函数
void DS1302_SetTime();//声明设置内部时间函数
void DS1302_ReadTime();//声明读取内部时间函数

#ifdef __cplusplus
}
#endif

#endif // _DS1302_H

