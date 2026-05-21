
#include "drv_param.h"

#include <rtthread.h>
#include <rtdevice.h>

#include "stdint.h"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_flash.h"

int read_dev_param_to_onchip_flash(sysParamOnchip_t *data);
static int write_dev_param_to_onchip_flash(sysParamOnchip_t data);

char *getSnFromOtp(void);
char *getHwVerFromOtp(void);

#define PARAM_FLAG_VAL 0x55AACCEE

#if defined(STM32H7B0xx) || defined(STM32H7B0xxQ)
#define PARAM_USING_ONCHIP_FLASH 0
#else
#define PARAM_USING_ONCHIP_FLASH 1
#endif

sysParamDef *gptSysParam = (sysParamDef *)0x38800000;	//缓存空间

#define CRC_CALC_LEN (gptSysParam->usParamLen - 2)

/* CRC16 余式表 */
static const uint16_t crctalbeabs[] = {
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
};

static uint16_t Crc16(uint8_t *ptr, uint32_t len)
{
    uint16_t crc = 0xffff;
    uint32_t i;
    uint8_t ch;

    for (i = 0; i < len; i++) {
        ch = *ptr++;
        crc = crctalbeabs[(ch ^ crc) & 15] ^ (crc >> 4);
        crc = crctalbeabs[((ch >> 4) ^ crc) & 15] ^ (crc >> 4);
    }

    return crc;
}

void paramBootCountIncrease(void)
{
    gptSysParam->ulBootCount++;

    paramUpdateChsm();

    rt_kprintf("------boot count:%u------\n", gptSysParam->ulBootCount);
}

void paramUpdateChsm(void)
{
    gptSysParam->ulFlag = PARAM_FLAG_VAL;

    gptSysParam->usChsm = Crc16((uint8_t *)gptSysParam, CRC_CALC_LEN);
}

#define OTP_PARAM_START    FLASH_OTP_BASE
#define OTP_PARAM_MAXLEN   (FLASH_OTP_END + 1 - FLASH_OTP_BASE)

typedef struct {
    uint8_t acDid[16];
    uint8_t acHwVer[8];
    uint8_t acHwDate[8];
} otpParamDef;

// 1024 bytes  32组

typedef enum {
    PARAM_DID = 0,
    PARAM_HWVER,
    PARAM_HWDATE
} OTP_PARAM_TYPE;

static otpParamDef tmpParam;

static char *getParamFromOtp(OTP_PARAM_TYPE paramName)
{
    static char acBuf[16];
    otpParamDef *ptParam = &tmpParam;

    rt_kprintf("get otp param:%d\r\n", paramName);

    rt_memset(acBuf, 0, sizeof(acBuf));

    for (uint32_t i = FLASH_OTP_END + 1 - sizeof(otpParamDef); i >= FLASH_OTP_BASE; i -= sizeof(otpParamDef)) {
        rt_memcpy(ptParam, (void *)i, sizeof(otpParamDef));
        if (paramName == PARAM_DID) {

            if (ptParam->acDid[0] == 0xFF) {
                continue;
            }
            rt_strncpy(acBuf, (char *)ptParam->acDid, sizeof(acBuf));
            rt_kprintf("did:%s\r\n", acBuf);
            rt_kprintf("otp at 0x%08X\r\n", i);
            break;
        } else if (paramName == PARAM_HWVER) {
            if (ptParam->acHwVer[0] == 0xFF) {
                continue;
            }
            rt_strncpy(acBuf, (char *)ptParam->acHwVer, sizeof(acBuf));
            rt_kprintf("ver:%s\r\n", acBuf);
            rt_kprintf("otp at 0x%08X\r\n", i);
            break;
        } else if (paramName == PARAM_HWDATE) {
            if (ptParam->acHwDate[0] == 0xFF) {
                continue;
            }
            rt_strncpy(acBuf, (char *)ptParam->acHwDate, sizeof(acBuf));
            rt_kprintf("date:%s\r\n", acBuf);
            rt_kprintf("otp at 0x%08X\r\n", i);
            break;
        }
    }
    rt_kprintf("get otp param:%s\r\n", acBuf);

    return acBuf;
}

