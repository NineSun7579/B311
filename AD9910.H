#ifndef __AD9910_H__
#define __AD9910_H__

#include "stm32h7xx_hal.h"

#define uchar unsigned char
#define uint  unsigned int
#define ulong unsigned long int

/* AD9910 Pin Definitions (STM32H750VBT6) */
#define AD9910_PWR_PIN       GPIO_PIN_0
#define AD9910_PWR_PORT      GPIOA
#define AD9910_SDIO_PIN      GPIO_PIN_1
#define AD9910_SDIO_PORT     GPIOA
#define DRHOLD_PIN           GPIO_PIN_4
#define DRHOLD_PORT          GPIOA
#define DROVER_PIN           GPIO_PIN_5
#define DROVER_PORT          GPIOA
#define UP_DAT_PIN           GPIO_PIN_7
#define UP_DAT_PORT          GPIOA
#define PROFILE1_PIN         GPIO_PIN_11
#define PROFILE1_PORT        GPIOA
#define MAS_REST_PIN         GPIO_PIN_12
#define MAS_REST_PORT        GPIOA
#define SCLK_PIN             GPIO_PIN_13
#define SCLK_PORT            GPIOB
#define DRCTL_PIN            GPIO_PIN_4
#define DRCTL_PORT           GPIOB
#define OSK_PIN              GPIO_PIN_7
#define OSK_PORT             GPIOB
#define PROFILE0_PIN         GPIO_PIN_8
#define PROFILE0_PORT        GPIOB
#define PROFILE2_PIN         GPIO_PIN_9
#define PROFILE2_PORT        GPIOB
#define CS_PIN               GPIO_PIN_12
#define CS_PORT              GPIOB

/* Fast GPIO macros via BSRR */
#define AD9910_PWR(x)   ((x) ? (AD9910_PWR_PORT->BSRR  = AD9910_PWR_PIN)  : (AD9910_PWR_PORT->BSRR  = (uint32_t)AD9910_PWR_PIN  << 16))
#define AD9910_SDIO(x)  ((x) ? (AD9910_SDIO_PORT->BSRR = AD9910_SDIO_PIN) : (AD9910_SDIO_PORT->BSRR = (uint32_t)AD9910_SDIO_PIN << 16))
#define DRHOLD(x)       ((x) ? (DRHOLD_PORT->BSRR      = DRHOLD_PIN)      : (DRHOLD_PORT->BSRR      = (uint32_t)DRHOLD_PIN      << 16))
#define DROVER()        ((DROVER_PORT->IDR & DROVER_PIN) ? 1 : 0)
#define UP_DAT(x)       ((x) ? (UP_DAT_PORT->BSRR      = UP_DAT_PIN)      : (UP_DAT_PORT->BSRR      = (uint32_t)UP_DAT_PIN      << 16))
#define PROFILE1(x)     ((x) ? (PROFILE1_PORT->BSRR    = PROFILE1_PIN)    : (PROFILE1_PORT->BSRR    = (uint32_t)PROFILE1_PIN    << 16))
#define MAS_REST(x)     ((x) ? (MAS_REST_PORT->BSRR    = MAS_REST_PIN)    : (MAS_REST_PORT->BSRR    = (uint32_t)MAS_REST_PIN    << 16))
#define SCLK(x)         ((x) ? (SCLK_PORT->BSRR        = SCLK_PIN)        : (SCLK_PORT->BSRR        = (uint32_t)SCLK_PIN        << 16))
#define DRCTL(x)        ((x) ? (DRCTL_PORT->BSRR       = DRCTL_PIN)       : (DRCTL_PORT->BSRR       = (uint32_t)DRCTL_PIN       << 16))
#define OSK(x)          ((x) ? (OSK_PORT->BSRR         = OSK_PIN)         : (OSK_PORT->BSRR         = (uint32_t)OSK_PIN         << 16))
#define PROFILE0(x)     ((x) ? (PROFILE0_PORT->BSRR    = PROFILE0_PIN)    : (PROFILE0_PORT->BSRR    = (uint32_t)PROFILE0_PIN    << 16))
#define PROFILE2(x)     ((x) ? (PROFILE2_PORT->BSRR    = PROFILE2_PIN)    : (PROFILE2_PORT->BSRR    = (uint32_t)PROFILE2_PIN    << 16))
#define CS(x)           ((x) ? (CS_PORT->BSRR          = CS_PIN)          : (CS_PORT->BSRR          = (uint32_t)CS_PIN          << 16))

void AD9910_IOInit(void);
void Init_AD9910(void);
void AD9910_FreWrite(ulong Freq);
void AD9910_AmpWrite(uint16_t Amp);

#endif
