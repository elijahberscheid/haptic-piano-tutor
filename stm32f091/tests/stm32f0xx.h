#ifndef __STM32F0XX_H__
#define __STM32F0XX_H__

#include <stdint.h>

typedef struct
{
  uint32_t CR;         /*!< RCC clock control register,                                  Address offset: 0x00 */
  uint32_t CFGR;       /*!< RCC clock configuration register,                            Address offset: 0x04 */
  uint32_t CIR;        /*!< RCC clock interrupt register,                                Address offset: 0x08 */
  uint32_t APB2RSTR;   /*!< RCC APB2 peripheral reset register,                          Address offset: 0x0C */
  uint32_t APB1RSTR;   /*!< RCC APB1 peripheral reset register,                          Address offset: 0x10 */
  uint32_t AHBENR;     /*!< RCC AHB peripheral clock register,                           Address offset: 0x14 */
  uint32_t APB2ENR;    /*!< RCC APB2 peripheral clock enable register,                   Address offset: 0x18 */
  uint32_t APB1ENR;    /*!< RCC APB1 peripheral clock enable register,                   Address offset: 0x1C */
  uint32_t BDCR;       /*!< RCC Backup domain control register,                          Address offset: 0x20 */ 
  uint32_t CSR;        /*!< RCC clock control & status register,                         Address offset: 0x24 */
  uint32_t AHBRSTR;    /*!< RCC AHB peripheral reset register,                           Address offset: 0x28 */
  uint32_t CFGR2;      /*!< RCC clock configuration register 2,                          Address offset: 0x2C */
  uint32_t CFGR3;      /*!< RCC clock configuration register 3,                          Address offset: 0x30 */
  uint32_t CR2;        /*!< RCC clock control register 2,                                Address offset: 0x34 */
} RCC_TypeDef;

static RCC_TypeDef RCC_struct;
static RCC_TypeDef *RCC = &RCC_struct;

/*****************  Bit definition for RCC_APB2ENR register  ******************/
#define  RCC_APB2ENR_SYSCFGCOMPEN            ((uint32_t)0x00000001)        /*!< SYSCFG and comparator clock enable */
#define  RCC_APB2ENR_USART6EN                ((uint32_t)0x00000020)        /*!< USART6 clock enable */
#define  RCC_APB2ENR_USART7EN                ((uint32_t)0x00000040)        /*!< USART7 clock enable */
#define  RCC_APB2ENR_USART8EN                ((uint32_t)0x00000080)        /*!< USART8 clock enable */
#define  RCC_APB2ENR_ADCEN                   ((uint32_t)0x00000200)        /*!< ADC1 clock enable */
#define  RCC_APB2ENR_TIM1EN                  ((uint32_t)0x00000800)        /*!< TIM1 clock enable */
#define  RCC_APB2ENR_SPI1EN                  ((uint32_t)0x00001000)        /*!< SPI1 clock enable */
#define  RCC_APB2ENR_USART1EN                ((uint32_t)0x00004000)        /*!< USART1 clock enable */
#define  RCC_APB2ENR_TIM15EN                 ((uint32_t)0x00010000)        /*!< TIM15 clock enable */
#define  RCC_APB2ENR_TIM16EN                 ((uint32_t)0x00020000)        /*!< TIM16 clock enable */
#define  RCC_APB2ENR_TIM17EN                 ((uint32_t)0x00040000)        /*!< TIM17 clock enable */
#define  RCC_APB2ENR_DBGMCUEN                ((uint32_t)0x00400000)        /*!< DBGMCU clock enable */

/* Old Bit definition maintained for legacy purpose */
#define  RCC_APB2ENR_SYSCFGEN                RCC_APB2ENR_SYSCFGCOMPEN        /*!< SYSCFG clock enable */
#define  RCC_APB2ENR_ADC1EN                  RCC_APB2ENR_ADCEN               /*!< ADC1 clock enable */

