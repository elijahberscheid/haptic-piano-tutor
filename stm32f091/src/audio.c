#include "stm32f0xx.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h> // for printf()
#include "modules/GlobalVariables.h"

//============================================================================
// setup_adc()
// Configure the ADC peripheral and analog input pins.
// Parameters: none
//============================================================================
static void setup_adc(void)
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
static void start_adc_channel(int n)
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
static int read_adc(void)
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
#define DECAY 50
#define MIN_T 1000
static int buffer_vals[N];
static char buffer_peak[N];
static int ind = 0;
static int expmin = 0;
static int peak_flag = 0;
static int last_peak = 0;
int thresh = 25;
int min_peak_time = 3;

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
    int peak_time = 0;
    if(buffer_vals[ind] > buffer_vals[prev] + thresh){
        last_peak = prev;
        peak_time = 0;
    }
    if(buffer_vals[ind] > buffer_vals[last_peak] + thresh) {
        peak_time++;
        if(peak_time > min_peak_time)
            peak_flag++;
    }
    else {
        peak_time = 0;
    }
    /*
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
    }*/
    expmin = buffer_vals[ind] - DECAY;//> expmin ? buffer_vals[ind] - DECAY : expmin - DECAY;
    prev = ind;
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
static void setup_gpioa(void) {
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA -> MODER |= GPIO_MODER_MODER10_0;
    GPIOA -> ODR &= ~(1<<10);
}

//============================================================================
// setup_tim6()
// Configure Timer 6 to raise an interrupt every t_ms milliseconds
// Parameters: none
//============================================================================
static void setup_tim6(int t_ms)
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
static void setup_tim7(int t_ms)
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

void Audio_Run(void) {
    if (check_peak() > 0) {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_SoundDetectedSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_SoundDetectedSignal, &signal);
    }
}

// Call this to setup all the peak detection functions
void setup_peak_detection(){
    setup_gpioa();
    setup_adc();
    start_adc_channel(10);
    setup_tim6(50);
    setup_tim7(5);
}
