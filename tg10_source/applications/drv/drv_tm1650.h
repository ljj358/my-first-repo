/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-23     Malongwei    first version
 */

#ifndef __DRV_TM1650_H__
#define __DRV_TM1650_H__

#include "board.h"

void tm1650Display(uint8_t ucDig, uint8_t ucVal);
void tm1650Lightness(uint8_t ucVal);
void tm1650RefreshBuf(uint8_t *pucBuf);

#endif
