/*
 * DS1302.c
 *
 * created: 2024/5/24
 *  author: 
 */
#include "ls1b_gpio.h"
#include "rtthread.h"

#define DS1302_SCLK(IN) gpio_write(40,IN)
#define DS1302_IO(IN)   gpio_write(41,IN)
#define DS1302_CE(IN)   gpio_write(44,IN)


#define red_DS1302_IO gpio_read(41)

//��Ӧ�Ĵ���λ�ö���
#define  DS1302_SECOND		0X80
#define  DS1302_MINUTE  	0X82
#define  DS1302_HOUR		0X84
#define  DS1302_DATE        0X86
#define  DS1302_MONTH		0X88
#define  DS1302_DAY         0X8A
#define  DS1302_YEAR        0X8C
#define  DS1302_WP          0X8E

//�������飬���ڴ���趨��ʱ�䣬������ʱ��������
unsigned char DS1302_Time[]={24,7,31,15,52,30,3};//˳��������ʱ��������
/**
   *@breaf DS1302��ʼ��
   *@param��
   *@retval��
   */
void  DS1302_Init()
{
    gpio_enable( 40, DIR_OUT );
    gpio_enable( 41, DIR_OUT );
    gpio_enable( 44, DIR_OUT );
    
    gpio_write(40,0);
    gpio_write(41,0);
    gpio_write(44,0);
}

/**
   *@breaf DS1302���ֽ�д�뺯��
   *@param command��д�����ָ���ָ�����Ҫд��Ĵ����ĵ�ַ��
	 *@param Data����Ҫд����������ݣ�
   *@retval ��
   */
void  DS1302_WriteBety(unsigned char command,unsigned char Data)
{
			unsigned char i;
			DS1302_CE(1);//ʹ��λ�øߵ�ƽ��
   	        /* �ر��ж� */
	   rt_base_t level = rt_hw_interrupt_disable();
			for(i=0;i<8;i++)//���ƼĴ���������Ҫͨ��IO��һ��һ��д����ƼĴ���;��λ��д��
		{
		       //�൱�ڰѵ�1--7λ��0��ֻ����0λ�������0λ��0����Ϊ0����֮��Ϊ1��
                if(command&(0x01<<i)) DS1302_IO(1);
                else DS1302_IO(0);

				DS1302_SCLK(1);
				DS1302_SCLK(0);
		}

				for(i=0;i<8;i++)//����д��
		{
				//�൱�ڰѵ�1--7λ��0��ֻ����0λ�������0λ��0����Ϊ0����֮��Ϊ1��
				if(Data&(0x01<<i)) DS1302_IO(1);
                else DS1302_IO(0);
                
				DS1302_SCLK(1);
				DS1302_SCLK(0);
		}
			DS1302_CE(0);
					/* �ָ��ж� */
        rt_hw_interrupt_enable(level);
}

/**
   *@breaf  DS1302���ֽڶ�������
   *@param command��д�����ָ���ָ�����Ҫ�����Ĵ����ĵ�ַ��
   *@retval Data�����������ݣ�
   */
unsigned char   DS1302_ReadBety(unsigned char command)
{
	    unsigned i,Data=0X00;
	    command|=0X01;//д��ָ�������ָ��ֻ�����һλ���1�����ڴ����û�����������
	        /* �ر��ж� */
	   rt_base_t level = rt_hw_interrupt_disable();
			DS1302_CE(1);//ʹ��λ�øߵ�ƽ��
		for(i=0;i<8;i++)//���ƼĴ���������Ҫͨ��IO��һ��һ��д����ƼĴ���;��λ��д��
		{
			    //�൱�ڰѵ�1--7λ��0��ֻ����0λ�������0λ��0����Ϊ0����֮��Ϊ1��
				 if(command&(0x01<<i)) DS1302_IO(1);
                else DS1302_IO(0);
				
				DS1302_SCLK(0);
				DS1302_SCLK(1);
		}
				DS1302_IO(0);
				
				gpio_enable( 41, DIR_IN );
			for(i=0;i<8;i++)//���ݶ���
		{
					DS1302_SCLK(1);
					DS1302_SCLK(0);
					if(red_DS1302_IO)
					{
                       Data |=(0X01<<i);
                    }
					
			
		}
		gpio_enable( 41, DIR_OUT );
		DS1302_CE(0);
		/* �ָ��ж� */
        rt_hw_interrupt_enable(level);
		return Data;
}


/**
   *@breaf ��DS1302���趨ʱ��
   *@param��
   *@retval��
   */
void DS1302_SetTime()
{
		DS1302_WriteBety(DS1302_WP,0x00);//���� DS1302 ֮ǰ���ر�д��������Ȼָ���޷�������ƼĴ�����
		DS1302_WriteBety(DS1302_YEAR, DS1302_Time[0]/10*16+DS1302_Time[0]%10);//д���꣬����10����ת��BCD�룻
		DS1302_WriteBety(DS1302_MONTH, DS1302_Time[1]/10*16+DS1302_Time[1]%10);//д���£�����10����ת��BCD�룻
		DS1302_WriteBety(DS1302_DATE, DS1302_Time[2]/10*16+DS1302_Time[2]%10);//д���գ�����10����ת��BCD�룻
		DS1302_WriteBety(DS1302_HOUR, DS1302_Time[3]/10*16+DS1302_Time[3]%10);//д��ʱ������10����ת��BCD�룻
		DS1302_WriteBety(DS1302_MINUTE, DS1302_Time[4]/10*16+DS1302_Time[4]%10);//д��֣�����10����ת��BCD�룻
		DS1302_WriteBety(DS1302_SECOND, DS1302_Time[5]/10*16+DS1302_Time[5]%10);//д���룬����10����ת��BCD�룻
		DS1302_WriteBety(DS1302_DAY, DS1302_Time[6]/10*16+DS1302_Time[6]%10);//д�����ڣ�����10����ת��BCD�룻
		DS1302_WriteBety( DS1302_WP,0x80);//д�����������д������
}

/**
*@breaf ��ȡDS1302��ʱ��
*@param��
*@retval��
*/
void DS1302_ReadTime()
{
	unsigned char Temp;//���������������ʱ�洢BCD��
    Temp=DS1302_ReadBety(DS1302_YEAR);//��ȡ��BCD�룻
		DS1302_Time[0]=Temp/16*10+Temp%16;//BCD��תʮ���ƣ�
	 Temp=DS1302_ReadBety(DS1302_MONTH);//��ȡ��BCD�룻
		DS1302_Time[1]=Temp/16*10+Temp%16;//BCD��תʮ���ƣ�
	 Temp=DS1302_ReadBety(DS1302_DATE);//��ȡ��BCD�룻
		DS1302_Time[2]=Temp/16*10+Temp%16;//BCD��תʮ���ƣ�
	 Temp=DS1302_ReadBety(DS1302_HOUR);//��ȡСʱBCD�룻
		DS1302_Time[3]=Temp/16*10+Temp%16;//BCD��תʮ���ƣ�
	 Temp=DS1302_ReadBety(DS1302_MINUTE);//��ȡ����BCD�룻
		DS1302_Time[4]=Temp/16*10+Temp%16;//BCD��תʮ���ƣ�
	 Temp=DS1302_ReadBety(DS1302_SECOND);//��ȡ��BCD�룻
		DS1302_Time[5]=Temp/16*10+Temp%16;//BCD��תʮ���ƣ�
	 Temp=DS1302_ReadBety(DS1302_DAY);//��ȡ����BCD�룻
		DS1302_Time[6]=Temp/16*10+Temp%16;//BCD��תʮ���ƣ�
}
