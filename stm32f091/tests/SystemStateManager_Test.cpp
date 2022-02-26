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
        SystemStateManager_Init();
    }

    void teardown() {
    }

    void PressStartButton() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_StartButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_StartButtonSignal, &signal);
    }

    void PressRightButton() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_RightButtonSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_RightButtonSignal, &signal);
    }

    void CheckStateIs(uint8_t actual) {
        SystemState_t expected;
        GlobalVariables_Read(Global_SystemState, &expected);
        CHECK_EQUAL(expected, actual);
    }

    void CheckSongIndexIs(uint8_t actual) {
        uint8_t expected;
        GlobalVariables_Read(Global_SongIndex, &expected);
        CHECK_EQUAL(expected, actual);
    }
};

TEST(SystemStateManager, ShouldSetStateToIdleOnInit) {
    CheckStateIs(SystemState_Idle);
}

TEST(SystemStateManager, ShouldTransitionIdleToRunningWhenStartPressed) {
    PressStartButton();

    CheckStateIs(SystemState_Running);
}

TEST(SystemStateManager, ShouldTransitionRunningToPausedWhenStartPressed) {
    PressStartButton();
    PressStartButton();

    CheckStateIs(SystemState_Paused);
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
    PressStartButton();

    uint8_t originalForwardSignal;
    GlobalVariables_Read(Global_NoteForwardSignal, &originalForwardSignal);
    PressRightButton();

    CheckStateIs(SystemState_Running);
    uint8_t currentForwardSignal;
    GlobalVariables_Read(Global_NoteForwardSignal, &currentForwardSignal);
    CHECK(originalForwardSignal != currentForwardSignal);
}
