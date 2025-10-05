#include <string.h>
#include "as608.h"
#include "ls1b.h"
#include "ns16550.h"
#include "rtthread.h"
unsigned int AS608Addr = 0XFFFFFFFF; //默认
char buff_uart4[400] = {0};
static unsigned char buff_uart4_tx[1] = {0};
 SysPara AS608Para;//指纹模块AS608参数
 unsigned short ValidN;//模块内有效指纹个数
 void UART4_Config_Init(void)
{
    /**
     * @brief 串口初始化(次要)
     * @param 1 void
     * @return void
     */
    unsigned int BaudRate = 115200;
    ls1x_uart_init(devUART4,(void *)BaudRate); //初始化串口
    ls1x_uart_open(devUART4,NULL); //打开串口
    
    unsigned char ensure;
	unsigned char key_num;

    while(PS_HandShake(&AS608Addr))//与AS608模块握手
	{
		rt_thread_mdelay(400);
		rt_kprintf("未检测到模块!!!\n");
		rt_thread_mdelay(800);
		rt_kprintf("尝试连接模块...\n");
	}
	rt_kprintf("通讯成功!!!\n");
    rt_thread_mdelay(10);
	ensure=PS_ValidTempleteNum(&ValidN);//读库指纹个数
	if(ensure!=0x00)
		rt_kprintf("%s\n",EnsureMessage(ensure));//显示确认码错误信息
	ensure=PS_ReadSysPara(&AS608Para);  //读参数
	if(ensure!=0x00)
	{
          rt_kprintf("%s\n",EnsureMessage(ensure));//显示确认码错误信息
	}
}



//刷指纹
char press_FR(int *pageID , int *mathscore )
{
	SearchResult seach;
	unsigned char ensure;
	char *str;
	ensure=PS_GetImage();
	if(ensure==0x00)//获取图像成功
	{
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //生成特征成功
		{
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//搜索成功
			{
			    *pageID = seach.pageID;
			    *mathscore = seach.mathscore;
			    return 1;
			}
	   }
	}
	return 0;
}
//删除指纹
char Del_FR(int num)
{
	unsigned char  ensure;
	unsigned esc = 0;
	ensure=PS_DeletChar(num,1);//删除单个指纹
	esc = ensure;
	if(esc!=0)
	   return esc;

	rt_thread_mdelay(1200);
	PS_ValidTempleteNum(&ValidN);//读库指纹个数
    return esc;
}
//串口发送一个字节
static void MYUSART_SendData(unsigned char data)
{
    buff_uart4_tx[0] = data;
    ls1x_uart_write(devUART4,buff_uart4_tx,1,NULL);
}
//发送包头
static void SendHead(void)
{
	MYUSART_SendData(0xEF);
	MYUSART_SendData(0x01);
}
//发送地址
static void SendAddr(void)
{
	MYUSART_SendData(AS608Addr>>24);
	MYUSART_SendData(AS608Addr>>16);
	MYUSART_SendData(AS608Addr>>8);
	MYUSART_SendData(AS608Addr);
}
//发送包标识,
static void SendFlag(unsigned char flag)
{
	MYUSART_SendData(flag);
}
//发送包长度
static void SendLength(int length)
{
	MYUSART_SendData(length>>8);
	MYUSART_SendData(length);
}
//发送指令码
static void Sendcmd(unsigned char cmd)
{
	MYUSART_SendData(cmd);
}
//发送校验和
static void SendCheck(unsigned short check)
{
	MYUSART_SendData(check>>8);
	MYUSART_SendData(check);
}
//判断中断接收的数组有没有应答包
//waittime为等待中断接收数据的时间（单位1ms）
//返回值：数据包首地址
static unsigned char *JudgeStr(unsigned short waittime)
{
    int len = 0;
    while(ls1x_uart_read(devUART4,&buff_uart4[100],1,NULL));
	char *data;
	unsigned char str[8];
	str[0]=0xef;str[1]=0x01;str[2]=AS608Addr>>24;
	str[3]=AS608Addr>>16;str[4]=AS608Addr>>8;
	str[5]=AS608Addr;str[6]=0x07;str[7]='\0';
	
    while(--waittime)
	{

		    len += ls1x_uart_read(devUART4,&buff_uart4[len],400,NULL);
		    rt_thread_mdelay(1);
		    if(len){
		        rt_thread_mdelay(10);
		        len += ls1x_uart_read(devUART4,&buff_uart4[len],400,NULL);
		        data=strstr((const char*)buff_uart4,(const char*)str);
			    if(data)
				    return (unsigned char*)data;
            }
	}
	return 0;
}
//录入图像 PS_GetImage
//功能:探测手指，探测到后录入指纹图像存于ImageBuffer。 
//模块返回确认字
unsigned char PS_GetImage(void)
{
  unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(0x01);
  temp =  0x01+0x03+0x01;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//生成特征 PS_GenChar
//功能:将ImageBuffer中的原始图像生成指纹特征文件存于CharBuffer1或CharBuffer2			 
//参数:BufferID --> charBuffer1:0x01	charBuffer1:0x02												
//模块返回确认字
unsigned char PS_GenChar(unsigned char BufferID)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x04);
	Sendcmd(0x02);
	MYUSART_SendData(BufferID);
	temp = 0x01+0x04+0x02+BufferID;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//精确比对两枚指纹特征 PS_Match
