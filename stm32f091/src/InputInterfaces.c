#include "stm32f0xx.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "InputInterfaces.h"
#include "modules/GlobalVariables.h"
#include "modules/Note.h"

#define MODER_INPUT (0x0)
#define MODER_OUTPUT (0x1)
#define MODER_AF (0x2)

#define PUPDR_PULLUP (0x1)
#define PUPDR_PULLDOWN (0x2)

enum {
    ClockFrequency = 48000000,
    Prescaler = 48,
    RxBufferLength = 64,
    DistanceArrayLength = 10,
    LastPositionUseCountThreshold = 3,
    // If any of these pin numbers change, double check that it still uses the correct ISR
    // and double check the setup
    IntPin = 10,
    PdnPin = 11,
    WakeupPin = 12,
    DebugButton0Pin = 0,
    DebugButton1Pin = 2,
    LeftButtonPin = 11,
    RightButtonPin = 12,
    ModeButtonPin = 9,
    TempoButtonPin = 8,
    StartButtonPin = 7,
    StopButtonPin = 6
    // The USART pins are not in this enum on purpose,
    // because changing the USART pins may also require the alternate function used and the USART channel used to change
};

// Example of configuration string:
// static const char macRequest[] = "TTM:MAC-?\r\n\0"; // the null terminator in this string does need to be sent

typedef struct {
    char *txMessage;
    uint8_t txMessageLength;
    uint8_t txIndex;
    char rxBuffer[RxBufferLength];
    volatile bool doneReceiving;
} Ble_t;

static Ble_t instance;

volatile static bool debugButton0Pressed = false;
volatile static bool debugButton1Pressed = false;
volatile static bool leftButtonPressed = false;
volatile static bool rightButtonPressed = false;
volatile static bool modeButtonPressed = false;
volatile static bool tempoButtonPressed = false;
volatile static bool startButtonPressed = false;
volatile static bool stopButtonPressed = false;

static Key_t lastPositions[DistanceArrayLength];
static uint8_t lastPositionUseCount[DistanceArrayLength];

static void SetGpioMode(GPIO_TypeDef *port, uint8_t pin, uint8_t mode) {
    port->MODER &= ~(3 << (pin * 2)); // clear MODER for pin
    port->MODER |= (mode & 3) << (pin * 2); // set MODER for pin
}

static void SetGpioPullUpPullDown(GPIO_TypeDef *port, uint8_t pin, uint8_t mode) {
    port->PUPDR &= ~(3 << (pin * 2)); // clear PUPDR for pin
    port->PUPDR |= (mode & 3) << (pin * 2); // set PUPDR for pin
}

static void SetGpioAlternateFunction(GPIO_TypeDef *port, uint8_t pin, uint8_t af) {
    uint8_t afrIndex = (pin >> 3) & 1; // determine if this pin is in AFRL or AFRH
    port->AFR[afrIndex] &= ~(0xF << ((pin & 0x7) * 4)); // clear AFR for pin
    port->AFR[afrIndex] |= (af & 0xF) << ((pin & 0x7) * 4); // set AFR for pin
}

static void IncrementSignal(uint8_t id) {
    uint8_t signal = 0;
    GlobalVariables_Read(id, &signal);
    signal++;
    GlobalVariables_Write(id, &signal);
}

static void Timer16_Init(uint32_t frequency) {
    TIM16->CR1 &= ~TIM_CR1_CEN; // make sure timer is off before configuring it
    RCC->APB2ENR |= RCC_APB2ENR_TIM16EN; // enable clock to timer
    TIM16->PSC = Prescaler - 1; // set prescaler
    if (frequency > ClockFrequency / Prescaler) { // set auto reload register
        printf("Requested frequency for TIM16 is too large, setting it to %d\n", ClockFrequency / Prescaler);
        TIM16->ARR = 1;
    }
    else {
        TIM16->ARR = (ClockFrequency / Prescaler / frequency) - 1;
    }
    TIM16->DIER |= TIM_DIER_UIE; // update interrupt enable
    NVIC->ISER[0] = 1 << TIM16_IRQn; // unmask in NVIC

    // enable the timer, then turn it off in its ISR
    // because the first time the timer is enabled, it instantly calls its ISR
    TIM16->CR1 |= TIM_CR1_CEN;
}

