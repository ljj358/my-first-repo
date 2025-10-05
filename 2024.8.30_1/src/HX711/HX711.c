#include "HX711.h"
#include "stdint.h"
#include "rtthread.h"


unsigned long Weight_Maopi = 0;
long  Weight_object = 0;
float GapValue = 163.67;	   //传感器曲率   当发现测试出来的重量偏大时，增加该数值。
//如果测试出来的重量偏小时，减小改数值。该值可以为小数，例如 429.5 等。

/****************************************************************
功  能：HX711端口初始化
参  数：无
返回值：无
****************************************************************/
void HX711_Init(void)
{
	
	gpio_enable(59,DIR_IN);
	gpio_enable(58,DIR_OUT);

}

/****************************************************************
功  能：读取HX711的值
参  数：无
返回值：
****************************************************************/
unsigned long HX711_Read(void)	//增益128
{
	unsigned long count; 
	uint8_t i; 
  	HX711_SCK(0); 
  	count = 0; 
  	if(HX711_DOUT){
  	    rt_thread_delay(100);
  	    if(HX711_DOUT) return 0 ;
      }
      
    /* 关闭中断 */
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
    count = count ^ 0x800000;  //第25个脉冲下降沿来时，转换数据
	HX711_SCK(0);  
	  /* 恢复中断 */
    rt_hw_interrupt_enable(level);
	return(count);
}

/****************************************************************
功  能：获取毛皮重量
参  数：无
返回值：无
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
功  能：校准(写出来还没验证过)
参  数：输入重量
返回值：传感器曲率
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
功  能：称重
参  数：无
返回值：无
****************************************************************/

long Get_Weight(void)
{

	Weight_object = HX711_Read() - Weight_Maopi;	   		// 获取实物的AD采样数值。
    
	if(Weight_object> 0 && Weight_Maopi != 0)
	{
		Weight_object= ((float)Weight_object / GapValue); 	// 计算实物的实际重量
	}
	else
	{
		return 0;
	}
//	rt_kprintf("Weight_object = %d\r\n",Weight_object);
    return Weight_object;
}