//功能:精确比对CharBuffer1 与CharBuffer2 中的特征文件 
//模块返回确认字
unsigned char PS_Match(void)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(0x03);
	temp = 0x01+0x03+0x03;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//搜索指纹 PS_Search
//功能:以CharBuffer1或CharBuffer2中的特征文件搜索整个或部分指纹库.若搜索到，则返回页码。			
//参数:  BufferID @ref CharBuffer1	CharBuffer2
//说明:  模块返回确认字，页码（相配指纹模板）
unsigned char PS_Search(unsigned char BufferID,unsigned short StartPage,unsigned short PageNum,SearchResult *p)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x08);
	Sendcmd(0x04);
	MYUSART_SendData(BufferID);
	MYUSART_SendData(StartPage>>8);
	MYUSART_SendData(StartPage);
	MYUSART_SendData(PageNum>>8);
	MYUSART_SendData(PageNum);
	temp = 0x01+0x08+0x04+BufferID
	+(StartPage>>8)+(unsigned char)StartPage
	+(PageNum>>8)+(unsigned char)PageNum;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
	{
		ensure = data[9];
		p->pageID   =(data[10]<<8)+data[11];
		p->mathscore=(data[12]<<8)+data[13];	
	}
	else
		ensure = 0xff;
	return ensure;	
}
//合并特征（生成模板）PS_RegModel
//功能:将CharBuffer1与CharBuffer2中的特征文件合并生成 模板,结果存于CharBuffer1与CharBuffer2	
//说明:  模块返回确认字
unsigned char PS_RegModel(void)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(0x05);
	temp = 0x01+0x03+0x05;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;		
}
//储存模板 PS_StoreChar
//功能:将 CharBuffer1 或 CharBuffer2 中的模板文件存到 PageID 号flash数据库位置。			
//参数:  BufferID @ref charBuffer1:0x01	charBuffer1:0x02
//       PageID（指纹库位置号）
//说明:  模块返回确认字
unsigned char PS_StoreChar(unsigned char BufferID,unsigned short PageID)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x06);
	Sendcmd(0x06);
	MYUSART_SendData(BufferID);
	MYUSART_SendData(PageID>>8);
	MYUSART_SendData(PageID);
	temp = 0x01+0x06+0x06+BufferID
	+(PageID>>8)+(unsigned char)PageID;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;	
}
//删除模板 PS_DeletChar
//功能:  删除flash数据库中指定ID号开始的N个指纹模板
//参数:  PageID(指纹库模板号)，N删除的模板个数。
//说明:  模块返回确认字
unsigned char PS_DeletChar(unsigned short PageID,unsigned short N)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x07);
	Sendcmd(0x0C);
	MYUSART_SendData(PageID>>8);
	MYUSART_SendData(PageID);
	MYUSART_SendData(N>>8);
	MYUSART_SendData(N);
	temp = 0x01+0x07+0x0C
	+(PageID>>8)+(unsigned char)PageID
	+(N>>8)+(unsigned char)N;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//清空指纹库 PS_Empty
//功能:  删除flash数据库中所有指纹模板
//参数:  无
//说明:  模块返回确认字
unsigned char PS_Empty(void)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(0x0D);
	temp = 0x01+0x03+0x0D;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//写系统寄存器 PS_WriteReg
