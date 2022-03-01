/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/
// AUDIO TEST

#include "stm32f0xx.h"
#include <math.h>
#include <stdio.h> // for printf()

void setup_output(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    //Setup ADCIN1-2 to analog operation
    GPIOC->MODER |= GPIO_MODER_MODER6_0;
    GPIOC->MODER |= GPIO_MODER_MODER7_0;
    GPIOC->MODER |= GPIO_MODER_MODER8_0;
    GPIOC->MODER |= GPIO_MODER_MODER9_0;
}

//============================================================================
// setup_adc()    (Autotest #1)
// Configure the ADC peripheral and analog input pins.
// Parameters: none
//============================================================================
void setup_adc(void)
{
    //Enable GPIO C
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    //Setup ADCIN1-2 to analog operation
    GPIOC->MODER |= GPIO_MODER_MODER0;
    //Enable ADC Clock
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    //Turn on 14 mhz clock
    RCC->CR2 |= RCC_CR2_HSI14ON;
    //Wait for clk ready
    while(!(RCC->CR2 & RCC_CR2_HSI14RDY));
    //Enable ADC (ADEN IN CR)
    ADC1->CR |= ADC_CR_ADEN;
    //Wait for ADC To be Ready
    while(!(ADC1->ISR & ADC_ISR_ADRDY));
}

//============================================================================
// start_adc_channel()    (Autotest #2)
// Select an ADC channel, and initiate an A-to-D conversion.
// Parameters: n: channel number
//============================================================================
void start_adc_channel(int n)
{
    //Select a single ADC channel (n)
    ADC1->CHSELR = 0;
    ADC1->CHSELR |= 1 << n;
    //Wait for ADRDY to be Set
    while(!(ADC1->ISR & ADC_ISR_ADRDY));
    //Set the ADSTART bit in CR
    ADC1->CR |= ADC_CR_ADSTART;
}

//============================================================================
// read_adc()    (Autotest #3)
// Wait for A-to-D conversion to complete, and return the result.
// Parameters: none
// Return value: converted result
//============================================================================
int read_adc(void)
{
    // Wait for the EOC Bit in ISR
    while(!(ADC1->ISR & ADC_ISR_EOC));
    // Return value from ADC in DR
    return ADC1->DR;
}


//============================================================================
// Parameters for peak detection
//============================================================================
#define N 10
int buffer[N];
int ind = 0;
int exp_drop = 50;
int threshold = 200;
char peak_flag = 0;
int last_peak = 0;

//============================================================================
// Timer 6 ISR    (Autotest #8)
// The ISR for Timer 6 samples the ADC, reads
// (Write the entire subroutine below.)
//============================================================================
void TIM6_DAC_IRQHandler(void) {
    // Acknowledge interrupt (UIF = 0)
    TIM6->SR = 0;
    buffer[ind] = read_adc();
    GPIOA -> ODR = 1 << 10;
    TIM7 -> CR1 |= TIM_CR1_CEN;
    ADC1->CR |= ADC_CR_ADSTART;
    if((buffer[ind] >= threshold) && (buffer[ind] > buffer[(ind+N-1)%N] - exp_drop) && last_peak > 3){
        peak_flag++;
        last_peak = 0;
    }
    else {
        last_peak++;
    }
    ind = (ind + 1) % N;
}

//============================================================================
// Timer 6 ISR    (Autotest #8)
// The ISR for Timer 6 samples the ADC, reads
// (Write the entire subroutine below.)
//============================================================================
void TIM7_IRQHandler(void) {
    // Acknowledge interrupt (UIF = 0)
    TIM7->SR = 0;
    GPIOA -> ODR = 0;
}

void setup_gpioa(void) {
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA -> MODER |= GPIO_MODER_MODER10_0;
    GPIOA -> ODR = 0;
}

//============================================================================
// setup_tim6()
// Configure Timer 6 to raise an interrupt every t_ms milliseconds
// Parameters: none
//============================================================================
void setup_tim6(int t_ms)
{
    RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;
    TIM6 -> PSC = 48000-1;
    TIM6 -> ARR = t_ms - 1;
    TIM6 -> DIER |= 1;
    TIM6 -> CR1 |= TIM_CR1_CEN;
    NVIC -> ISER[0] = 1<<TIM6_DAC_IRQn;
}

//============================================================================
// setup_tim7()
// Configure Timer 6 to raise an interrupt every t_ms milliseconds
// Parameters: none
//============================================================================
void setup_tim7(int t_ms)
{
    RCC -> APB1ENR |= RCC_APB1ENR_TIM7EN;
    TIM7 -> PSC = 48000-1;
    TIM7 -> ARR = t_ms - 1;
    TIM7 -> DIER |= 1;
    TIM7 -> CR1 |= TIM_CR1_OPM;
    NVIC -> ISER[0] = 1<<TIM7_IRQn;
}

int main(void)
{
    int count = 0;
    setup_gpioa();
    setup_adc();
    start_adc_channel(10);
    setup_output();
    setup_tim6(100);
    setup_tim7(5);
    while(1) {
        count += peak_flag;
        peak_flag = 0;
        GPIOC->ODR = (count % 16) << 6;
//        asm volatile("wfi");

    }
}


