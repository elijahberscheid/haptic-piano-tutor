#include <stdint.h>
#include <stdio.h>
#include "FingerDistanceCalculator.h"
#include "GlobalVariables.h"
#include "Note.h"

#define IGNORE(x) ((void)x)

enum {
    DistanceArrayLength = 10
};

static void WriteDistances(void) {
    uint8_t expected[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, &expected);

    uint8_t actual[DistanceArrayLength] = { 0 };
    GlobalVariables_Read(Global_FingerPositions, &actual);

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
    IGNORE(data);

    WriteDistances();
}

void FingerDistanceCalculator_Init(void) {
    const GlobalVariables_Subscription_t desiredPositionSubscription = { .context = NULL, .callback = DesiredPositionsChanged };
    GlobalVariables_Subscribe(Global_DesiredFingerPositions, &desiredPositionSubscription);

    WriteDistances();
}
