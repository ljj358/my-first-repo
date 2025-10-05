#ifndef __AS608_H
#define __AS608_H
#include <stdio.h>

#define PS_Sta   PAin(6)//读指纹模块状态引脚
#define CharBuffer1 0x01
#define CharBuffer2 0x02

extern unsigned int AS608Addr;//模块地址

typedef struct  
{
	unsigned short pageID;//指纹ID
	unsigned short mathscore;//匹配得分
}SearchResult;

typedef struct
{
	unsigned short PS_max;//指纹最大容量
	unsigned char  PS_level;//安全等级
	unsigned int PS_addr;
	unsigned char  PS_size;//通讯数据包大小
	unsigned char  PS_N;//波特率基数N
}SysPara;

void UART4_Config_Init(void);    //UART4串口初始化
	
unsigned char PS_GetImage(void); //录入图像
 
unsigned char PS_GenChar(unsigned char BufferID);//生成特征

unsigned char PS_Match(void);//精确比对两枚指纹特征

unsigned char PS_Search(unsigned char BufferID,unsigned short StartPage,unsigned short PageNum,SearchResult *p);//搜索指纹
 
unsigned char PS_RegModel(void);//合并特征（生成模板）
 
unsigned char PS_StoreChar(unsigned char BufferID,unsigned short PageID);//储存模板

unsigned char PS_DeletChar(unsigned short PageID,unsigned short N);//删除模板

unsigned char PS_Empty(void);//清空指纹库

unsigned char PS_WriteReg(unsigned char RegNum,unsigned char DATA);//写系统寄存器
 
unsigned char PS_ReadSysPara(SysPara *p); //读系统基本参数

unsigned char PS_SetAddr(unsigned int addr);  //设置模块地址

unsigned char PS_WriteNotepad(unsigned char NotePageNum,unsigned char *content);//写记事本

unsigned char PS_ReadNotepad(unsigned char NotePageNum,unsigned char *note);//读记事

unsigned char PS_HighSpeedSearch(unsigned char BufferID,unsigned short StartPage,unsigned short PageNum,SearchResult *p);//高速搜索
  
unsigned char PS_ValidTempleteNum(unsigned short *ValidN);//读有效模板个数

unsigned char PS_HandShake(unsigned int *PS_Addr); //与AS608模块握手

const char *EnsureMessage(unsigned char ensure);//确认码错误信息解析

char press_FR(int *pageID , int *mathscore );            //刷指纹

const char *EnsureMessage_UTF8(unsigned char ensure);

char Del_FR(int num);                                    //删除指纹

extern SysPara AS608Para;//指纹模块AS608参数
extern unsigned short ValidN;//模块内有效指纹个数
#endif

