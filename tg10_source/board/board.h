/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-07-29     RealThread   first version
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <rtthread.h>
#include <stm32h7xx.h>
#include <drv_common.h>

#include "easyblink.h"
#include "beep.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*-------------------------- CHIP CONFIG BEGIN --------------------------*/

#define CHIP_FAMILY_STM32
#define CHIP_SERIES_STM32H7
#define CHIP_NAME_STM32H750XBHX

/*-------------------------- CHIP CONFIG END --------------------------*/

/*-------------------------- ROM/RAM CONFIG BEGIN --------------------------*/
 #define ROM_START              ((uint32_t)0x90000000)
 #define ROM_SIZE               (8192)
 #define ROM_END                ((uint32_t)(ROM_START + ROM_SIZE * 1024))

#define RAM1_START              (0x20000000)
#define RAM1_SIZE               (128)
#define RAM1_END                (RAM1_START + RAM1_SIZE * 1024)

#define RAM2_START              (0x24000000)
#define RAM2_SIZE               (1024)
#define RAM2_END                (RAM2_START + RAM2_SIZE * 1024)

//#define RAM3_START              (0x30000000)
//#define RAM3_SIZE               (128)
//#define RAM3_START              (0x30000000)
//#define RAM3_SIZE               (64)
#define RAM3_START              (0x30010000)
#define RAM3_SIZE               (64)
#define RAM3_END                (RAM3_START + RAM3_SIZE * 1024)

#define RAM4_START              (0x38000000)
#define RAM4_SIZE               (32)
#define RAM4_END                (RAM4_START + RAM4_SIZE * 1024)


/*-------------------------- ROM/RAM CONFIG END --------------------------*/

/*-------------------------- CLOCK CONFIG BEGIN --------------------------*/

#define BSP_CLOCK_SOURCE                  ("HSE")
#define BSP_CLOCK_SOURCE_FREQ_MHZ         ((int32_t)0)
#define BSP_CLOCK_SYSTEM_FREQ_MHZ         ((int32_t)280)

/*-------------------------- CLOCK CONFIG END --------------------------*/

/*-------------------------- UART CONFIG BEGIN --------------------------*/

/** After configuring corresponding UART or UART DMA, you can use it.
 *
 * STEP 1, define macro define related to the serial port opening based on the serial port number
 *                 such as     #define BSP_USING_UATR1
 *
 * STEP 2, according to the corresponding pin of serial port, define the related serial port information macro
 *                 such as     #define BSP_UART1_TX_PIN       "PA9"
 *                             #define BSP_UART1_RX_PIN       "PA10"
 *
 * STEP 3, if you want using SERIAL DMA, you must open it in the RT-Thread Settings.
 *                 RT-Thread Setting -> Components -> Device Drivers -> Serial Device Drivers -> Enable Serial DMA Mode
 *
 * STEP 4, according to serial port number to define serial port tx/rx DMA function in the board.h file
 *                 such as     #define BSP_UART1_RX_USING_DMA
 *
 */

#ifdef BSP_USING_UART1
#define BSP_UART1_TX_PIN       "PB14"
#define BSP_UART1_RX_PIN       "PB15"
#endif

#ifdef BSP_USING_UART2
#define BSP_UART2_TX_PIN       "PA2"
#define BSP_UART2_RX_PIN       "PA3"
#endif

#ifdef BSP_USING_UART3
#define BSP_UART3_TX_PIN       "PB10"
#define BSP_UART3_RX_PIN       "PB11"
#endif

#ifdef BSP_USING_UART4
#define BSP_UART4_TX_PIN       "PA0"
#define BSP_UART4_RX_PIN       "PA1"
#endif

#ifdef BSP_USING_UART6
#define BSP_UART6_TX_PIN       "PC6"
#define BSP_UART6_RX_PIN       "PC7"
#endif

#ifdef BSP_USING_UART8
#define BSP_UART8_TX_PIN       "PE1"
#define BSP_UART8_RX_PIN       "PE0"
#endif

#ifdef BSP_USING_UART10
#define BSP_UART10_TX_PIN       "PE3"
#define BSP_UART10_RX_PIN       "PE2"
#endif

#define EB_LED_DI1         25
#define EB_LED_DI2         26
#define EB_LED_DI3         27
#define EB_LED_DI4         28

#define EB_LED_DO1         29
#define EB_LED_DO2         30
#define EB_LED_DO3         31
#define EB_LED_DO4         32

#define EB_LED_RLY1        1
#define EB_LED_RLY2        2
#define EB_LED_LORA_TX     3
#define EB_LED_LORA_RX     4

#define EB_LED_CAN1_TX     17
#define EB_LED_CAN1_RX     18
#define EB_LED_CAN2_TX     19
#define EB_LED_CAN2_RX     20

#define EB_LED_RS232_1_TX  21
#define EB_LED_RS232_1_RX  22
#define EB_LED_RS232_2_TX  23
#define EB_LED_RS232_2_RX  24

#define EB_LED_RS485_1_TX  9
#define EB_LED_RS485_1_RX  10
#define EB_LED_RS485_2_TX  11
#define EB_LED_RS485_2_RX  12

#define EB_LED_RS485_3_TX  13
#define EB_LED_RS485_3_RX  14

#define EB_LED_CPU         15
#define EB_LED_ERROR       16
#define EB_LED_ETHERNET    5

void SystemClock_Config(void);
void SystemClock_ReConfig(uint8_t mode);

/*-------------------------- UART CONFIG END --------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */
