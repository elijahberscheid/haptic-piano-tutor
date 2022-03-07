#include "CppUTest/TestHarness.h"

extern "C"
{
#include <stdlib.h>
#include <assert.h>
#include "GlobalVariables.h"
#include "MusicManager.h"
#include "Song.h"
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
    { .key = Key_F5, .length = NoteLength_Quarter, .finger = Finger_Right5 },
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
    { .key = Key_C5, .length = NoteLength_Half, .finger = Finger_Right4 },
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
}

TEST(MusicManager, ShouldRequestNextNoteWhenSoundIsDetected) {
    ChangeSoundDetectedSignal();

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right2] = Key_D4;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);

    ChangeSoundDetectedSignal();

    expected[Finger_Right2] = Key_Rest;
    expected[Finger_Right3] = Key_E4;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
}

TEST(MusicManager, ShouldStartAnotherSongAtBeginningWhenSongChanges) {
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

    ChangeSoundDetectedSignal();

    expected[Finger_Right1] = Key_F4;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
}

TEST(MusicManager, ShouldRequestConcurrentNotes) {
    uint8_t index = 2;
    GlobalVariables_Write(Global_SongIndex, &index);

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right1] = Key_D4;
    expected[Finger_Right5] = Key_D5;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);

    ChangeSoundDetectedSignal();

    expected[Finger_Right1] = Key_F4;
    expected[Finger_Right5] = Key_F5;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
}

TEST(MusicManager, ShouldAdvanceToClosestNoteInAnyChannelOnSoundDetected) { // for music with syncopation
    uint8_t index = 3;
    GlobalVariables_Write(Global_SongIndex, &index);

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Left1] = Key_C3;
    expected[Finger_Right5] = Key_D5;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);

    ChangeSoundDetectedSignal();

    expected[Finger_Left1] = Key_C4;
    expected[Finger_Right5] = Key_Rest;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);

    ChangeSoundDetectedSignal();

    expected[Finger_Left1] = Key_Rest;
    expected[Finger_Right5] = Key_F5;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
}

// same as previous test case, except uses note forward signal instead of sound detected
TEST(MusicManager, ShouldAdvanceToClosestNoteInAnyChannelOnNoteForward) { // for music with syncopation
    uint8_t index = 3;
    GlobalVariables_Write(Global_SongIndex, &index);

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Left1] = Key_C3;
    expected[Finger_Right5] = Key_D5;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);

    ChangeNoteForwardSignal();

    expected[Finger_Left1] = Key_C4;
    expected[Finger_Right5] = Key_Rest;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);

    ChangeNoteForwardSignal();

    expected[Finger_Left1] = Key_Rest;
    expected[Finger_Right5] = Key_F5;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
}

TEST(MusicManager, ShouldGoBackToClosestNoteInAnyChannelOnNoteBackward) { // for music with syncopation
    uint8_t index = 3;
    GlobalVariables_Write(Global_SongIndex, &index);
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

    ChangeNoteBackwardSignal();

    expected[Finger_Left1] = Key_C3;
    expected[Finger_Right5] = Key_D5;
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
}

TEST(MusicManager, ShouldRequestRestWhenEndOfChannelIsReached) {
    uint8_t index = 4;
    GlobalVariables_Write(Global_SongIndex, &index);

    ChangeSoundDetectedSignal();

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    expected[Finger_Right4] = Key_C5;
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);
}

TEST(MusicManager, ShouldRequestRestWhenEndOfSongIsReached) {
    uint8_t index = 4;
    GlobalVariables_Write(Global_SongIndex, &index);

    ChangeSoundDetectedSignal();
    ChangeSoundDetectedSignal();

    Key_t expected[] = {
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest,
        Key_Rest, Key_Rest, Key_Rest, Key_Rest, Key_Rest
    };
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);
    CheckDesiredFingerPositions(expected, actual);

    ChangeSoundDetectedSignal();
    CheckDesiredFingerPositions(expected, actual);
}

// TODO: Should??IfMultipleNotesAreMappedToSameFinger, probably just take the top most one