typedef struct
{
  uint16_t CR1;             /*!< TIM control register 1,                      Address offset: 0x00 */
  uint16_t RESERVED0;       /*!< Reserved,                                                    0x02 */
  uint16_t CR2;             /*!< TIM control register 2,                      Address offset: 0x04 */
  uint16_t RESERVED1;       /*!< Reserved,                                                    0x06 */
  uint16_t SMCR;            /*!< TIM slave Mode Control register,             Address offset: 0x08 */
  uint16_t RESERVED2;       /*!< Reserved,                                                    0x0A */
  uint16_t DIER;            /*!< TIM DMA/interrupt enable register,           Address offset: 0x0C */
  uint16_t RESERVED3;       /*!< Reserved,                                                    0x0E */
  uint16_t SR;              /*!< TIM status register,                         Address offset: 0x10 */
  uint16_t RESERVED4;       /*!< Reserved,                                                    0x12 */
  uint16_t EGR;             /*!< TIM event generation register,               Address offset: 0x14 */
  uint16_t RESERVED5;       /*!< Reserved,                                                    0x16 */
  uint16_t CCMR1;           /*!< TIM  capture/compare mode register 1,        Address offset: 0x18 */
  uint16_t RESERVED6;       /*!< Reserved,                                                    0x1A */
  uint16_t CCMR2;           /*!< TIM  capture/compare mode register 2,        Address offset: 0x1C */
  uint16_t RESERVED7;       /*!< Reserved,                                                    0x1E */
  uint16_t CCER;            /*!< TIM capture/compare enable register,         Address offset: 0x20 */
  uint16_t RESERVED8;       /*!< Reserved,                                                    0x22 */
  uint32_t CNT;             /*!< TIM counter register,                        Address offset: 0x24 */
  uint16_t PSC;             /*!< TIM prescaler register,                      Address offset: 0x28 */
  uint16_t RESERVED10;      /*!< Reserved,                                                    0x2A */
  uint32_t ARR;             /*!< TIM auto-reload register,                    Address offset: 0x2C */
  uint16_t RCR;             /*!< TIM  repetition counter register,            Address offset: 0x30 */
  uint16_t RESERVED12;      /*!< Reserved,                                                    0x32 */
  uint32_t CCR1;            /*!< TIM capture/compare register 1,              Address offset: 0x34 */
  uint32_t CCR2;            /*!< TIM capture/compare register 2,              Address offset: 0x38 */
  uint32_t CCR3;            /*!< TIM capture/compare register 3,              Address offset: 0x3C */
  uint32_t CCR4;            /*!< TIM capture/compare register 4,              Address offset: 0x40 */
  uint16_t BDTR;            /*!< TIM break and dead-time register,            Address offset: 0x44 */
  uint16_t RESERVED17;      /*!< Reserved,                                                    0x26 */
  uint16_t DCR;             /*!< TIM DMA control register,                    Address offset: 0x48 */
  uint16_t RESERVED18;      /*!< Reserved,                                                    0x4A */
  uint16_t DMAR;            /*!< TIM DMA address for full transfer register,  Address offset: 0x4C */
  uint16_t RESERVED19;      /*!< Reserved,                                                    0x4E */
  uint16_t OR;              /*!< TIM option register,                         Address offset: 0x50 */
  uint16_t RESERVED20;      /*!< Reserved,                                                    0x52 */
} TIM_TypeDef;

static TIM_TypeDef TIM17_struct;
static TIM_TypeDef *TIM17 = &TIM17_struct;

/*******************  Bit definition for TIM_CR1 register  ********************/
#define  TIM_CR1_CEN                         ((uint16_t)0x0001)            /*!<Counter enable */
#define  TIM_CR1_UDIS                        ((uint16_t)0x0002)            /*!<Update disable */
#define  TIM_CR1_URS                         ((uint16_t)0x0004)            /*!<Update request source */
#define  TIM_CR1_OPM                         ((uint16_t)0x0008)            /*!<One pulse mode */
#define  TIM_CR1_DIR                         ((uint16_t)0x0010)            /*!<Direction */

#define  TIM_CR1_CMS                         ((uint16_t)0x0060)            /*!<CMS[1:0] bits (Center-aligned mode selection) */
#define  TIM_CR1_CMS_0                       ((uint16_t)0x0020)            /*!<Bit 0 */
#define  TIM_CR1_CMS_1                       ((uint16_t)0x0040)            /*!<Bit 1 */

#define  TIM_CR1_ARPE                        ((uint16_t)0x0080)            /*!<Auto-reload preload enable */

