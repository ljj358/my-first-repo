/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-01-26     armink       the first version
 */

#include <fal.h>
#include "board.h"

#define OSPI_MEM_START_ADDR (ROM_START) ///< 4M偏移

static int init(void)
{
    /* do nothing now */
    return 0;
}

static int read(long offset, uint8_t *buf, size_t size)
{
    size_t i;
    uint32_t addr = OSPI_MEM_START_ADDR + offset;
    for (i = 0; i < size; i++, addr++, buf++)
    {
        *buf = *(uint8_t *) addr;
    }

    return size;
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    return 0;
}

static int erase(long offset, size_t size)
{
    return 0;
}

const struct fal_flash_dev ospi_mem_flash =
{
    .name       = OSPI_FLASH_DEV_NAME,
    .addr       = 0,
    .len        = 8 * 1024 * 1024,
    .blk_size   = 4 * 1024,
    .ops        = {init, read, write, erase},
    .write_gran = 8
};

