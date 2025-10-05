/*  Copyright (s) 2019 ���ڰ������Ƽ����޹�˾
 *  All rights reserved
 * 
 * �ļ����ƣ�ring_buffer.h
 * ժҪ��
 *  
 * �޸���ʷ     �汾��        Author       �޸�����
 *--------------------------------------------------
 * 2021.8.21      v01         ���ʿƼ�      �����ļ�
 *--------------------------------------------------
*/
#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

//#include "stm32f1xx_hal.h"

#define BUFFER_SIZE 2048        /* ���λ������Ĵ�С */
typedef struct
{
    unsigned char buffer[BUFFER_SIZE];  /* �������ռ� */
    volatile unsigned int pW;           /* д��ַ */
    volatile unsigned int pR;           /* ����ַ */
} ring_buffer;

/*
 *  ��������void ring_buffer_init(ring_buffer *dst_buf)
 *  ���������dst_buf --> ָ��Ŀ�껺����
 *  �����������
 *  ����ֵ����
 *  �������ã���ʼ��������
*/
extern void ring_buffer_init(ring_buffer *dst_buf);

/*
 *  ��������void ring_buffer_write(unsigned char c, ring_buffer *dst_buf)
 *  ���������c --> Ҫд�������
 *            dst_buf --> ָ��Ŀ�껺����
 *  �����������
 *  ����ֵ����
 *  �������ã���Ŀ�껺����д��һ���ֽڵ����ݣ�������������˾Ͷ���������
*/
extern void ring_buffer_write(unsigned char c, ring_buffer *dst_buf);

/*
 *  ��������int ring_buffer_read(unsigned char *c, ring_buffer *dst_buf)
 *  ���������c --> ָ�򽫶��������ݱ��浽�ڴ��еĵ�ַ
 *            dst_buf --> ָ��Ŀ�껺����
 *  �����������
 *  ����ֵ���������ݷ���0�����򷵻�-1
 *  �������ã���Ŀ�껺������ȡһ���ֽڵ����ݣ�������������˷���-1������ȡʧ��
*/
extern int ring_buffer_read(unsigned char *c, ring_buffer *dst_buf);

#endif /* __RING_BUFFER_H */