static int writeDataToOtp(uint32_t addr, uint8_t *data, uint16_t len)
{
    static uint32_t aulWrBuf[4];
    rt_memset(aulWrBuf, 0, sizeof(aulWrBuf));
    rt_strncpy((char *)aulWrBuf, (char *)data, sizeof(aulWrBuf));

    // __disable_irq();
    HAL_FLASH_Unlock(); //unlock
    HAL_FLASH_OB_Unlock(); //unlock

    if (len % 2 != 0) {
        len++;
    }

    for (int i = 0; i < len / 2; ++i) {
        rt_kprintf("write otp word addr:0x%08X\r\n", addr + i * 2);
        HAL_StatusTypeDef ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_OTPWORD, (uint32_t)(addr + i * 2), ((uint32_t)&aulWrBuf[0]) + i * 2);

        if (ret != HAL_OK) {
            rt_kprintf("failed to write otp:0x%08X , status : %d\r\n", addr + i * 2,ret);
            break;
        }
    }

    HAL_FLASH_OB_Lock(); //lock again
    HAL_FLASH_Lock(); //lock again

    // __enable_irq();

	return 0;
}

static int setParamToOtp(OTP_PARAM_TYPE paramName, char *data)
{
    static uint32_t aulWrBuf[4];

    const uint16_t usOtpSize = OTP_PARAM_MAXLEN;

    otpParamDef *ptParam;

    // 判断是否已经设置过该参数
    if (rt_strcmp(getParamFromOtp(paramName), data) == 0) {
        rt_kprintf("already set %d to:%s\n", paramName, data);
        return 0;
    }

    for (size_t idx = 0; idx < usOtpSize; idx += sizeof(otpParamDef))
    {
        // rt_memcpy(ptParam, (void *)(idx + FLASH_OTP_BASE), sizeof(otpParamDef));
        ptParam = (otpParamDef *)(idx + FLASH_OTP_BASE);

        rt_kprintf("check otp at:0x%08X\r\n", (uint32_t)(FLASH_OTP_BASE + idx));

        // 根据参数名判断是否已经设置过该参数
        if (paramName == PARAM_DID && ptParam->acDid[0] != 0xFF) {
            rt_kprintf("already exists did at 0x%08X:%s\n", (uint32_t)&ptParam->acDid[0], (char *)ptParam->acDid);
            continue;
        } else if (paramName == PARAM_HWVER && ptParam->acHwVer[0] != 0xFF) {
            continue;
        } else if (paramName == PARAM_HWDATE && ptParam->acHwDate[0] != 0xFF) {
            continue;
        }

        rt_kprintf("write otp at:0x%08X\r\n", (uint32_t)(FLASH_OTP_BASE + idx));
        rt_memset(aulWrBuf, 0, sizeof(aulWrBuf));
        rt_strncpy((char *)aulWrBuf, data, sizeof(aulWrBuf));

        // 根据参数名选择写入的数据位置和大小
        if (paramName == PARAM_DID) {
            writeDataToOtp((uint32_t)&ptParam->acDid[0], (uint8_t *)data, sizeof(ptParam->acDid));
        } else if (paramName == PARAM_HWVER) {
            writeDataToOtp((uint32_t)&ptParam->acHwVer[0], (uint8_t *)data, sizeof(ptParam->acHwVer));
        } else if (paramName == PARAM_HWDATE) {
            writeDataToOtp((uint32_t)&ptParam->acHwDate[0], (uint8_t *)data, sizeof(ptParam->acHwDate));
        }

        return 0;
    }

    rt_kprintf("failed to set %d to:%s\n", paramName, data);
    return -2;
}

