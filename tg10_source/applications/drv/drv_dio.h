#ifndef DRV_DIO_H
#define DRV_DIO_H

#include <rtthread.h>
#include "drv_common.h"

typedef enum {
    DIO_CHNL_DI1 = 0,
    DIO_CHNL_DI2,
    DIO_CHNL_DI3,
    DIO_CHNL_DI4,
    DIO_CHNL_DO1,
    DIO_CHNL_DO2,
    DIO_CHNL_DO3,
    DIO_CHNL_DO4,
    DIO_CHNL_RL1,
    DIO_CHNL_RL2,
    DIO_CHNL_MAX,
} DIO_CHNL_DEF;

int dioChannelGet(int chnl);
void dioChannelSet(int chnl, int val);
uint32_t dioStateGet(void);
void dioUpdateLedState(void);
int dioPwmSet(int chnl, uint32_t ulPeriod, uint32_t ulPulse);

#endif // DRV_DIO_H
