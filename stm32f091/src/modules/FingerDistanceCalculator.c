#include <stdint.h>
#include <stdio.h>
#include "FingerDistanceCalculator.h"
#include "GlobalVariables.h"
#include "Note.h"

#define IGNORE(x) ((void)x)

enum {
    DistanceArrayLength = 10
};

static void WriteDistances(uint8_t *expected, uint8_t *actual) {
    int8_t difference[DistanceArrayLength] = { 0 };
    for (uint8_t fingerIndex = 0; fingerIndex < DistanceArrayLength; fingerIndex++) {
        if (expected[fingerIndex] == Key_Rest) {
            difference[fingerIndex] = Key_Rest;
        }
        else {
            difference[fingerIndex] = expected[fingerIndex] - actual[fingerIndex];
        }
    }
    GlobalVariables_Write(Global_FingerDistances, &difference);
}

static void DesiredPositionsChanged(void *context, const void *data) {
    IGNORE(context);
    uint8_t *expected = (uint8_t *) data;

    uint8_t actual[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_FingerPositions, &actual);

    WriteDistances(expected, actual);
}

static void FingerPositionsChanged(void *context, const void *data) {
    IGNORE(context);
    uint8_t *actual = (uint8_t *) data;

    uint8_t expected[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, &expected);

    WriteDistances(expected, actual);
}

void FingerDistanceCalculator_Init(void) {
    const GlobalVariables_Subscription_t desiredPositionsSubscription = { .context = NULL, .callback = DesiredPositionsChanged };
    GlobalVariables_Subscribe(Global_DesiredFingerPositions, &desiredPositionsSubscription);

    const GlobalVariables_Subscription_t fingerPositionsSubscription = { .context = NULL, .callback = FingerPositionsChanged };
    GlobalVariables_Subscribe(Global_FingerPositions, &fingerPositionsSubscription);

    uint8_t expected[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, &expected);
    uint8_t actual[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_FingerPositions, &actual);

    WriteDistances(expected, actual);
}
