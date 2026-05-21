/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-23     Malongwei    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#ifdef BSP_USING_TM1650
#include "drv_tm1650.h"


#define DRV_DEBUG
#define LOG_TAG             "drv.tm1650"
#include <drv_log.h>

//显示参数
#define TM1650_BRIGHT1       0x11   /*一级亮度,打开LED显示*/
#define TM1650_BRIGHT2       0x21   /*二级亮度,打开LED显示*/
#define TM1650_BRIGHT3       0x31   /*三级亮度,打开LED显示*/
#define TM1650_BRIGHT4       0x41   /*四级亮度,打开LED显示*/
#define TM1650_BRIGHT5       0x51   /*五级亮度,打开LED显示*/
#define TM1650_BRIGHT6       0x61   /*六级亮度,打开LED显示*/
#define TM1650_BRIGHT7       0x71   /*七级亮度,打开LED显示*/
#define TM1650_BRIGHT8       0x01   /*八级亮度,打开LED显示*/
#define TM1650_DSP_OFF       0x00   /*关闭LED显示*/

//数码管位选
#define TM1650_DIG1     0
#define TM1650_DIG2     1
#define TM1650_DIG3     2
#define TM1650_DIG4     3


#define SCL_PIN GET_PIN(E, 8)
#define SDA_PIN GET_PIN(E, 9)


//========【配置IIC总线的信号读写和时序】=======

#define TM1650_SDA_OUT_MODE		rt_pin_mode(SDA_PIN, PIN_MODE_OUTPUT_OD);

#define TM1650_SDA_IN_MODE		rt_pin_mode(SDA_PIN, PIN_MODE_INPUT_PULLUP);


//主机拉高SCL
#define TM1650_IIC_SCL_HIGH     rt_pin_write(SCL_PIN, PIN_HIGH)

//主机拉低SCL
#define TM1650_IIC_SCL_LOW      rt_pin_write(SCL_PIN, PIN_LOW)


//主机拉高SDA
#define TM1650_IIC_SDA_HIGH     rt_pin_write(SDA_PIN, PIN_HIGH)

//主机拉低SDA
#define TM1650_IIC_SDA_LOW      rt_pin_write(SDA_PIN, PIN_LOW)

//参数b为0时主机拉低SDA&#xff0c;非0则拉高SDA
#define TM1650_IIC_SDA_WR(b)    do{                                       \
                               if(b) rt_pin_write(SDA_PIN, PIN_HIGH);   \
                               else  rt_pin_write(SDA_PIN, PIN_LOW); \
                              }while(0)


//主机读取SDA线电平状态&#xff0c;返回值为0为低电平&#xff0c;非0则为高电平
#define TM1650_IIC_SDA_RD()    rt_pin_read(SDA_PIN)

//软件延时2us
#define TM1650_IIC_DELAY_2US   do{for(int ii_=0;ii_<25;ii_++);}while(0)

//软件延时4us
#define TM1650_IIC_DELAY_4US   do{for(int ii_=0;ii_<50;ii_++);}while(0)
//================================

static const uint8_t _gaucCmdTable[] = {
    TM1650_DSP_OFF,
    TM1650_BRIGHT1,
    TM1650_BRIGHT2,
    TM1650_BRIGHT3,
    TM1650_BRIGHT4,
    TM1650_BRIGHT5,
    TM1650_BRIGHT6,
    TM1650_BRIGHT7,
    TM1650_BRIGHT8,
};

void TM1650_cfg_display(uint8_t param);
int TM1650_print(uint8_t dig,uint8_t seg_data);
void TM1650_clear(void);

//产生IIC总线起始信号
static void TM1650_IIC_start(void)
{
	TM1650_SDA_OUT_MODE;
	
	TM1650_IIC_SDA_HIGH;    //SDA=1
    TM1650_IIC_SCL_HIGH;     //SCL=1
    
    TM1650_IIC_DELAY_4US;
    
	TM1650_IIC_SDA_LOW;     //SDA=0
   
	TM1650_IIC_DELAY_4US;
    
	TM1650_IIC_SCL_LOW;      //SCL=0
}

//产生IIC总线结束信号
static void TM1650_IIC_stop(void)
{
	TM1650_SDA_OUT_MODE;
	
    TM1650_IIC_SCL_LOW;      //SCL=0
    TM1650_IIC_SDA_LOW;      //SDA=0
	
    TM1650_IIC_DELAY_4US;
	
    TM1650_IIC_SCL_HIGH;     //SCL=1
	
    TM1650_IIC_DELAY_4US;
	
    TM1650_IIC_SDA_HIGH;    //SDA=1
}

//通过IIC总线发送一个字节
static void TM1650_IIC_write_byte(uint8_t dat)
{
    uint8_t i;

	TM1650_SDA_OUT_MODE;
    
    for(i=0;i<8;i++)
    {
		TM1650_IIC_SCL_LOW;
        TM1650_IIC_SDA_WR(dat&0x80);
        dat<<=1;

        TM1650_IIC_DELAY_2US;
        TM1650_IIC_SCL_HIGH;
        TM1650_IIC_DELAY_2US;
        TM1650_IIC_SCL_LOW;
        TM1650_IIC_DELAY_2US;
    }
}

