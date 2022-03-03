#include "stm32f0xx.h"
#include <stdio.h>
#include "ScreenInterface.h"
#include "lcd.h"
#include "modules/Note.h"
#include "modules/GlobalVariables.h"
#include "modules/SystemStateManager.h"

#define IGNORE(x) ((void)x)

enum {
    DistanceArrayLength = 10
};

static const char *keyToStringTable[] = {
    "A0",
    "ASharp0",
    "B0",
    "C1",
    "CSharp1",
    "D1",
    "DSharp1",
    "E1",
    "F1",
    "FSharp1",
    "G1",
    "GSharp1",
    "A1",
    "ASharp1",
    "B1",
    "C2",
    "CSharp2",
    "D2",
    "DSharp2",
    "E2",
    "F2",
    "FSharp2",
    "G2",
    "GSharp2",
    "A2",
    "ASharp2",
    "B2",
    "C3",
    "CSharp3",
    "D3",
    "DSharp3",
    "E3",
    "F3",
    "FSharp3",
    "G3",
    "GSharp3",
    "A3",
    "ASharp3",
    "B3",
    "C4",
    "CSharp4",
    "D4",
    "DSharp4",
    "E4",
    "F4",
    "FSharp4",
    "G4",
    "GSharp4",
    "A4",
    "ASharp4",
    "B4",
    "C5",
    "CSharp5",
    "D5",
    "DSharp5",
    "E5",
    "F5",
    "FSharp5",
    "G5",
    "GSharp5",
    "A5",
    "ASharp5",
    "B5",
    "C6",
    "CSharp6",
    "D6",
    "DSharp6",
    "E6",
    "F6",
    "FSharp6",
    "G6",
    "GSharp6",
    "A6",
    "ASharp6",
    "B6",
    "C7",
    "CSharp7",
    "D7",
    "DSharp7",
    "E7",
    "F7",
    "FSharp7",
    "G7",
    "GSharp7",
    "A7",
    "ASharp7",
    "B7",
    "C8",
    "Invalid",
    "Rest"
};

static void UpdateExpectedNotes(uint8_t *notes) {
    LCD_Clear(BLACK);
    LCD_DrawString(40, 60,  WHITE, BLACK, "Expected Notes:", 16, 0);
    uint16_t xpos = 40;
    for (uint8_t i = 0; i < DistanceArrayLength; i++) {
        if (notes[i] < Key_Invalid) {
            LCD_DrawString(xpos, 80,  WHITE, BLACK, keyToStringTable[notes[i]], 16, 0);
            xpos += 40;
        }
    }
}

static void UpdateSongName(uint8_t songIndex) {
    char *songName = (songIndex == 0) ? "Mary Had a Little Lamb" : "C Major Scale";

    LCD_Clear(BLACK);
    LCD_DrawString(40, 60,  WHITE, BLACK, "Current Song:", 16, 0);
    LCD_DrawString(40, 80,  WHITE, BLACK, songName, 16, 0);
}

static void SystemStateChanged(void *context, const void *data) {
    IGNORE(context);
    SystemState_t *state = (SystemState_t *) data;

    switch (*state) {
        case SystemState_Idle: {
            uint8_t songIndex = 0;
            GlobalVariables_Read(Global_SongIndex, &songIndex);
            UpdateSongName(songIndex);
            break;
        }
        case SystemState_Running: {
            uint8_t notes[DistanceArrayLength] = { 0 };
            GlobalVariables_Read(Global_DesiredFingerPositions, &notes);
            UpdateExpectedNotes(notes);
            break;
        }
        default:
            break;
    }
}

static void DesiredPositionsChanged(void *context, const void *data) {
    IGNORE(context);
    uint8_t *expected = (uint8_t *) data;

    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            break;
        }
        case SystemState_Running: {
            UpdateExpectedNotes(expected);
            break;
        }
        default:
            break;
    }
}

static void SongIndexChanged(void *context, const void *data) {
    IGNORE(context);
    uint8_t *songIndex = (uint8_t *) data;

    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            UpdateSongName(*songIndex);
            break;
        }
        case SystemState_Running: {
            break;
        }
        default:
            break;
    }
}

static void SPI1_Init(void) {
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

static void PowerLCD(uint8_t enable) {
    if (enable) {
        GPIOB->BSRR = 1 << 4;
    }
    else {
        GPIOB->BRR = 1 << 4;
    }
}

void ScreenInterface_Init(void) {
    SPI1_Init();

    PowerLCD(0);
    LCD_Init();
    LCD_Clear(BLACK);
    PowerLCD(1);

    const GlobalVariables_Subscription_t stateSubscription = { .context = NULL, .callback = SystemStateChanged };
    GlobalVariables_Subscribe(Global_SystemState, &stateSubscription);

    const GlobalVariables_Subscription_t desiredPositionsSubscription = { .context = NULL, .callback = DesiredPositionsChanged };
    GlobalVariables_Subscribe(Global_DesiredFingerPositions, &desiredPositionsSubscription);

    const GlobalVariables_Subscription_t songIndexSubscription = { .context = NULL, .callback = SongIndexChanged };
    GlobalVariables_Subscribe(Global_SongIndex, &songIndexSubscription);

    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);
    SystemStateChanged(NULL, &state);
}
