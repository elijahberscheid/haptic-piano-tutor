#include "CppUTest/TestHarness.h"

extern "C"
{
#include <stdlib.h>
#include <assert.h>
#include "GlobalVariables.h"
#include "Song.h"
#include "SystemStateManager.h"
}

TEST_GROUP(SystemStateManager) {
    enum {
        DistanceArrayLength = 10,
        DefaultTempo = 60,
        MinTempo = 1,
        MaxTempo = 255,
    };

    void setup() {
        GlobalVariables_Init();
        int8_t distances[] = {
            -5, -4, -3, -2, -1,
            1, 2, 3, 4, 5
        };
        GlobalVariables_Write(Global_FingerDistances, distances);

        SystemStateManager_Init();
    }

    void teardown() {
    }

    void PressLeftButton() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_LeftButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_LeftButtonSignal, &signal);
    }

    void PressRightButton() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_RightButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_RightButtonSignal, &signal);
    }

    void PressModeButton() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_ModeButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_ModeButtonSignal, &signal);
    }

    void PressTempoButton() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_TempoButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_TempoButtonSignal, &signal);
    }

    void PressStartButton() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_StartButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_StartButtonSignal, &signal);
    }

    void PressStopButton() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_StopButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_StopButtonSignal, &signal);
    }

    void CheckStateIs(SystemState_t expected) {
        SystemState_t actual;
        GlobalVariables_Read(Global_SystemState, &actual);
        CHECK_EQUAL(expected, actual);
    }

    void CheckSongIndexIs(uint8_t expected) {
        uint8_t actual;
        GlobalVariables_Read(Global_SongIndex, &actual);
        CHECK_EQUAL(expected, actual);
    }

    void CheckModeIs(uint8_t expected) {
        uint8_t actual;
        GlobalVariables_Read(Global_HandedMode, &actual);
        CHECK_EQUAL(expected, actual);
    }

    void CheckTempoIs(uint8_t expected) {
        uint8_t actual;
        GlobalVariables_Read(Global_Tempo, &actual);
        CHECK_EQUAL(expected, actual);
    }

    void CheckResolvedDistances(int8_t *expected) {
        int8_t actual[DistanceArrayLength] = { 0 };
        GlobalVariables_Read(Global_ResolvedFingerDistances, actual);
        for (uint8_t i = 0; i < GlobalVariables_GetLength(Global_FingerDistances); i++) {
            CHECK_EQUAL(expected[i], actual[i]);
        }
    }
};

TEST(SystemStateManager, ShouldSetStateToIdleOnInit) {
    CheckStateIs(SystemState_Idle);
}

TEST(SystemStateManager, ShouldTransitionFromAnyOtherStateToErrorOnCalibrationError) {
    for (SystemState_t state = 0; state < SystemState_NumberOfStates; state++) {
        if (state != SystemState_CalibrationError) {
            bool error = false;
            GlobalVariables_Write(Global_CalibrationError, &error);

            GlobalVariables_Write(Global_SystemState, &state);
            error = true;
            GlobalVariables_Write(Global_CalibrationError, &error);

            CheckStateIs(SystemState_CalibrationError);
        }
    }
}

TEST(SystemStateManager, ShouldTransitionRunningToErrorOnCalibrationError) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    bool error = true;
    GlobalVariables_Write(Global_CalibrationError, &error);

    CheckStateIs(SystemState_CalibrationError);
}

TEST(SystemStateManager, ShouldTransitionPausedToErrorOnCalibrationError) {
    SystemState_t state = SystemState_Paused;
    GlobalVariables_Write(Global_SystemState, &state);

    bool error = true;
    GlobalVariables_Write(Global_CalibrationError, &error);

    CheckStateIs(SystemState_CalibrationError);
}

TEST(SystemStateManager, ShouldTransitionToIdleWhenCalibrationErrorCleared) {
    bool error = true;
    GlobalVariables_Write(Global_CalibrationError, &error);

    error = false;
    GlobalVariables_Write(Global_CalibrationError, &error);

    CheckStateIs(SystemState_Idle);
}

