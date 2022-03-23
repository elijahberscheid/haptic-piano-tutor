#include "stm32f0xx.h"
#include <stdio.h>
#include <string.h>
#include "ScreenInterface.h"
#include "lcd.h"
#include "modules/Note.h"
#include "modules/GlobalVariables.h"
#include "modules/SystemStateManager.h"

#define IGNORE(x) ((void)x)

enum {
    DistanceArrayLength = 10,
    CharacterWidth = 8, // an approximate value, only used to space out text from each other
    CharacterHeight = 16,
    DefaultX = 40,
    StateX = 220,
    CalibrationErrorStateX = 180,
    StateY = 20,
    CurrentSongY = 40,
    SongNameY = CurrentSongY + CharacterHeight,
    ExpectedNotesY = SongNameY + 2 * CharacterHeight,
    ExpectedNoteNamesY = ExpectedNotesY + CharacterHeight,
    ModeY = ExpectedNoteNamesY + 2 * CharacterHeight,
    TempoY = ModeY + CharacterHeight
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

static const char *stateStringTable[] = {
    "Idle", // not used
    "Running",
    "Paused",
    "Tempo",
    "Calibration Error"
};

// TODO: does not make sense to store song names here, store them with the songs instead
static const char *songNameStringTable[] = {
    "Mary Had A Little Lamb",
    "C Major Scale"
};

static const char *modeStringTable[] = {
    "Left Hand",
    "Right Hand",
    "Both Hands"
};

static const char *errorStringTable[] = {
    "Error 0",
    "Error 1",
    "Error 2"
};

static void UpdateState(SystemState_t state, uint16_t color) {
    LCD_DrawFillRectangle(0, StateY, lcddev.width -1, StateY + CharacterHeight, WHITE);
    LCD_DrawString(StateX, StateY, color, WHITE, stateStringTable[state], CharacterHeight, 0);
}

static void UpdateSongName(uint8_t songIndex) {
    LCD_DrawFillRectangle(0, CurrentSongY, lcddev.width -1, CurrentSongY + CharacterHeight, WHITE);
    LCD_DrawString(DefaultX, CurrentSongY, BLACK, WHITE, "Current Song:", CharacterHeight, 0);

    LCD_DrawFillRectangle(0, SongNameY, lcddev.width -1, SongNameY + CharacterHeight, WHITE);
    LCD_DrawString(DefaultX, SongNameY, BLACK, WHITE, songNameStringTable[songIndex], CharacterHeight, 0);
}

static void UpdateExpectedNotes(uint8_t *notes, uint16_t color) {
    LCD_DrawFillRectangle(0, ExpectedNotesY, lcddev.width -1, ExpectedNotesY + CharacterHeight, WHITE);
    LCD_DrawFillRectangle(0, ExpectedNoteNamesY, lcddev.width -1, ExpectedNoteNamesY + CharacterHeight, WHITE);

    LCD_DrawString(DefaultX, ExpectedNotesY, color, WHITE, "Expected Notes:", 16, 0);
    uint16_t xpos = DefaultX;
    uint8_t invalidNoteCount = 0;
    for (uint8_t i = 0; i < DistanceArrayLength; i++) {
        if (notes[i] < Key_Invalid) {
            LCD_DrawString(xpos, ExpectedNoteNamesY, color, WHITE, keyToStringTable[notes[i]], 16, 0);
            xpos += (strlen(keyToStringTable[notes[i]]) + 1) * CharacterWidth;
        }
        else {
            invalidNoteCount++;
        }
    }
    if (invalidNoteCount == DistanceArrayLength) {
            LCD_DrawFillRectangle(0, ExpectedNotesY, lcddev.width -1, ExpectedNotesY + CharacterHeight, WHITE);
            LCD_DrawString(DefaultX, ExpectedNotesY, color, WHITE, "End of Song", 16, 0);
    }
}

static void UpdateHandedMode(uint8_t mode, uint16_t color) {
    LCD_DrawFillRectangle(0, ModeY, lcddev.width -1, ModeY + CharacterHeight, WHITE);
    LCD_DrawString(DefaultX, ModeY, color, WHITE, "Mode: ", CharacterHeight, 0);
    LCD_DrawString(DefaultX + strlen("Mode: ") * CharacterWidth, ModeY, color, WHITE, modeStringTable[mode], CharacterHeight, 0);
}

static void UpdateTempo(uint8_t tempo, uint16_t color) {
    LCD_DrawFillRectangle(0, TempoY, lcddev.width -1, TempoY + CharacterHeight, WHITE);
    LCD_DrawString(DefaultX, TempoY, color, WHITE, "Tempo (bpm): ", CharacterHeight, 0);
    char tempoBuffer[4] = { 0 }; // assumes tempo will never have more than 3 digits, plus 1 digit for null terminator
    sprintf(tempoBuffer, "%d", tempo);
    LCD_DrawString(DefaultX + strlen("Tempo (bpm): ") * CharacterWidth, TempoY, color, WHITE, tempoBuffer, CharacterHeight, 0);
}

static void DisplayError(uint8_t errorCode) {
    LCD_Clear(WHITE);
    LCD_DrawString(DefaultX, StateY, RED, WHITE, "Error:", CharacterHeight, 0);
    if (errorCode < sizeof(errorStringTable) / sizeof(errorStringTable[0])) {
        LCD_DrawString(DefaultX, SongNameY, RED, WHITE, errorStringTable[errorCode], CharacterHeight, 0);
    }
}

static void SystemStateChanged(void *context, const void *data) {
    IGNORE(context);
    SystemState_t *state = (SystemState_t *) data;

    switch (*state) {
        case SystemState_Idle: {
            LCD_Clear(WHITE);

            UpdateState(*state, WHITE);

            uint8_t songIndex = 0;
            GlobalVariables_Read(Global_SongIndex, &songIndex);
            UpdateSongName(songIndex);

            uint8_t notes[DistanceArrayLength] = { 0 };
            GlobalVariables_Read(Global_DesiredFingerPositions, &notes);
            UpdateExpectedNotes(notes, WHITE);

            uint8_t mode = 0;
            GlobalVariables_Read(Global_HandedMode, &mode);
            UpdateHandedMode(mode, BLACK);

            uint8_t tempo = 0;
            GlobalVariables_Read(Global_Tempo, &tempo);
            UpdateTempo(tempo, BLACK);
            break;
        }
        case SystemState_Running: {
            UpdateState(*state, BLACK);

            uint8_t notes[DistanceArrayLength] = { 0 };
            GlobalVariables_Read(Global_DesiredFingerPositions, &notes);
            UpdateExpectedNotes(notes, BLUE);
            break;
        }
        case SystemState_Paused: {
            UpdateState(*state, BLACK);

            uint8_t notes[DistanceArrayLength] = { 0 };
            GlobalVariables_Read(Global_DesiredFingerPositions, &notes);
            UpdateExpectedNotes(notes, RED);
            break;
        }
        case SystemState_Tempo: {
            UpdateState(*state, BLACK);

            uint8_t tempo = 0;
            GlobalVariables_Read(Global_Tempo, &tempo);
            UpdateTempo(tempo, BLUE);
            break;
        }
        case SystemState_CalibrationError: {
            uint8_t errorCode = 0;
            GlobalVariables_Read(Global_ErrorCode, &errorCode);
            DisplayError(errorCode);
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
        case SystemState_Running: {
            UpdateExpectedNotes(expected, BLUE);
            break;
        }
        case SystemState_Paused: {
            UpdateExpectedNotes(expected, RED);
            break;
        }
        default: {
            UpdateExpectedNotes(expected, WHITE);
            break;
        }
    }
}

static void SongIndexChanged(void *context, const void *data) {
    IGNORE(context);
    uint8_t *songIndex = (uint8_t *) data;

    UpdateSongName(*songIndex);
}

static void TempoChanged(void *context, const void *data) {
    IGNORE(context);
    uint8_t *tempo = (uint8_t *) data;

    UpdateTempo(*tempo, BLUE);
}

static void HandedModeChanged(void *context, const void *data) {
    IGNORE(context);
    uint8_t *mode = (uint8_t *) data;

    UpdateHandedMode(*mode, BLACK);
}

static void ErrorCodeChanged(void *context, const void *data) {
    IGNORE(context);
    uint8_t *errorCode = (uint8_t *) data;

    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);
    if (state == SystemState_CalibrationError) {
        DisplayError(*errorCode);
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
    LCD_Clear(WHITE);
    PowerLCD(1);

    const GlobalVariables_Subscription_t stateSubscription = { .context = NULL, .callback = SystemStateChanged };
    GlobalVariables_Subscribe(Global_SystemState, &stateSubscription);

    const GlobalVariables_Subscription_t desiredPositionsSubscription = { .context = NULL, .callback = DesiredPositionsChanged };
    GlobalVariables_Subscribe(Global_DesiredFingerPositions, &desiredPositionsSubscription);

    const GlobalVariables_Subscription_t songIndexSubscription = { .context = NULL, .callback = SongIndexChanged };
    GlobalVariables_Subscribe(Global_SongIndex, &songIndexSubscription);

    const GlobalVariables_Subscription_t tempoSubscription = { .context = NULL, .callback = TempoChanged };
    GlobalVariables_Subscribe(Global_Tempo, &tempoSubscription);

    const GlobalVariables_Subscription_t modeSubscription = { .context = NULL, .callback = HandedModeChanged };
    GlobalVariables_Subscribe(Global_HandedMode, &modeSubscription);

    const GlobalVariables_Subscription_t errorCodeSubscription = { .context = NULL, .callback = ErrorCodeChanged };
    GlobalVariables_Subscribe(Global_ErrorCode, &errorCodeSubscription);

    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);
    SystemStateChanged(NULL, &state);
}
