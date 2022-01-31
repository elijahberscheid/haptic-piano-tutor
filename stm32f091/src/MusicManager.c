#include "MusicManager.h"
#include "GlobalVariables.h"

#include <stdio.h>

#define IGNORE(x) (void)x

static void PlaceInMusicChanged(void *context, const void *data) {
    IGNORE(context);
    IGNORE(data);
}

static void FingerPositionsChanged(void *context, const void *data) {
    MusicManager_t *instance = (MusicManager_t *) context;
    const uint8_t *fingerPositions = (const uint8_t *) data;

    printf("placeholder = %c\n", instance->placeholder);
    printf("fingerPositions[0] = %u\n", fingerPositions[0]);

    const GlobalVariables_Subscription_t fingerPositionsSubscription = { .context = instance, .callback = FingerPositionsChanged };
    GlobalVariables_Unsubscribe(Global_FingerPositions, &fingerPositionsSubscription);
}

void MusicManager_Init(MusicManager_t *instance) {
    instance->placeholder = 'A';

    const GlobalVariables_Subscription_t fingerPositionsSubscription = { .context = instance, .callback = FingerPositionsChanged };
    GlobalVariables_Subscribe(Global_FingerPositions, &fingerPositionsSubscription);

    const GlobalVariables_Subscription_t soundDetectedSubscription = { .context = instance, .callback = PlaceInMusicChanged };
    GlobalVariables_Subscribe(Global_SoundDetectedSignal, &soundDetectedSubscription);
}
