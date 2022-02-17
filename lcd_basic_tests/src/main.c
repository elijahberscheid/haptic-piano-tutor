#include "stm32f0xx.h"
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

int main(void) {
    setupSPI1_LCD();
    powerLCD(0);
    LCD_Init();
    LCD_Clear(BLACK);
    LCD_DrawLine(10,20,100,200, WHITE);
    LCD_DrawRectangle(10,20,100,200, GREEN);
    LCD_DrawFillRectangle(120,20,220,200, RED);
    LCD_Circle(50, 260, 50, 1, BLUE);
    LCD_DrawFillTriangle(130,130, 130,200, 190,160, YELLOW);
    LCD_DrawChar(150,155, BLACK, WHITE, 'X', 16, 1);
    LCD_DrawString(140,60,  WHITE, BLACK, "Is it", 16, 0);
    LCD_DrawString(140,80,  WHITE, BLACK, "working?", 16, 1);
    LCD_DrawString(130,100, BLACK, GREEN, "How bout now?", 16, 0);
    powerLCD(1);
}
