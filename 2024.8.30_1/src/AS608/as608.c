#include <string.h>
#include "as608.h"
#include "ls1b.h"
#include "ns16550.h"
#include "rtthread.h"
unsigned int AS608Addr = 0XFFFFFFFF; //Ĭ��
char buff_uart4[400] = {0};
static unsigned char buff_uart4_tx[1] = {0};
 SysPara AS608Para;//ָ��ģ��AS608����
 unsigned short ValidN;//ģ������Чָ�Ƹ���
 void UART4_Config_Init(void)
{
    /**
     * @brief ���ڳ�ʼ��(��Ҫ)
     * @param 1 void
     * @return void
     */
    unsigned int BaudRate = 115200;
    ls1x_uart_init(devUART4,(void *)BaudRate); //��ʼ������
    ls1x_uart_open(devUART4,NULL); //�򿪴���
    
    unsigned char ensure;
	unsigned char key_num;

    while(PS_HandShake(&AS608Addr))//��AS608ģ������
	{
		rt_thread_mdelay(400);
		rt_kprintf("δ��⵽ģ��!!!\n");
		rt_thread_mdelay(800);
		rt_kprintf("��������ģ��...\n");
	}
	rt_kprintf("ͨѶ�ɹ�!!!\n");
    rt_thread_mdelay(10);
	ensure=PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
	if(ensure!=0x00)
		rt_kprintf("%s\n",EnsureMessage(ensure));//��ʾȷ���������Ϣ
	ensure=PS_ReadSysPara(&AS608Para);  //������
	if(ensure!=0x00)
	{
          rt_kprintf("%s\n",EnsureMessage(ensure));//��ʾȷ���������Ϣ
	}
}



