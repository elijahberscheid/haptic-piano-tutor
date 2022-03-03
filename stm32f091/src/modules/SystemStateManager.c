#include <stdio.h>
#include "GlobalVariables.h"
#include "Song.h"
#include "SystemStateManager.h"

#define IGNORE(x) ((void)x)

enum {
    RightButtonAction,
    ModeButtonAction,
    StartButtonAction,
    CalibrationErrorAction,
    DistanceArrayLength = 10
};

static void IncrementHandedMode(void) {
    HandedMode_t mode = 0;
    GlobalVariables_Read(Global_HandedMode, &mode);
    mode++;
    if (mode >= HandedMode_NumberOfModes) {
        mode = 0;
    }
    GlobalVariables_Write(Global_HandedMode, &mode);
}

static void ResolveFingerDistances() {
    int8_t distances[DistanceArrayLength];
    GlobalVariables_Read(Global_FingerDistances, distances);

    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    HandedMode_t mode = 0;
    GlobalVariables_Read(Global_HandedMode, &mode);

    if (state != SystemState_Running) {
        for (Finger_t finger = 0; finger < DistanceArrayLength; finger++) {
            distances[finger] = Key_Rest;
        }
    }
    else if (mode == HandedMode_Left) {
        for (Finger_t finger = Finger_Right1; finger < DistanceArrayLength; finger++) {
            distances[finger] = Key_Rest;
        }
    }
    else if (mode == HandedMode_Right) {
        for (Finger_t finger = Finger_Left5; finger < Finger_Right1; finger++) {
            distances[finger] = Key_Rest;
        }
    }

    GlobalVariables_Write(Global_ResolvedFingerDistances, distances);
}

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
        case ModeButtonAction: {
            IncrementHandedMode();
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
        case ModeButtonAction: {
            IncrementHandedMode();
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
        case StartButtonAction: {
            SystemState_t state = SystemState_Running;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case ModeButtonAction: {
            IncrementHandedMode();
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

static void ModeButtonPressed(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    uint8_t action = ModeButtonAction;
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

static void FingerDistancesChanged(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    ResolveFingerDistances();
}

static void ModeChanged(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

    ResolveFingerDistances();
}

void SystemStateManager_Init(void) {
    const GlobalVariables_Subscription_t rightButtonSubscription = { .context = NULL, .callback = RightButtonPressed };
    GlobalVariables_Subscribe(Global_RightButtonSignal, &rightButtonSubscription);

    const GlobalVariables_Subscription_t modeButtonSubscription = { .context = NULL, .callback = ModeButtonPressed };
    GlobalVariables_Subscribe(Global_ModeButtonSignal, &modeButtonSubscription);

    const GlobalVariables_Subscription_t startButtonSubscription = { .context = NULL, .callback = StartButtonPressed };
    GlobalVariables_Subscribe(Global_StartButtonSignal, &startButtonSubscription);

    const GlobalVariables_Subscription_t calibrationErrorSubscription = { .context = NULL, .callback = CalibrationErrorChanged };
    GlobalVariables_Subscribe(Global_CalibrationError, &calibrationErrorSubscription);

    const GlobalVariables_Subscription_t fingerDistancesSubscription = { .context = NULL, .callback = FingerDistancesChanged };
    GlobalVariables_Subscribe(Global_FingerDistances, &fingerDistancesSubscription);

    const GlobalVariables_Subscription_t modeSubscription = { .context = NULL, .callback = ModeChanged };
    GlobalVariables_Subscribe(Global_HandedMode, &modeSubscription);

    SystemState_t state = SystemState_Idle;
    GlobalVariables_Write(Global_SystemState, &state);

    HandedMode_t mode = HandedMode_Both;
    GlobalVariables_Write(Global_HandedMode, &mode);
    // don't need to resolve if initializing handed mode to something not 0, but here just in case it changes
    int8_t distances[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_FingerDistances, distances);
    ResolveFingerDistances(mode, distances);
}
