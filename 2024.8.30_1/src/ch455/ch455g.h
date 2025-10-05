/*
 * ch55g.h
 *
 * created: 2023-05-11
 *  author:
 */

#ifndef _CH455G_H
#define _CH455G_H

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

/* 2�߽ӿڵ�λ����,�뵥Ƭ���й� */
//#define		CH455_SCL_SET		{CH455_SCL=1;}
//#define		CH455_SCL_CLR		{CH455_SCL=0;}
//#define		CH455_SCL_D_OUT		{}				// ����SCLΪ�������,����˫��I/O���л�Ϊ���
//#define		CH455_SDA_SET		{CH455_SDA=1;}
//#define		CH455_SDA_CLR		{CH455_SDA=0;}
//#define		CH455_SDA_IN		(CH455_SDA)
//#define		CH455_SDA_D_OUT		{}				// ����SDAΪ�������,����˫��I/O���л�Ϊ���
//#define		CH455_SDA_D_IN		{CH455_SDA=1;}	// ����SDAΪ���뷽��,����˫��I/O���л�Ϊ����

/* �뵥Ƭ���й�,���ж����ӷ�ʽ�й� */
#define		USE_CH455_KEY		1			// ʹ����CH455�İ����ж�

#ifdef USE_CH455_KEY
	#define		DISABLE_KEY_INTERRUPT	{EX1=0;}
	#define		ENABLE_KEY_INTERRUPT	{EX1=1;}
	#define		CLEAR_KEY_INTER_FLAG	{IE1=0;}
#else
	#define		DISABLE_KEY_INTERRUPT	{}
	#define		ENABLE_KEY_INTERRUPT	{}
	#define		CLEAR_KEY_INTER_FLAG	{}
#endif

/* ********************************************************************************************* */
// ����ϵͳ��������

#define CH455_BIT_ENABLE	0x01		// ����/�ر�λ
#define CH455_BIT_SLEEP		0x04		// ˯�߿���λ
#define CH455_BIT_7SEG		0x08		// 7�ο���λ
#define CH455_BIT_INTENS1	0x10		// 1������
#define CH455_BIT_INTENS2	0x20		// 2������
#define CH455_BIT_INTENS3	0x30		// 3������
#define CH455_BIT_INTENS4	0x40		// 4������
#define CH455_BIT_INTENS5	0x50		// 5������
#define CH455_BIT_INTENS6	0x60		// 6������
#define CH455_BIT_INTENS7	0x70		// 7������
#define CH455_BIT_INTENS8	0x00		// 8������

#define CH455_SYSOFF	0x0400			// �ر���ʾ���رռ���
#define CH455_SYSON		( CH455_SYSOFF | CH455_BIT_ENABLE )	// ������ʾ������
#define CH455_SLEEPOFF	CH455_SYSOFF	// �ر�˯��
#define CH455_SLEEPON	( CH455_SYSOFF | CH455_BIT_SLEEP )	// ����˯��
#define CH455_7SEG_ON	( CH455_SYSON | CH455_BIT_7SEG )	// �����߶�ģʽ
#define CH455_8SEG_ON	( CH455_SYSON | 0x00 )	// �����˶�ģʽ
#define CH455_SYSON_4	( CH455_SYSON | CH455_BIT_INTENS4 )	// ������ʾ�����̡�4������
#define CH455_SYSON_8	( CH455_SYSON | CH455_BIT_INTENS8 )	// ������ʾ�����̡�8������


// ��������������
#define CH455_DIG0		0x1400			// �����λ0��ʾ,�����8λ����
#define CH455_DIG1		0x1500			// �����λ1��ʾ,�����8λ����
#define CH455_DIG2		0x1600			// �����λ2��ʾ,�����8λ����
#define CH455_DIG3		0x1700			// �����λ3��ʾ,�����8λ����



// ��ȡ������������
#define CH455_GET_KEY	0x0700					// ��ȡ����,���ذ�������


// CH455�ӿڶ���
#define		CH455_I2C_ADDR		0x24			// CH455�ĵ�ַ
#define		CH455_I2C_MASK		0x3E			// CH455�ĸ��ֽ���������


void CH455_SDA_D_OUT(void);
void CH455_SDA_D_IN(void);
void CH455_Init(void);
unsigned char Keyboard_proc(void);
#endif // _CH55G_H