//ˢָ��
char press_FR(int *pageID , int *mathscore )
{
	SearchResult seach;
	unsigned char ensure;
	char *str;
	ensure=PS_GetImage();
	if(ensure==0x00)//��ȡͼ��ɹ�
	{
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //���������ɹ�
		{
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//�����ɹ�
			{
			    *pageID = seach.pageID;
			    *mathscore = seach.mathscore;
			    return 1;
			}
	   }
	}
	return 0;
}
//ɾ��ָ��
char Del_FR(int num)
{
	unsigned char  ensure;
	unsigned esc = 0;
	ensure=PS_DeletChar(num,1);//ɾ������ָ��
	esc = ensure;
	if(esc!=0)
	   return esc;

	rt_thread_mdelay(1200);
	PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
    return esc;
}
//���ڷ���һ���ֽ�
static void MYUSART_SendData(unsigned char data)
{
    buff_uart4_tx[0] = data;
    ls1x_uart_write(devUART4,buff_uart4_tx,1,NULL);
}
//���Ͱ�ͷ
static void SendHead(void)
{
	MYUSART_SendData(0xEF);
	MYUSART_SendData(0x01);
}
//���͵�ַ
static void SendAddr(void)
{
	MYUSART_SendData(AS608Addr>>24);
	MYUSART_SendData(AS608Addr>>16);
	MYUSART_SendData(AS608Addr>>8);
	MYUSART_SendData(AS608Addr);
}
//���Ͱ���ʶ,
static void SendFlag(unsigned char flag)
{
	MYUSART_SendData(flag);
}
//���Ͱ�����
static void SendLength(int length)
{
	MYUSART_SendData(length>>8);
	MYUSART_SendData(length);
}
//����ָ����
static void Sendcmd(unsigned char cmd)
{
	MYUSART_SendData(cmd);
}
//����У���
static void SendCheck(unsigned short check)
{
	MYUSART_SendData(check>>8);
	MYUSART_SendData(check);
}
//�ж��жϽ��յ�������û��Ӧ���
//waittimeΪ�ȴ��жϽ������ݵ�ʱ�䣨��λ1ms��
//����ֵ�����ݰ��׵�ַ
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
//¼��ͼ�� PS_GetImage
//����:̽����ָ��̽�⵽��¼��ָ��ͼ�����ImageBuffer�� 
//ģ�鷵��ȷ����
unsigned char PS_GetImage(void)
{
  unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//�������� PS_GenChar
//����:��ImageBuffer�е�ԭʼͼ������ָ�������ļ�����CharBuffer1��CharBuffer2			 
//����:BufferID --> charBuffer1:0x01	charBuffer1:0x02												
//ģ�鷵��ȷ����
unsigned char PS_GenChar(unsigned char BufferID)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//��ȷ�ȶ���öָ������ PS_Match
//����:��ȷ�ȶ�CharBuffer1 ��CharBuffer2 �е������ļ� 
//ģ�鷵��ȷ����
unsigned char PS_Match(void)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//����ָ�� PS_Search
//����:��CharBuffer1��CharBuffer2�е������ļ����������򲿷�ָ�ƿ�.�����������򷵻�ҳ�롣			
//����:  BufferID @ref CharBuffer1	CharBuffer2
//˵��:  ģ�鷵��ȷ���֣�ҳ�루����ָ��ģ�壩
unsigned char PS_Search(unsigned char BufferID,unsigned short StartPage,unsigned short PageNum,SearchResult *p)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//�ϲ�����������ģ�壩PS_RegModel
//����:��CharBuffer1��CharBuffer2�е������ļ��ϲ����� ģ��,�������CharBuffer1��CharBuffer2	
//˵��:  ģ�鷵��ȷ����
unsigned char PS_RegModel(void)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//����ģ�� PS_StoreChar
//����:�� CharBuffer1 �� CharBuffer2 �е�ģ���ļ��浽 PageID ��flash���ݿ�λ�á�			
//����:  BufferID @ref charBuffer1:0x01	charBuffer1:0x02
//       PageID��ָ�ƿ�λ�úţ�
//˵��:  ģ�鷵��ȷ����
unsigned char PS_StoreChar(unsigned char BufferID,unsigned short PageID)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//ɾ��ģ�� PS_DeletChar
//����:  ɾ��flash���ݿ���ָ��ID�ſ�ʼ��N��ָ��ģ��
//����:  PageID(ָ�ƿ�ģ���)��Nɾ����ģ�������
//˵��:  ģ�鷵��ȷ����
unsigned char PS_DeletChar(unsigned short PageID,unsigned short N)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//���ָ�ƿ� PS_Empty
//����:  ɾ��flash���ݿ�������ָ��ģ��
//����:  ��
//˵��:  ģ�鷵��ȷ����
unsigned char PS_Empty(void)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//дϵͳ�Ĵ��� PS_WriteReg
//����:  дģ��Ĵ���
//����:  �Ĵ������RegNum:4\5\6
//˵��:  ģ�鷵��ȷ����
unsigned char PS_WriteReg(unsigned char RegNum,unsigned char DATA)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
		rt_kprintf("\r\n���ò����ɹ���");
	else
		rt_kprintf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//��ϵͳ�������� PS_ReadSysPara
//����:  ��ȡģ��Ļ��������������ʣ�����С��)
//����:  ��
//˵��:  ģ�鷵��ȷ���� + ����������16bytes��
unsigned char PS_ReadSysPara(SysPara *p)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
		rt_kprintf("\r\nģ�����ָ������=%d",p->PS_max);
		rt_kprintf("\r\n�Աȵȼ�=%d",p->PS_level);
		rt_kprintf("\r\n��ַ=%x",p->PS_addr);
		rt_kprintf("\r\n������=%d",p->PS_N*9600);
	}
	else 
			rt_kprintf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//����ģ���ַ PS_SetAddr
//����:  ����ģ���ַ
//����:  PS_addr
//˵��:  ģ�鷵��ȷ����
unsigned char PS_SetAddr(unsigned int PS_addr)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
	AS608Addr=PS_addr;//������ָ�������ַ
  data=JudgeStr(2000);
	if(data)
		ensure=data[9];
	else
		ensure=0xff;	
		AS608Addr = PS_addr;
	if(ensure==0x00)
		rt_kprintf("\r\n���õ�ַ�ɹ���");
	else
		rt_kprintf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//���ܣ� ģ���ڲ�Ϊ�û�������256bytes��FLASH�ռ����ڴ��û����±�,