//功能:  写模块寄存器
//参数:  寄存器序号RegNum:4\5\6
//说明:  模块返回确认字
unsigned char PS_WriteReg(unsigned char RegNum,unsigned char DATA)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x05);
	Sendcmd(0x0E);
	MYUSART_SendData(RegNum);
	MYUSART_SendData(DATA);
	temp = RegNum+DATA+0x01+0x05+0x0E;
	SendCheck(temp);
	data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	if(ensure==0)
		rt_kprintf("\r\n设置参数成功！");
	else
		rt_kprintf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//读系统基本参数 PS_ReadSysPara
//功能:  读取模块的基本参数（波特率，包大小等)
//参数:  无
//说明:  模块返回确认字 + 基本参数（16bytes）
unsigned char PS_ReadSysPara(SysPara *p)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(0x0F);
	temp = 0x01+0x03+0x0F;
	SendCheck(temp);
	data=JudgeStr(1000);
	if(data)
	{
		ensure = data[9];
		p->PS_max = (data[14]<<8)+data[15];
		p->PS_level = data[17];
		p->PS_addr=(data[18]<<24)+(data[19]<<16)+(data[20]<<8)+data[21];
		p->PS_size = data[23];
		p->PS_N = data[25];
	}		
	else
		ensure=0xff;
	if(ensure==0x00)
	{
		rt_kprintf("\r\n模块最大指纹容量=%d",p->PS_max);
		rt_kprintf("\r\n对比等级=%d",p->PS_level);
		rt_kprintf("\r\n地址=%x",p->PS_addr);
		rt_kprintf("\r\n波特率=%d",p->PS_N*9600);
	}
	else 
			rt_kprintf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//设置模块地址 PS_SetAddr
//功能:  设置模块地址
//参数:  PS_addr
//说明:  模块返回确认字
unsigned char PS_SetAddr(unsigned int PS_addr)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x07);
	Sendcmd(0x15);
	MYUSART_SendData(PS_addr>>24);
	MYUSART_SendData(PS_addr>>16);
	MYUSART_SendData(PS_addr>>8);
	MYUSART_SendData(PS_addr);
	temp = 0x01+0x07+0x15
	+(unsigned char)(PS_addr>>24)+(unsigned char)(PS_addr>>16)
	+(unsigned char)(PS_addr>>8) +(unsigned char)PS_addr;
	SendCheck(temp);
	AS608Addr=PS_addr;//发送完指令，更换地址
  data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;	
		AS608Addr = PS_addr;
	if(ensure==0x00)
		rt_kprintf("\r\n设置地址成功！");
	else
		rt_kprintf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//功能： 模块内部为用户开辟了256bytes的FLASH空间用于存用户记事本,
//	该记事本逻辑上被分成 16 个页。
//参数:  NotePageNum(0~15),Byte32(要写入内容，32个字节)
//说明:  模块返回确认字
unsigned char PS_WriteNotepad(unsigned char NotePageNum,unsigned char *Byte32)
{
	unsigned short temp;
  unsigned char  ensure,i;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(36);
	Sendcmd(0x18);
	MYUSART_SendData(NotePageNum);
	for(i=0;i<32;i++)
	 {
		 MYUSART_SendData(Byte32[i]);
		 temp += Byte32[i];
	 }
  temp =0x01+36+0x18+NotePageNum+temp;
	SendCheck(temp);
  data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;
	return ensure;
}
//读记事PS_ReadNotepad
//功能：  读取FLASH用户区的128bytes数据
//参数:  NotePageNum(0~15)
//说明:  模块返回确认字+用户信息
unsigned char PS_ReadNotepad(unsigned char NotePageNum,unsigned char *Byte32)
{
	unsigned short temp;
  unsigned char  ensure,i;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x04);
	Sendcmd(0x19);
	MYUSART_SendData(NotePageNum);
	temp = 0x01+0x04+0x19+NotePageNum;
	SendCheck(temp);
  data=JudgeStr(2000);
	if(data)
	{
		ensure=data[9];
		for(i=0;i<32;i++)
		{
			Byte32[i]=data[10+i];
		}
	}
	else
		ensure=0xff;
	return ensure;
}
//高速搜索PS_HighSpeedSearch
//功能：以 CharBuffer1或CharBuffer2中的特征文件高速搜索整个或部分指纹库。
//		  若搜索到，则返回页码,该指令对于的确存在于指纹库中 ，且登录时质量
//		  很好的指纹，会很快给出搜索结果。
//参数:  BufferID， StartPage(起始页)，PageNum（页数）
//说明:  模块返回确认字+页码（相配指纹模板）
unsigned char PS_HighSpeedSearch(unsigned char BufferID,unsigned short StartPage,unsigned short PageNum,SearchResult *p)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x08);
	Sendcmd(0x1b);
	MYUSART_SendData(BufferID);
	MYUSART_SendData(StartPage>>8);
	MYUSART_SendData(StartPage);
	MYUSART_SendData(PageNum>>8);
	MYUSART_SendData(PageNum);
	temp = 0x01+0x08+0x1b+BufferID
	+(StartPage>>8)+(unsigned char)StartPage
	+(PageNum>>8)+(unsigned char)PageNum;
	SendCheck(temp);
	data=JudgeStr(2000);
 	if(data)
	{
		ensure=data[9];
		p->pageID 	=(data[10]<<8) +data[11];
		p->mathscore=(data[12]<<8) +data[13];
	}
	else
		ensure=0xff;
	return ensure;
}
//读有效模板个数 PS_ValidTempleteNum
//功能：读有效模板个数
//参数: 无
//说明: 模块返回确认字+有效模板个数ValidN
unsigned char PS_ValidTempleteNum(unsigned short *ValidN)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//命令包标识
	SendLength(0x03);
	Sendcmd(0x1d);
	temp = 0x01+0x03+0x1d;
	SendCheck(temp);
    data=JudgeStr(2000);
	if(data)
	{
		ensure=data[9];
		*ValidN = (data[10]<<8) +data[11];
	}		
	else
		ensure=0xff;
	
	if(ensure==0x00)
	{
		rt_kprintf("\r\n有效指纹个数=%d",(data[10]<<8)+data[11]);
	}
	else
		rt_kprintf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//与AS608握手 PS_HandShake