#define  TIM_CR1_CKD                         ((uint16_t)0x0300)            /*!<CKD[1:0] bits (clock division) */
#define  TIM_CR1_CKD_0                       ((uint16_t)0x0100)            /*!<Bit 0 */
#define  TIM_CR1_CKD_1                       ((uint16_t)0x0200)            /*!<Bit 1 */

/*******************  Bit definition for TIM_DIER register  *******************/
#define  TIM_DIER_UIE                        ((uint16_t)0x0001)            /*!<Update interrupt enable */
#define  TIM_DIER_CC1IE                      ((uint16_t)0x0002)            /*!<Capture/Compare 1 interrupt enable */
#define  TIM_DIER_CC2IE                      ((uint16_t)0x0004)            /*!<Capture/Compare 2 interrupt enable */
#define  TIM_DIER_CC3IE                      ((uint16_t)0x0008)            /*!<Capture/Compare 3 interrupt enable */
#define  TIM_DIER_CC4IE                      ((uint16_t)0x0010)            /*!<Capture/Compare 4 interrupt enable */
#define  TIM_DIER_COMIE                      ((uint16_t)0x0020)            /*!<COM interrupt enable */
#define  TIM_DIER_TIE                        ((uint16_t)0x0040)            /*!<Trigger interrupt enable */
#define  TIM_DIER_BIE                        ((uint16_t)0x0080)            /*!<Break interrupt enable */
#define  TIM_DIER_UDE                        ((uint16_t)0x0100)            /*!<Update DMA request enable */
#define  TIM_DIER_CC1DE                      ((uint16_t)0x0200)            /*!<Capture/Compare 1 DMA request enable */
#define  TIM_DIER_CC2DE                      ((uint16_t)0x0400)            /*!<Capture/Compare 2 DMA request enable */
#define  TIM_DIER_CC3DE                      ((uint16_t)0x0800)            /*!<Capture/Compare 3 DMA request enable */
#define  TIM_DIER_CC4DE                      ((uint16_t)0x1000)            /*!<Capture/Compare 4 DMA request enable */
#define  TIM_DIER_COMDE                      ((uint16_t)0x2000)            /*!<COM DMA request enable */
#define  TIM_DIER_TDE                        ((uint16_t)0x4000)            /*!<Trigger DMA request enable */

/********************  Bit definition for TIM_SR register  ********************/
#define  TIM_SR_UIF                          ((uint16_t)0x0001)            /*!<Update interrupt Flag */
#define  TIM_SR_CC1IF                        ((uint16_t)0x0002)            /*!<Capture/Compare 1 interrupt Flag */
#define  TIM_SR_CC2IF                        ((uint16_t)0x0004)            /*!<Capture/Compare 2 interrupt Flag */
#define  TIM_SR_CC3IF                        ((uint16_t)0x0008)            /*!<Capture/Compare 3 interrupt Flag */
#define  TIM_SR_CC4IF                        ((uint16_t)0x0010)            /*!<Capture/Compare 4 interrupt Flag */
#define  TIM_SR_COMIF                        ((uint16_t)0x0020)            /*!<COM interrupt Flag */
#define  TIM_SR_TIF                          ((uint16_t)0x0040)            /*!<Trigger interrupt Flag */
#define  TIM_SR_BIF                          ((uint16_t)0x0080)            /*!<Break interrupt Flag */
#define  TIM_SR_CC1OF                        ((uint16_t)0x0200)            /*!<Capture/Compare 1 Overcapture Flag */
#define  TIM_SR_CC2OF                        ((uint16_t)0x0400)            /*!<Capture/Compare 2 Overcapture Flag */
#define  TIM_SR_CC3OF                        ((uint16_t)0x0800)            /*!<Capture/Compare 3 Overcapture Flag */
#define  TIM_SR_CC4OF                        ((uint16_t)0x1000)            /*!<Capture/Compare 4 Overcapture Flag */

typedef struct
{
  uint32_t ISER[1];                 /*!< Offset: 0x000 (R/W)  Interrupt Set Enable Register           */
  uint32_t RESERVED0[31];
  uint32_t ICER[1];                 /*!< Offset: 0x080 (R/W)  Interrupt Clear Enable Register          */
  uint32_t RSERVED1[31];
  uint32_t ISPR[1];                 /*!< Offset: 0x100 (R/W)  Interrupt Set Pending Register           */
  uint32_t RESERVED2[31];
  uint32_t ICPR[1];                 /*!< Offset: 0x180 (R/W)  Interrupt Clear Pending Register         */
  uint32_t RESERVED3[31];
  uint32_t RESERVED4[64];
  uint32_t IP[8];                   /*!< Offset: 0x300 (R/W)  Interrupt Priority Register              */
}  NVIC_Type;

