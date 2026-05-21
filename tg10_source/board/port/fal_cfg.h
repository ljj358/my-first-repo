/*
 * File      : fal_cfg.h
 * This file is part of FAL (Flash Abstraction Layer) package
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-05-17     armink       the first version
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <rtconfig.h>
#include <board.h>

//#define NOR_FLASH_DEV_NAME             "norflash0"
#define OSPI_FLASH_DEV_NAME             "ospiflash0"
#define OSPI_FLASH_PARTITION_NAME       "flash_fs"

#define ONCHIP_FLASH_DEV_NAME             "onchipflash"
#define ONCHIP_FLASH_PARTITION_NAME       "bootloader"


#define ONCHIP_FLASH_PARTITION_NAME_DEV_1	"dev1"
#define ONCHIP_FLASH_PARTITION_NAME_DEV_2	"dev2"
#define ONCHIP_FLASH_PARTITION_NAME_DEV_3	"dev3"

#define ONCHIP_FLASH_PARTITION_NAME_TEST	"test"



/* ===================== Flash device Configuration ========================= */
//extern struct fal_flash_dev nor_flash0;
extern const struct fal_flash_dev ospi_mem_flash;
extern const struct fal_flash_dev stm32_onchip_flash_128k;

/* flash device table */
#define FAL_FLASH_DEV_TABLE                                          \
{                                                                    \
    &ospi_mem_flash,                                                     \
    &stm32_onchip_flash_128k \
}

/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
#define FAL_PART_TABLE                                                                     \
{                                                                                          \
    {FAL_PART_MAGIC_WORD, OSPI_FLASH_PARTITION_NAME,		 OSPI_FLASH_DEV_NAME, 	4*1024*1024, 4*1024*1024, 0}, \
    {FAL_PART_MAGIC_WORD, ONCHIP_FLASH_PARTITION_NAME, 		 ONCHIP_FLASH_DEV_NAME, 0, 128*1024, 0}, \
}
#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
