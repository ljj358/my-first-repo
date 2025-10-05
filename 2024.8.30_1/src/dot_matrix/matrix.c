/*
 * matrix.c
 *
 * created: 2021/5/23
 *  author:
 */
 #include "matrix.h"
 #include "hc595.h"
 #include "ls1x_rtc.h"

 //��ɨ����������
 unsigned short Data_H[32] = {  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //��һ���й�HC595
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};//�ڶ����й�HC595

 //��ɨ����������
 unsigned short Data_L[32] = {  ~0x01, ~0x02, ~0x04, ~0x08, ~0x10, ~0x20, ~0x40, ~0x80, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, //��һ���й�HC595
                                ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x01, ~0x02, ~0x04, ~0x08, ~0x10, ~0x20, ~0x40, ~0x80};//�ڶ����й�HC595
/*************************************************************************************
 **�������ܣ���ʼ��IO��
 **˵����
 **************************************************************************************/
 void Matrix_init(void)
 {
    HC595_init();   //HC595IO��ʼ��
    LED_ALL_OFF();

     LED_ALL_OFF();
 }
 
 

 /*************************************************************************************
 **�������ܣ������ϵĵ�ȫ����Ϩ��
 **˵����
 **************************************************************************************/
 void LED_ALL_OFF(void)
 {
    HC595_Send_Data(0x00);
    HC595_Send_Data(0x00);
    HC595_Send_Data(0x00);
    HC595_Send_Data(0x00);
    HC595_Out();
 }


 /*************************************************************************************
 **�������ܣ���̬��ʾ
 **˵��������ʾһ�����ֺ��ַ���ͼƬ��
 **������@unsigned char *data  -- ����
 **************************************************************************************/
 void Dis_Char(unsigned char *data)
 {
    unsigned char i,j = 0;
    for(i = 0; i < 16; i++)//ɨ��16��
    {
        HC595_Send_Data(data[j+1]);
        HC595_Send_Data(data[j]);
        HC595_Send_Data(Data_H[i+16]);
        HC595_Send_Data(Data_H[i]);
        HC595_Out();
        j += 2;
    }
    j = 0;
 }

 void Dis_Number(unsigned char *data,unsigned char *data1)
 {
            static unsigned char i;
            HC595_Send_Data(data1[i]);
            HC595_Send_Data(data[i]);
            HC595_Send_Data(Data_H[i+16]);
            HC595_Send_Data(Data_H[i]);
            HC595_Out();
            i++;
            if(i >= 15)
            {
                i = 0;
            }
 }

/*************************************************************************************
 **�������ܣ���̬��ʾ
 **˵������������ʾ���ֺ��ַ���ͼƬ��
 **������@unsigned char *data  -- ����
         @unsigned char len -- ����֡����
 **************************************************************************************/
 void Dis_Dynamic(unsigned char *data, unsigned int len)
 {
    static unsigned int cnt = 0;
    unsigned char i,k;
    unsigned char j = 0;
    for(k = 0; k < 200; k++)//���Ʊ任�ַ����ٶ�
    {
        for(i = 0; i < 16; i++)//ɨ��16��
        {
            HC595_Send_Data(data[j+1+cnt]);
            HC595_Send_Data(data[j+cnt]);
            HC595_Send_Data(Data_H[i+16]);
            HC595_Send_Data(Data_H[i]);
            HC595_Out();
            j += 2;
        }
        j = 0;
    }
    cnt += 32;
    if(cnt > len - 32)
        cnt=0;
 }

 
 /*************************************************************************************
 **�������ܣ��������Ϲ�����ʾ
 **������@unsigned char *data  -- ����
         @unsigned char len -- ����֡����
 **************************************************************************************/
 void Dis_Roll(unsigned char *data, unsigned int len)
 {
    static unsigned int cnt = 0,cnt1 = 0;
    static unsigned char i,j = 0;
    
            HC595_Send_Data(data[j+1+cnt]);
            HC595_Send_Data(data[j+cnt]);
            HC595_Send_Data(Data_H[i+16]);
            HC595_Send_Data(Data_H[i]);
            
            HC595_Out();
            j += 2;
            i++;
        if(i == 15)
        {
            i = 0,j = 0;
            if(cnt1++>=5)
              {
                   cnt1 = 0,cnt += 2;
                    if(cnt > len-32)
                    cnt=0;
              }
        }
 }

/*************************************************************************************
 **�������ܣ��������������ʾ
 **������@unsigned char *data  -- ����
         @unsigned char len -- ����֡����
 **************************************************************************************/
 void Dis_1_Roll(unsigned char *data, unsigned int len)
 {
    static unsigned int cnt = 0,cnt1 = 0;

    static unsigned char i,j = 0;
        
            HC595_Send_Data(Data_L[i+16]);
            HC595_Send_Data(Data_L[i]);
            HC595_Send_Data(data[j+1+cnt]);
            HC595_Send_Data(data[j+cnt]);
            HC595_Out();
            j += 2;
            i++;
        if(i == 15)
        {
            i = 0,j = 0;
            if(cnt1++>=5)
              {
                   cnt1 = 0,cnt += 2;
                    if(cnt > len-32)
                    cnt=0;
              }
        }
        
 }
