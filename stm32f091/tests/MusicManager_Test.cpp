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
    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t channel2[] = {
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_D5, .length = NoteLength_Quarter, .finger = Finger_Right5 },
    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Song_t songs[] = {
    { .defaultTempo = 60, .channels = { channel1, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
    { .defaultTempo = 80, .channels = { channel2, NULL, NULL, NULL, NULL, NULL, NULL, NULL } }
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

    void CheckDesiredFingerPositions(uint8_t *expected, uint8_t *actual) {
        for (uint8_t i = 0; i < GlobalVariables_GetLength(Global_DesiredFingerPositions); i++) {
            CHECK_EQUAL(expected[i], actual[i]);
        }
    }

};

TEST(MusicManager, ShouldRequestFirstNoteOnInit) {
    Key_t expected[] = {
        Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid,
        Key_C4, Key_Invalid, Key_Invalid, Key_Invalid, Key_Invalid,
    };
    Key_t actual[10] = { 0 };
    GlobalVariables_Read(Global_DesiredFingerPositions, actual);

    CheckDesiredFingerPositions(expected, actual);
}

