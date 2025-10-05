/*
 * matrix.c
 *
 * created: 2021/5/23
 *  author:
 */
 #include "matrix.h"
 #include "hc595.h"
 #include "ls1x_rtc.h"

 //行扫描数据数组
 unsigned short Data_H[32] = {  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //第一个行管HC595
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};//第二个行管HC595

 //列扫描数据数组
 unsigned short Data_L[32] = {  ~0x01, ~0x02, ~0x04, ~0x08, ~0x10, ~0x20, ~0x40, ~0x80, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, //第一个行管HC595
                                ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x00, ~0x01, ~0x02, ~0x04, ~0x08, ~0x10, ~0x20, ~0x40, ~0x80};//第二个行管HC595
/*************************************************************************************
 **函数功能：初始化IO口
 **说明：
 **************************************************************************************/
 void Matrix_init(void)
 {
    HC595_init();   //HC595IO初始化
    LED_ALL_OFF();

     LED_ALL_OFF();
 }
 
 

 /*************************************************************************************
 **函数功能：点阵上的点全部被熄灭
 **说明：
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
 **函数功能：静态显示
 **说明：可显示一个汉字和字符，图片等
 **参数：@unsigned char *data  -- 数据
 **************************************************************************************/
 void Dis_Char(unsigned char *data)
 {
    unsigned char i,j = 0;
    for(i = 0; i < 16; i++)//扫描16行
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
 **函数功能：动态显示
 **说明：可连续显示汉字和字符，图片等
 **参数：@unsigned char *data  -- 数据
         @unsigned char len -- 数据帧长度
 **************************************************************************************/
 void Dis_Dynamic(unsigned char *data, unsigned int len)
 {
    static unsigned int cnt = 0;
    unsigned char i,k;
    unsigned char j = 0;
    for(k = 0; k < 200; k++)//控制变换字符的速度
    {
        for(i = 0; i < 16; i++)//扫描16行
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
 **函数功能：从下往上滚动显示
 **参数：@unsigned char *data  -- 数据
         @unsigned char len -- 数据帧长度
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
 **函数功能：从右往左滚动显示
 **参数：@unsigned char *data  -- 数据
         @unsigned char len -- 数据帧长度
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