#ifdef FINSH_USING_MSH
static int __setDid(uint8_t argc, char **argv)
{
	sysParamOnchip_t read_data = {0};
	
    static int iSetFlag = 0;
    if (argc != 2) {
        rt_kprintf("set_did 10220100001\n");
        return 0;
    }

    if (rt_strlen(argv[1]) != 11) {
        rt_kprintf("invalid did len\n");
        return 0;
    }

    if (iSetFlag == 0) {
        iSetFlag = 1;
    } else {
        rt_kprintf("!!!Only once set can be done every poweron,Please reset to set did again!!!\n");
        return 0;
    }

    rt_kprintf("to set did to:%s\n", argv[1]);
    setParamToOtp(PARAM_DID, argv[1]);

    if (rt_strncmp(getParamFromOtp(PARAM_DID), argv[1], 11) != 0) {
        rt_kprintf("failed to set did\n");
        return 0;
    }
	
	rt_memcpy(read_data.acDevUid, getParamFromOtp(PARAM_DID), rt_strlen(getParamFromOtp(PARAM_DID)) + 1);
	write_dev_param_to_onchip_flash(read_data);
	

    rt_memset(gptSysParam->acDevUid, 0, sizeof(gptSysParam->acDevUid));
    rt_strncpy(gptSysParam->acDevUid, getParamFromOtp(PARAM_DID), sizeof(gptSysParam->acDevUid));
    paramUpdateChsm();

    rt_kprintf("set did to:%s\n", gptSysParam->acDevUid);

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__setDid, set_did, set device id);
#endif

#ifdef FINSH_USING_MSH
static int __setHwVersion(uint8_t argc, char **argv)
{
    static int iSetFlag = 0;
    if (argc != 2) {
        rt_kprintf("example: set_hw_ver V1.4\n");
        return 0;
    }

    if (rt_strlen(argv[1]) != 4) {
        rt_kprintf("invalid hw version len\n");
        return 0;
    }

    if (iSetFlag == 0) {
        iSetFlag = 1;
    } else {
        rt_kprintf("!!!Only once set can be done every poweron,Please reset to set hw version again!!!\n");
        return 0;
    }

    if (rt_strncmp(getParamFromOtp(PARAM_HWVER), argv[1], 4) == 0) {
        rt_kprintf("already set hw version to:%s\n", argv[1]);
        return 0;
    }

    setParamToOtp(PARAM_HWVER, argv[1]);

    rt_memset(gptSysParam->acHwVersion, 0, sizeof(gptSysParam->acHwVersion));
    rt_strncpy(gptSysParam->acHwVersion, getParamFromOtp(PARAM_HWVER), sizeof(gptSysParam->acHwVersion));
    paramUpdateChsm();

    rt_kprintf("set HW version to:%s\n", gptSysParam->acHwVersion);

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__setHwVersion, set_hw_ver, set HW version);
#endif

#ifdef FINSH_USING_MSH
static int __setHwDate(uint8_t argc, char **argv)
{
    static int iSetFlag = 0;
    if (argc != 2) {
        rt_kprintf("example: set_hw_date 230201\n");
        return 0;
    }

    if (rt_strlen(argv[1]) != 6) {
        rt_kprintf("invalid hw date len\n");
        return 0;
    }

    if (iSetFlag == 0) {
        iSetFlag = 1;
    } else {
        rt_kprintf("!!!Only once set can be done every poweron,Please reset to set hw date again!!!\n");
        return 0;
    }

    if (rt_strncmp(getParamFromOtp(PARAM_HWDATE), argv[1], 6) == 0) {
        rt_kprintf("already set hw date to:%s\n", argv[1]);
        return 0;
    }

    setParamToOtp(PARAM_HWDATE, argv[1]);

    rt_memset(gptSysParam->acHwDate, 0, sizeof(gptSysParam->acHwDate));
    rt_strncpy(gptSysParam->acHwDate, getParamFromOtp(PARAM_HWDATE), sizeof(gptSysParam->acHwDate));
    paramUpdateChsm();

    rt_kprintf("set HW version to:%s\n", gptSysParam->acHwDate);

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__setHwDate, set_hw_date, set HW manufacture date);
#endif

