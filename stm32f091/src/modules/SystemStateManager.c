#include <stdio.h>
#include "GlobalVariables.h"
#include "Song.h"
#include "SystemStateManager.h"

#define IGNORE(x) ((void)x)

enum {
    LeftButtonAction,
    RightButtonAction,
    ModeButtonAction,
    TempoButtonAction,
    StartButtonAction,
    StopButtonAction,
    CalibrationErrorAction,
    DistanceArrayLength = 10,
    DefaultTempo = 60,
    MinTempo = 1,
    MaxTempo = 255,
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

static void State_Idle(SystemStateManager_t *instance, uint8_t action) {
    switch (action) {
        case LeftButtonAction: {
            uint8_t songIndex = 0;
            GlobalVariables_Read(Global_SongIndex, &songIndex);
            if (songIndex == 0) {
                songIndex = Song_NumberOfSongs - 1;
            }
            else {
                songIndex--;
            }
            GlobalVariables_Write(Global_SongIndex, &songIndex);
            break;
        }
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
        case TempoButtonAction: {
            SystemState_t state = SystemState_Tempo;
            instance->previousState = SystemState_Idle;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case StartButtonAction: {
            SystemState_t state = SystemState_Running;
            instance->previousState = SystemState_Idle;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case CalibrationErrorAction: {
            SystemState_t state = SystemState_CalibrationError;
            instance->previousState = SystemState_Idle;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        default:
            break;
    }
}

static void State_Running(SystemStateManager_t *instance, uint8_t action) {
    switch (action) {
        case LeftButtonAction: {
            uint8_t noteBackwardSignal = 0;
            GlobalVariables_Read(Global_NoteBackwardSignal, &noteBackwardSignal);
            noteBackwardSignal++;
            GlobalVariables_Write(Global_NoteBackwardSignal, &noteBackwardSignal);
            break;
        }
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
            instance->previousState = SystemState_Running;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case StopButtonAction: {
            SystemState_t state = SystemState_Idle;
            instance->previousState = SystemState_Running;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case CalibrationErrorAction: {
            SystemState_t state = SystemState_CalibrationError;
            instance->previousState = SystemState_Running;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        default:
            break;
    }
}

static void State_Paused(SystemStateManager_t *instance, uint8_t action) {
    switch (action) {
        case StartButtonAction: {
            SystemState_t state = SystemState_Running;
            instance->previousState = SystemState_Paused;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case ModeButtonAction: {
            IncrementHandedMode();
            break;
        }
        case TempoButtonAction: {
            SystemState_t state = SystemState_Tempo;
            instance->previousState = SystemState_Paused;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case StopButtonAction: {
            SystemState_t state = SystemState_Idle;
            instance->previousState = SystemState_Paused;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case CalibrationErrorAction: {
            SystemState_t state = SystemState_CalibrationError;
            instance->previousState = SystemState_Paused;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        default:
            break;
    }
}

static void State_Tempo(SystemStateManager_t *instance, uint8_t action) {
    switch (action) {
        case LeftButtonAction: {
            uint8_t tempo = 0;
            GlobalVariables_Read(Global_Tempo, &tempo);
            if (tempo > MinTempo) {
                tempo--;
                GlobalVariables_Write(Global_Tempo, &tempo);
            }
            break;
        }
        case RightButtonAction: {
            uint8_t tempo = 0;
            GlobalVariables_Read(Global_Tempo, &tempo);
            if (tempo < MaxTempo) {
                tempo++;
                GlobalVariables_Write(Global_Tempo, &tempo);
            }
            break;
        }
        case ModeButtonAction: {
            IncrementHandedMode();
            break;
        }
        case TempoButtonAction: {
            SystemState_t state = instance->previousState;
            instance->previousState = SystemState_Tempo;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        case CalibrationErrorAction: {
            SystemState_t state = SystemState_CalibrationError;
            instance->previousState = SystemState_Tempo;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        default:
            break;
    }
}

static void State_CalibrationError(SystemStateManager_t *instance, uint8_t action) {
    switch (action) {
        case CalibrationErrorAction: {
            SystemState_t state = SystemState_Idle;
            instance->previousState = SystemState_CalibrationError;
            GlobalVariables_Write(Global_SystemState, &state);
            break;
        }
        default:
            break;
    }
}

static void LeftButtonPressed(void *context, const void *data) {
    SystemStateManager_t *instance = (SystemStateManager_t *) context;
    IGNORE(data);

    uint8_t action = LeftButtonAction;
    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            State_Idle(instance, action);
            break;
        }
        case SystemState_Running: {
            State_Running(instance, action);
            break;
        }
        case SystemState_Paused: {
            State_Paused(instance, action);
            break;
        }
        case SystemState_Tempo: {
            State_Tempo(instance, action);
            break;
        }
        case SystemState_CalibrationError: {
            State_CalibrationError(instance, action);
            break;
        }
        default:
            break;
    }
}
static void RightButtonPressed(void *context, const void *data) {
    SystemStateManager_t *instance = (SystemStateManager_t *) context;
    IGNORE(data);

    uint8_t action = RightButtonAction;
    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            State_Idle(instance, action);
            break;
        }
        case SystemState_Running: {
            State_Running(instance, action);
            break;
        }
        case SystemState_Paused: {
            State_Paused(instance, action);
            break;
        }
        case SystemState_Tempo: {
            State_Tempo(instance, action);
            break;
        }
        case SystemState_CalibrationError: {
            State_CalibrationError(instance, action);
            break;
        }
        default:
            break;
    }
}

static void ModeButtonPressed(void *context, const void *data) {
    SystemStateManager_t *instance = (SystemStateManager_t *) context;
    IGNORE(data);

    uint8_t action = ModeButtonAction;
    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            State_Idle(instance, action);
            break;
        }
        case SystemState_Running: {
            State_Running(instance, action);
            break;
        }
        case SystemState_Paused: {
            State_Paused(instance, action);
            break;
        }
        case SystemState_Tempo: {
            State_Tempo(instance, action);
            break;
        }
        case SystemState_CalibrationError: {
            State_CalibrationError(instance, action);
            break;
        }
        default:
            break;
    }
}

static void TempoButtonPressed(void *context, const void *data) {
    SystemStateManager_t *instance = (SystemStateManager_t *) context;
    IGNORE(data);

    uint8_t action = TempoButtonAction;
    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            State_Idle(instance, action);
            break;
        }
        case SystemState_Running: {
            State_Running(instance, action);
            break;
        }
        case SystemState_Paused: {
            State_Paused(instance, action);
            break;
        }
        case SystemState_Tempo: {
            State_Tempo(instance, action);
            break;
        }
        case SystemState_CalibrationError: {
            State_CalibrationError(instance, action);
            break;
        }
        default:
            break;
    }
}

static void StartButtonPressed(void *context, const void *data) {
    SystemStateManager_t *instance = (SystemStateManager_t *) context;
    IGNORE(data);

    uint8_t action = StartButtonAction;
    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            State_Idle(instance, action);
            break;
        }
        case SystemState_Running: {
            State_Running(instance, action);
            break;
        }
        case SystemState_Paused: {
            State_Paused(instance, action);
            break;
        }
        case SystemState_Tempo: {
            State_Tempo(instance, action);
            break;
        }
        case SystemState_CalibrationError: {
            State_CalibrationError(instance, action);
            break;
        }
        default:
            break;
    }
}

static void StopButtonPressed(void *context, const void *data) {
    SystemStateManager_t *instance = (SystemStateManager_t *) context;
    IGNORE(data);

    uint8_t action = StopButtonAction;
    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            State_Idle(instance, action);
            break;
        }
        case SystemState_Running: {
            State_Running(instance, action);
            break;
        }
        case SystemState_Paused: {
            State_Paused(instance, action);
            break;
        }
        case SystemState_Tempo: {
            State_Tempo(instance, action);
            break;
        }
        case SystemState_CalibrationError: {
            State_CalibrationError(instance, action);
            break;
        }
        default:
            break;
    }
}

