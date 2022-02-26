#include "stm32f0xx.h"
#include "ButtonInterface.h"
#include "modules/GlobalVariables.h"
#include <stdio.h>

#define MODER_INPUT (0x0)
#define MODER_OUTPUT (0x1)
#define MODER_AF (0x2)

#define PUPDR_PULLUP (0x1)
#define PUPDR_PULLDOWN (0x2)

volatile static bool debugButtonPressed;
volatile static bool rightButtonPressed;
volatile static bool startButtonPressed;

enum {
    // if any of these pin numbers change, double check that it still uses the correct ISR
    // and double check the setup
    DebugButtonPin = 2,
    RightButtonPin = 6,
    StartButtonPin = 7
};

static void SetGpioMode(GPIO_TypeDef *port, uint8_t pin, uint8_t mode) {
    port->MODER &= ~(3 << (pin * 2)); // clear MODER for pin
    port->MODER |= (mode & 3) << (pin * 2); // set MODER for pin
}

static void SetGpioPullUpPullDown(GPIO_TypeDef *port, uint8_t pin, uint8_t mode) {
    port->PUPDR &= ~(3 << (pin * 2)); // clear PUPDR for pin
    port->PUPDR |= (mode & 3) << (pin * 2); // set PUPDR for pin
}

void EXTI2_3_IRQHandler(void) {
    EXTI->PR = 1 << DebugButtonPin;
    debugButtonPressed = 1;
}

void EXTI4_15_IRQHandler(void) {
    if (EXTI->PR & (1 << RightButtonPin)) {
        EXTI->PR = 1 << RightButtonPin; // acknowledge interrupt
        if (GPIOC->IDR & (1 << RightButtonPin)) {
            // sometimes the pending bit for this button is mysteriously set when the other button is pressed
            rightButtonPressed = 1;
        }
    }

    if (EXTI->PR & (1 << StartButtonPin)) {
        EXTI->PR = 1 << StartButtonPin; // acknowledge interrupt
        if (GPIOC->IDR & (1 << StartButtonPin)) {
            // sometimes the pending bit for this button is mysteriously set when the other button is pressed
            startButtonPressed = 1;
        }
    }
}

void ButtonInterface_Run(void) {
    uint8_t signal = 0;

    if (debugButtonPressed == 1) {
        GlobalVariables_Read(Global_DebugButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_DebugButtonSignal, &signal);
        debugButtonPressed = 0;
    }

    if (rightButtonPressed == 1) {
        GlobalVariables_Read(Global_RightButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_RightButtonSignal, &signal);
        rightButtonPressed = 0;
    }

    if (startButtonPressed == 1) {
        GlobalVariables_Read(Global_StartButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_StartButtonSignal, &signal);
        startButtonPressed = 0;
    }
}

void ButtonInterface_Init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    SetGpioMode(GPIOB, DebugButtonPin, MODER_INPUT);
    SetGpioPullUpPullDown(GPIOB, DebugButtonPin, PUPDR_PULLDOWN);

    SetGpioMode(GPIOB, RightButtonPin, MODER_INPUT);
    SetGpioMode(GPIOB, StartButtonPin, MODER_INPUT);

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
    SYSCFG->EXTICR[0] &= ~(0xf0);
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PB; // PB2 for interrupts
    SYSCFG->EXTICR[1] &= ~(0xff00);
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR1_EXTI2_PC | SYSCFG_EXTICR1_EXTI3_PC; // PC6, PC7 for interrupts

    EXTI->RTSR |= 1 << DebugButtonPin | 1 << RightButtonPin | 1 << StartButtonPin; // rising trigger
    EXTI->IMR |= 1 << DebugButtonPin | 1 << RightButtonPin | 1 << StartButtonPin; // unmask
    NVIC->ISER[0] = 1 << EXTI2_3_IRQn | 1 << EXTI4_15_IRQn; // unmask in NVIC
}
