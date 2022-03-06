#include "stm32f0xx.h"
#include "ButtonInterface.h"
#include "modules/GlobalVariables.h"

#define MODER_INPUT (0x0)
#define MODER_OUTPUT (0x1)
#define MODER_AF (0x2)

#define PUPDR_PULLUP (0x1)
#define PUPDR_PULLDOWN (0x2)

volatile static bool debugButton0Pressed = false;
volatile static bool debugButton1Pressed = false;
volatile static bool leftButtonPressed = false;
volatile static bool rightButtonPressed = false;
volatile static bool modeButtonPressed = false;
volatile static bool tempoButtonPressed = false;
volatile static bool startButtonPressed = false;
volatile static bool stopButtonPressed = false;

enum {
    // if any of these pin numbers change, double check that it still uses the correct ISR
    // and double check the setup
    DebugButton0Pin = 0,
    DebugButton1Pin = 2,
    LeftButtonPin = 11,
    RightButtonPin = 12,
    ModeButtonPin = 9,
    TempoButtonPin = 8,
    StartButtonPin = 7,
    StopButtonPin = 6
};

static void SetGpioMode(GPIO_TypeDef *port, uint8_t pin, uint8_t mode) {
    port->MODER &= ~(3 << (pin * 2)); // clear MODER for pin
    port->MODER |= (mode & 3) << (pin * 2); // set MODER for pin
}

static void SetGpioPullUpPullDown(GPIO_TypeDef *port, uint8_t pin, uint8_t mode) {
    port->PUPDR &= ~(3 << (pin * 2)); // clear PUPDR for pin
    port->PUPDR |= (mode & 3) << (pin * 2); // set PUPDR for pin
}

static void IncrementSignal(uint8_t id) {
    uint8_t signal = 0;
    GlobalVariables_Read(id, &signal);
    signal++;
    GlobalVariables_Write(id, &signal);
}

void EXTI0_1_IRQHandler(void) {
    EXTI->PR = 1 << DebugButton0Pin;
    debugButton0Pressed = true;
}

void EXTI2_3_IRQHandler(void) {
    EXTI->PR = 1 << DebugButton1Pin;
    debugButton1Pressed = true;
}

void EXTI4_15_IRQHandler(void) {
    // sometimes the pending bit for buttons are mysteriously set when other buttons are pressed,
    // so read from IDR to check if a button has really been pressed

    if (EXTI->PR & (1 << LeftButtonPin)) {
        EXTI->PR = 1 << LeftButtonPin; // acknowledge interrupt
        if (GPIOC->IDR & (1 << LeftButtonPin)) {
           leftButtonPressed = true;
        }
    }

    if (EXTI->PR & (1 << RightButtonPin)) {
        EXTI->PR = 1 << RightButtonPin; // acknowledge interrupt
        if (GPIOC->IDR & (1 << RightButtonPin)) {
            rightButtonPressed = true;
        }
    }

    if (EXTI->PR & (1 << ModeButtonPin)) {
        EXTI->PR = 1 << ModeButtonPin; // acknowledge interrupt
        if (GPIOC->IDR & (1 << ModeButtonPin)) {
            modeButtonPressed = true;
        }
    }

    if (EXTI->PR & (1 << TempoButtonPin)) {
        EXTI->PR = 1 << TempoButtonPin; // acknowledge interrupt
        if (GPIOC->IDR & (1 << TempoButtonPin)) {
            tempoButtonPressed = true;
        }
    }

    if (EXTI->PR & (1 << StartButtonPin)) {
        EXTI->PR = 1 << StartButtonPin; // acknowledge interrupt
        if (GPIOC->IDR & (1 << StartButtonPin)) {
            startButtonPressed = true;
        }
    }

    if (EXTI->PR & (1 << StopButtonPin)) {
        EXTI->PR = 1 << StopButtonPin; // acknowledge interrupt
        if (GPIOC->IDR & (1 << StopButtonPin)) {
            stopButtonPressed = true;
        }
    }
}

void ButtonInterface_Run(void) {
    if (debugButton0Pressed) {
        IncrementSignal(Global_DebugButton0Signal);
        debugButton0Pressed = false;
    }

    if (debugButton1Pressed) {
        IncrementSignal(Global_DebugButton1Signal);
        debugButton1Pressed = false;
    }

    if (leftButtonPressed) {
        IncrementSignal(Global_LeftButtonSignal);
        leftButtonPressed = false;
    }

    if (rightButtonPressed) {
        IncrementSignal(Global_RightButtonSignal);
        rightButtonPressed = false;
    }

    if (modeButtonPressed) {
        IncrementSignal(Global_ModeButtonSignal);
        modeButtonPressed = false;
    }

    if (tempoButtonPressed) {
        IncrementSignal(Global_TempoButtonSignal);
        tempoButtonPressed = false;
    }

    if (startButtonPressed) {
        IncrementSignal(Global_StartButtonSignal);
        startButtonPressed = false;
    }

    if (stopButtonPressed) {
        IncrementSignal(Global_StopButtonSignal);
        stopButtonPressed = false;
    }
}

void ButtonInterface_Init(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    SetGpioMode(GPIOA, DebugButton0Pin, MODER_INPUT);
    SetGpioPullUpPullDown(GPIOA, DebugButton0Pin, PUPDR_PULLDOWN);
    SetGpioMode(GPIOB, DebugButton1Pin, MODER_INPUT);
    SetGpioPullUpPullDown(GPIOB, DebugButton1Pin, PUPDR_PULLDOWN);

    SetGpioMode(GPIOC, LeftButtonPin, MODER_INPUT);
    SetGpioMode(GPIOC, RightButtonPin, MODER_INPUT);
    SetGpioMode(GPIOC, ModeButtonPin, MODER_INPUT);
    SetGpioMode(GPIOC, TempoButtonPin, MODER_INPUT);
    SetGpioMode(GPIOC, StartButtonPin, MODER_INPUT);
    SetGpioMode(GPIOC, StopButtonPin, MODER_INPUT);

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
    SYSCFG->EXTICR[0] &= ~(0x0f0f);
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA | SYSCFG_EXTICR1_EXTI2_PB; // PA0, PB2 for interrupts
    SYSCFG->EXTICR[1] &= ~(0xff00);
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR1_EXTI2_PC | SYSCFG_EXTICR1_EXTI3_PC; // PC6, PC7 for interrupts
    // PC8, PC9, PC11 for interrupts
    SYSCFG->EXTICR[2] &= ~(0xf0ff);
    SYSCFG->EXTICR[2] |= SYSCFG_EXTICR1_EXTI0_PC | SYSCFG_EXTICR1_EXTI1_PC | SYSCFG_EXTICR1_EXTI3_PC;
    SYSCFG->EXTICR[3] &= ~(0x000f);
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR1_EXTI0_PC; // PC12 for interrupts

    EXTI->RTSR |= 1 << DebugButton0Pin | 1 << DebugButton1Pin
            | 1 << LeftButtonPin | 1 << RightButtonPin
            | 1 << ModeButtonPin | 1 << TempoButtonPin
            | 1 << StartButtonPin | 1 << StopButtonPin; // rising trigger
    EXTI->IMR |= 1 << DebugButton0Pin | 1 << DebugButton1Pin
            | 1 << LeftButtonPin | 1 << RightButtonPin
            | 1 << ModeButtonPin | 1 << TempoButtonPin
            | 1 << StartButtonPin | 1 << StopButtonPin; // unmask
    NVIC->ISER[0] = 1 << EXTI0_1_IRQn | 1 << EXTI2_3_IRQn | 1 << EXTI4_15_IRQn; // unmask in NVIC
}
