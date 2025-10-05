#include "HX711.h"
#include "stdint.h"
#include "rtthread.h"


unsigned long Weight_Maopi = 0;
long  Weight_object = 0;
float GapValue = 163.67;	   //����������   �����ֲ��Գ���������ƫ��ʱ�����Ӹ���ֵ��
//������Գ���������ƫСʱ����С����ֵ����ֵ����ΪС�������� 429.5 �ȡ�

/****************************************************************
��  �ܣ�HX711�˿ڳ�ʼ��
��  ������
����ֵ����
****************************************************************/
void HX711_Init(void)
{
	
	gpio_enable(59,DIR_IN);
	gpio_enable(58,DIR_OUT);

}

/****************************************************************
��  �ܣ���ȡHX711��ֵ
��  ������
����ֵ��
****************************************************************/
unsigned long HX711_Read(void)	//����128
{
	unsigned long count; 
	uint8_t i; 
  	HX711_SCK(0); 
  	count = 0; 
  	if(HX711_DOUT){
  	    rt_thread_delay(100);
  	    if(HX711_DOUT) return 0 ;
      }
      
    /* �ر��ж� */
	rt_base_t level = rt_hw_interrupt_disable();
  	for(i=0; i<24; i++)
	{ 
	  	HX711_SCK(1); 
	  	count = count << 1; 
		HX711_SCK(0); 
	  	if(HX711_DOUT)
			count++; 
	} 
 	HX711_SCK(1); 
    count = count ^ 0x800000;  //��25�������½�����ʱ��ת������
	HX711_SCK(0);  
	  /* �ָ��ж� */
    rt_hw_interrupt_enable(level);
	return(count);
}

/****************************************************************
��  �ܣ���ȡëƤ����
��  ������
����ֵ����
****************************************************************/
void Get_Maopi(void)
{
    Weight_Maopi = 0;
    char i = 0;
    while(!Weight_Maopi){
        Weight_Maopi = HX711_Read();
        if(++i>=20) {
            return;
        }
    }
   
} 

/****************************************************************
��  �ܣ�У׼(д������û��֤��)
��  ������������
����ֵ������������
****************************************************************/
float calibration(char weight)
{
    unsigned long data = 0;
    char i = 0;
    while(!data){
        data = HX711_Read();
        if(++i>=20) {
            return 0;
        }
    }
    data -= Weight_Maopi;
    if(data>0) return data / (float)weight;
    else return 0;

}
/****************************************************************
��  �ܣ�����
��  ������
����ֵ����
****************************************************************/

long Get_Weight(void)
{

	Weight_object = HX711_Read() - Weight_Maopi;	   		// ��ȡʵ���AD������ֵ��
    
	if(Weight_object> 0 && Weight_Maopi != 0)
	{
		Weight_object= ((float)Weight_object / GapValue); 	// ����ʵ���ʵ������
	}
	else
	{
		return 0;
	}
//	rt_kprintf("Weight_object = %d\r\n",Weight_object);
    return Weight_object;
}