#ifdef FINSH_USING_MSH
static int __set232LogEnable(uint8_t argc, char **argv)
{
    if (argc != 2) {
        rt_kprintf("example: uart_log 0\n");
        return 0;
    }


    if (argv[1][0] == '0') {
        gptSysParam->ucRs232_1_LogEnable = 0;
    } else {
        gptSysParam->ucRs232_1_LogEnable = 1;
    }
    paramUpdateChsm();

    rt_kprintf("set RS232 log to:%d\n", gptSysParam->ucRs232_1_LogEnable);

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__set232LogEnable, uart_log, enable or disable RS232 uart log);
#endif


#ifdef FINSH_USING_MSH
static int __dumpOtp(uint8_t argc, char **argv)
{
    FLASH_OBProgramInitTypeDef obInfo;
    uint8_t *pucPtr = (uint8_t *)FLASH_OTP_BASE;
    const uint16_t usOtpSize = (FLASH_OTP_END + 1 - FLASH_OTP_BASE);

    rt_kprintf("otp size:%u\r\n", usOtpSize);

    HAL_FLASHEx_OBGetConfig(&obInfo);

    for (int i = 0; i < usOtpSize; ++i) {
        if ((i > 0) && (i % 16 == 0)) {
            rt_kprintf("\r\n");
        }
        if (i % 16 == 0) {
            rt_kprintf("%02d. ", i / 16);
        }
        rt_kprintf("%02X ", pucPtr[i]);
    }
    rt_kprintf("\r\n");

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__dumpOtp, otp_dump, dump otp area);
#endif


