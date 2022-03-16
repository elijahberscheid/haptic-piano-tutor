#include "CppUTest/TestHarness.h"

extern "C"
{
#include <stdlib.h>
#include <assert.h>
#include "GlobalVariables.h"
#include "FingerDistanceResolver.h"
#include "Song.h"
#include "SystemStateManager.h"
}

TEST_GROUP(FingerDistanceResolver) {
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

        // set the defaults that system state manager would have set
        SystemState_t state = SystemState_Idle;
        GlobalVariables_Write(Global_SystemState, &state);

        HandedMode_t mode = HandedMode_Both;
        GlobalVariables_Write(Global_HandedMode, &mode);

        FingerDistanceResolver_Init();
    }

    void teardown() {
    }

    void CheckResolvedDistances(int8_t *expected) {
        int8_t actual[DistanceArrayLength] = { 0 };
        GlobalVariables_Read(Global_ResolvedFingerDistances, actual);
        for (uint8_t i = 0; i < GlobalVariables_GetLength(Global_FingerDistances); i++) {
            CHECK_EQUAL(expected[i], actual[i]);
        }
    }
};

TEST(FingerDistanceResolver, ShouldResolveFingerDistancesOnInit) {
    int8_t distances[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
    };
    CheckResolvedDistances(distances);
}

TEST(FingerDistanceResolver, ShouldResolveFingerDistancesWhenDistancesChange) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    int8_t distances[] = {
        -5, -5, -5, -5, -5,
        5, 5, 5, 5, 5
    };
    GlobalVariables_Write(Global_FingerDistances, distances);

    CheckResolvedDistances(distances);
}

TEST(FingerDistanceResolver, ShouldResolveFingerDistancesWhenModeChangesToLeftHanded) {
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

TEST(FingerDistanceResolver, ShouldResolveFingerDistancesWhenModeChangesToRightHanded) {
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

TEST(FingerDistanceResolver, ShouldResolveFingerDistancesWhenModeChangesToBothHands) {
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

TEST(FingerDistanceResolver, ShouldResolveFingerDistancesToRestWhenStateIsNotRunning) {
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

