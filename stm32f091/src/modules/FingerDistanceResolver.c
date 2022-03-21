// #define CPPUTEST // define this only when unit testing
#ifndef CPPUTEST
#include "stm32f0xx.h"
#else
#include "../../tests/stm32f0xx.h"
#endif
#include <stdio.h>
#include "FingerDistanceResolver.h"
#include "GlobalVariables.h"
#include "Song.h"
#include "SystemStateManager.h"

#define IGNORE(x) ((void)x)

volatile static bool enableOutput = true;
volatile static bool enableChanged = false;

enum {
    ClockFrequency = 48000000,
    Prescaler = 48000, // needs to be not too small, since ARR is only 16 bits
    DistanceArrayLength = 10
};

static void ResolveFingerDistances(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    int8_t distances[DistanceArrayLength];
    if (enableOutput) {
        GlobalVariables_Read(Global_FingerDistances, distances);

        SystemState_t state = 0;
        GlobalVariables_Read(Global_SystemState, &state);

        HandedMode_t mode = 0;
        GlobalVariables_Read(Global_HandedMode, &mode);

        if (state != SystemState_Running) {
            for (Finger_t finger = 0; finger < DistanceArrayLength; finger++) {
                distances[finger] = Key_Rest;
            }
        } else if (mode == HandedMode_Left) {
            for (Finger_t finger = Finger_Right1; finger < DistanceArrayLength;
                    finger++) {
                distances[finger] = Key_Rest;
            }
        } else if (mode == HandedMode_Right) {
            for (Finger_t finger = Finger_Left5; finger < Finger_Right1;
                    finger++) {
                distances[finger] = Key_Rest;
            }
        }
    }
    else {
        for (Finger_t finger = 0; finger < DistanceArrayLength; finger++) {
            distances[finger] = Key_Rest;
        }
    }

    GlobalVariables_Write(Global_ResolvedFingerDistances, distances);
}

static void SoundDetected(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);
    if (state == SystemState_Running) {
        // stop timer before changing its config, and so that the ISR cannot be called in the middle of this function
        TIM17->CR1 &= ~TIM_CR1_CEN;

        uint8_t noteLength = 0;
        GlobalVariables_Read(Global_PreviousNoteLength, &noteLength);
        if (noteLength == 0) {
            // should not hit this case, only here to make sure ARR is not set to -1
            // unless this function is called before tempo is updated to match the next note
            // because noteLength is only 0 before any note has been played
            enableOutput = true;
            ResolveFingerDistances(NULL, NULL);
        }
        else {
            // temporarily disable motors
            // depending on the subscription order, when a sound is detected,
            // music manager may cause the resolved distances to change first,
            // and then the resolved distances are set to rests here
            enableOutput = false;
            ResolveFingerDistances(NULL, NULL);

            uint8_t tempo = 0;
            GlobalVariables_Read(Global_Tempo, &tempo);

            // fastest note supported is 64th note, which is 1/16 the duration of a quarter note
            // that is the duration of one "tick"
            // tempo is in beats per minute, which is number of quarter notes per minute
            // tempo / 60 * 16 is ticks per second
            // tempo / 60 * 16 / noteLength is (noteLength ticks) per second
            // doing multiplication before most of the division to get more resolution out of integer division
            TIM17->ARR = ClockFrequency / Prescaler * 60 * noteLength / tempo / 16 - 1;
            TIM17->CR1 |= TIM_CR1_CEN;
        }
    }
}

static void NoteChanged(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    TIM17->CR1 &= ~TIM_CR1_CEN;

    // if changing expected note via UI, enable motors immediately
    if (!enableOutput) {
        enableOutput = true;
        ResolveFingerDistances(NULL, NULL);
    }
}

void TIM17_IRQHandler(void) {
    TIM17->SR &= ~TIM_SR_UIF; // acknowledge interrupt

    if (!enableOutput) {
        enableOutput = true;
        enableChanged = true;
    }
}

static void Timer17_Init(uint32_t ARR) {
    TIM17->CR1 &= ~TIM_CR1_CEN; // make sure timer is off before configuring it
    RCC->APB2ENR |= RCC_APB2ENR_TIM17EN; // enable clock to timer
    TIM17->PSC = Prescaler - 1; // set prescaler
    TIM17->ARR = ARR; // set auto reload register
    TIM17->DIER |= TIM_DIER_UIE; // update interrupt enable
    NVIC->ISER[0] = 1 << TIM17_IRQn; // unmask in NVIC
    TIM17->CR1 |= TIM_CR1_OPM; // one-pulse mode, to turn itself off at next update event

    // enable the timer once
    // because the first time the timer is enabled, it instantly calls its ISR
    TIM17->CR1 |= TIM_CR1_CEN;
}

void FingerDistanceResolver_Run(void) {
    if (enableChanged) {
        enableChanged = false;
        ResolveFingerDistances(NULL, NULL);
    }
}

void FingerDistanceResolver_Init(void) {
    const GlobalVariables_Subscription_t resolveFingerDistancesSubscription = { .context = NULL, .callback = ResolveFingerDistances };
    GlobalVariables_Subscribe(Global_FingerDistances, &resolveFingerDistancesSubscription);
    GlobalVariables_Subscribe(Global_HandedMode, &resolveFingerDistancesSubscription);
    GlobalVariables_Subscribe(Global_SystemState, &resolveFingerDistancesSubscription);

    ResolveFingerDistances(NULL, NULL);

    const GlobalVariables_Subscription_t soundDetectedSubscription = { .context = NULL, .callback = SoundDetected };
    GlobalVariables_Subscribe(Global_SoundDetectedSignal, &soundDetectedSubscription);

    const GlobalVariables_Subscription_t noteChangedSubscription = { .context = NULL, .callback = NoteChanged };
    GlobalVariables_Subscribe(Global_NoteForwardSignal, &noteChangedSubscription);
    GlobalVariables_Subscribe(Global_NoteBackwardSignal, &noteChangedSubscription);

    // ARR can be anything non-zero on init, it will get set to a meaningful value when there is an expected note
    Timer17_Init(1000 - 1);
}