int param_init(void)
{
	sysParamOnchip_t read_data = {0};
	
	    /* 通过将 RCC AHB1 外设时钟使能寄存器 (RCC_AHB1ENR) 中的 BKPSRAMEN 位置 1， 使能备份 SRAM 时钟 */
    __HAL_RCC_BKPRAM_CLK_ENABLE();

    /* DBP 位置 1，使能对备份域的访问 */
    HAL_PWR_EnableBkUpAccess();

    rt_kprintf("init backup sram\n");

    /* 应用程序必须等待备份调压器就绪标志 (BRR) 置 1，指示在待机模式和 VBAT 模式下会保持写入 RAM 中的数据。 */
    HAL_PWREx_EnableBkUpReg();

    gptSysParam->usParamLen = sizeof(sysParamDef);

    rt_kprintf("check sram\n");

    gptSysParam->usParamLen = sizeof(sysParamDef);

    if ((gptSysParam->ulFlag != PARAM_FLAG_VAL) ||
            (gptSysParam->usChsm != Crc16((uint8_t *)gptSysParam, sizeof(sysParamDef) - 2))) {
        rt_memset(gptSysParam, 0, sizeof(sysParamDef));
        gptSysParam->ulFlag = PARAM_FLAG_VAL;
        gptSysParam->ucRs232_1_LogEnable = 0;
        gptSysParam->ulBootCount = 1;
        gptSysParam->usParamLen = sizeof(sysParamDef);
        rt_strcpy(gptSysParam->acDevUid, "10230100001");

        rt_kprintf("update checksum\n");
        paramUpdateChsm();

        rt_kprintf("set default sys param\n");
    }

    rt_kprintf("check otp DID\n");
	
	if(read_dev_param_to_onchip_flash(&read_data) != 0)
	{
		rt_memset(&read_data, 0x0, sizeof(sysParamOnchip_t));
		rt_kprintf("[err]read onchip flash error \r\n");
	}
	
	
	if (getParamFromOtp(PARAM_DID)[0] == '1') 
	{
		if(read_data.acDevUid[0] == '1')
		{
			if(rt_memcmp(getParamFromOtp(PARAM_DID), read_data.acDevUid, rt_strlen(getParamFromOtp(PARAM_DID))) != 0)
			{
				rt_kprintf("otp / onchip param error , set onchip param\r\n");
				rt_memcpy(read_data.acDevUid, getParamFromOtp(PARAM_DID), rt_strlen(getParamFromOtp(PARAM_DID)) + 1);
				write_dev_param_to_onchip_flash(read_data);
			}
		}
		else
		{	
			rt_kprintf("[err]onchip param error , set onchip param\r\n");
			rt_memcpy(read_data.acDevUid, getParamFromOtp(PARAM_DID), rt_strlen(getParamFromOtp(PARAM_DID)) + 1);
			write_dev_param_to_onchip_flash(read_data);
		}
		
		if(rt_memcmp(gptSysParam->acDevUid, getParamFromOtp(PARAM_DID), rt_strlen(getParamFromOtp(PARAM_DID))) != 0)
		{
			rt_kprintf("[err 1]backup param error, set backup param \r\n");
			rt_strncpy(gptSysParam->acDevUid, getParamFromOtp(PARAM_DID), rt_strlen(getParamFromOtp(PARAM_DID)) + 1);
			paramUpdateChsm();
		}
	}
	else
	{
		rt_kprintf("[err]otp param get error \r\n");
		
		if(read_data.acDevUid[0] == '1')
		{
			rt_kprintf("set otp param \r\n");
			
			setParamToOtp(PARAM_DID, read_data.acDevUid);
			
			
			if(rt_memcmp(gptSysParam->acDevUid, read_data.acDevUid, rt_strlen(read_data.acDevUid)) != 0)
			{
				rt_kprintf("[err 2]backup param error, set backup param \r\n");
				rt_strncpy(gptSysParam->acDevUid, read_data.acDevUid, rt_strlen(read_data.acDevUid) + 1);
				paramUpdateChsm();
			}
			
		}
		else
		{
			if(rt_memcmp(gptSysParam->acDevUid, "10230100001", rt_strlen("10230100001")) != 0)
			{
				rt_kprintf("get backup param, set otp \r\n");
				
				setParamToOtp(PARAM_DID, gptSysParam->acDevUid);
				
				rt_kprintf("get backup param, set onchip param \r\n");
				
				rt_memcpy(read_data.acDevUid, gptSysParam->acDevUid, rt_strlen(gptSysParam->acDevUid) + 1);
				write_dev_param_to_onchip_flash(read_data);
				
			}
			else
			{
				rt_kprintf("[ERROR]ALL OF PARAM ERROR !!!!!!!!!!!!!!!!!! \r\n");
			}
		}
	}
	
	

//	if (getParamFromOtp(PARAM_DID)[0] == '1') {
//		rt_strncpy(gptSysParam->acDevUid, getParamFromOtp(PARAM_DID), sizeof(gptSysParam->acDevUid));
//        paramUpdateChsm();
//    }
//	// else if (getParamFromOtp(PARAM_DID)[0] == 0) {
////     rt_kprintf("set did to otp from backup sram\n");
////     setParamToOtp(PARAM_DID, gptSysParam->acDevUid);
//// }


    rt_kprintf("check otp HW_VER\n");
    if (getParamFromOtp(PARAM_HWVER)[0] == 'V') {
        rt_strncpy(gptSysParam->acHwVersion, getParamFromOtp(PARAM_HWVER), sizeof(gptSysParam->acHwVersion));
        paramUpdateChsm();
    }

    rt_kprintf("check otp HW_DATE\n");
    if (getParamFromOtp(PARAM_HWDATE)[0] == '2') {
        rt_strncpy(gptSysParam->acHwDate, getParamFromOtp(PARAM_HWDATE), sizeof(gptSysParam->acHwDate));
        paramUpdateChsm();
    }

	return 0;
}
// INIT_COMPONENT_EXPORT(param_init); 在vConsoleInit中调用

static int test_otp(uint8_t argc, char **argv)
{
	uint8_t data[32] = {0xFF,0xFF,0xFF,0xFF};
	int offset_addr = atoi(argv[1]);


    // __disable_irq();
    HAL_FLASH_Unlock(); //unlock
    HAL_FLASH_OB_Unlock(); //unlock

	HAL_StatusTypeDef ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_OTPWORD, FLASH_OTP_BASE+offset_addr, (uint32_t)data);
	rt_kprintf("write flash offset : 0x%08X ,write otp status : %d \r\n",FLASH_OTP_BASE+offset_addr,ret);

    HAL_FLASH_OB_Lock(); //lock again
    HAL_FLASH_Lock(); //lock again

    // __enable_irq();

	char *read_data = (char *)(FLASH_OTP_BASE+offset_addr);

	rt_kprintf("otp data : %s \r\n",read_data);