//参数: PS_Addr地址指针
//说明: 模块返新地址（正确地址）	
unsigned char PS_HandShake(unsigned int *PS_Addr)
{
    while(ls1x_uart_read(devUART4,&buff_uart4[100],1,NULL));
	SendHead();
	SendAddr();
	MYUSART_SendData(0X01);
	MYUSART_SendData(0X00);
	MYUSART_SendData(0X00);
	rt_thread_mdelay(200);
	if(ls1x_uart_read(devUART4,buff_uart4,400,NULL)){
        if(//判断是不是模块返回的应答包
		  buff_uart4[0]==0XEF
		  &&buff_uart4[1]==0X01
		  &&buff_uart4[6]==0X07
			)
	   {
	       *PS_Addr=(buff_uart4[2]<<24) + (buff_uart4[3]<<16)
		      	+(buff_uart4[4]<<8) + (buff_uart4[5]);
			 return 0;
	   }
    }
	return 1;		
}
//模块应答包确认码信息解析
//功能：解析确认码错误信息返回信息
//参数: ensure
const char *EnsureMessage(unsigned char ensure)
{
	const char *p;
	switch(ensure)
	{
		case  0x00:
			p="OK";break;		
		case  0x01:
			p="数据包接收错误";break;
		case  0x02:
			p="传感器上没有手指";break;
		case  0x03:
			p="录入指纹图像失败";break;
		case  0x04:
			p="指纹图像太干、太淡而生不成特征";break;
		case  0x05:
			p="指纹图像太湿、太糊而生不成特征";break;
		case  0x06:
			p="指纹图像太乱而生不成特征";break;
		case  0x07:
			p="指纹图像正常，但特征点太少（或面积太小）而生不成特征";break;
		case  0x08:
			p="指纹不匹配";break;
		case  0x09:
			p="没搜索到指纹";break;
		case  0x0a:
			p="特征合并失败";break;
		case  0x0b:
			p="访问指纹库时地址序号超出指纹库范围";
		case  0x10:
			p="删除模板失败";break;
		case  0x11:
			p="清空指纹库失败";break;	
		case  0x15:
			p="缓冲区内没有有效原始图而生不成图像";break;
		case  0x18:
			p="读写 FLASH 出错";break;
		case  0x19:
			p="未定义错误";break;
		case  0x1a:
			p="无效寄存器号";break;
		case  0x1b:
			p="寄存器设定内容错误";break;
		case  0x1c:
			p="记事本页码指定错误";break;
		case  0x1f:
			p="指纹库满";break;
		case  0x20:
			p="地址错误";break;
		default :
			p="模块返回确认码有误";break;
	}
 return p;	
}
//模块应答包确认码信息解析
//功能：解析确认码错误信息返回信息
//参数: ensure
const char *EnsureMessage_UTF8(unsigned char ensure)
{
	const char *p;
	switch(ensure)
	{
		case  0x00:
			p="OK";break;
		case  0x01:
			p="""\xE6\x95\xB0"/*数*/"""\xE6\x8D\xAE"/*据*/"""\xE5\x8C\x85"/*包*/"""\xE6\x8E\xA5"/*接*/"""\xE6\x94\xB6"/*收*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"";break;
		case  0x02:
			p="""\xE4\xBC\xA0"/*传*/"""\xE6\x84\x9F"/*感*/"""\xE5\x99\xA8"/*器*/"""\xE4\xB8\x8A"/*上*/"""\xE6\xB2\xA1"/*没*/"""\xE6\x9C\x89"/*有*/"""\xE6\x89\x8B"/*手*/"""\xE6\x8C\x87"/*指*/"";break;
		case  0x03:
			p="""\xE5\xBD\x95"/*录*/"""\xE5\x85\xA5"/*入*/"""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"""\xE5\x9B\xBE"/*图*/"""\xE5\x83\x8F"/*像*/"""\xE5\xA4\xB1"/*失*/"""\xE8\xB4\xA5"/*败*/"";break;
		case  0x04:
			p="""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"""\xE5\x9B\xBE"/*图*/"""\xE5\x83\x8F"/*像*/"""\xE5\xA4\xAA"/*太*/"""\xE5\xB9\xB2"/*干*/"""\xE3\x80\x81"/*、*/"""\xE5\xA4\xAA"/*太*/"""\xE6\xB7\xA1"/*淡*/"""\xE8\x80\x8C"/*而*/"""\xE7\x94\x9F"/*生*/"""\xE4\xB8\x8D"/*不*/"""\xE6\x88\x90"/*成*/"""\xE7\x89\xB9"/*特*/"""\xE5\xBE\x81"/*征*/"";break;
		case  0x05:
			p="""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"""\xE5\x9B\xBE"/*图*/"""\xE5\x83\x8F"/*像*/"""\xE5\xA4\xAA"/*太*/"""\xE6\xB9\xBF"/*湿*/"""\xE3\x80\x81"/*、*/"""\xE5\xA4\xAA"/*太*/"""\xE7\xB3\x8A"/*糊*/"""\xE8\x80\x8C"/*而*/"""\xE7\x94\x9F"/*生*/"""\xE4\xB8\x8D"/*不*/"""\xE6\x88\x90"/*成*/"""\xE7\x89\xB9"/*特*/"""\xE5\xBE\x81"/*征*/"";break;
		case  0x06:
			p="""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"""\xE5\x9B\xBE"/*图*/"""\xE5\x83\x8F"/*像*/"""\xE5\xA4\xAA"/*太*/"""\xE4\xB9\xB1"/*乱*/"""\xE8\x80\x8C"/*而*/"""\xE7\x94\x9F"/*生*/"""\xE4\xB8\x8D"/*不*/"""\xE6\x88\x90"/*成*/"""\xE7\x89\xB9"/*特*/"""\xE5\xBE\x81"/*征*/"";break;
		case  0x07:
			p="""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"""\xE5\x9B\xBE"/*图*/"""\xE5\x83\x8F"/*像*/"""\xE6\xAD\xA3"/*正*/"""\xE5\xB8\xB8"/*常*/"""\xEF\xBC\x8C"/*，*/"""\xE4\xBD\x86"/*但*/"""\xE7\x89\xB9"/*特*/"""\xE5\xBE\x81"/*征*/"""\xE7\x82\xB9"/*点*/"""\xE5\xA4\xAA"/*太*/"""\xE5\xB0\x91"/*少*/"";break;
		case  0x08:
			p="""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"""\xE4\xB8\x8D"/*不*/"""\xE5\x8C\xB9"/*匹*/"""\xE9\x85\x8D"/*配*/"";break;
		case  0x09:
			p="""\xE6\xB2\xA1"/*没*/"""\xE6\x90\x9C"/*搜*/"""\xE7\xB4\xA2"/*索*/"""\xE5\x88\xB0"/*到*/"""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"";break;
		case  0x0a:
			p="""\xE7\x89\xB9"/*特*/"""\xE5\xBE\x81"/*征*/"""\xE5\x90\x88"/*合*/"""\xE5\xB9\xB6"/*并*/"""\xE5\xA4\xB1"/*失*/"""\xE8\xB4\xA5"/*败*/"";break;
		case  0x0b:
			p="""\xE8\xAE\xBF"/*访*/"""\xE9\x97\xAE"/*问*/"""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"""\xE5\xBA\x93"/*库*/"""\xE6\x97\xB6"/*时*/"""\xE5\x9C\xB0"/*地*/"""\xE5\x9D\x80"/*址*/"""\xE5\xBA\x8F"/*序*/"""\xE5\x8F\xB7"/*号*/"""\xE8\xB6\x85"/*超*/"""\xE5\x87\xBA"/*出*/"""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"""\xE5\xBA\x93"/*库*/"""\xE8\x8C\x83"/*范*/"""\xE5\x9B\xB4"/*围*/"";
		case  0x10:
			p="""\xE5\x88\xA0"/*删*/"""\xE9\x99\xA4"/*除*/"""\xE6\xA8\xA1"/*模*/"""\xE6\x9D\xBF"/*板*/"""\xE5\xA4\xB1"/*失*/"""\xE8\xB4\xA5"/*败*/"";break;
		case  0x11:
			p="""\xE6\xB8\x85"/*清*/"""\xE7\xA9\xBA"/*空*/"""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"""\xE5\xBA\x93"/*库*/"""\xE5\xA4\xB1"/*失*/"""\xE8\xB4\xA5"/*败*/"";break;
		case  0x15:
			p="""\xE7\xBC\x93"/*缓*/"""\xE5\x86\xB2"/*冲*/"""\xE5\x8C\xBA"/*区*/"""\xE5\x86\x85"/*内*/"""\xE6\xB2\xA1"/*没*/"""\xE6\x9C\x89"/*有*/"""\xE6\x9C\x89"/*有*/"""\xE6\x95\x88"/*效*/"""\xE5\x8E\x9F"/*原*/"""\xE5\xA7\x8B"/*始*/"""\xE5\x9B\xBE"/*图*/"""\xE8\x80\x8C"/*而*/"""\xE7\x94\x9F"/*生*/"""\xE4\xB8\x8D"/*不*/"""\xE6\x88\x90"/*成*/"""\xE5\x9B\xBE"/*图*/"""\xE5\x83\x8F"/*像*/"";break;
		case  0x18:
			p="""\xE8\xAF\xBB"/*读*/"""\xE5\x86\x99"/*写*/" FLASH ""\xE5\x87\xBA"/*出*/"""\xE9\x94\x99"/*错*/"";break;
		case  0x19:
			p="""\xE6\x9C\xAA"/*未*/"""\xE5\xAE\x9A"/*定*/"""\xE4\xB9\x89"/*义*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"";break;
		case  0x1a:
			p="""\xE6\x97\xA0"/*无*/"""\xE6\x95\x88"/*效*/"""\xE5\xAF\x84"/*寄*/"""\xE5\xAD\x98"/*存*/"""\xE5\x99\xA8"/*器*/"""\xE5\x8F\xB7"/*号*/"";break;
		case  0x1b:
			p="""\xE5\xAF\x84"/*寄*/"""\xE5\xAD\x98"/*存*/"""\xE5\x99\xA8"/*器*/"""\xE8\xAE\xBE"/*设*/"""\xE5\xAE\x9A"/*定*/"""\xE5\x86\x85"/*内*/"""\xE5\xAE\xB9"/*容*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"";break;
		case  0x1c:
			p="""\xE8\xAE\xB0"/*记*/"""\xE4\xBA\x8B"/*事*/"""\xE6\x9C\xAC"/*本*/"""\xE9\xA1\xB5"/*页*/"""\xE7\xA0\x81"/*码*/"""\xE6\x8C\x87"/*指*/"""\xE5\xAE\x9A"/*定*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"";break;
		case  0x1f:
			p="""\xE6\x8C\x87"/*指*/"""\xE7\xBA\xB9"/*纹*/"""\xE5\xBA\x93"/*库*/"""\xE6\xBB\xA1"/*满*/"";break;
		case  0x20:
			p="""\xE5\x9C\xB0"/*地*/"""\xE5\x9D\x80"/*址*/"""\xE9\x94\x99"/*错*/"""\xE8\xAF\xAF"/*误*/"";break;
		default :
			p="""\xE6\xA8\xA1"/*模*/"""\xE5\x9D\x97"/*块*/"""\xE8\xBF\x94"/*返*/"""\xE5\x9B\x9E"/*回*/"""\xE7\xA1\xAE"/*确*/"""\xE8\xAE\xA4"/*认*/"""\xE7\xA0\x81"/*码*/"""\xE6\x9C\x89"/*有*/"""\xE8\xAF\xAF"/*误*/"";break;
	}
 return p;
}




