#include "MusicManager.h"
#include "GlobalVariables.h"

#include <stdio.h>

#define IGNORE(x) (void)x

static void PlaceInMusicChanged(const void *data) {
    IGNORE(data);
}

static void FingerPositionsChanged(const void *data) {
    const uint8_t *fingerPositions = (const uint8_t *) data;
    printf("fingerPositions[0] = %u\n", fingerPositions[0]);
}

void MusicManager_Init(void) {
    GlobalVariables_Subscribe(Global_FingerPositions, FingerPositionsChanged);
    GlobalVariables_Subscribe(Global_SoundDetectedSignal, PlaceInMusicChanged);
}
