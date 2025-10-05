#include "paj7620u2.h"
#include "paj7620u2_cfg.h"
#include "rtthread.h"

static void paj7620u2_selectBank(bank_e bank);//ѡ��PAJ7620U2 BANK����
static unsigned char  paj7620u2_wakeup(void);//PAJ7620U2����


//PAJ7620U2��ʼ��
//����ֵ��0:ʧ�� 1:�ɹ�
unsigned char  paj7620u2_init()
{

    unsigned int i;
    
    unsigned char status;
    GS_i2c_init();//������I2C��ʼ��
	status = paj7620u2_wakeup();		//����PAJ7620U2
	if(!status){

        return 0;
    }
	paj7620u2_selectBank(BANK0);	//����BANK0�Ĵ�������
	for(i=0;i<INIT_SIZE;i++)						//��ʼ��ģ��
	{
		GS_Write_Byte(init_Array[i][0], init_Array[i][1]);//��ʼ��PAJ7620U2
	}
    paj7620u2_selectBank(BANK0);//�л���BANK0�Ĵ�������
	return 1;
}


void Gesture_Init(void)
{
	unsigned char  i;
	paj7620u2_selectBank(BANK0);//����BANK0�Ĵ�������
	for(i=0;i<GESTURE_SIZE;i++)
	{
		GS_Write_Byte(gesture_arry[i][0],gesture_arry[i][1]);//����ʶ��ģʽ��ʼ��
	}
	paj7620u2_selectBank(BANK0);//�л���BANK0�Ĵ�������
	
	//gesture = (GestureData *)malloc(sizeof(GestureData));
	//if(NULL == gesture){
		
		//rt_kprintf("Error: struct \"GESTURE_DATA\" malloc failed\r\n");
	//}
	//memset(gesture, 0, sizeof(GestureData));

}

//ѡ��PAJ7620U2 BANK����
void paj7620u2_selectBank(bank_e bank)
{
	switch(bank)
	{
		case BANK0: GS_Write_Byte(PAJ_REGITER_BANK_SEL,PAJ_BANK0);break;//BANK0�Ĵ�������
		case BANK1: GS_Write_Byte(PAJ_REGITER_BANK_SEL,PAJ_BANK1);break;//BANK1�Ĵ�������
	}		
}

//PAJ7620U2����
unsigned char  paj7620u2_wakeup()
{ 
	unsigned char  data=0x0a;
	GS_WakeUp();//����PAJ7620U2
	rt_thread_mdelay(5);//����ʱ��>400us
	GS_WakeUp();//����PAJ7620U2
	rt_thread_mdelay(5);//����ʱ��>400us
	paj7620u2_selectBank(BANK0);//����BANK0�Ĵ�������

	data = GS_Read_Byte(0x00);//��ȡ״̬
	if(data!=0x20)
	{
	    return 0; //����ʧ��
    }
	return 1;
}

void  PAJ7620_Init(void)
{
	while(!paj7620u2_init())//PAJ7620U2��������ʼ��
	{
		rt_kprintf("PAJ7620U2_B Error!!!\r\n");
		rt_thread_mdelay(500);
	}
	rt_thread_mdelay(1000);
	Gesture_Init();
    rt_kprintf("PAJ7620U2 OK\r\n");
}
