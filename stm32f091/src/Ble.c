#include "stm32f0xx.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "Ble.h"
#include "modules/GlobalVariables.h"

#define MODER_INPUT (0x0)
#define MODER_OUTPUT (0x1)
#define MODER_AF (0x2)

enum {
    ClockFrequency = 48000000,
    Prescaler = 48,
    RxBufferLength = 64,
    IntPin = 10,
    PdnPin = 11,
    WakeupPin = 12
    // The USART pins are not in this enum on purpose,
    // because changing the USART pins may also require the alternate function used and the USART peripheral used to change
};

// Example of configuration string:
// static const char macRequest[] = "TTM:MAC-?\r\n\0"; // the null terminator in this string does need to be sent

typedef struct {
    char *txMessage;
    uint8_t txMessageLength;
    uint8_t txIndex;
    char rxBuffer[RxBufferLength];
    uint8_t rxIndex;
    volatile bool doneReceiving;
} Ble_t;

static Ble_t instance;

static void SetGpioMode(GPIO_TypeDef *port, uint8_t pin, uint8_t mode) {
    port->MODER &= ~(3 << (pin * 2)); // clear MODER for pin
    port->MODER |= (mode & 3) << (pin * 2); // set MODER for pin
}

static void SetGpioAlternateFunction(GPIO_TypeDef *port, uint8_t pin, uint8_t af) {
    uint8_t afrIndex = (pin >> 3) & 1; // determine if this pin is in AFRL or AFRH
    port->AFR[afrIndex] &= ~(0xF << ((pin & 0x7) * 4)); // clear AFR for pin
    port->AFR[afrIndex] |= (af & 0xF) << ((pin & 0x7) * 4); // set AFR for pin
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
    if (USART3->ISR & USART_ISR_RXNE) { // if receiver is filled
        if (instance.rxIndex < RxBufferLength - 1) { // ignore data if buffer is full, leave space for null terminator
            instance.rxBuffer[instance.rxIndex] = USART3->RDR;
            instance.rxIndex++;
        }
    }

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

void EXTI4_15_IRQHandler(void) {
    if (EXTI->PR & (1 << IntPin)) { // make sure it is the line that corresponds to IntPin that triggered the interrupt
        EXTI->PR = 1 << IntPin; // acknowledge interrupt
        if (GPIOB->IDR & (1 << IntPin)) { // done receiving
            instance.doneReceiving = true;
        }
        else { // start receiving
            memset(instance.rxBuffer, 0, RxBufferLength * sizeof(*instance.rxBuffer));
            instance.rxIndex = 0;
        }

    }
}

// using strlen() on message to find its length does not suffice,
// because configuration messages require a null terminator to be sent
void Ble_SendString(char *message, uint8_t length) {
    GPIOB->BRR = 1 << 12; // wakeup
    TIM16->CR1 |= TIM_CR1_CEN; // turn on timer for a delay of at least 1 ms before sending message

    instance.txMessage = message;
    instance.txMessageLength = length;
    instance.txIndex = 0;
}

void Ble_Run(void) {
    if (instance.doneReceiving) {
        instance.doneReceiving = false;
        GlobalVariables_Write(Global_FingerPositions, &instance.rxBuffer);
        // TODO: remove printing because it's for development only
        if (instance.rxBuffer[0] > 31) { // if the first character is printable, print the whole buffer
            printf("Received from BLE: %s\n", instance.rxBuffer);
        }
    }
}

void Ble_Init(void) {
    instance.txMessage = "";
    instance.txMessageLength = 0;
    instance.txIndex = 0;
    memset(instance.rxBuffer, 0, RxBufferLength * sizeof(*instance.rxBuffer));
    instance.rxIndex = 0;

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
    USART3->CR1 |= USART_CR1_TE | USART_CR1_RE; // enable transmitter and receiver
    USART3->CR1 |= USART_CR1_UE; // enable USART

    while (!((USART3->ISR & USART_ISR_TEACK) && (USART3->ISR & USART_ISR_REACK))); // wait for TE and RE to take effect

    USART3->CR1 |= USART_CR1_RXNEIE; // interrupt on receiving data
    NVIC->ISER[0] = (1 << USART3_8_IRQn); // unmask in NVIC

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
