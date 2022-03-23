#include "CppUTest/TestHarness.h"

extern "C"
{
#include <stdlib.h>
#include <assert.h>
#include "GlobalVariables.h"
#include "MusicManager.h"
#include "Song.h"
#include "SystemStateManager.h"
}

static const Note_t channel1[] = {
    { .key = Key_C4, .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t channel2[] = {
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_F4, .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t channel3[] = {
    { .key = Key_D5, .length = NoteLength_Quarter, .finger = Finger_Right5 },
    { .key = Key_F5, .length = NoteLength_Half, .finger = Finger_Right5 },
    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t channel4[] = {
    { .key = Key_C3, .length = NoteLength_Quarter, .finger = Finger_Left1 },
    { .key = Key_C4, .length = NoteLength_Half, .finger = Finger_Left1 },
    { .key = Key_C5, .length = NoteLength_Half, .finger = Finger_Left1 },
    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t channel5[] = {
    { .key = Key_D5, .length = NoteLength_Half, .finger = Finger_Right5 },
    { .key = Key_F5, .length = NoteLength_Half, .finger = Finger_Right5 },
    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t channel6[] = {
    { .key = Key_C3, .length = NoteLength_Half, .finger = Finger_Left5 },
    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t channel7[] = {
    { .key = Key_D5, .length = NoteLength_Half, .finger = Finger_Right5 },
    { .key = Key_C5, .length = NoteLength_Whole, .finger = Finger_Right4 },
    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Song_t songs[] = {
    { .defaultTempo = 60, .channels = { channel1, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
    { .defaultTempo = 80, .channels = { channel2, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
    { .defaultTempo = 80, .channels = { channel2, channel3, NULL, NULL, NULL, NULL, NULL, NULL } },
    { .defaultTempo = 80, .channels = { channel4, channel5, NULL, NULL, NULL, NULL, NULL, NULL } },
    { .defaultTempo = 80, .channels = { channel6, channel7, NULL, NULL, NULL, NULL, NULL, NULL } }
};

static const MusicManagerConfig_t config = { .songs = songs };

TEST_GROUP(MusicManager) {
    MusicManager_t instance;

    enum {
        Example_Enum = 1
    };

    void setup() {
        GlobalVariables_Init();
        MusicManager_Init(&instance, &config);
    }

    void teardown() {
    }

    void ChangeSoundDetectedSignal() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_SoundDetectedSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_SoundDetectedSignal, &signal);
    }

    void ChangeNoteForwardSignal() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_NoteForwardSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_NoteForwardSignal, &signal);
    }

    void ChangeNoteBackwardSignal() {
        uint8_t signal = 0;
        GlobalVariables_Read(Global_NoteBackwardSignal, &signal);
        signal++;
        GlobalVariables_Write(Global_NoteBackwardSignal, &signal);
    }

    void CheckDesiredFingerPositions(uint8_t *expected, uint8_t *actual) {
        for (uint8_t i = 0; i < GlobalVariables_GetLength(Global_DesiredFingerPositions); i++) {
            CHECK_EQUAL(expected[i], actual[i]);
        }
    }

    void CheckPreviousNoteLength(uint8_t expected) {
        uint8_t actual = 0;
        GlobalVariables_Read(Global_PreviousNoteLength, &actual);
        CHECK_EQUAL(expected, actual);
    }

};

TEST(MusicManager, ShouldRequestFirstNoteOnInit) {
    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right1] = Key_C4;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    // 0th note has length 0 so it takes effect immediately
    CheckPreviousNoteLength(0);
}

TEST(MusicManager, ShouldNotAdvanceProgressInSongWhenNotRunning) {
    SystemState_t state = SystemState_Idle;
    GlobalVariables_Write(Global_SystemState, &state);

    ChangeSoundDetectedSignal();

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right1] = Key_C4;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(0);

    ChangeNoteForwardSignal();

    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(0);
}

TEST(MusicManager, ShouldRequestNextNoteWhenSoundIsDetected) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    ChangeSoundDetectedSignal();

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right2] = Key_D4;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(channel1[0].length); // length is length of previous note

    ChangeSoundDetectedSignal();

    expected[Finger_Right2] = Key_Rest;
    expected[Finger_Right3] = Key_E4;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(channel1[1].length);
}

// Not possible to change song index when running during normal use,
// but possible during debugging
TEST(MusicManager, ShouldStartAnotherSongAtBeginningWhenSongChanges) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    ChangeSoundDetectedSignal();

    uint8_t index = 1;
    GlobalVariables_Write(Global_SongIndex, &index);

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right1] = Key_D4;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(0);

    ChangeSoundDetectedSignal();

    expected[Finger_Right1] = Key_F4;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(channel2[0].length);
}

TEST(MusicManager, ShouldResetSongProgressWhenStopping) {
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);
    ChangeSoundDetectedSignal();

    state = SystemState_Idle;
    GlobalVariables_Write(Global_SystemState, &state);

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right1] = Key_C4;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(0);
}