//	STMFLASH_Write(FLASH_OTP_BASE+offset_addr,(uint32_t *)data,1);


    return 0;
}
MSH_CMD_EXPORT_ALIAS(test_otp, test_otp, test_otp);



////////////////////write onchip flash param//////////////////////

#include "drv_flash.h"
#include "crc32.h"

#define PARAM_FLASH_ADDR_1	FLASH_BANK1_BASE + 128*1024
#define PARAM_FLASH_ADDR_2	FLASH_BANK1_BASE + 136*1024
#define PARAM_FLASH_ADDR_3	FLASH_BANK1_BASE + 144*1024

#define PARAM_FLASH_SIZE	256

static int read_dev_param_to_onchip_flash_fun(rt_uint32_t addr, sysParamOnchip_t *data)
{
	sysParamOnchip_t read_data = {0};
	uint32_t crc = 0;
	
	for(int i = 0; i < 6; i++)
	{
		rt_memset(&read_data, 0x0, sizeof(sysParamOnchip_t));
		
		stm32_flash_read(addr, (rt_uint8_t *)(&read_data), sizeof(sysParamOnchip_t));
		
		crc = Crc32_ComputeBuf(0, &read_data, sizeof(sysParamOnchip_t)-4);
		if(read_data.crc32 == crc)
		{
			rt_kprintf("read : 0x%08X  ok \r\n",addr);
			rt_memcpy(data, &read_data, sizeof(sysParamOnchip_t));
			return 0;
		}
		
	}
	
	rt_kprintf("[error]read : 0x%08X  error , id :%s , cnt: %d, crc = %u\r\n",addr,read_data.acDevUid,read_data.cnt,read_data.crc32);
	
	return -1;
}

static int write_dev_param_to_onchip_flash_fun(rt_uint32_t addr, sysParamOnchip_t data)
{
	sysParamOnchip_t read_data = {0};
	uint32_t crc = 0;

	for(int i = 0; i < 6; i++)
	{
		rt_memset(&read_data, 0x0, sizeof(sysParamOnchip_t));

		data.crc32 =  Crc32_ComputeBuf(0, &data, sizeof(sysParamOnchip_t)-4);
		
		stm32_flash_erase(addr, PARAM_FLASH_SIZE);

		stm32_flash_write(addr, (rt_uint8_t *)(&data), sizeof(sysParamOnchip_t));

		stm32_flash_read(addr, (rt_uint8_t *)(&read_data), sizeof(sysParamOnchip_t));

		crc = Crc32_ComputeBuf(0, &read_data, sizeof(sysParamOnchip_t)-4);
		
		if(read_data.crc32 == crc)
		{
			rt_kprintf("write : 0x%08X  ok \r\n",addr);
			return 0;
		}
	}

	rt_kprintf("[error]write : 0x%08X  error \r\n",addr);

	return -1;
}





int read_dev_param_to_onchip_flash(sysParamOnchip_t *data)
{
#if !PARAM_USING_ONCHIP_FLASH
	if (data != RT_NULL)
	{
		rt_memset(data, 0x0, sizeof(sysParamOnchip_t));
	}
	return -1;
#else


	sysParamOnchip_t read_data1 = {0};
	sysParamOnchip_t read_data2 = {0};
	sysParamOnchip_t read_data3 = {0};
	
	uint32_t crc1,crc2,crc3;

	read_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_1, &read_data1);
