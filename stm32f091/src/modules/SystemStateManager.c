#include <stdio.h>
#include "GlobalVariables.h"
#include "Song.h"
#include "SystemStateManager.h"

#define IGNORE(x) ((void)x)

enum {
    RightButtonAction,
    StartButtonAction,
    CalibrationErrorAction
};

static void State_Idle(uint8_t action) {
    switch (action) {
        case RightButtonAction: {
            uint8_t songIndex = 0;
            GlobalVariables_Read(Global_SongIndex, &songIndex);
            songIndex++;
            if (songIndex >= Song_NumberOfSongs) {
                songIndex = 0;
            }
            GlobalVariables_Write(Global_SongIndex, &songIndex);
            break;
        }
        case StartButtonAction: {
            SystemState_t state = SystemState_Running;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case CalibrationErrorAction: {
            SystemState_t state = SystemState_CalibrationError;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        default:
            break;
    }
}

static void State_Running(uint8_t action) {
    switch (action) {
        case RightButtonAction: {
            uint8_t noteForwardSignal = 0;
            GlobalVariables_Read(Global_NoteForwardSignal, &noteForwardSignal);
            noteForwardSignal++;
            GlobalVariables_Write(Global_NoteForwardSignal, &noteForwardSignal);
            break;
        }
        case StartButtonAction: {
            SystemState_t state = SystemState_Paused;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case CalibrationErrorAction: {
            SystemState_t state = SystemState_CalibrationError;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        default:
            break;
    }
}

static void State_Paused(uint8_t action) {
    switch (action) {
        case CalibrationErrorAction: {
            SystemState_t state = SystemState_CalibrationError;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        default:
            break;
    }
}

static void State_CalibrationError(uint8_t action) {
    switch (action) {
        case CalibrationErrorAction: {
            SystemState_t state = SystemState_Idle;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        default:
            break;
    }
}

static void RightButtonPressed(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    uint8_t action = RightButtonAction;
    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            State_Idle(action);
            break;
        }
        case SystemState_Running: {
            State_Running(action);
            break;
        }
        default:
            break;
    }
}

static void StartButtonPressed(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    uint8_t action = StartButtonAction;
    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            State_Idle(action);
            break;
        }
        case SystemState_Running: {
            State_Running(action);
            break;
        }
        default:
            break;
    }
}

static void CalibrationErrorChanged(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    uint8_t action = CalibrationErrorAction;
    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            State_Idle(action);
            break;
        }
        case SystemState_Running: {
            State_Running(action);
            break;
        }
        case SystemState_Paused: {
            State_Paused(action);
            break;
        }
        case SystemState_CalibrationError: {
            State_CalibrationError(action);
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

    const GlobalVariables_Subscription_t calibrationErrorSubscription = { .context = NULL, .callback = CalibrationErrorChanged };
    GlobalVariables_Subscribe(Global_CalibrationError, &calibrationErrorSubscription);

    SystemState_t state = SystemState_Idle;
    GlobalVariables_Write(Global_SystemState, &state);
}
