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
#include "lcd.h"

void setupSPI1_LCD() {
    // SPI1_NSS   PA15  AF0
    // RESET      PB0   output
    // RS         PB1   output
    // SPI1_MOSI  PB5   AF0
    // SPI1_SCK   PB3   AF0
    // backlight  PB4   output

    // enable GPIO
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    // clear mode
    GPIOA->MODER &= ~(GPIO_MODER_MODER15);
    GPIOB->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER5
            | GPIO_MODER_MODER3 | GPIO_MODER_MODER4);

    // set RESET, RS, backlight as output
    GPIOB->MODER |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER1_0
            | GPIO_MODER_MODER4_0;

    // set SPI1 pins as alternate function
    GPIOA->MODER |= GPIO_MODER_MODER15_1;
    GPIOB->MODER |= GPIO_MODER_MODER5_1 | GPIO_MODER_MODER3_1;
    GPIOA->AFR[1] &= ~(0xf0000000); // set AF0 in AFR15
    GPIOB->AFR[0] &= ~(0x00f0f000); // set AF0 in AFR3,5

    // enable SPI1
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    SPI1->CR1 |= SPI_CR1_MSTR;
    SPI1->CR1 &= ~(SPI_CR1_BR);
    SPI1->CR1 |= SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
    SPI1->CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP;
    SPI1->CR1 |= SPI_CR1_SPE;
}

void powerLCD(uint8_t enable) {
    GPIOB->ODR &= ~GPIO_ODR_4;
    if (enable)
        GPIOB->ODR |= GPIO_ODR_4;
}

void setup_output(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    //Setup ADCIN1-2 to analog operation
    GPIOC->MODER |= GPIO_MODER_MODER6_0;
    GPIOC->MODER |= GPIO_MODER_MODER7_0;
    GPIOC->MODER |= GPIO_MODER_MODER8_0;
    GPIOC->MODER |= GPIO_MODER_MODER9_0;
}

//============================================================================
// setup_adc()
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
// start_adc_channel()
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
// read_adc()
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
#define DECAY 25
#define MIN_T 200
int buffer_vals[N];
char buffer_peak[N];
int ind = 0;
int expmin = 0;
char peak_flag = 0;
int last_peak = 0;

//============================================================================
// Timer 6 ISR    (Autotest #8)
// The ISR for Timer 6 samples the ADC, and updates the peak_flag
//============================================================================
void TIM6_DAC_IRQHandler(void) {
    // Acknowledge interrupt (UIF = 0)
    TIM6->SR = 0;
    buffer_vals[ind] = read_adc();
    GPIOA -> ODR |= 1 << 10;
    TIM7 -> CR1 |= TIM_CR1_CEN;
    ADC1->CR |= ADC_CR_ADSTART;

    if((buffer_vals[ind] >= MIN_T) &&
       (buffer_vals[ind] > expmin) &&
       (last_peak > 2)){
        peak_flag++;
        last_peak = 0;
        buffer_peak[ind] = 1;
    }
    else {
        buffer_peak[ind] = 0;
        last_peak++;
    }
    expmin = buffer_vals[ind] - DECAY;//> expmin ? buffer_vals[ind] - DECAY : expmin - DECAY;
    ind = (ind + 1) % N;
}

//============================================================================
// Timer 7 ISR
// Turns off the peak draining transistor
//============================================================================
void TIM7_IRQHandler(void) {
    // Acknowledge interrupt (UIF = 0)
    TIM7->SR = 0;
    GPIOA -> ODR = 0;
}

//============================================================================
// GPIO A Setup
// Configures GPIO A for output
//============================================================================
void setup_gpioa(void) {
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA -> MODER |= GPIO_MODER_MODER10_0;
    GPIOA -> ODR &= ~(1<<10);
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
// Configure Timer 7 to raise an interrupt every t_ms milliseconds (one pulse)
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

// Return number of peaks since last check
int check_peak(){
    int ret_val = peak_flag;
    peak_flag = 0;
    return ret_val;
}

void init_plot(){
    powerLCD(0);
    LCD_Init();
    LCD_Clear(BLACK);
    powerLCD(1);
}

void update_plot(int count){
    static const int spacing = 300/N;
    static old_ind = 0;
    static char str[4];
    sprintf(str, "%d, %d", count, buffer_vals[ind]);
    LCD_DrawString(10,210,  WHITE, BLACK, str, 16, 0);
}

// Call this to setup all the peak detection functions
void setup_peak_detection(){
    setup_gpioa();
    setup_adc();
    start_adc_channel(10);
    setup_tim6(50);
    setup_tim7(5);
}

int main(void)
{
    int count = 0;
    setup_peak_detection();

    setupSPI1_LCD();
    init_plot();

    while(1) {
        count += check_peak();
        update_plot(count);
    }
}
