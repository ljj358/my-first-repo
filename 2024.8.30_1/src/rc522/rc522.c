/*
 * rc522.c
 *
 * created: 2022/3/2
 *  author: 
 */
#include "ls1b.h"
#include "ls1b_gpio.h"
#include "tick.h"
#include "bsp.h"
#include "rc522_iic.h"
#include "rtthread.h"
#include "rc522.h"
#include "ls1x_fb.h"
#define  MODE         1                    	 // 模式  1：先写后读   0：只读
#define  DATA_LEN    16                      // 定义数据字节长度
/*全局变量*/
unsigned char CT[2];//卡类型
unsigned char SN[4]; //卡号
unsigned char RFID[16];			//存放RFID
unsigned char KEY[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char WRITE_RFID[DATA_LEN+2]="AaBbCcDdEe123456";
unsigned char READ_RFID[DATA_LEN+2]= {0};
static rt_mutex_t RC522_mutex;
/**************************************************************
*功  能：RC522 芯片初始化
*参  数: 无
*返回值: 无
**************************************************************/
void InitRc522(void)
{
    RC522_mutex = rt_mutex_create("RC522_mutex", RT_IPC_FLAG_PRIO);
    if (RC522_mutex == RT_NULL)
    {
        rt_kprintf("RC522_mutex互斥量创建失败\r\n");
        return ;
    }
    
    IIC_Init();
    PcdReset();     //复位
    PcdAntennaOff();    //关闭天线 清除寄存器
    rt_thread_delay(2);
    PcdAntennaOn();     //开启天线（每次启动或关闭天险发射之间应至少有1ms的间隔）RC632寄存器
    M500PcdConfigISOType( 'A' );      //ISO14443_A模式
}

void Reset_RC522(void)
{
    PcdReset();
    PcdAntennaOff();
    rt_thread_delay(2);
    PcdAntennaOn();
}
/**************************************************************
*功  能：寻卡
*参  数: req_code[IN]:寻卡方式
*		 0x52 = 寻感应区内所有符合14443A标准的卡
*		 0x26 = 寻未进入休眠状态的卡
*		 pTagType[OUT]：卡片类型代码
*		 0x4400 = Mifare_UltraLight
*		 0x0400 = Mifare_One(S50)
*		 0x0200 = Mifare_One(S70)
*		 0x0800 = Mifare_Pro(X)
*		 0x4403 = Mifare_DESFire
*返回值: 成功返回MI_OK
**************************************************************/
char PcdRequest(unsigned char   req_code,unsigned char *pTagType)
{
    char   status;
    unsigned char   unLen;
    unsigned char   ucComMF522Buf[MAXRLEN];

    PcdAntennaOff();    //关闭天线 清除寄存器
    rt_thread_delay(2);
    PcdAntennaOn();     //开启天线（每次启动或关闭天险发射之间应至少有1ms的间隔）RC632寄存器
    
    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x07);
    SetBitMask(TxControlReg,0x03);

    ucComMF522Buf[0] = req_code;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType     = ucComMF522Buf[0];
        *(pTagType+1) = ucComMF522Buf[1];
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**************************************************************
*功  能：防冲撞
*参  数: pSnr[OUT]:卡片序列号，4字节
*返回值: 成功返回MI_OK
**************************************************************/
char PcdAnticoll(unsigned char *pSnr)
{
    char   status;
    unsigned char   i,snr_check=0;
    unsigned char   unLen;
    unsigned char   ucComMF522Buf[MAXRLEN];


    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
        for (i=0; i<4; i++)
        {
            *(pSnr+i)  = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg,0x80);
    return status;
}

/**************************************************************
*功  能：选定卡片
*参  数: pSnr[IN]:卡片序列号，4字节
*返回值: 成功返回MI_OK
**************************************************************/
char PcdSelect(unsigned char *pSnr)
{
    char   status;
    unsigned char   i;
    unsigned char   unLen;
    unsigned char   ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
        ucComMF522Buf[i+2] = *(pSnr+i);
        ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);

    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**************************************************************
*功  能：验证卡片密码
*参  数: auth_mode[IN]: 密码验证模式 0x60 = 验证A密钥 0x61 = 验证B密钥
*		 addr[IN]：块地址   pKey[IN]：密码  pSnr[IN]：卡片序列号，4字节
*返回值: 成功返回MI_OK
**************************************************************/
char PcdAuthState(unsigned char   auth_mode,unsigned char   addr,unsigned char *pKey,unsigned char *pSnr)
{
    char   status;
    unsigned char   unLen;
    unsigned char   ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    memcpy(&ucComMF522Buf[2], pKey, 6);
    memcpy(&ucComMF522Buf[8], pSnr, 4);

    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}

/**************************************************************
*功  能：读取M1卡一块数据
*参  数: addr[IN]：块地址 pData[OUT]：读出的数据，16字节
*返回值: 成功返回MI_OK
**************************************************************/
char PcdRead(unsigned char   addr,unsigned char *p )
{
    char   status;
    unsigned char   unLen;
    unsigned char   i,ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        for (i=0; i<16; i++)
        {
            *(p +i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**************************************************************
*功  能：写数据到M1卡一块
*参  数: addr[IN]：块地址  pData[IN]：写入的数据，16字节
*返回值: 成功返回MI_OK
**************************************************************/
char PcdWrite(unsigned char   addr,unsigned char *p )
{
    char   status;
    unsigned char   unLen;
    unsigned char   i,ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, p , 16);
        for (i=0; i<16; i++)
        {
            ucComMF522Buf[i] = *(p +i);
        }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

/**************************************************************
*功  能：命令卡片进入休眠状态
*参  数: 无
*返回值: 成功返回MI_OK
**************************************************************/
char PcdHalt(void)
{
    unsigned char   status;
    unsigned char   unLen;
    unsigned char   ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return status;
}

/**************************************************************
*功  能：用MF522计算CRC16函数
*参  数: 无
*返回值: 无
**************************************************************/
void CalulateCRC(unsigned char *pIn,unsigned char   len,unsigned char *pOut )
{
    unsigned char   i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIn +i));
    }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOut [0] = ReadRawRC(CRCResultRegL);
    pOut [1] = ReadRawRC(CRCResultRegM);
}

/**************************************************************
*功  能：复位RC522
*参  数: 无
*返回值: 成功返回MI_OK
**************************************************************/
char PcdReset(void)
{
    
    delay_us(10);
    WriteRawRC(CommandReg,PCD_RESETPHASE); //掉电
    WriteRawRC(CommandReg,PCD_RESETPHASE); //掉电
    delay_us(10);

    WriteRawRC(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL,30);          //定时器低8位 //48
    WriteRawRC(TReloadRegH,0);           //定时器高8位   //9.5ms
    WriteRawRC(TModeReg,0x8D);           //定时器时钟预分频高4位
    WriteRawRC(TPrescalerReg,0x3E);      //定时器时钟预分频高8位  //1342

    WriteRawRC(TxAutoReg,0x40);//必须要       //手册里面没有

    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//设置RC632的工作方式
//////////////////////////////////////////////////////////////////////
char M500PcdConfigISOType(unsigned char   type)
{
    if (type == 'A')                     //ISO14443_A
    {
        ClearBitMask(Status2Reg,0x08);
        WriteRawRC(ModeReg,0x3D);//3F
        WriteRawRC(RxSelReg,0x86);//84
        WriteRawRC(RFCfgReg,0x7F);   //4F
        WriteRawRC(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
        WriteRawRC(TReloadRegH,0);
        WriteRawRC(TModeReg,0x8D);
        WriteRawRC(TPrescalerReg,0x3E);
        delay_us(1000);
        PcdAntennaOn();
    }
    else {
        return 1;
    }

    return MI_OK;
}

/**************************************************************
*功  能：置RC522寄存器位
*参  数: reg[IN]:寄存器地址  mask[IN]:置位值
*返回值: 无
**************************************************************/
void SetBitMask(unsigned char   reg,unsigned char   mask)
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/**************************************************************
*功  能：清RC522寄存器位
*参  数: reg[IN]:寄存器地址  mask[IN]:清位值
*返回值: 无
**************************************************************/
void ClearBitMask(unsigned char   reg,unsigned char   mask)
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

/**************************************************************
*功  能：通过RC522和ISO14443卡通讯
*参  数: Command[IN]:RC522命令字
*		 pInData[IN]:通过RC522发送到卡片的数据
*		 InLenByte[IN]:发送数据的字节长度
*		 pOutData[OUT]:接收到的卡片返回数据
*		 *pOutLenBit[OUT]:返回数据的位长度
*返回值: 无
**************************************************************/
char PcdComMF522(unsigned char   Command,
                 unsigned char *pIn,
                 unsigned char   InLenByte,
                 unsigned char *pOut,
                 unsigned char *pOutLenBit)
{
    char   status = MI_ERR;
    unsigned char   irqEn   = 0x00;
    unsigned char   waitFor = 0x00;
    unsigned char   lastBits;
    unsigned char   n;
    unsigned int   i;
    switch (Command)
    {
    case PCD_AUTHENT:
        irqEn   = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn   = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);	//清所有中断位
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);	 	//清FIFO缓存

    for (i=0; i<InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pIn [i]);
    }
    WriteRawRC(CommandReg, Command);
//   	 n = ReadRawRC(CommandReg);

    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg,0x80);     //开始传送
    }

    //i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
    i = 2000;
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);

    if (i!=0)
    {
        if(!(ReadRawRC(ErrorReg)&0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n-1)*8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n*8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }
                for (i=0; i<n; i++)
                {
                    pOut [i] = ReadRawRC(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }

    }


    SetBitMask(ControlReg,0x80);           // stop timer now
    WriteRawRC(CommandReg,PCD_IDLE);
    return status;
}

