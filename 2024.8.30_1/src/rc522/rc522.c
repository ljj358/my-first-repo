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
#define  MODE         1                    	 // ģʽ  1����д���   0��ֻ��
#define  DATA_LEN    16                      // ���������ֽڳ���
/*ȫ�ֱ���*/
unsigned char CT[2];//������
unsigned char SN[4]; //����
unsigned char RFID[16];			//���RFID
unsigned char KEY[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char WRITE_RFID[DATA_LEN+2]="AaBbCcDdEe123456";
unsigned char READ_RFID[DATA_LEN+2]= {0};
static rt_mutex_t RC522_mutex;
/**************************************************************
*��  �ܣ�RC522 оƬ��ʼ��
*��  ��: ��
*����ֵ: ��
**************************************************************/
void InitRc522(void)
{
    RC522_mutex = rt_mutex_create("RC522_mutex", RT_IPC_FLAG_PRIO);
    if (RC522_mutex == RT_NULL)
    {
        rt_kprintf("RC522_mutex����������ʧ��\r\n");
        return ;
    }
    
    IIC_Init();
    PcdReset();     //��λ
    PcdAntennaOff();    //�ر����� ����Ĵ���
    rt_thread_delay(2);
    PcdAntennaOn();     //�������ߣ�ÿ��������ر����շ���֮��Ӧ������1ms�ļ����RC632�Ĵ���
    M500PcdConfigISOType( 'A' );      //ISO14443_Aģʽ
}

void Reset_RC522(void)
{
    PcdReset();
    PcdAntennaOff();
    rt_thread_delay(2);
    PcdAntennaOn();
}
/**************************************************************
*��  �ܣ�Ѱ��
*��  ��: req_code[IN]:Ѱ����ʽ
*		 0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
*		 0x26 = Ѱδ��������״̬�Ŀ�
*		 pTagType[OUT]����Ƭ���ʹ���
*		 0x4400 = Mifare_UltraLight
*		 0x0400 = Mifare_One(S50)
*		 0x0200 = Mifare_One(S70)
*		 0x0800 = Mifare_Pro(X)
*		 0x4403 = Mifare_DESFire
*����ֵ: �ɹ�����MI_OK
**************************************************************/
char PcdRequest(unsigned char   req_code,unsigned char *pTagType)
{
    char   status;
    unsigned char   unLen;
    unsigned char   ucComMF522Buf[MAXRLEN];

    PcdAntennaOff();    //�ر����� ����Ĵ���
    rt_thread_delay(2);
    PcdAntennaOn();     //�������ߣ�ÿ��������ر����շ���֮��Ӧ������1ms�ļ����RC632�Ĵ���
    
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
*��  �ܣ�����ײ
*��  ��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ�ѡ����Ƭ
*��  ��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ���֤��Ƭ����
*��  ��: auth_mode[IN]: ������֤ģʽ 0x60 = ��֤A��Կ 0x61 = ��֤B��Կ
*		 addr[IN]�����ַ   pKey[IN]������  pSnr[IN]����Ƭ���кţ�4�ֽ�
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ���ȡM1��һ������
*��  ��: addr[IN]�����ַ pData[OUT]�����������ݣ�16�ֽ�
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ�д���ݵ�M1��һ��
*��  ��: addr[IN]�����ַ  pData[IN]��д������ݣ�16�ֽ�
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ����Ƭ��������״̬
*��  ��: ��
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ���MF522����CRC16����
*��  ��: ��
*����ֵ: ��
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
*��  �ܣ���λRC522
*��  ��: ��
*����ֵ: �ɹ�����MI_OK
**************************************************************/
char PcdReset(void)
{
    
    delay_us(10);
    WriteRawRC(CommandReg,PCD_RESETPHASE); //����
    WriteRawRC(CommandReg,PCD_RESETPHASE); //����
    delay_us(10);

    WriteRawRC(ModeReg,0x3D);            //��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TReloadRegL,30);          //��ʱ����8λ //48
    WriteRawRC(TReloadRegH,0);           //��ʱ����8λ   //9.5ms
    WriteRawRC(TModeReg,0x8D);           //��ʱ��ʱ��Ԥ��Ƶ��4λ
    WriteRawRC(TPrescalerReg,0x3E);      //��ʱ��ʱ��Ԥ��Ƶ��8λ  //1342

    WriteRawRC(TxAutoReg,0x40);//����Ҫ       //�ֲ�����û��

    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//����RC632�Ĺ�����ʽ
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
*��  �ܣ���RC522�Ĵ���λ
*��  ��: reg[IN]:�Ĵ�����ַ  mask[IN]:��λֵ
*����ֵ: ��
**************************************************************/
void SetBitMask(unsigned char   reg,unsigned char   mask)
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/**************************************************************
*��  �ܣ���RC522�Ĵ���λ
*��  ��: reg[IN]:�Ĵ�����ַ  mask[IN]:��λֵ
*����ֵ: ��
**************************************************************/
void ClearBitMask(unsigned char   reg,unsigned char   mask)
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

/**************************************************************
*��  �ܣ�ͨ��RC522��ISO14443��ͨѶ
*��  ��: Command[IN]:RC522������
*		 pInData[IN]:ͨ��RC522���͵���Ƭ������
*		 InLenByte[IN]:�������ݵ��ֽڳ���
*		 pOutData[OUT]:���յ��Ŀ�Ƭ��������
*		 *pOutLenBit[OUT]:�������ݵ�λ����
*����ֵ: ��
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
    ClearBitMask(ComIrqReg,0x80);	//�������ж�λ
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);	 	//��FIFO����

    for (i=0; i<InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pIn [i]);
    }
    WriteRawRC(CommandReg, Command);
//   	 n = ReadRawRC(CommandReg);

    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg,0x80);     //��ʼ����
    }

    //i = 600;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
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
*��  �ܣ��������ߣ�ÿ��������ر����շ���֮��Ӧ������1ms�ļ����
*��  ��: ��
*����ֵ: ��
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
*��  �ܣ��ر�����
*��  ��: ��
*����ֵ: ��
**************************************************************/
void PcdAntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);  //��Ĵ���
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//��    �أ�������ֵ
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char   Address)
{
    unsigned char ucResult=0;
    ucResult = RC522_RD_Reg(SLA_ADDR,  Address);
    return ucResult;          		//�����յ�������
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�дRC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//          value[IN]:д���ֵ
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char   Address, unsigned char   value)
{
    RC522_WR_Reg(SLA_ADDR,  Address, value);
}



/**************************************************************
*��  �ܣ����Ĵ���
*��  ��: addr:�Ĵ�����ַ
*����ֵ: ������ֵ
**************************************************************/
unsigned char RC522_RD_Reg(unsigned char RCsla,unsigned char addr)
{
    /* �ر��ж� */
	rt_base_t level = rt_hw_interrupt_disable();
    unsigned char temp=0;
    IIC_Start();
    IIC_Send_Byte(RCsla);	//����д����ָ��
    temp=IIC_Wait_Ack();
    IIC_Send_Byte(addr);   		//���ͼĴ�����ַ
    temp=IIC_Wait_Ack();
    IIC_Start();  	 	   		//��������
    IIC_Send_Byte(RCsla+1);	//���Ͷ�����ָ��
    temp=IIC_Wait_Ack();
    temp=IIC_Read_Byte(0);		//��ȡһ���ֽ�,�������ٶ�,����NAK
    IIC_Stop();					//����һ��ֹͣ����
     /* �ָ��ж� */
    rt_hw_interrupt_enable(level);
    return temp;				//���ض�����ֵ
}



//д�Ĵ���
//addr:�Ĵ�����ַ
//val:Ҫд���ֵ
//����ֵ:��
void RC522_WR_Reg(unsigned char RCsla,unsigned char addr,unsigned char val)
{
    /* �ر��ж� */
	rt_base_t level = rt_hw_interrupt_disable();
    IIC_Start();
    IIC_Send_Byte(RCsla);     	//����д����ָ��
    IIC_Wait_Ack();
    IIC_Send_Byte(addr);   			//���ͼĴ�����ַ
    IIC_Wait_Ack();
    IIC_Send_Byte(val);     		//����ֵ
    IIC_Wait_Ack();
    IIC_Stop();						//����һ��ֹͣ����
    /* �ָ��ж� */
    rt_hw_interrupt_enable(level);
}


//�ȴ����뿪
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
* ����:����
* ����1: �洢��ȡ���ݵĵ�ַ
* ����2: ������ַ
* ����4: ���ַ
* ����4: ��Կ
* ����5: ��֤��ԿA������ԿB,1 = A,2 = B
* ����: MI_ERRʧ�ܣ�MI_OK�ɹ�
*/
char Read_RFID(char *data,char sector,char block,char * secret_key,char secret_keyA_OR_secret_keyB)
{
    rt_mutex_take(RC522_mutex, RT_WAITING_FOREVER);
    unsigned char s = (sector*4)+block;
    unsigned char card_key = (s / 4) * 4 + 3;  //��Կ��ַ
    PcdAntennaOff();    //�ر����� ����Ĵ���
    rt_thread_delay(2);
    PcdAntennaOn();     //�������ߣ�ÿ��������ر����շ���֮��Ӧ������1ms�ļ����RC632�Ĵ���
    rt_thread_delay(2);
     if(PcdRequest(PICC_REQALL,CT)==MI_OK)/*Ѱ��*/
    {
       // rt_kprintf("1\r\n");
        if (PcdAnticoll(SN)==MI_OK)//����ײ�ɹ�
        {   
          // rt_kprintf("2\r\n");
            if(PcdSelect(SN) == MI_OK) //ѡ���ɹ�
            {   //rt_kprintf("3\r\n");
                 if(PcdAuthState(secret_keyA_OR_secret_keyB?0x60:0X61,card_key,secret_key,SN) == MI_OK) //��֤��Ƭ����
                 {
                    // rt_kprintf("4\r\n");
                     if(PcdRead(s,data) == MI_OK);	 //������
                     { //rt_kprintf("5\r\n");
                        rt_mutex_release(RC522_mutex);
                        return MI_OK; //�ɹ�
                     }
                 }
            }
        }
        
    }
    rt_mutex_release(RC522_mutex);
    return MI_ERR;  //ʧ��
}
/*
* ����:д��
* ����1: Ҫд�����ݵĵ�ַ
* ����2: ������ַ
* ����4: ���ַ
* ����4: ��Կ
* ����5: ��֤��ԿA������ԿB
* ����: MI_ERRʧ�ܣ�MI_OK�ɹ�
*/
char Write_RFID(char *data,char sector,char block,char * secret_key,char secret_keyA_OR_secret_keyB)
{
    rt_mutex_take(RC522_mutex, RT_WAITING_FOREVER);
    unsigned char s = (sector*4)+block;
    unsigned char card_key = (s / 4) * 4 + 3;  //��Կ��ַ
    PcdAntennaOff();    //�ر����� ����Ĵ���
    rt_thread_delay(2);
    PcdAntennaOn();     //�������ߣ�ÿ��������ر����շ���֮��Ӧ������1ms�ļ����RC632�Ĵ���
    rt_thread_delay(2);
     if(PcdRequest(PICC_REQALL,CT)==MI_OK)/*Ѱ��*/
    {
        if (PcdAnticoll(SN)==MI_OK)//����ײ�ɹ�
        {
            if(PcdSelect(SN) == MI_OK) //ѡ���ɹ�
            {
                 if(PcdAuthState(secret_keyA_OR_secret_keyB?0x60:0X61,card_key,secret_key,SN) == MI_OK) //��֤��Ƭ����
                 {
                     if(PcdWrite(s,data) == MI_OK);	 //д������
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
    status = PcdRequest(PICC_REQALL,CT);/*����*/
    if(status==MI_OK)//�����ɹ�
    {
        rt_thread_delay(200);
        status=MI_ERR;
        status = PcdAnticoll(SN);/*����ײ*/
    }
    else{
               static i = 0;
               if(i++==2){
                   i=0;
                   fb_fillrect(150, 220, 700, 236, cidxBLACK);
               }

        }
   
    if (status==MI_OK)//���nײ�ɹ�
    {
        status=MI_ERR;
        status =PcdSelect(SN);

    }
    if(status==MI_OK)//�x���ɹ�
    {
        status=MI_ERR;
        status =PcdAuthState(0x60,card_key,KEY,SN);
    }
    //if(MODE)	// �ж��Ƿ�д��
    //{
        //if(status==MI_OK)		//��֤�ɹ�
        //{
            //status=MI_ERR;
            //status=PcdWrite(s,WRITE_RFID);	 //д������
			//fb_textout(150, 190, WRITE_RFID);
			
        //}
    //}
    if(status==MI_OK)		//��֤�ɹ�
    {
             status=PcdRead(s,READ_RFID);				//����
              if(status==MI_OK)//�x���ɹ�
              {
                 fb_fillrect(150, 220, 700, 236, cidxBLACK);
                 fb_textout(150, 220, READ_RFID);
                //WaitCardOff( );
                //status=MI_ERR;
                //rt_thread_delay(100);
              }
               
        
    }
}