//	stm32_flash_read(PARAM_FLASH_ADDR_1, (rt_uint8_t *)(&read_data1), sizeof(sysParamOnchip_t));
	crc1 = Crc32_ComputeBuf(0, &read_data1, sizeof(sysParamOnchip_t)-4);
	if(crc1 != read_data1.crc32)
	{
		read_data1.cnt = 0;
		rt_kprintf("[err flash]read arr1 error  \r\n");
	}
	else
	{
		rt_kprintf("[ok flash]read arr1 ok  \r\n");
	}

	read_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_2, &read_data2);
//	stm32_flash_read(PARAM_FLASH_ADDR_2, (rt_uint8_t *)(&read_data2), sizeof(sysParamOnchip_t));
	crc2 = Crc32_ComputeBuf(0, &read_data2, sizeof(sysParamOnchip_t)-4);
	if(crc2 != read_data2.crc32)
	{
		read_data2.cnt = 0;
		rt_kprintf("[err flash]read arr2 error  \r\n");
	}
	else
	{
		rt_kprintf("[ok flash]read arr2 ok  \r\n");
	}

	read_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_3, &read_data3);
//	stm32_flash_read(PARAM_FLASH_ADDR_3, (rt_uint8_t *)(&read_data3), sizeof(sysParamOnchip_t));
	crc3 = Crc32_ComputeBuf(0, &read_data3, sizeof(sysParamOnchip_t)-4);	
	if(crc3 != read_data3.crc32)
	{
		read_data3.cnt = 0;
		rt_kprintf("[err flash]read arr3 error  \r\n");
	}
	else
	{
		rt_kprintf("[ok flash]read arr3 ok  \r\n");
	}

	if (read_data1.cnt > read_data2.cnt && read_data1.cnt > read_data3.cnt)
	{
		rt_kprintf("[flash] read write addr1 \r\n");
		
		crc1 = Crc32_ComputeBuf(0, &read_data1, sizeof(sysParamOnchip_t)-4);
        if(crc1 != read_data1.crc32)
		{
			rt_kprintf("[flash] read write addr1 crc err\r\n");
			return -1;
		}

		rt_memcpy(data, &read_data1, sizeof(sysParamOnchip_t));
		
		if(rt_memcmp(&read_data1, &read_data2, sizeof(sysParamOnchip_t)) != 0)
		{	
			write_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_2, read_data1);
			rt_kprintf("[flash][1]2 \r\n");
		}
		
		if(rt_memcmp(&read_data1, &read_data3, sizeof(sysParamOnchip_t)) != 0)
		{
			write_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_3, read_data1);
			rt_kprintf("[flash][1]3 \r\n");
		}
		
		return 0;
    }
	else if (read_data2.cnt > read_data1.cnt && read_data2.cnt > read_data3.cnt)
	{
		rt_kprintf("[flash] read write addr2 \r\n");
		
		crc2 = Crc32_ComputeBuf(0, &read_data2, sizeof(sysParamOnchip_t)-4);
        if(crc2 != read_data2.crc32)
		{
			rt_kprintf("[flash] read write addr2 crc err\r\n");
			return -1;
		}

		rt_memcpy(data, &read_data2, sizeof(sysParamOnchip_t));

		if(rt_memcmp(&read_data1, &read_data2, sizeof(sysParamOnchip_t)) != 0)
		{
			write_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_1, read_data2);
			rt_kprintf("[flash][2]1 \r\n");
		}
		
		if(rt_memcmp(&read_data3, &read_data2, sizeof(sysParamOnchip_t)) != 0)
		{
			write_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_3, read_data2);
			rt_kprintf("[flash][2]3 \r\n");
		}
		
		return 0;
    }
	else
	{
		rt_kprintf("[flash] read write addr3 \r\n");
		
		crc3 = Crc32_ComputeBuf(0, &read_data3, sizeof(sysParamOnchip_t)-4);
        if(crc3 != read_data3.crc32)
		{
			rt_kprintf("[flash] read write addr3 crc err\r\n");
			return -1;
		}

		rt_memcpy(data, &read_data3, sizeof(sysParamOnchip_t));

		if(rt_memcmp(&read_data1, &read_data3, sizeof(sysParamOnchip_t)) != 0)
		{
			write_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_1, read_data3);
			rt_kprintf("[flash][3]1 \r\n");
		}
		
		if(rt_memcmp(&read_data3, &read_data2, sizeof(sysParamOnchip_t)) != 0)
		{
			write_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_2, read_data3);
			rt_kprintf("[flash][3]2 \r\n");
		}
		
		return 0;
    }



	return -1;