/**************************************************************
*功  能：开启天线（每次启动或关闭天险发射之间应至少有1ms的间隔）
*参  数: 无
*返回值: 无
**************************************************************/
void PcdAntennaOn(void)
{
    unsigned char   i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

/**************************************************************
*功  能：关闭天线
*参  数: 无
*返回值: 无
**************************************************************/
void PcdAntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);  //清寄存器
}

/////////////////////////////////////////////////////////////////////
//功    能：读RC632寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char   Address)
{
    unsigned char ucResult=0;
    ucResult = RC522_RD_Reg(SLA_ADDR,  Address);
    return ucResult;          		//返回收到的数据
}

/////////////////////////////////////////////////////////////////////
//功    能：写RC632寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char   Address, unsigned char   value)
{
    RC522_WR_Reg(SLA_ADDR,  Address, value);
}



/**************************************************************
*功  能：读寄存器
*参  数: addr:寄存器地址
*返回值: 读到的值
**************************************************************/
unsigned char RC522_RD_Reg(unsigned char RCsla,unsigned char addr)
{
    /* 关闭中断 */
	rt_base_t level = rt_hw_interrupt_disable();
    unsigned char temp=0;
    IIC_Start();
    IIC_Send_Byte(RCsla);	//发送写器件指令
    temp=IIC_Wait_Ack();
    IIC_Send_Byte(addr);   		//发送寄存器地址
    temp=IIC_Wait_Ack();
    IIC_Start();  	 	   		//重新启动
    IIC_Send_Byte(RCsla+1);	//发送读器件指令
    temp=IIC_Wait_Ack();
    temp=IIC_Read_Byte(0);		//读取一个字节,不继续再读,发送NAK
    IIC_Stop();					//产生一个停止条件
     /* 恢复中断 */
    rt_hw_interrupt_enable(level);
    return temp;				//返回读到的值
}