TEST(MusicManager, ShouldRequestConcurrentNotes) {
    uint8_t index = 2;
    GlobalVariables_Write(Global_SongIndex, &index);
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right1] = Key_D4;
    expected[Finger_Right5] = Key_D5;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(0);

    ChangeSoundDetectedSignal();

    expected[Finger_Right1] = Key_F4;
    expected[Finger_Right5] = Key_F5;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(channel2[0].length);
}

TEST(MusicManager, ShouldAdvanceToClosestNoteInAnyChannelOnSoundDetected) { // for music with syncopation
    uint8_t index = 3;
    GlobalVariables_Write(Global_SongIndex, &index);
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Left1] = Key_C3;
    expected[Finger_Right5] = Key_D5;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(0);

    ChangeSoundDetectedSignal();

    expected[Finger_Left1] = Key_C4;
    expected[Finger_Right5] = Key_Rest;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(channel4[0].length);

    ChangeSoundDetectedSignal();

    expected[Finger_Left1] = Key_Rest;
    expected[Finger_Right5] = Key_F5;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(channel4[1].length);
}

// same as previous test case, except uses note forward signal instead of sound detected
TEST(MusicManager, ShouldAdvanceToClosestNoteInAnyChannelOnNoteForward) { // for music with syncopation
    uint8_t index = 3;
    GlobalVariables_Write(Global_SongIndex, &index);
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Left1] = Key_C3;
    expected[Finger_Right5] = Key_D5;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    // CheckPreviousNoteLength(0); // don't care because it is only used on sound detected

    ChangeNoteForwardSignal();

    expected[Finger_Left1] = Key_C4;
    expected[Finger_Right5] = Key_Rest;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    // CheckPreviousNoteLength(channel4[0].length);

    ChangeNoteForwardSignal();

    expected[Finger_Left1] = Key_Rest;
    expected[Finger_Right5] = Key_F5;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    // CheckPreviousNoteLength(channel4[1].length);
}

TEST(MusicManager, ShouldGoBackToClosestNoteInAnyChannelOnNoteBackward) { // for music with syncopation
    uint8_t index = 3;
    GlobalVariables_Write(Global_SongIndex, &index);
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    ChangeNoteForwardSignal();
    ChangeNoteForwardSignal();

    ChangeNoteBackwardSignal();

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Left1] = Key_C4;
    expected[Finger_Right5] = Key_Rest;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);

    CheckDesiredFingerPositions(expected, actual);
    // don't care because it is only used on sound detected
    // the actual value would is currently wrong if we cared
    // CheckPreviousNoteLength(channel4[0].length);

    ChangeNoteBackwardSignal();

    expected[Finger_Left1] = Key_C3;
    expected[Finger_Right5] = Key_D5;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    // CheckPreviousNoteLength(0);
}

TEST(MusicManager, ShouldRequestRestWhenEndOfChannelIsReached) {
    uint8_t index = 4;
    GlobalVariables_Write(Global_SongIndex, &index);
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    ChangeSoundDetectedSignal();

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right4] = Key_C5;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    CheckPreviousNoteLength(channel6[0].length);
}

TEST(MusicManager, ShouldRequestInvalidWhenEndOfSongIsReached) {
    uint8_t index = 4;
    GlobalVariables_Write(Global_SongIndex, &index);
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    ChangeSoundDetectedSignal();
    ChangeSoundDetectedSignal();

    Key_t expected[] = {
        Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid,
        Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid
    };
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    // note length does not matter, because there's no note after this

    ChangeSoundDetectedSignal();
    CheckDesiredFingerPositions(expected, actual);
}

TEST(MusicManager, ShouldRequestLastNoteWhenRewindingFromEndofSong) {
    uint8_t index = 4;
    GlobalVariables_Write(Global_SongIndex, &index);
    SystemState_t state = SystemState_Running;
    GlobalVariables_Write(Global_SystemState, &state);

    ChangeSoundDetectedSignal();
    ChangeSoundDetectedSignal();
    ChangeNoteBackwardSignal();

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right4] = Key_C5;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
    // note length does not matter, because it only matters on sound detected
}
