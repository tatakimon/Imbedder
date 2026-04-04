/**
 ******************************************************************************
 * @file    STWIN.box_conf.h
 * @brief   STWIN.box configuration for Battery Monitor environment
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STWIN_BOX_CONF_H__
#define STWIN_BOX_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"
#include "STWIN.box_errno.h"

/* Battery Charger Configuration -----------------------------------------------*/
/* Define 1 to use already implemented callback; 0 to implement callback
   into an application file */
#define USE_BC_TIM_IRQ_CALLBACK         0U
#define USE_BC_GPIO_IRQ_HANDLER         1U
#define USE_BC_GPIO_IRQ_CALLBACK        1U

#ifdef __cplusplus
}
#endif

#endif /* STWIN_BOX_CONF_H__ */