//通过IIC总线接收从机响应的ACK信号
static uint8_t TM1650_IIC_wait_ack(void)
{
    uint8_t ack_signal = 0;
	
	TM1650_SDA_IN_MODE;

//    TM1650_IIC_SDA_HIGH;    //SDA=1
//    TM1650_IIC_DELAY_2US;
    TM1650_IIC_SCL_HIGH;
	
    TM1650_IIC_DELAY_2US;
	
    if(TM1650_IIC_SDA_RD()) ack_signal = 1;   //如果读取到的是NACK信号
    
	TM1650_IIC_SCL_LOW;
    
	TM1650_IIC_DELAY_2US;
	
    return ack_signal;
}


//TM1650初始化
void TM1650_init(void)
{
    rt_pin_mode(SCL_PIN, PIN_MODE_OUTPUT_OD);
    rt_pin_mode(SDA_PIN, PIN_MODE_OUTPUT_OD);

    TM1650_IIC_SDA_HIGH;   //释放SDA线
    TM1650_IIC_SCL_HIGH;   //释放SCL线

    TM1650_cfg_display(TM1650_BRIGHT5);   //初始化为5级亮度,打开显示
    TM1650_clear();     //将显存内容清0
}


//作用:设置显示参数
//备注:这个操作不影响显存中的数据
//用例:
//	设置亮度并打开显示:TM1650_cfg_display(TM1650_BRIGHTx)
//	关闭显示:TM1650_cfg_display(TM1650_DSP_OFF)
void TM1650_cfg_display(uint8_t param)
{
    TM1650_IIC_start();
    TM1650_IIC_write_byte(0x48);  TM1650_IIC_wait_ack();     //固定命令
    TM1650_IIC_write_byte(param); TM1650_IIC_wait_ack();    //参数值
    TM1650_IIC_stop();
}


//将显存数据全部刷为0,清空显示
void TM1650_clear(void)
{
    uint8_t dig;
    for(dig = TM1650_DIG1 ; dig<= TM1650_DIG4 ;dig++)
    {
        TM1650_print(dig,0);   //将显存数据刷为0
    }
}

//往一个指定的数码管位写入指定的显示数据
//共阴数码管段码表:
//const uint8_t TUBE_TABLE_0[10] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};                                  //共阴,0~9的数字
//const uint8_t TUBE_TABLE_0[16] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71};    //共阴,0~9~A~F
//用例:
//	在DIG1位上显示数字3: TM1650_print(TM1650_DIG1,TUBE_TABLE_0[3]);
int TM1650_print(uint8_t dig,uint8_t seg_data)
{
	uint8_t ack_flag = 0;
	
    TM1650_IIC_start();
    TM1650_IIC_write_byte(dig*2+0x68); 

	ack_flag = TM1650_IIC_wait_ack();  //显存起始地址为0x68
	
	if(ack_flag != 0)
	{

		return ack_flag;
	}	
	
//	rt_kprintf("[ack_flag111---] flag : %d \r\n",ack_flag);
	
	
    TM1650_IIC_write_byte(seg_data); 
	
	
	ack_flag = TM1650_IIC_wait_ack();    //发送段码
	
	if(ack_flag != 0)
	{
		return ack_flag;
	}	
	
//	rt_kprintf("[ack_flag222---] flag : %d \r\n",ack_flag);
	
    TM1650_IIC_stop();
	
	return ack_flag;
}

static int tm1650_init(void)
{
    TM1650_init();

    return 0;
}
INIT_BOARD_EXPORT(tm1650_init);

void tm1650Display(uint8_t ucDig, uint8_t ucVal)
{
    TM1650_print(ucDig, ucVal);
}

void tm1650Lightness(uint8_t ucVal)
{
    if (ucVal > 8) {
        return;
    }
    TM1650_cfg_display(_gaucCmdTable[ucVal]);
}

void tm1650RefreshBuf(uint8_t *pucBuf)
{
    uint8_t i;
	
	rt_pin_mode(SCL_PIN, PIN_MODE_OUTPUT_OD);
	rt_pin_mode(SDA_PIN, PIN_MODE_OUTPUT_OD);

	TM1650_IIC_SDA_HIGH;   //释放SDA线
	TM1650_IIC_SCL_HIGH;   //释放SCL线
	
	TM1650_IIC_DELAY_4US;
	
    for(i = TM1650_DIG1 ; i<= TM1650_DIG4 ;i++)
    {
		TM1650_cfg_display(TM1650_BRIGHT5);   //初始化为5级亮度,打开显示
		
		TM1650_IIC_DELAY_4US;
		
        if(TM1650_print(i, pucBuf[i]) != 0)
		{
			return;
		}
    }
}


#ifdef DRV_DEBUG
#ifdef FINSH_USING_MSH
static int tm1650_set(uint8_t argc, char **argv)
{
    if (argc != 3) {
        rt_kprintf("example:led_set digit value\n");
        return 0;
    }

    TM1650_print(atoi(argv[1]), atoi(argv[2]));

    return 0;
}
MSH_CMD_EXPORT(tm1650_set, set tm1650 state);

#endif /* FINSH_USING_MSH */
#endif /* DRV_DEBUG */
#endif /* BSP_USING_TM1650 */