//	�ü��±��߼��ϱ��ֳ� 16 ��ҳ��
//����:  NotePageNum(0~15),Byte32(Ҫд�����ݣ�32���ֽ�)
//˵��:  ģ�鷵��ȷ����
unsigned char PS_WriteNotepad(unsigned char NotePageNum,unsigned char *Byte32)
{
	unsigned short temp;
  unsigned char  ensure,i;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//������PS_ReadNotepad
//���ܣ�  ��ȡFLASH�û�����128bytes����
//����:  NotePageNum(0~15)
//˵��:  ģ�鷵��ȷ����+�û���Ϣ
unsigned char PS_ReadNotepad(unsigned char NotePageNum,unsigned char *Byte32)
{
	unsigned short temp;
  unsigned char  ensure,i;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//��������PS_HighSpeedSearch
//���ܣ��� CharBuffer1��CharBuffer2�е������ļ��������������򲿷�ָ�ƿ⡣
//		  �����������򷵻�ҳ��,��ָ����ڵ�ȷ������ָ�ƿ��� ���ҵ�¼ʱ����
//		  �ܺõ�ָ�ƣ���ܿ�������������
//����:  BufferID�� StartPage(��ʼҳ)��PageNum��ҳ����
//˵��:  ģ�鷵��ȷ����+ҳ�루����ָ��ģ�壩
unsigned char PS_HighSpeedSearch(unsigned char BufferID,unsigned short StartPage,unsigned short PageNum,SearchResult *p)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
//����Чģ����� PS_ValidTempleteNum
//���ܣ�����Чģ�����
//����: ��
//˵��: ģ�鷵��ȷ����+��Чģ�����ValidN
unsigned char PS_ValidTempleteNum(unsigned short *ValidN)
{
	unsigned short temp;
  unsigned char  ensure;
	unsigned char  *data;
	SendHead();
	SendAddr();
	SendFlag(0x01);//�������ʶ
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
		rt_kprintf("\r\n��Чָ�Ƹ���=%d",(data[10]<<8)+data[11]);
	}
	else
		rt_kprintf("\r\n%s",EnsureMessage(ensure));
	return ensure;
}
//��AS608���� PS_HandShake
//����: PS_Addr��ַָ��
//˵��: ģ�鷵�µ�ַ����ȷ��ַ��	
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
        if(//�ж��ǲ���ģ�鷵�ص�Ӧ���
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
//ģ��Ӧ���ȷ������Ϣ����
//���ܣ�����ȷ���������Ϣ������Ϣ
//����: ensure
const char *EnsureMessage(unsigned char ensure)
{
	const char *p;
	switch(ensure)
	{
		case  0x00:
			p="OK";break;		
		case  0x01:
			p="���ݰ����մ���";break;
		case  0x02:
			p="��������û����ָ";break;
		case  0x03:
			p="¼��ָ��ͼ��ʧ��";break;
		case  0x04:
			p="ָ��ͼ��̫�ɡ�̫��������������";break;
		case  0x05:
			p="ָ��ͼ��̫ʪ��̫��������������";break;
		case  0x06:
			p="ָ��ͼ��̫�Ҷ�����������";break;
		case  0x07:
			p="ָ��ͼ����������������̫�٣������̫С��������������";break;
		case  0x08:
			p="ָ�Ʋ�ƥ��";break;
		case  0x09:
			p="û������ָ��";break;
		case  0x0a:
			p="�����ϲ�ʧ��";break;
		case  0x0b:
			p="����ָ�ƿ�ʱ��ַ��ų���ָ�ƿⷶΧ";
		case  0x10:
			p="ɾ��ģ��ʧ��";break;
		case  0x11:
			p="���ָ�ƿ�ʧ��";break;	
		case  0x15:
			p="��������û����Чԭʼͼ��������ͼ��";break;
		case  0x18:
			p="��д FLASH ����";break;
		case  0x19:
			p="δ�������";break;
		case  0x1a:
			p="��Ч�Ĵ�����";break;
		case  0x1b:
			p="�Ĵ����趨���ݴ���";break;
		case  0x1c:
			p="���±�ҳ��ָ������";break;
		case  0x1f:
			p="ָ�ƿ���";break;
		case  0x20:
			p="��ַ����";break;
		default :
			p="ģ�鷵��ȷ��������";break;
	}
 return p;	
}
//ģ��Ӧ���ȷ������Ϣ����
//���ܣ�����ȷ���������Ϣ������Ϣ
//����: ensure
const char *EnsureMessage_UTF8(unsigned char ensure)
{
	const char *p;
	switch(ensure)
	{
		case  0x00:
			p="OK";break;
		case  0x01:
			p="""\xE6\x95\xB0"/*��*/"""\xE6\x8D\xAE"/*��*/"""\xE5\x8C\x85"/*��*/"""\xE6\x8E\xA5"/*��*/"""\xE6\x94\xB6"/*��*/"""\xE9\x94\x99"/*��*/"""\xE8\xAF\xAF"/*��*/"";break;
		case  0x02:
			p="""\xE4\xBC\xA0"/*��*/"""\xE6\x84\x9F"/*��*/"""\xE5\x99\xA8"/*��*/"""\xE4\xB8\x8A"/*��*/"""\xE6\xB2\xA1"/*û*/"""\xE6\x9C\x89"/*��*/"""\xE6\x89\x8B"/*��*/"""\xE6\x8C\x87"/*ָ*/"";break;
		case  0x03:
			p="""\xE5\xBD\x95"/*¼*/"""\xE5\x85\xA5"/*��*/"""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"""\xE5\x9B\xBE"/*ͼ*/"""\xE5\x83\x8F"/*��*/"""\xE5\xA4\xB1"/*ʧ*/"""\xE8\xB4\xA5"/*��*/"";break;
		case  0x04:
			p="""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"""\xE5\x9B\xBE"/*ͼ*/"""\xE5\x83\x8F"/*��*/"""\xE5\xA4\xAA"/*̫*/"""\xE5\xB9\xB2"/*��*/"""\xE3\x80\x81"/*��*/"""\xE5\xA4\xAA"/*̫*/"""\xE6\xB7\xA1"/*��*/"""\xE8\x80\x8C"/*��*/"""\xE7\x94\x9F"/*��*/"""\xE4\xB8\x8D"/*��*/"""\xE6\x88\x90"/*��*/"""\xE7\x89\xB9"/*��*/"""\xE5\xBE\x81"/*��*/"";break;
		case  0x05:
			p="""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"""\xE5\x9B\xBE"/*ͼ*/"""\xE5\x83\x8F"/*��*/"""\xE5\xA4\xAA"/*̫*/"""\xE6\xB9\xBF"/*ʪ*/"""\xE3\x80\x81"/*��*/"""\xE5\xA4\xAA"/*̫*/"""\xE7\xB3\x8A"/*��*/"""\xE8\x80\x8C"/*��*/"""\xE7\x94\x9F"/*��*/"""\xE4\xB8\x8D"/*��*/"""\xE6\x88\x90"/*��*/"""\xE7\x89\xB9"/*��*/"""\xE5\xBE\x81"/*��*/"";break;
		case  0x06:
			p="""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"""\xE5\x9B\xBE"/*ͼ*/"""\xE5\x83\x8F"/*��*/"""\xE5\xA4\xAA"/*̫*/"""\xE4\xB9\xB1"/*��*/"""\xE8\x80\x8C"/*��*/"""\xE7\x94\x9F"/*��*/"""\xE4\xB8\x8D"/*��*/"""\xE6\x88\x90"/*��*/"""\xE7\x89\xB9"/*��*/"""\xE5\xBE\x81"/*��*/"";break;
		case  0x07:
			p="""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"""\xE5\x9B\xBE"/*ͼ*/"""\xE5\x83\x8F"/*��*/"""\xE6\xAD\xA3"/*��*/"""\xE5\xB8\xB8"/*��*/"""\xEF\xBC\x8C"/*��*/"""\xE4\xBD\x86"/*��*/"""\xE7\x89\xB9"/*��*/"""\xE5\xBE\x81"/*��*/"""\xE7\x82\xB9"/*��*/"""\xE5\xA4\xAA"/*̫*/"""\xE5\xB0\x91"/*��*/"";break;
		case  0x08:
			p="""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"""\xE4\xB8\x8D"/*��*/"""\xE5\x8C\xB9"/*ƥ*/"""\xE9\x85\x8D"/*��*/"";break;
		case  0x09:
			p="""\xE6\xB2\xA1"/*û*/"""\xE6\x90\x9C"/*��*/"""\xE7\xB4\xA2"/*��*/"""\xE5\x88\xB0"/*��*/"""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"";break;
		case  0x0a:
			p="""\xE7\x89\xB9"/*��*/"""\xE5\xBE\x81"/*��*/"""\xE5\x90\x88"/*��*/"""\xE5\xB9\xB6"/*��*/"""\xE5\xA4\xB1"/*ʧ*/"""\xE8\xB4\xA5"/*��*/"";break;
		case  0x0b:
			p="""\xE8\xAE\xBF"/*��*/"""\xE9\x97\xAE"/*��*/"""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"""\xE5\xBA\x93"/*��*/"""\xE6\x97\xB6"/*ʱ*/"""\xE5\x9C\xB0"/*��*/"""\xE5\x9D\x80"/*ַ*/"""\xE5\xBA\x8F"/*��*/"""\xE5\x8F\xB7"/*��*/"""\xE8\xB6\x85"/*��*/"""\xE5\x87\xBA"/*��*/"""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"""\xE5\xBA\x93"/*��*/"""\xE8\x8C\x83"/*��*/"""\xE5\x9B\xB4"/*Χ*/"";
		case  0x10:
			p="""\xE5\x88\xA0"/*ɾ*/"""\xE9\x99\xA4"/*��*/"""\xE6\xA8\xA1"/*ģ*/"""\xE6\x9D\xBF"/*��*/"""\xE5\xA4\xB1"/*ʧ*/"""\xE8\xB4\xA5"/*��*/"";break;
		case  0x11:
			p="""\xE6\xB8\x85"/*��*/"""\xE7\xA9\xBA"/*��*/"""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"""\xE5\xBA\x93"/*��*/"""\xE5\xA4\xB1"/*ʧ*/"""\xE8\xB4\xA5"/*��*/"";break;
		case  0x15:
			p="""\xE7\xBC\x93"/*��*/"""\xE5\x86\xB2"/*��*/"""\xE5\x8C\xBA"/*��*/"""\xE5\x86\x85"/*��*/"""\xE6\xB2\xA1"/*û*/"""\xE6\x9C\x89"/*��*/"""\xE6\x9C\x89"/*��*/"""\xE6\x95\x88"/*Ч*/"""\xE5\x8E\x9F"/*ԭ*/"""\xE5\xA7\x8B"/*ʼ*/"""\xE5\x9B\xBE"/*ͼ*/"""\xE8\x80\x8C"/*��*/"""\xE7\x94\x9F"/*��*/"""\xE4\xB8\x8D"/*��*/"""\xE6\x88\x90"/*��*/"""\xE5\x9B\xBE"/*ͼ*/"""\xE5\x83\x8F"/*��*/"";break;
		case  0x18:
			p="""\xE8\xAF\xBB"/*��*/"""\xE5\x86\x99"/*д*/" FLASH ""\xE5\x87\xBA"/*��*/"""\xE9\x94\x99"/*��*/"";break;
		case  0x19:
			p="""\xE6\x9C\xAA"/*δ*/"""\xE5\xAE\x9A"/*��*/"""\xE4\xB9\x89"/*��*/"""\xE9\x94\x99"/*��*/"""\xE8\xAF\xAF"/*��*/"";break;
		case  0x1a:
			p="""\xE6\x97\xA0"/*��*/"""\xE6\x95\x88"/*Ч*/"""\xE5\xAF\x84"/*��*/"""\xE5\xAD\x98"/*��*/"""\xE5\x99\xA8"/*��*/"""\xE5\x8F\xB7"/*��*/"";break;
		case  0x1b:
			p="""\xE5\xAF\x84"/*��*/"""\xE5\xAD\x98"/*��*/"""\xE5\x99\xA8"/*��*/"""\xE8\xAE\xBE"/*��*/"""\xE5\xAE\x9A"/*��*/"""\xE5\x86\x85"/*��*/"""\xE5\xAE\xB9"/*��*/"""\xE9\x94\x99"/*��*/"""\xE8\xAF\xAF"/*��*/"";break;
		case  0x1c:
			p="""\xE8\xAE\xB0"/*��*/"""\xE4\xBA\x8B"/*��*/"""\xE6\x9C\xAC"/*��*/"""\xE9\xA1\xB5"/*ҳ*/"""\xE7\xA0\x81"/*��*/"""\xE6\x8C\x87"/*ָ*/"""\xE5\xAE\x9A"/*��*/"""\xE9\x94\x99"/*��*/"""\xE8\xAF\xAF"/*��*/"";break;
		case  0x1f:
			p="""\xE6\x8C\x87"/*ָ*/"""\xE7\xBA\xB9"/*��*/"""\xE5\xBA\x93"/*��*/"""\xE6\xBB\xA1"/*��*/"";break;
		case  0x20:
			p="""\xE5\x9C\xB0"/*��*/"""\xE5\x9D\x80"/*ַ*/"""\xE9\x94\x99"/*��*/"""\xE8\xAF\xAF"/*��*/"";break;
		default :
			p="""\xE6\xA8\xA1"/*ģ*/"""\xE5\x9D\x97"/*��*/"""\xE8\xBF\x94"/*��*/"""\xE5\x9B\x9E"/*��*/"""\xE7\xA1\xAE"/*ȷ*/"""\xE8\xAE\xA4"/*��*/"""\xE7\xA0\x81"/*��*/"""\xE6\x9C\x89"/*��*/"""\xE8\xAF\xAF"/*��*/"";break;
	}
 return p;
}