#endif
}


static int write_dev_param_to_onchip_flash(sysParamOnchip_t data)
{
#if !PARAM_USING_ONCHIP_FLASH
	(void)data;
	return -1;
#else
	sysParamOnchip_t read_data1 = {0};
	sysParamOnchip_t read_data2 = {0};
	sysParamOnchip_t read_data3 = {0};
	
	uint32_t crc1,crc2,crc3;

	uint32_t cnt = 0;

	read_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_1, &read_data1);
//	stm32_flash_read(PARAM_FLASH_ADDR_1, (rt_uint8_t *)(&read_data1), sizeof(sysParamOnchip_t));
	crc1 = Crc32_ComputeBuf(0, &read_data1, sizeof(sysParamOnchip_t)-4);
	
	if(crc1 != read_data1.crc32)
	{
		read_data1.cnt = 0;
		rt_kprintf("[err][flash]read addr1 error \r\n");
	}
	else
	{
		cnt = read_data1.cnt;
		
		rt_kprintf("[ok][flash]read addr1 ok \r\n");
	}

	read_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_2, &read_data2);
//	stm32_flash_read(PARAM_FLASH_ADDR_2, (rt_uint8_t *)(&read_data2), sizeof(sysParamOnchip_t));
	crc2 = Crc32_ComputeBuf(0, &read_data2, sizeof(sysParamOnchip_t)-4);

	if(crc2 != read_data2.crc32)
	{
		read_data2.cnt = 0;
		rt_kprintf("[err][flash]read addr2 error \r\n");
	}
	else
	{
		if(cnt < read_data2.cnt)
		{
			cnt = read_data2.cnt;
		}
		
		rt_kprintf("[ok][flash]read addr2 ok \r\n");
	}

	read_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_3, &read_data3);
//	stm32_flash_read(PARAM_FLASH_ADDR_3, (rt_uint8_t *)(&read_data3), sizeof(sysParamOnchip_t));
	crc3 = Crc32_ComputeBuf(0, &read_data3, sizeof(sysParamOnchip_t)-4);

	if(crc3 != read_data3.crc32)
	{
		read_data3.cnt = 0;
		rt_kprintf("[err][flash]read addr3 error \r\n");
	}
	else
	{
		if(cnt < read_data3.cnt)
		{
			cnt = read_data3.cnt;
		}
		
		rt_kprintf("[ok][flash]read addr3 ok \r\n");
	}

	data.cnt = ++cnt;

	data.crc32 = Crc32_ComputeBuf(0, &data, sizeof(sysParamOnchip_t)-4);

	rt_kprintf("[flash]write addr 1  \r\n");
	write_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_1, data);

	rt_kprintf("[flash]write addr 2  \r\n");
	write_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_2, data);

	rt_kprintf("[flash]write addr 3  \r\n");
	write_dev_param_to_onchip_flash_fun(PARAM_FLASH_ADDR_3, data);


	return 0;
#endif
}




//static int z_flash(uint8_t argc, char **argv)
//{
//	sysParamOnchip_t data = {0};
//	sysParamOnchip_t read_data = {0};
//	
//	rt_memcpy(data.acDevUid, "10086100111", strlen("10086100111") + 1);
//	
//	write_dev_param_to_onchip_flash(data);
//	
//	if(read_dev_param_to_onchip_flash(&read_data) == 0)
//	{
//		rt_kprintf("read ok , id :%s\r\n",read_data.acDevUid);
//	}
//	else
//	{
//		rt_kprintf("[err]read error , id :%s \r\n",read_data.acDevUid);
//	
//	}
//	
//	
//	
//	
//}
//MSH_CMD_EXPORT(z_flash, z_flash);





















































