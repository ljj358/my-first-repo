#ifndef DRV_PARAM_H
#define DRV_PARAM_H

#include <rtthread.h>
#include "drv_common.h"

#pragma pack(1)
/*
 * 此结构体大小不得超过4KB，数据保存在备份SRAM中，主电源掉电数据不丢失
 */
typedef struct __sysParamDef {
    uint32_t ulFlag; ///< 有效值为 0x55AACCEE
    uint16_t usParamLen; ///< 长度为整个结构体的大小
    char acDevUid[20]; ///< 设备唯一ID
    char acHwVersion[10]; ///< 硬件版本
    char acHwDate[10]; ///< 硬件生产日期
    char acBootVersion[10]; ///< bootloader版本
    char acFwVersion[10]; ///< 固件版本

    uint32_t ulBootCount; ///<设备启动计数
    uint8_t ucRs232_1_LogEnable; ///<是否允许通过RS232_1打印日志
    uint8_t aucEthMac[10]; ///< 以太网mac地址
    char acModemModel[32]; ///< 4G模块型号
    char acModemImei[20]; ///< 4G模块IMEI
    char acSimCcid[30]; ///< 物联网卡ccid
    /* 以上字段于2023-02-02定稿，不得修改，之后增加数据在下面补充 */

    uint16_t usChsm; ///< CRC16校验值
} sysParamDef;


typedef struct{
	char acDevUid[20]; ///< 设备唯一ID
	uint32_t 	cnt;
	uint32_t	crc32;
}sysParamOnchip_t;




#pragma pack()

extern sysParamDef *gptSysParam;

void paramBootCountIncrease(void);
void paramUpdateChsm(void);
int param_init(void);

#endif // DRV_PARAM_H
