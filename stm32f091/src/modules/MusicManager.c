#include "GlobalVariables.h"
#include "MusicManager.h"
#include "SystemStateManager.h"

#include <stdio.h>

#define IGNORE(x) ((void)x)

enum {
    NoteIndexNotFound = UINT32_MAX,
    TickNumberNotFound = UINT32_MAX
};

// Returns TickNumberNotFound if the next note is invalid (i.e. there is no next note)
static uint32_t FindNextTicks(MusicManager_t *instance, const Note_t *channel) {
    uint32_t nextTicks = 0;
    uint32_t noteIndex = 0;

    if (channel == NULL) {
        nextTicks = TickNumberNotFound;
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
            nextTicks = TickNumberNotFound;
        }
    }

    return nextTicks;
}

// if instance->ticks is past the end of channel, returns the ticks corresponding to the last note
static uint32_t FindPreviousTicks(MusicManager_t *instance, const Note_t *channel) {
    uint32_t previousTicks = 0;
    uint32_t noteIndex = 0;

    if (channel == NULL) {
        previousTicks = 0;
    }
    else {
        for (noteIndex = 0; channel[noteIndex].key != Key_Invalid; noteIndex++) {
            if (previousTicks < instance->ticks) {
                previousTicks += channel[noteIndex].length;
            }
            else {
                break;
            }
        }
        if (noteIndex > 0) { // correct for overshooting by one note
            noteIndex--;
            previousTicks -= channel[noteIndex].length;
        }
        else { // already at the first note in the channel
            previousTicks = 0;
        }
    }

    return previousTicks;
}

// Returns NoteIndexNotFound if there is no note at the exactly the current number of ticks
static uint32_t FindCurrentNoteIndex(MusicManager_t *instance, const Note_t *channel) {
    uint32_t nextTicks = 0;
    uint32_t noteIndex = 0;

    if (channel == NULL) {
        nextTicks = NoteIndexNotFound;
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

    return (nextTicks == instance->ticks)? noteIndex : NoteIndexNotFound;
}

static void UpdateDesiredFingerPositions(MusicManager_t *instance) {
    uint8_t index = 0;
    GlobalVariables_Read(Global_SongIndex, &index);
    const Song_t *song = &instance->config->songs[index];

    Key_t positions[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
    };
    NoteLength_t shortestNoteLength = UINT8_MAX;

    for (uint8_t channelIndex = 0; channelIndex < Song_MaxConcurrentNotes; channelIndex++) {
        if (song->channels[channelIndex] != NULL) {
            uint32_t noteIndex = FindCurrentNoteIndex(instance, song->channels[channelIndex]);
            if (noteIndex != NoteIndexNotFound) {
                Finger_t finger = song->channels[channelIndex][noteIndex].finger;
                positions[finger] = song->channels[channelIndex][noteIndex].key;
                NoteLength_t length = song->channels[channelIndex][noteIndex].length;
                if (length < shortestNoteLength) {
                    shortestNoteLength = length;
                }
            }

        }
    }

    GlobalVariables_Write(Global_DesiredFingerPositions, positions);
    GlobalVariables_Write(Global_CurrentNoteLength, &instance->currentNoteLength);

    instance->currentNoteLength = shortestNoteLength;
}

static void SoundDetected(void *context, const void *data) {
    MusicManager_t *instance = (MusicManager_t *) context;
    IGNORE(data);

    SystemState_t state;
    GlobalVariables_Read(Global_SystemState, &state);

    if (state == SystemState_Running) {
        uint8_t index = 0;
        GlobalVariables_Read(Global_SongIndex, &index);
        const Song_t *song = &instance->config->songs[index];

        uint32_t possibleNextTicks[Song_MaxConcurrentNotes] = { 0 };
        for (uint8_t channelIndex = 0; channelIndex < Song_MaxConcurrentNotes; channelIndex++) {
            possibleNextTicks[channelIndex] = FindNextTicks(instance, song->channels[channelIndex]);
        }

        uint32_t minPossibleNextTick = UINT32_MAX;
        for (uint8_t channelIndex = 0; channelIndex < Song_MaxConcurrentNotes; channelIndex++) {
            if ((possibleNextTicks[channelIndex] < minPossibleNextTick) && (possibleNextTicks[channelIndex] != TickNumberNotFound)) {
                minPossibleNextTick = possibleNextTicks[channelIndex];
            }
        }
        instance->ticks = minPossibleNextTick;

        UpdateDesiredFingerPositions(instance);
    }
}

static void NoteBackwardChanged(void *context, const void *data) {
    MusicManager_t *instance = (MusicManager_t *) context;
    IGNORE(data);

    // not needed to check for running state,
    // because will always be at beginnning of song in any state other than running, so it will not go back
    uint8_t index = 0;
    GlobalVariables_Read(Global_SongIndex, &index);
    const Song_t *song = &instance->config->songs[index];

    uint32_t possiblePreviousTicks[Song_MaxConcurrentNotes] = { 0 };
    for (uint8_t channelIndex = 0; channelIndex < Song_MaxConcurrentNotes; channelIndex++) {
        possiblePreviousTicks[channelIndex] = FindPreviousTicks(instance, song->channels[channelIndex]);
    }

    uint32_t maxPossiblePreviousTick = 0;
    for (uint8_t channelIndex = 0; channelIndex < Song_MaxConcurrentNotes; channelIndex++) {
        if (possiblePreviousTicks[channelIndex] > maxPossiblePreviousTick) {
            maxPossiblePreviousTick = possiblePreviousTicks[channelIndex];
        }
    }
    instance->ticks = maxPossiblePreviousTick;

    UpdateDesiredFingerPositions(instance);
}

static void SongIndexChanged(void *context, const void *data) {
    MusicManager_t *instance = (MusicManager_t *) context;
    IGNORE(data);

    instance->ticks = 0;
    instance->currentNoteLength = 0;
    UpdateDesiredFingerPositions(instance);
}

static void SystemStateChanged(void *context, const void *data) {
    MusicManager_t *instance = (MusicManager_t *) context;
    SystemState_t *state = (SystemState_t *) data;

    if (*state == SystemState_Idle) {
        instance->ticks = 0;
        instance->currentNoteLength = 0;
        UpdateDesiredFingerPositions(instance);
    }
}

void MusicManager_Init(MusicManager_t *instance, const MusicManagerConfig_t *config) {
    instance->config = config;
    instance->ticks = 0;
    instance->currentNoteLength = 0;

    UpdateDesiredFingerPositions(instance);

    const GlobalVariables_Subscription_t soundDetectedSubscription = { .context = instance, .callback = SoundDetected };
    GlobalVariables_Subscribe(Global_SoundDetectedSignal, &soundDetectedSubscription);
    // note forward does the exact same thing as sound detected
    GlobalVariables_Subscribe(Global_NoteForwardSignal, &soundDetectedSubscription);
    const GlobalVariables_Subscription_t noteBackwardSubscription = { .context = instance, .callback = NoteBackwardChanged };
    GlobalVariables_Subscribe(Global_NoteBackwardSignal, &noteBackwardSubscription);
    const GlobalVariables_Subscription_t songIndexSubscription = { .context = instance, .callback = SongIndexChanged };
    GlobalVariables_Subscribe(Global_SongIndex, &songIndexSubscription);
    const GlobalVariables_Subscription_t systemStateSubscription = { .context = instance, .callback = SystemStateChanged };
    GlobalVariables_Subscribe(Global_SystemState, &systemStateSubscription);
}