void USART3_4_5_6_7_8_IRQHandler(void) {
    // RX handled by DMA

    if (USART3->ISR & USART_ISR_TXE) { // if transmitter is empty
        if (instance.txIndex == instance.txMessageLength) {
            USART3->CR1 &= ~(USART_CR1_TXEIE); // turn off transmitter empty interrupt
            GPIOB->BSRR = 1 << WakeupPin; // wakeup high to sleep again
            // we do not need to wait until the ble module receives all the data to pull wakeup high
        } else {
            USART3->TDR = instance.txMessage[instance.txIndex];
            instance.txIndex++;
        }
    }
}

void TIM16_IRQHandler(void) {
    TIM16->SR &= ~TIM_SR_UIF; // acknowledge interrupt

    TIM16->CR1 &= ~TIM_CR1_CEN; // turn itself off
    USART3->CR1 |= USART_CR1_TXEIE; // interrupt when char is sent
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
    if (EXTI->PR & (1 << IntPin)) { // make sure it is the line that corresponds to IntPin that triggered the interrupt
        EXTI->PR = 1 << IntPin; // acknowledge interrupt
        if (GPIOB->IDR & (1 << IntPin)) { // done receiving
            instance.doneReceiving = true;
            DMA2_Channel2->CCR &= ~(DMA_CCR_EN); // disable DMA
        }
        else { // start receiving
            memset(instance.rxBuffer, 0, RxBufferLength * sizeof(*instance.rxBuffer));
            DMA2_Channel2->CNDTR = RxBufferLength; // reset number of data to transfer, so transfer starts at beginning of buffer
            DMA2_Channel2->CCR |= DMA_CCR_EN; // enable DMA
        }
    }

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

// using strlen() on message to find its length does not suffice,
// because configuration messages require a null terminator to be sent
void Ble_SendString(char *message, uint8_t length) {
    GPIOB->BRR = 1 << WakeupPin; // wakeup low to indicate start sending
    TIM16->CR1 |= TIM_CR1_CEN; // turn on timer for a delay of at least 1 ms before sending message

    instance.txMessage = message;
    instance.txMessageLength = length;
    instance.txIndex = 0;
}

void Ble_Run(void) {
    if (instance.doneReceiving) {
        instance.doneReceiving = false;
        if (strncmp("error code", instance.rxBuffer, strlen("error code")) == 0) {
            uint8_t errorCode = strtol(instance.rxBuffer + strlen("error code"), NULL, 10);
            GlobalVariables_Write(Global_ErrorCode, &errorCode);
            bool error = true;
            GlobalVariables_Write(Global_CalibrationError, &error);

            Key_t positions[] = {
                Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid,
                Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid
            };
            GlobalVariables_Write(Global_FingerPositions, positions);
        }
        else if (strncmp("TTM", instance.rxBuffer, strlen("TTM")) != 0) {
            // if the data received is not a configuration string, then it must be finger positions
            // if finger positions are received, then there is no error
            bool error = false;
            GlobalVariables_Write(Global_CalibrationError, &error);

            Key_t positions[DistanceArrayLength];
            GlobalVariables_Read(Global_FingerPositions, positions);
            for (uint8_t finger = 0; finger < DistanceArrayLength; finger++) {
                if (positions[finger] != Key_Invalid) {
                    lastPositions[finger] = positions[finger];
                    lastPositionUseCount[finger] = 0;
                }

                if ((instance.rxBuffer[finger] == Key_Invalid)
                        && (lastPositionUseCount[finger] < LastPositionUseCountThreshold)) {
                    positions[finger] = lastPositions[finger];
                    lastPositionUseCount[finger]++;
                }
                else {
                    positions[finger] = instance.rxBuffer[finger];
                }
            }

            GlobalVariables_Write(Global_FingerPositions, positions);
        }
    }
}

void Ble_Init(void) {
    instance.txMessage = "";
    instance.txMessageLength = 0;
    instance.txIndex = 0;
    memset(instance.rxBuffer, 0, RxBufferLength * sizeof(*instance.rxBuffer));

    for (uint8_t finger = 0; finger < DistanceArrayLength; finger++) {
        lastPositions[finger] = Key_Invalid;
        lastPositionUseCount[finger] = 0;
    }
    GlobalVariables_Write(Global_FingerPositions, lastPositions);

    // configure port B
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    SetGpioMode(GPIOB, IntPin, MODER_INPUT);
    SetGpioMode(GPIOB, PdnPin, MODER_OUTPUT);
    SetGpioMode(GPIOB, WakeupPin, MODER_OUTPUT);
    GPIOB->BRR = 1 << PdnPin; // PDN low to wake up module and start advertising
    GPIOB->BSRR = 1 << WakeupPin; // WAKEUP high to indicate not currently sending message

    // configure port C
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    SetGpioMode(GPIOC, 4, MODER_AF);
    SetGpioMode(GPIOC, 5, MODER_AF);
    SetGpioAlternateFunction(GPIOC, 4, 1); // PC4 to AF1
    SetGpioAlternateFunction(GPIOC, 5, 1); // PC5 to AF1

    // configure USART
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    USART3->CR1 &= ~(USART_CR1_UE); // disable USART so we can change its config
    USART3->CR1 &= ~((1 << 28) | (1 << 12)); // 1 start bit, 8 data bits
    USART3->CR2 &= ~(USART_CR2_STOP); // 1 stop bit
    USART3->CR1 &= ~(USART_CR1_PCE); // no parity bit
    USART3->CR1 &= ~(USART_CR1_OVER8); // 16 oversampling
    USART3->BRR = 0x1A1; // 115200 baud, value is from FRM 26.5.4
    USART3->CR3 |= USART_CR3_OVRDIS; // disable overrun flag, so previous data just gets overwritten
    USART3->CR1 |= USART_CR1_TE | USART_CR1_RE; // enable transmitter and receiver
    USART3->CR3 |= USART_CR3_DMAR; // enable receiver to trigger DMA
    USART3->CR1 |= USART_CR1_UE; // enable USART

    while (!((USART3->ISR & USART_ISR_TEACK) && (USART3->ISR & USART_ISR_REACK))); // wait for TE and RE to take effect

    USART3->CR1 |= USART_CR1_RXNEIE; // interrupt on receiving data
    NVIC->ISER[0] = (1 << USART3_8_IRQn); // unmask in NVIC

    // configure DMA to transfer data from USART receive register to buffer
    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2_Channel2->CCR &= ~(DMA_CCR_EN); // disable DMA before configuring
    DMA2_Channel2->CCR &= ~(DMA_CCR_MSIZE | DMA_CCR_PSIZE); // 8 bit word size
    DMA2_Channel2->CCR |= DMA_CCR_MINC; // increment memory address
    DMA2_Channel2->CCR &= ~(DMA_CCR_PINC); // do not increment peripheral address
    DMA2_Channel2->CCR &= ~(DMA_CCR_DIR); // transfer from peripheral to memory
    DMA2_Channel2->CNDTR = RxBufferLength; // number of data to transfer
    DMA2_Channel2->CPAR = (uint32_t) &(USART3->RDR);
    DMA2_Channel2->CMAR = (uint32_t) &instance.rxBuffer;
    DMA2->RMPCR |= DMA_RMPCR2_CH2_USART3_RX; // select USART3 Rx for channel 2, named DMA_CSELR in FRM

    Timer16_Init(500); // for sending data

    // configure EXTI to detect when we have started and finished receiving data
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
    SYSCFG->EXTICR[IntPin >> 2] &= ~(0xF << ((IntPin & 0x3) * 4));
    SYSCFG->EXTICR[IntPin >> 2] |= SYSCFG_EXTICR1_EXTI0_PB << ((IntPin & 0x3) * 4); // port B for interrupts
    EXTI->RTSR |= 1 << IntPin; // rising trigger
    EXTI->FTSR |= 1 << IntPin; // falling trigger
    EXTI->IMR |= 1 << IntPin; // unmask interrupt
    NVIC->ISER[0] = 1 << EXTI4_15_IRQn; // unmask in NVIC
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