static NVIC_Type NVIC_struct;
static NVIC_Type *NVIC = &NVIC_struct;

typedef enum IRQn {
  WWDG_IRQn                   = 0,      /*!< Window WatchDog Interrupt                                       */
  PVD_VDDIO2_IRQn             = 1,      /*!< PVD & VDDIO2 Interrupts through EXTI Lines 16 and 31            */
  RTC_IRQn                    = 2,      /*!< RTC Interrupt through EXTI Lines 17, 19 and 20                  */
  FLASH_IRQn                  = 3,      /*!< FLASH global Interrupt                                          */
  RCC_CRS_IRQn                = 4,      /*!< RCC & CRS Global Interrupts                                     */
  EXTI0_1_IRQn                = 5,      /*!< EXTI Line 0 and 1 Interrupts                                    */
  EXTI2_3_IRQn                = 6,      /*!< EXTI Line 2 and 3 Interrupts                                    */
  EXTI4_15_IRQn               = 7,      /*!< EXTI Line 4 to 15 Interrupts                                    */
  TSC_IRQn                    = 8,      /*!< Touch Sensing Controller Interrupts                             */
  DMA1_Ch1_IRQn               = 9,      /*!< DMA1 Channel 1 Interrupt                                        */
  DMA1_Ch2_3_DMA2_Ch1_2_IRQn  = 10,     /*!< DMA1 Channel 2 and 3 & DMA2 Channel 1 and 2 Interrupts          */
  DMA1_Ch4_7_DMA2_Ch3_5_IRQn  = 11,     /*!< DMA1 Channel 4 to 7 & DMA2 Channel 3 to 5 Interrupts            */
  ADC1_COMP_IRQn               = 12,     /*!< ADC, COMP1 and COMP2 Interrupts (EXTI Lines 21 and 22)          */
  TIM1_BRK_UP_TRG_COM_IRQn    = 13,     /*!< TIM1 Break, Update, Trigger and Commutation Interrupts          */
  TIM1_CC_IRQn                = 14,     /*!< TIM1 Capture Compare Interrupt                                  */
  TIM2_IRQn                   = 15,     /*!< TIM2 global Interrupt                                           */
  TIM3_IRQn                   = 16,     /*!< TIM3 global Interrupt                                           */
  TIM6_DAC_IRQn               = 17,     /*!< TIM6 global and DAC channel underrun error Interrupts           */
  TIM7_IRQn                   = 18,     /*!< TIM7 global Interrupt                                           */
  TIM14_IRQn                  = 19,     /*!< TIM14 global Interrupt                                          */
  TIM15_IRQn                  = 20,     /*!< TIM15 global Interrupt                                          */
  TIM16_IRQn                  = 21,     /*!< TIM16 global Interrupt                                          */
  TIM17_IRQn                  = 22,     /*!< TIM17 global Interrupt                                          */
  I2C1_IRQn                   = 23,     /*!< I2C1 Event Interrupt & EXTI Line23 Interrupt (I2C1 wakeup)      */
  I2C2_IRQn                   = 24,     /*!< I2C2 Event Interrupt & EXTI Line24 Interrupt (I2C2 wakeup)      */
  SPI1_IRQn                   = 25,     /*!< SPI1 global Interrupt                                           */
  SPI2_IRQn                   = 26,     /*!< SPI2 global Interrupt                                           */
  USART1_IRQn                 = 27,     /*!< USART1 global Interrupt & EXTI Line25 Interrupt (USART1 wakeup) */
  USART2_IRQn                 = 28,     /*!< USART2 global Interrupt & EXTI Line26 Interrupt (USART2 wakeup) */
  USART3_8_IRQn               = 29,     /*!< USART3 to USART8 global Interrupts                              */
  CEC_CAN_IRQn                = 30      /*!< CEC and CAN global Interrupts & EXTI Line27 Interrupt           */
} IRQn_Type;

#endif
