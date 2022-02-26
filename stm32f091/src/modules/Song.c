#include <stdlib.h>
#include "Song.h"

static const Note_t maryHadALittleLamb1[] = {
    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_C4, .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },

    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_E4, .length = NoteLength_Half, .finger = Finger_Right3 },

    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_D4, .length = NoteLength_Half, .finger = Finger_Right2 },

    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_G4, .length = NoteLength_Quarter, .finger = Finger_Right5 },
    { .key = Key_G4, .length = NoteLength_Half, .finger = Finger_Right5 },

    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_C4, .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },

    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_C4, .length = NoteLength_Quarter, .finger = Finger_Right1 },

    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },

    { .key = Key_C4, .length = NoteLength_Whole, .finger = Finger_Right1 },

    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t cMajorScale1[] = {
    { .key = Key_C3, .length = NoteLength_Quarter, .finger = Finger_Left5 },
    { .key = Key_D3, .length = NoteLength_Quarter, .finger = Finger_Left4 },
    { .key = Key_E3, .length = NoteLength_Quarter, .finger = Finger_Left3 },
    { .key = Key_F3, .length = NoteLength_Quarter, .finger = Finger_Left2 },
    { .key = Key_G3, .length = NoteLength_Quarter, .finger = Finger_Left1 },
    { .key = Key_A3, .length = NoteLength_Quarter, .finger = Finger_Left3 },
    { .key = Key_B3, .length = NoteLength_Quarter, .finger = Finger_Left2 },
    { .key = Key_C4, .length = NoteLength_Quarter, .finger = Finger_Left1 },

    { .key = Key_B3, .length = NoteLength_Quarter, .finger = Finger_Left2 },
    { .key = Key_A3, .length = NoteLength_Quarter, .finger = Finger_Left3 },
    { .key = Key_G3, .length = NoteLength_Quarter, .finger = Finger_Left1 },
    { .key = Key_F3, .length = NoteLength_Quarter, .finger = Finger_Left2 },
    { .key = Key_E3, .length = NoteLength_Quarter, .finger = Finger_Left3 },
    { .key = Key_D3, .length = NoteLength_Quarter, .finger = Finger_Left4 },
    { .key = Key_C3, .length = NoteLength_Quarter, .finger = Finger_Left5 },

    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t cMajorScale2[] = {
    { .key = Key_C4, .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_F4, .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_G4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_A4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_B4, .length = NoteLength_Quarter, .finger = Finger_Right4 },
    { .key = Key_C5, .length = NoteLength_Quarter, .finger = Finger_Right5 },

    { .key = Key_B4, .length = NoteLength_Quarter, .finger = Finger_Right4 },
    { .key = Key_A4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_G4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_F4, .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_E4, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_D4, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_C4, .length = NoteLength_Quarter, .finger = Finger_Right1 },

    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

const Song_t Song_songs[] = { // Update Song_NumberOfSongs when changing this array
    { .defaultTempo = 60, .channels = { maryHadALittleLamb1, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
    { .defaultTempo = 60, .channels = { cMajorScale1, cMajorScale2, NULL, NULL, NULL, NULL, NULL, NULL } }
};
