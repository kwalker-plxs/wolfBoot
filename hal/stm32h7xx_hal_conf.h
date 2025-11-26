#ifndef __STM32H7xx_HAL_CONF_H
#define __STM32H7xx_HAL_CONF_H

#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED

/* Many ST examples define these in stm32h7xx_hal_conf.h */
#ifndef TICK_INT_PRIORITY
#define TICK_INT_PRIORITY 0U
#endif
#if !defined(HSI_VALUE)
#define HSI_VALUE \
  ((uint32_t)16000000U) /*!< Value of the Internal oscillator in Hz*/
#endif                  /* HSI_VALUE */
#if !defined(HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT \
  ((uint32_t)100U) /*!< Time out for HSE start up, in ms */
#endif             /* HSE_STARTUP_TIMEOUT */
#if !defined(LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT 5000U /*!< Time out for LSE start up, in ms */
#endif                            /* HSE_STARTUP_TIMEOUT */
#if !defined(CSI_VALUE)
#define CSI_VALUE (4000000UL) /*!< Value of the Internal oscillator in Hz*/
#endif                        /* CSI_VALUE */
#if !defined(HSE_VALUE)
#define HSE_VALUE                                                            \
  (25000000UL) /*!< Value of the External oscillator in Hz : FPGA case fixed \
                  to 60MHZ */
#endif         /* HSE_VALUE */

#include "stm32h7xx_hal_def.h"

/* Pull in headers for the enabled modules */
#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32h7xx_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */
#if defined(HAL_CORTEX_MODULE_ENABLED)
#include "stm32h7xx_hal_cortex.h"
#endif
#if defined(HAL_FLASH_MODULE_ENABLED)
#include "stm32h7xx_hal_flash.h"
#include "stm32h7xx_hal_flash_ex.h"
#endif
#if defined(HAL_RCC_MODULE_ENABLED)
#include "stm32h7xx_hal_rcc.h"
#endif
#if defined(HAL_PWR_MODULE_ENABLED)
#include "stm32h7xx_hal_pwr.h"
#include "stm32h7xx_hal_pwr_ex.h"
#endif

#define assert_param(expr) ((void)0U)

#define STM32H7_WORD_SIZE_BYTES 32

#endif /* __STM32H7xx_HAL_CONF_H */
