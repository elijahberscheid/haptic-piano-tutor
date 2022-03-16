#include <stdio.h>
#include "FingerDistanceResolver.h"
#include "GlobalVariables.h"
#include "Song.h"
#include "SystemStateManager.h"

#define IGNORE(x) ((void)x)

enum {
    DistanceArrayLength = 10
};

static void ResolveFingerDistances(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);

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

void FingerDistanceResolver_Init(void) {
    const GlobalVariables_Subscription_t resolveFingerDistancesSubscription = { .context = NULL, .callback = ResolveFingerDistances };
    GlobalVariables_Subscribe(Global_FingerDistances, &resolveFingerDistancesSubscription);
    GlobalVariables_Subscribe(Global_HandedMode, &resolveFingerDistancesSubscription);
    GlobalVariables_Subscribe(Global_SystemState, &resolveFingerDistancesSubscription);

    ResolveFingerDistances(NULL, NULL);
}
