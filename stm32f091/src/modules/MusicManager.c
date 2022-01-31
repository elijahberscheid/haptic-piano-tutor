#include "GlobalVariables.h"
#include "MusicManager.h"

#include <stdio.h>

#define IGNORE(x) (void)x

// static Note_t FindCurrentNote(MusicManager_t *instance, Note_t *channel);

static void UpdateDesiredFingerPositions(MusicManager_t *instance) {
    uint8_t index = 0;
    GlobalVariables_Read(Global_SongIndex, &index);
    const Song_t *song = &instance->config->songs[index];

    Key_t positions[] = {
        Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid,
        Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid,
    };

    for (uint8_t i = 0; i < Song_MaxConcurrentNotes; i++) {
        if (song->channels[i] != NULL) {
            Finger_t finger = song->channels[i][0].finger;
            positions[finger] = song->channels[i][0].key;
        }
    }

    GlobalVariables_Write(Global_DesiredFingerPositions, positions);
}

static void PlaceInMusicChanged(const void *data) {
    IGNORE(data);
}

// TODO: delete this, not needed for this module, just for demo purposes
static void FingerPositionsChanged(const void *data) {
    const uint8_t *fingerPositions = (const uint8_t *) data;
    printf("fingerPositions[0] = %u\n", fingerPositions[0]);
}

void MusicManager_Init(MusicManager_t *instance, const MusicManagerConfig_t *config) {
    instance->config = config;
    instance->ticks = 0;

    UpdateDesiredFingerPositions(instance);

    GlobalVariables_Subscribe(Global_FingerPositions, FingerPositionsChanged);
    GlobalVariables_Subscribe(Global_SoundDetectedSignal, PlaceInMusicChanged);
}
