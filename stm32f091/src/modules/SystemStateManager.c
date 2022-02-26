#include <stdio.h>
#include "GlobalVariables.h"
#include "Song.h"
#include "SystemStateManager.h"

#define IGNORE(x) ((void)x)

static void RightButtonPressed(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            uint8_t songIndex = 0;
            GlobalVariables_Read(Global_SongIndex, &songIndex);
            songIndex++;
            if (songIndex >= Song_NumberOfSongs) {
                songIndex = 0;
            }
            GlobalVariables_Write(Global_SongIndex, &songIndex);
            break;
        }
        case SystemState_Running: {
            uint8_t noteForwardSignal = 0;
            GlobalVariables_Read(Global_NoteForwardSignal, &noteForwardSignal);
            noteForwardSignal++;
            GlobalVariables_Write(Global_NoteForwardSignal, &noteForwardSignal);
            break;
        }
        default:
            break;
    }
}

static void StartButtonPressed(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            state = SystemState_Running;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case SystemState_Running: {
            state = SystemState_Paused;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        default:
            break;
    }
}

void SystemStateManager_Init(void) {
    const GlobalVariables_Subscription_t rightButtonSubscription = { .context = NULL, .callback = RightButtonPressed };
    GlobalVariables_Subscribe(Global_RightButtonSignal, &rightButtonSubscription);

    const GlobalVariables_Subscription_t startButtonSubscription = { .context = NULL, .callback = StartButtonPressed };
    GlobalVariables_Subscribe(Global_StartButtonSignal, &startButtonSubscription);

    SystemState_t state = SystemState_Idle;
    GlobalVariables_Write(Global_SystemState, &state);
}