TEST(SystemStateManager, ShouldCycleThroughModesInIdleWhenModePressed) {
    CheckModeIs(HandedMode_Both);

    PressModeButton();
    CheckModeIs(HandedMode_Left);

    PressModeButton();
    CheckModeIs(HandedMode_Right);
}

TEST(SystemStateManager, ShouldCycleThroughModesInRunningWhenModePressed) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);
    CheckModeIs(HandedMode_Both);

    PressModeButton();
    CheckModeIs(HandedMode_Left);

    PressModeButton();
    CheckModeIs(HandedMode_Right);
}

TEST(SystemStateManager, ShouldCycleThroughModesInPausedWhenModePressed) {
    SystemState_t state = SystemState_Paused;
    GlobalVariables_Write(Global_SystemState, &state);
    CheckModeIs(HandedMode_Both);

    PressModeButton();
    CheckModeIs(HandedMode_Left);

    PressModeButton();
    CheckModeIs(HandedMode_Right);
}

TEST(SystemStateManager, ShouldResolveFingerDistancesOnInit) {
    int8_t distances[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
    };
    CheckResolvedDistances(distances);
}

TEST(SystemStateManager, ShouldResolveFingerDistancesWhenDistancesChange) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    int8_t distances[] = {
        -5, -5, -5, -5, -5,
        5, 5, 5, 5, 5
    };
    GlobalVariables_Write(Global_FingerDistances, distances);

    CheckResolvedDistances(distances);
}

TEST(SystemStateManager, ShouldResolveFingerDistancesWhenModeChangesToLeftHanded) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);
    HandedMode_t mode = HandedMode_Left;
    GlobalVariables_Write(Global_HandedMode, &mode);

    int8_t expected[] = {
        -5, -4, -3, -2, -1,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    CheckResolvedDistances(expected);
}

TEST(SystemStateManager, ShouldResolveFingerDistancesWhenModeChangesToRightHanded) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);
    HandedMode_t mode = HandedMode_Right;
    GlobalVariables_Write(Global_HandedMode, &mode);

    int8_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        1, 2, 3, 4, 5
    };
    CheckResolvedDistances(expected);
}

TEST(SystemStateManager, ShouldResolveFingerDistancesWhenModeChangesToBothHands) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);
    HandedMode_t mode = HandedMode_Both;
    GlobalVariables_Write(Global_HandedMode, &mode);

    int8_t expected[] = {
        -5, -4, -3, -2, -1,
        1, 2, 3, 4, 5
    };
    CheckResolvedDistances(expected);
}

TEST(SystemStateManager, ShouldResolveFingerDistancesToRestWhenStateIsNotRunning) {
    for (SystemState_t state = 0; state < SystemState_NumberOfStates; state++) {
        GlobalVariables_Write(Global_SystemState, &state);
        if (state != SystemState_Running) {
            int8_t expected[] = {
                Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
                Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
            };
            CheckResolvedDistances(expected);
        }
        else {
            int8_t expected[] = {
                -5, -4, -3, -2, -1,
                1, 2, 3, 4, 5
            };
            CheckResolvedDistances(expected);
        }
    }
}

TEST(SystemStateManager, ShouldTransitionIdleToTempoWhenTempoPressed) {
    PressTempoButton();

    CheckStateIs(SystemState_Tempo);
}

TEST(SystemStateManager, ShouldTransitionTempoToIdleWhenTempoPressed) {
    SystemState_t state = SystemState_Tempo;
    GlobalVariables_Write(Global_SystemState, &state);

    PressTempoButton();

    CheckStateIs(SystemState_Idle);
}

TEST(SystemStateManager, ShouldSetTempoToDefaultOnInit) {
    CheckTempoIs(DefaultTempo);
}

TEST(SystemStateManager, ShouldTransitionIdleToRunningWhenStartPressed) {
    PressStartButton();

    CheckStateIs(SystemState_Running);
}

TEST(SystemStateManager, ShouldTransitionRunningToPausedWhenStartPressed) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    PressStartButton();

    CheckStateIs(SystemState_Paused);
}

TEST(SystemStateManager, ShouldTransitionPausedToRunningWhenStartPressed) {
    SystemState_t state = SystemState_Paused;
    GlobalVariables_Write(Global_SystemState, &state);

    PressStartButton();

    CheckStateIs(SystemState_Running);
}