static void CalibrationErrorChanged(void *context, const void *data) {
    SystemStateManager_t *instance = (SystemStateManager_t *) context;
    IGNORE(data);

    uint8_t action = CalibrationErrorAction;
    SystemState_t state = 0;
    GlobalVariables_Read(Global_SystemState, &state);

    switch (state) {
        case SystemState_Idle: {
            State_Idle(instance, action);
            break;
        }
        case SystemState_Running: {
            State_Running(instance, action);
            break;
        }
        case SystemState_Paused: {
            State_Paused(instance, action);
            break;
        }
        case SystemState_Tempo: {
            State_Tempo(instance, action);
            break;
        }
        case SystemState_CalibrationError: {
            State_CalibrationError(instance, action);
            break;
        }
        default:
            break;
    }
}

void SystemStateManager_Init(SystemStateManager_t *instance) {
    instance->previousState = SystemState_Idle;

    const GlobalVariables_Subscription_t leftButtonSubscription = { .context = instance, .callback = LeftButtonPressed };
    GlobalVariables_Subscribe(Global_LeftButtonSignal, &leftButtonSubscription);

    const GlobalVariables_Subscription_t rightButtonSubscription = { .context = instance, .callback = RightButtonPressed };
    GlobalVariables_Subscribe(Global_RightButtonSignal, &rightButtonSubscription);

    const GlobalVariables_Subscription_t modeButtonSubscription = { .context = instance, .callback = ModeButtonPressed };
    GlobalVariables_Subscribe(Global_ModeButtonSignal, &modeButtonSubscription);

    const GlobalVariables_Subscription_t tempoButtonSubscription = { .context = instance, .callback = TempoButtonPressed };
    GlobalVariables_Subscribe(Global_TempoButtonSignal, &tempoButtonSubscription);

    const GlobalVariables_Subscription_t startButtonSubscription = { .context = instance, .callback = StartButtonPressed };
    GlobalVariables_Subscribe(Global_StartButtonSignal, &startButtonSubscription);

    const GlobalVariables_Subscription_t stopButtonSubscription = { .context = instance, .callback = StopButtonPressed };
    GlobalVariables_Subscribe(Global_StopButtonSignal, &stopButtonSubscription);

    const GlobalVariables_Subscription_t calibrationErrorSubscription = { .context = instance, .callback = CalibrationErrorChanged };
    GlobalVariables_Subscribe(Global_CalibrationError, &calibrationErrorSubscription);

    SystemState_t state = SystemState_Idle;
    GlobalVariables_Write(Global_SystemState, &state);

    HandedMode_t mode = HandedMode_Both;
    GlobalVariables_Write(Global_HandedMode, &mode);

    uint8_t tempo = DefaultTempo;
    GlobalVariables_Write(Global_Tempo, &tempo);
}
