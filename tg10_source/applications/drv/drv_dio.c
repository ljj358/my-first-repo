#include "drv_dio.h"

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "../app_bringup_diag.h"

static const uint32_t _gaulDiPins[] = {
    GET_PIN(C, 6), ///< TIM8_CH1
    GET_PIN(C, 7), ///< TIM8_CH2
    GET_PIN(B, 6), ///< TIM4_CH1
    GET_PIN(B, 7)  ///< TIM4_CH2
};

static const uint32_t _gaulDoPins[] = {
    GET_PIN(E, 5), ///< TIM15_CH1
    GET_PIN(B, 9), ///< TIM17_CH1
    GET_PIN(B, 1), ///< TIM3_CH4
    GET_PIN(A, 6)  ///< TIM13_CH1
};

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim13;
TIM_HandleTypeDef htim15;
TIM_HandleTypeDef htim17;

static const TIM_HandleTypeDef *_gatPwmTims[] = {
    &htim15, ///< TIM15_CH1
    &htim17, ///< TIM17_CH1
    &htim3,  ///< TIM3_CH4
    &htim13  ///< TIM13_CH1
};

static const uint32_t _gatPwmChnls[] = {
    TIM_CHANNEL_1, ///< TIM15_CH1
    TIM_CHANNEL_1, ///< TIM17_CH1
    TIM_CHANNEL_4, ///< TIM3_CH4
    TIM_CHANNEL_1  ///< TIM13_CH1
};

#if APP_BRINGUP_ENABLE_BEEP_RELAY
static const uint32_t _gaulRelayPins[] = {
    GET_PIN(E, 7),
    GET_PIN(C, 0)
};
#endif

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(uint32_t ulPeriod, uint32_t ulPulse)
{

  /* USER CODE BEGIN TIM3_Init 0 */
    if (ulPeriod > 0xFFFF) {
        ulPeriod = 0xFFFF;
    }
    if (ulPulse > 0xFFFF) {
        ulPulse = 0xFFFF;
    }
  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 279;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = ulPeriod - 1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = ulPulse - 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM13 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM13_Init(uint32_t ulPeriod, uint32_t ulPulse)
{

  /* USER CODE BEGIN TIM13_Init 0 */

  /* USER CODE END TIM13_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM13_Init 1 */

  /* USER CODE END TIM13_Init 1 */
  htim13.Instance = TIM13;
  htim13.Init.Prescaler = 279;
  htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim13.Init.Period = ulPeriod - 1;
  htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim13.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim13) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = ulPulse - 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim13, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM13_Init 2 */

  /* USER CODE END TIM13_Init 2 */
  HAL_TIM_MspPostInit(&htim13);

}

/**
  * @brief TIM15 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM15_Init(uint32_t ulPeriod, uint32_t ulPulse)
{

  /* USER CODE BEGIN TIM15_Init 0 */

  /* USER CODE END TIM15_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM15_Init 1 */

  /* USER CODE END TIM15_Init 1 */
  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 279;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = ulPeriod - 1;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = ulPulse - 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim15, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim15, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM15_Init 2 */

  /* USER CODE END TIM15_Init 2 */
  HAL_TIM_MspPostInit(&htim15);

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(uint32_t ulPeriod, uint32_t ulPulse)
{

  /* USER CODE BEGIN TIM17_Init 0 */
    if (ulPeriod > 0xFFFF) {
        ulPeriod = 0xFFFF;
    }
    if (ulPulse > 0xFFFF) {
        ulPulse = 0xFFFF;
    }

  /* USER CODE END TIM17_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 279;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = ulPeriod - 1;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = ulPulse - 1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim17, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim17, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */
  HAL_TIM_MspPostInit(&htim17);
}

static int dioInit(void)
{
    for (int i = 0; i < sizeof(_gaulDiPins) / sizeof(_gaulDiPins[0]); ++i) {
        rt_pin_mode(_gaulDiPins[i], PIN_MODE_INPUT_PULLUP);
    }
    for (int i = 0; i < sizeof(_gaulDoPins) / sizeof(_gaulDoPins[0]); ++i) {
        rt_pin_mode(_gaulDoPins[i], PIN_MODE_OUTPUT);
    }
#if APP_BRINGUP_ENABLE_BEEP_RELAY
    for (int i = 0; i < sizeof(_gaulRelayPins) / sizeof(_gaulRelayPins[0]); ++i) {
        rt_pin_mode(_gaulRelayPins[i], PIN_MODE_OUTPUT);
    }
#endif

    return 0;
}
INIT_ENV_EXPORT(dioInit);

