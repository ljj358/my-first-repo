#ifndef __AS608_H
#define __AS608_H
#include <stdio.h>

#define PS_Sta   PAin(6)//��ָ��ģ��״̬����
#define CharBuffer1 0x01
#define CharBuffer2 0x02

extern unsigned int AS608Addr;//ģ���ַ

typedef struct  
{
	unsigned short pageID;//ָ��ID
	unsigned short mathscore;//ƥ��÷�
}SearchResult;

typedef struct
{
	unsigned short PS_max;//ָ���������
	unsigned char  PS_level;//��ȫ�ȼ�
	unsigned int PS_addr;
	unsigned char  PS_size;//ͨѶ���ݰ���С
	unsigned char  PS_N;//�����ʻ���N
}SysPara;

void UART4_Config_Init(void);    //UART4���ڳ�ʼ��
	
unsigned char PS_GetImage(void); //¼��ͼ��
 
unsigned char PS_GenChar(unsigned char BufferID);//��������

unsigned char PS_Match(void);//��ȷ�ȶ���öָ������

unsigned char PS_Search(unsigned char BufferID,unsigned short StartPage,unsigned short PageNum,SearchResult *p);//����ָ��
 
unsigned char PS_RegModel(void);//�ϲ�����������ģ�壩
 
unsigned char PS_StoreChar(unsigned char BufferID,unsigned short PageID);//����ģ��

unsigned char PS_DeletChar(unsigned short PageID,unsigned short N);//ɾ��ģ��

unsigned char PS_Empty(void);//���ָ�ƿ�

unsigned char PS_WriteReg(unsigned char RegNum,unsigned char DATA);//дϵͳ�Ĵ���
 
unsigned char PS_ReadSysPara(SysPara *p); //��ϵͳ��������

unsigned char PS_SetAddr(unsigned int addr);  //����ģ���ַ

unsigned char PS_WriteNotepad(unsigned char NotePageNum,unsigned char *content);//д���±�

unsigned char PS_ReadNotepad(unsigned char NotePageNum,unsigned char *note);//������

unsigned char PS_HighSpeedSearch(unsigned char BufferID,unsigned short StartPage,unsigned short PageNum,SearchResult *p);//��������
  
unsigned char PS_ValidTempleteNum(unsigned short *ValidN);//����Чģ�����

unsigned char PS_HandShake(unsigned int *PS_Addr); //��AS608ģ������

const char *EnsureMessage(unsigned char ensure);//ȷ���������Ϣ����

char press_FR(int *pageID , int *mathscore );            //ˢָ��

const char *EnsureMessage_UTF8(unsigned char ensure);

char Del_FR(int num);                                    //ɾ��ָ��

extern SysPara AS608Para;//ָ��ģ��AS608����
extern unsigned short ValidN;//ģ������Чָ�Ƹ���
#endif

