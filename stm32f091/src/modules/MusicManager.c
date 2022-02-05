#include "GlobalVariables.h"
#include "MusicManager.h"

#include <stdio.h>

#define IGNORE(x) ((void)x)

// Returns UINT32_MAX if the next note is invalid (i.e. there is no next note)
static uint32_t FindNextTicks(MusicManager_t *instance, const Note_t *channel) {
    uint32_t nextTicks = 0;
    uint32_t noteIndex = 0;

    if (channel == NULL) {
        nextTicks = UINT32_MAX;
    }
    else {
        for (noteIndex = 0; channel[noteIndex].key != Key_Invalid; noteIndex++) {
            if (nextTicks <= instance->ticks) {
                nextTicks += channel[noteIndex].length;
            }
            else {
                break;
            }
        }

        if (channel[noteIndex].key == Key_Invalid) {
            nextTicks = UINT32_MAX;
        }
    }

    return nextTicks;
}

// Returns UINT32_MAX if there is no note at the exactly the current number of ticks
static uint32_t FindCurrentNoteIndex(MusicManager_t *instance, const Note_t *channel) {
    uint32_t nextTicks = 0;
    uint32_t noteIndex = 0;

    if (channel == NULL) {
        nextTicks = UINT32_MAX;
    }
    else {
        for (noteIndex = 0; channel[noteIndex].key != Key_Invalid; noteIndex++) {
            if (nextTicks <= instance->ticks) {
                nextTicks += channel[noteIndex].length;
            }
            else {
                break;
            }
        }
        // correct for overshooting by one note
        noteIndex--;
        nextTicks -= channel[noteIndex].length;
    }

    return (nextTicks == instance->ticks)? noteIndex : UINT32_MAX;
}

static void UpdateDesiredFingerPositions(MusicManager_t *instance) {
    uint8_t index = 0;
    GlobalVariables_Read(Global_SongIndex, &index);
    const Song_t *song = &instance->config->songs[index];

    Key_t positions[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
    };

    for (uint8_t channelIndex = 0; channelIndex < Song_MaxConcurrentNotes; channelIndex++) {
        if (song->channels[channelIndex] != NULL) {
            uint32_t noteIndex = FindCurrentNoteIndex(instance, song->channels[channelIndex]);
            Finger_t finger = song->channels[channelIndex][noteIndex].finger;
            positions[finger] = song->channels[channelIndex][noteIndex].key;
        }
    }

    GlobalVariables_Write(Global_DesiredFingerPositions, positions);
}

static void PlaceInMusicChanged(void *context, const void *data) {
    MusicManager_t *instance = (MusicManager_t *) context;
    IGNORE(data);

    uint8_t index = 0;
    GlobalVariables_Read(Global_SongIndex, &index);
    const Song_t *song = &instance->config->songs[index];

    uint32_t possibleNextTicks[Song_MaxConcurrentNotes] = { 0 };
    for (uint8_t channelIndex = 0; channelIndex < Song_MaxConcurrentNotes; channelIndex++) {
        possibleNextTicks[channelIndex] = FindNextTicks(instance, song->channels[channelIndex]);
    }

    for (uint8_t channelIndex = 0; channelIndex < Song_MaxConcurrentNotes; channelIndex++) {
        if ((possibleNextTicks[channelIndex] > instance->ticks) && (possibleNextTicks[channelIndex] != UINT32_MAX)) {
            instance->ticks = possibleNextTicks[channelIndex];
        }
    }

    UpdateDesiredFingerPositions(instance);
}

void MusicManager_Init(MusicManager_t *instance, const MusicManagerConfig_t *config) {
    instance->config = config;
    instance->ticks = 0;

    UpdateDesiredFingerPositions(instance);

    const GlobalVariables_Subscription_t soundDetectedSubscription = { .context = instance, .callback = PlaceInMusicChanged };
    GlobalVariables_Subscribe(Global_SoundDetectedSignal, &soundDetectedSubscription);
}