int dioPwmSet(int chnl, uint32_t ulPeriod, uint32_t ulPulse)
{
    if (!((chnl >= DIO_CHNL_DO1) && (chnl <= DIO_CHNL_DO4))) {
        return 0;
    }

    rt_kprintf("dio set pwm %d %d %d", chnl, ulPeriod, ulPulse);

    switch (chnl) {
    case DIO_CHNL_DO1:
        HAL_TIM_PWM_MspInit(&htim15);
        MX_TIM15_Init(ulPeriod, ulPulse);
        HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1);
        break;
    case DIO_CHNL_DO2:
        HAL_TIM_PWM_MspInit(&htim17);
        MX_TIM17_Init(ulPeriod, ulPulse);
        HAL_TIM_PWM_Start(&htim17, TIM_CHANNEL_1);
        break;
    case DIO_CHNL_DO3:
        HAL_TIM_PWM_MspInit(&htim3);
        MX_TIM3_Init(ulPeriod, ulPulse);
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
        break;
    case DIO_CHNL_DO4:
        HAL_TIM_PWM_MspInit(&htim13);
        MX_TIM13_Init(ulPeriod, ulPulse);
        HAL_TIM_PWM_Start(&htim13, TIM_CHANNEL_1);
        break;
    default:
        break;
    }

    return 1;
}

int dioChannelGet(int chnl)
{
    if ((chnl >= DIO_CHNL_DI1) && (chnl <= DIO_CHNL_DI4)) {
        return rt_pin_read(_gaulDiPins[chnl]);
    }

    if ((chnl >= DIO_CHNL_DO1) && (chnl <= DIO_CHNL_DO4)) {
        return rt_pin_read(_gaulDoPins[chnl - DIO_CHNL_DO1]);
    }

#if APP_BRINGUP_ENABLE_BEEP_RELAY
    if ((chnl >= DIO_CHNL_RL1) && (chnl <= DIO_CHNL_RL2)) {
        return rt_pin_read(_gaulRelayPins[chnl - DIO_CHNL_RL1]);
    }
#endif

    return 0;
}

void dioChannelSet(int chnl, int val)
{
    if ((chnl >= DIO_CHNL_DO1) && (chnl <= DIO_CHNL_DO4)) {
        rt_pin_write(_gaulDoPins[chnl - DIO_CHNL_DO1], val ? PIN_HIGH : PIN_LOW);
#if APP_BRINGUP_ENABLE_BEEP_RELAY
    } else if ((chnl >= DIO_CHNL_RL1) && (chnl <= DIO_CHNL_RL2)) {
        rt_pin_write(_gaulRelayPins[chnl - DIO_CHNL_RL1], val ? PIN_HIGH : PIN_LOW);
#endif
    }
}

uint32_t dioStateGet(void)
{
    uint32_t ulRet = 0;
    for (int i = DIO_CHNL_DI1; i < DIO_CHNL_MAX; ++i) {
        if (dioChannelGet(i)) {
            ulRet |= (1 << i);
        }
    }

    return ulRet;
}

void diStateChange(int chnl, int sta)
{
    static uint8_t aucSta[4] = {0};
    if ((chnl >= DIO_CHNL_DI1) && (chnl <= DIO_CHNL_DI4)) {
        if (aucSta[chnl - DIO_CHNL_DI1] != sta) {
            aucSta[chnl - DIO_CHNL_DI1] = sta;
        }
    }
}

void dioUpdateLedState(void)
{
    int i;
    static rt_uint32_t ulDioState = 0;
    rt_uint32_t ulRet;
    ulRet = dioStateGet();

    if (ulDioState != ulRet) {
        ulDioState = ulRet;
//        rt_kprintf("DIO state:%04X\n", ulRet);
        for (i = DIO_CHNL_DI1; i <= DIO_CHNL_DI4; ++i) {
            if (ulRet & (1 << i)) {
                eb_led_on(easyblinkGetLed(EB_LED_DI1 + i - DIO_CHNL_DI1));
                diStateChange(i, 1);
            } else {
                eb_led_off(easyblinkGetLed(EB_LED_DI1 + i - DIO_CHNL_DI1));
                diStateChange(i, 0);
            }
        }
        for (i = DIO_CHNL_DO1; i <= DIO_CHNL_DO4; ++i) {
            if (ulRet & (1 << i)) {
                eb_led_on(easyblinkGetLed(EB_LED_DO1 + i - DIO_CHNL_DO1));
            } else {
                eb_led_off(easyblinkGetLed(EB_LED_DO1 + i - DIO_CHNL_DO1));
            }
        }
        for (i = DIO_CHNL_RL1; i <= DIO_CHNL_RL2; ++i) {
            if (ulRet & (1 << i)) {
                eb_led_on(easyblinkGetLed(EB_LED_RLY1 + i - DIO_CHNL_RL1));
            } else {
                eb_led_off(easyblinkGetLed(EB_LED_RLY1 + i - DIO_CHNL_RL1));
            }
        }
        easyblink_refresh();
    }
}

#ifdef FINSH_USING_MSH
static int __doSet(uint8_t argc, char **argv)
{
    if (argc != 3) {
        rt_kprintf("example:led_set digit value\n");
        return 0;
    }

    if (atoi(argv[2]) != 0) {
        dioChannelSet(atoi(argv[1]) - 1 + DIO_CHNL_DO1, 1);
    } else {
        dioChannelSet(atoi(argv[1]) - 1 + DIO_CHNL_DO1, 0);
    }

    return 0;
}
MSH_CMD_EXPORT_ALIAS(__doSet, do_set, set do state);
#endif