TEST(SystemStateManager, ShouldTransitionRunningToPausedWhenStopPressed) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    PressStopButton();

    CheckStateIs(SystemState_Idle);
}

TEST(SystemStateManager, ShouldTransitionPausedToIdleWhenStopPressed) {
    SystemState_t state = SystemState_Paused;
    GlobalVariables_Write(Global_SystemState, &state);

    PressStopButton();

    CheckStateIs(SystemState_Idle);
}

TEST(SystemStateManager, ShouldChangeSongInIdleWhenLeftPressed) {
    uint8_t index = 1;
    GlobalVariables_Write(Global_SongIndex, &index);
    PressLeftButton();

    CheckStateIs(SystemState_Idle);
    CheckSongIndexIs(0);
}

TEST(SystemStateManager, ShouldChangeSongInIdleWhenRightPressed) {
    PressRightButton();

    CheckStateIs(SystemState_Idle);
    CheckSongIndexIs(1);
}

TEST(SystemStateManager, ShouldWrapAroundSongIndexInIdleWhenLeftPressed) {
    uint8_t index = 0;
    GlobalVariables_Write(Global_SongIndex, &index);
    PressLeftButton();

    CheckStateIs(SystemState_Idle);
    CheckSongIndexIs(Song_NumberOfSongs - 1);
}

TEST(SystemStateManager, ShouldWrapAroundSongIndexInIdleWhenRightPressed) {
    uint8_t index = Song_NumberOfSongs - 1;
    GlobalVariables_Write(Global_SongIndex, &index);
    PressRightButton();

    CheckStateIs(SystemState_Idle);
    CheckSongIndexIs(0);
}

TEST(SystemStateManager, ShouldMoveBackwardOneNoteInRunningWhenLeftPressed) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    uint8_t originalBackwardSignal;
    GlobalVariables_Read(Global_NoteBackwardSignal, &originalBackwardSignal);
    PressLeftButton();

    CheckStateIs(SystemState_Running);
    uint8_t currentBackwardSignal;
    GlobalVariables_Read(Global_NoteBackwardSignal, &currentBackwardSignal);
    CHECK(originalBackwardSignal != currentBackwardSignal);
}

TEST(SystemStateManager, ShouldMoveForwardOneNoteInRunningWhenRightPressed) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    uint8_t originalForwardSignal;
    GlobalVariables_Read(Global_NoteForwardSignal, &originalForwardSignal);
    PressRightButton();

    CheckStateIs(SystemState_Running);
    uint8_t currentForwardSignal;
    GlobalVariables_Read(Global_NoteForwardSignal, &currentForwardSignal);
    CHECK(originalForwardSignal != currentForwardSignal);
}

TEST(SystemStateManager, ShouldDecreaseTempoInTempoStateWhenLeftPressed) {
    SystemState_t state = SystemState_Tempo;
    GlobalVariables_Write(Global_SystemState, &state);

    uint8_t tempo = DefaultTempo;
    GlobalVariables_Write(Global_Tempo, &tempo);
    PressLeftButton();

    CheckTempoIs(DefaultTempo - 1);
}

TEST(SystemStateManager, ShouldIncreaseTempoInTempoStateWhenRightPressed) {
    SystemState_t state = SystemState_Tempo;
    GlobalVariables_Write(Global_SystemState, &state);

    uint8_t tempo = DefaultTempo;
    GlobalVariables_Write(Global_Tempo, &tempo);
    PressRightButton();

    CheckTempoIs(DefaultTempo + 1);
}

TEST(SystemStateManager, ShouldNotAllowTempoToChangeOutOfBounds) {
    SystemState_t state = SystemState_Tempo;
    GlobalVariables_Write(Global_SystemState, &state);

    uint8_t tempo = MinTempo;
    GlobalVariables_Write(Global_Tempo, &tempo);
    PressLeftButton();

    CheckTempoIs(MinTempo);

    tempo = MaxTempo;
    GlobalVariables_Write(Global_Tempo, &tempo);
    PressRightButton();

    CheckTempoIs(MaxTempo);
}
