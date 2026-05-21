#ifndef DRV_AIN_H
#define DRV_AIN_H


#include <rtthread.h>
#include "drv_common.h"


typedef enum {
    AIN_CHNL_AIN1 = 0,
    AIN_CHNL_AIN2,
    AIN_CHNL_AIN3,
    AIN_CHNL_AIN4,
    AIN_CHNL_VIN,
    AIN_CHNL_TEMP,
    AIN_CHNL_MAX,
} AIN_CHNL_DEF;

float ainGetVoltage(int chnnl);

float ainGetBoardTemp(void);

float ainGetPowerVoltage(void);

#endif // DRV_AIN_H
