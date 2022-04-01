#include "CppUTest/TestHarness.h"

extern "C"
{
#include <stdlib.h>
#include <assert.h>
#include "FingerDistanceCalculator.h"
#include "GlobalVariables.h"
#include "Note.h"
}

TEST_GROUP(FingerDistanceCalculator) {
    enum {
        DistanceArrayLength = 10
    };

    void setup() {
        GlobalVariables_Init();
        uint8_t positions[] = {
            Key_C3, Key_D3, Key_E3, Key_F3, Key_G3,
            Key_C4, Key_D4, Key_E4, Key_F4, Key_G4
        };
        GlobalVariables_Write(Global_FingerPositions, &positions);

        for (uint8_t fingerIndex = 0; fingerIndex < DistanceArrayLength; fingerIndex++) {
            positions[fingerIndex] = Key_Rest;
        }
        GlobalVariables_Write(Global_DesiredFingerPositions, &positions);

        FingerDistanceCalculator_Init();
    }

    void teardown() {
    }

    void CheckDistances(int8_t *expected, int8_t *actual) {
        for (uint8_t i = 0; i < GlobalVariables_GetLength(Global_FingerDistances); i++) {
            CHECK_EQUAL(expected[i], actual[i]);
        }
    }

};

TEST(FingerDistanceCalculator, ShouldSetDistancesToRestWhenDesiredPositionsAreRest) {
    int8_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    int8_t actual[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_FingerDistances, actual);
    CheckDistances(expected, actual);
}

TEST(FingerDistanceCalculator, ShouldSetDistancesToRestWhenDesiredPositionsAreInvalid) {
    uint8_t positions[] = {
        Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid,
        Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid
    };
    GlobalVariables_Write(Global_DesiredFingerPositions, &positions);

    int8_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    int8_t actual[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_FingerDistances, actual);
    CheckDistances(expected, actual);
}

TEST(FingerDistanceCalculator, ShouldSetDistancesWhenDesiredPositionsChange) {
    uint8_t positions[] = {
        Key_G1, Key_A2, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_C7, Key_D7
    };
    GlobalVariables_Write(Global_DesiredFingerPositions, &positions);

    int8_t expected[] = {
        Key_G1 - Key_C3, Key_A2 - Key_D3, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_C7 - Key_F4, Key_D7 - Key_G4
    };
    int8_t actual[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_FingerDistances, actual);
    CheckDistances(expected, actual);
}

TEST(FingerDistanceCalculator, ShouldSetDistancesWhenFingerPositionsChange) {
    uint8_t desiredFingerPositions[] = {
        Key_A1, Key_B1, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_D7, Key_E7
    };
    GlobalVariables_Write(Global_DesiredFingerPositions, &desiredFingerPositions);

    uint8_t fingerPositions[] = {
        Key_A1, Key_A2, Key_A3, Key_A4, Key_A5,
        Key_B1, Key_B2, Key_B3, Key_B4, Key_B5
    };
    GlobalVariables_Write(Global_FingerPositions, &fingerPositions);

    int8_t expected[] = {
        Key_A1 - Key_A1, Key_B1 - Key_A2, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_D7 - Key_B4, Key_E7 - Key_B5
    };
    int8_t actual[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_FingerDistances, actual);
    CheckDistances(expected, actual);
}

TEST(FingerDistanceCalculator, ShouldSetDistancesToRestForFingersThatAreNotDetected) {
    uint8_t positions[] = {
        Key_G1, Key_A2, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_C7, Key_D7
    };
    GlobalVariables_Write(Global_DesiredFingerPositions, &positions);

    uint8_t fingerPositions[] = {
        Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid,
        Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid
    };
    GlobalVariables_Write(Global_FingerPositions, &fingerPositions);

    int8_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    int8_t actual[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_FingerDistances, actual);
    CheckDistances(expected, actual);
}