//写寄存器
//addr:寄存器地址
//val:要写入的值
//返回值:无
void RC522_WR_Reg(unsigned char RCsla,unsigned char addr,unsigned char val)
{
    /* 关闭中断 */
	rt_base_t level = rt_hw_interrupt_disable();
    IIC_Start();
    IIC_Send_Byte(RCsla);     	//发送写器件指令
    IIC_Wait_Ack();
    IIC_Send_Byte(addr);   			//发送寄存器地址
    IIC_Wait_Ack();
    IIC_Send_Byte(val);     		//发送值
    IIC_Wait_Ack();
    IIC_Stop();						//产生一个停止条件
    /* 恢复中断 */
    rt_hw_interrupt_enable(level);
}


//等待卡离开
void WaitCardOff(void)
{
     rt_mutex_take(RC522_mutex, RT_WAITING_FOREVER);
    unsigned char status, TagType[2];

    while(1)
    {
        status = PcdRequest(REQ_ALL, TagType);
        if(status)
        {
            status = PcdRequest(REQ_ALL, TagType);
            if(status)
            {
                status = PcdRequest(REQ_ALL, TagType);
                if(status)
                {
                    rt_mutex_release(RC522_mutex);
                    return;
                }
            }
        }
        rt_thread_delay(10);
    }
}
/*
* 功能:读卡
* 参数1: 存储读取数据的地址
* 参数2: 扇区地址
* 参数4: 块地址
* 参数4: 密钥
* 参数5: 验证密钥A还是密钥B,1 = A,2 = B
* 返回: MI_ERR失败，MI_OK成功
*/
char Read_RFID(char *data,char sector,char block,char * secret_key,char secret_keyA_OR_secret_keyB)
{
    rt_mutex_take(RC522_mutex, RT_WAITING_FOREVER);
    unsigned char s = (sector*4)+block;
    unsigned char card_key = (s / 4) * 4 + 3;  //密钥地址
    PcdAntennaOff();    //关闭天线 清除寄存器
    rt_thread_delay(2);
    PcdAntennaOn();     //开启天线（每次启动或关闭天险发射之间应至少有1ms的间隔）RC632寄存器
    rt_thread_delay(2);
     if(PcdRequest(PICC_REQALL,CT)==MI_OK)/*寻卡*/
    {
       // rt_kprintf("1\r\n");
        if (PcdAnticoll(SN)==MI_OK)//防冲撞成功
        {   
          // rt_kprintf("2\r\n");
            if(PcdSelect(SN) == MI_OK) //选卡成功
            {   //rt_kprintf("3\r\n");
                 if(PcdAuthState(secret_keyA_OR_secret_keyB?0x60:0X61,card_key,secret_key,SN) == MI_OK) //验证卡片密码
                 {
                    // rt_kprintf("4\r\n");
                     if(PcdRead(s,data) == MI_OK);	 //读数据
                     { //rt_kprintf("5\r\n");
                        rt_mutex_release(RC522_mutex);
                        return MI_OK; //成功
                     }
                 }
            }
        }
        
    }
    rt_mutex_release(RC522_mutex);
    return MI_ERR;  //失败
}
/*
* 功能:写卡
* 参数1: 要写入数据的地址
* 参数2: 扇区地址
* 参数4: 块地址
* 参数4: 密钥
* 参数5: 验证密钥A还是密钥B
* 返回: MI_ERR失败，MI_OK成功
*/
char Write_RFID(char *data,char sector,char block,char * secret_key,char secret_keyA_OR_secret_keyB)
{
    rt_mutex_take(RC522_mutex, RT_WAITING_FOREVER);
    unsigned char s = (sector*4)+block;
    unsigned char card_key = (s / 4) * 4 + 3;  //密钥地址
    PcdAntennaOff();    //关闭天线 清除寄存器
    rt_thread_delay(2);
    PcdAntennaOn();     //开启天线（每次启动或关闭天险发射之间应至少有1ms的间隔）RC632寄存器
    rt_thread_delay(2);
     if(PcdRequest(PICC_REQALL,CT)==MI_OK)/*寻卡*/
    {
        if (PcdAnticoll(SN)==MI_OK)//防冲撞成功
        {
            if(PcdSelect(SN) == MI_OK) //选卡成功
            {
                 if(PcdAuthState(secret_keyA_OR_secret_keyB?0x60:0X61,card_key,secret_key,SN) == MI_OK) //验证卡片密码
                 {
                     if(PcdWrite(s,data) == MI_OK);	 //写入数据
                     {
                         rt_mutex_release(RC522_mutex);
                        return MI_OK;
                     }
                 }
            }
        }

    }
    rt_mutex_release(RC522_mutex);
    return MI_ERR;
}
void Read_Card(void)
{
    unsigned char status;
    unsigned char s=0x14;
    unsigned char card_key = (s / 4) * 4 + 3;
    status = PcdRequest(PICC_REQALL,CT);/*尋卡*/
    if(status==MI_OK)//尋卡成功
    {
        rt_thread_delay(200);
        status=MI_ERR;
        status = PcdAnticoll(SN);/*防冲撞*/
    }
    else{
               static i = 0;
               if(i++==2){
                   i=0;
                   fb_fillrect(150, 220, 700, 236, cidxBLACK);
               }

        }
   
    if (status==MI_OK)//防衝撞成功
    {
        status=MI_ERR;
        status =PcdSelect(SN);

    }
    if(status==MI_OK)//選卡成功
    {
        status=MI_ERR;
        status =PcdAuthState(0x60,card_key,KEY,SN);
    }
    //if(MODE)	// 判读是否写入
    //{
        //if(status==MI_OK)		//验证成功
        //{
            //status=MI_ERR;
            //status=PcdWrite(s,WRITE_RFID);	 //写入数据
			//fb_textout(150, 190, WRITE_RFID);
			
        //}
    //}
    if(status==MI_OK)		//验证成功
    {
             status=PcdRead(s,READ_RFID);				//读卡
              if(status==MI_OK)//讀卡成功
              {
                 fb_fillrect(150, 220, 700, 236, cidxBLACK);
                 fb_textout(150, 220, READ_RFID);
                //WaitCardOff( );
                //status=MI_ERR;
                //rt_thread_delay(100);
              }
               
        
    }
}

