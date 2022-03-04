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
        DistanceArrayLength = 10
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

TEST(SystemStateManager, ShouldResolveFingerDistancesToRestWhenStateIsNotRunning) {
    int8_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
    };

    for (SystemState_t state = 0; state < SystemState_NumberOfStates; state++) {
        if (state != SystemState_Idle) {
            GlobalVariables_Write(Global_SystemState, &state);
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

TEST(SystemStateManager, ShouldChangeSongInIdleWhenRightPressed) {
    PressRightButton();

    CheckStateIs(SystemState_Idle);
    CheckSongIndexIs(1);
}

TEST(SystemStateManager, ShouldWrapAroundSongIndexInIdleWhenRightPressed) {
    uint8_t index = Song_NumberOfSongs - 1;
    GlobalVariables_Write(Global_SongIndex, &index);
    PressRightButton();

    CheckStateIs(SystemState_Idle);
    CheckSongIndexIs(0);
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
