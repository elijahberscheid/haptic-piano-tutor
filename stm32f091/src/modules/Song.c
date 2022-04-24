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

static const Note_t canonD1[] = {
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_DSharp4, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_FSharp4, .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_E3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_C4     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_B2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_DSharp4, .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_FSharp2, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_C3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_G2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_G3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_DSharp2, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_G2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_G3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_E3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_C4     , .length = NoteLength_Eighth, .finger = Finger_Left4 },


    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_DSharp4, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_FSharp4, .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_E3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_C4     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_B2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_DSharp4, .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_FSharp2, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_C3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_G2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_G3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_DSharp2, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_G2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_G3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_E3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_C4     , .length = NoteLength_Eighth, .finger = Finger_Left4 },


    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_DSharp4, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_FSharp4, .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_E3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_C4     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_B2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_DSharp4, .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_FSharp2, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_C3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_G2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_G3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_DSharp2, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_G2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_G3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_E3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_C4     , .length = NoteLength_Eighth, .finger = Finger_Left4 },


    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_DSharp4, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_FSharp4, .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_E3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_C4     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_B2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_DSharp4, .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_FSharp2, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_C3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_G2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_G3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_DSharp2, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_G2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_G3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_E3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_C4     , .length = NoteLength_Eighth, .finger = Finger_Left4 },


    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_DSharp4, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_FSharp4, .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_E3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_C4     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_B2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_DSharp4, .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_FSharp2, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_C3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_G2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_G3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_DSharp2, .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_FSharp3, .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_G2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_DSharp3, .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_G3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_B3     , .length = NoteLength_Eighth, .finger = Finger_Left4 },
    { .key = Key_A2     , .length = NoteLength_Eighth, .finger = Finger_Left1 },
    { .key = Key_E3     , .length = NoteLength_Eighth, .finger = Finger_Left2 },
    { .key = Key_A3     , .length = NoteLength_Eighth, .finger = Finger_Left3 },
    { .key = Key_C4     , .length = NoteLength_Eighth, .finger = Finger_Left4 },

    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t canonD2[] = {
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },

    { .key = Key_FSharp5, .length = NoteLength_Half, .finger = Finger_Right5 },
    { .key = Key_E5     , .length = NoteLength_Half, .finger = Finger_Right4 },
    { .key = Key_D5     , .length = NoteLength_Half, .finger = Finger_Right3 },
    { .key = Key_CSharp5, .length = NoteLength_Half, .finger = Finger_Right2 },
    { .key = Key_B4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_A4     , .length = NoteLength_Half, .finger = Finger_Right2 },
    { .key = Key_B4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_CSharp5, .length = NoteLength_Half, .finger = Finger_Right2 },

    { .key = Key_A4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_A4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_FSharp4, .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_FSharp4, .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_D4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_D4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_D4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_E3     , .length = NoteLength_Half, .finger = Finger_Right1 },

    { .key = Key_D5     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_FSharp5, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_A5     , .length = NoteLength_Quarter, .finger = Finger_Right5 },
    { .key = Key_G5     , .length = NoteLength_Quarter, .finger = Finger_Right4 },

    { .key = Key_FSharp5, .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_D5     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_FSharp5, .length = NoteLength_DottedQuarter, .finger = Finger_Right3 },
    { .key = Key_E5     , .length = NoteLength_Eighth, .finger = Finger_Right2 },

    { .key = Key_D5     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_B4     , .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_D5     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_FSharp5, .length = NoteLength_Quarter, .finger = Finger_Right2 },

    { .key = Key_G5     , .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_B5     , .length = NoteLength_Quarter, .finger = Finger_Right5 },
    { .key = Key_A5     , .length = NoteLength_DottedQuarter, .finger = Finger_Right4 },
    { .key = Key_G5     , .length = NoteLength_Eighth, .finger = Finger_Right3 },

    { .key = Key_D5     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_FSharp5, .length = NoteLength_Quarter, .finger = Finger_Right2 },
    { .key = Key_E5     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_G5     , .length = NoteLength_Quarter, .finger = Finger_Right3 },

    { .key = Key_D5     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_D5     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_CSharp5, .length = NoteLength_DottedQuarter, .finger = Finger_Right1 },
    { .key = Key_E5     , .length = NoteLength_Eighth, .finger = Finger_Right2 },

    { .key = Key_B4     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_B4     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_A4     , .length = NoteLength_Quarter, .finger = Finger_Right1 },
    { .key = Key_FSharp5, .length = NoteLength_Quarter, .finger = Finger_Right4 },

    { .key = Key_B5     , .length = NoteLength_Quarter, .finger = Finger_Right3 },
    { .key = Key_B5     , .length = NoteLength_Quarter, .finger = Finger_Right5 },
    { .key = Key_A5     , .length = NoteLength_DottedQuarter, .finger = Finger_Right4 },
    { .key = Key_G5     , .length = NoteLength_Eighth, .finger = Finger_Right3 },

    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

static const Note_t canonD3[] = {
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },

    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },

    { .key = Key_A4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_A4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_F4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_F4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_D4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_D4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_D4     , .length = NoteLength_Half, .finger = Finger_Right1 },
    { .key = Key_E4     , .length = NoteLength_Half, .finger = Finger_Right1 },

    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },
    { .key = Key_Rest   , .length = NoteLength_Whole,   .finger = Finger_Right5 },

    { .key = Key_Invalid, .length = 0, .finger = 0 }
};

const Song_t Song_songs[] = { // Update Song_NumberOfSongs when changing this array
    { .channels = { maryHadALittleLamb1, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
    { .channels = { cMajorScale1, cMajorScale2, NULL, NULL, NULL, NULL, NULL, NULL } },
    { .channels = { canonD1, canonD2, canonD3, NULL, NULL, NULL, NULL, NULL } }
};
