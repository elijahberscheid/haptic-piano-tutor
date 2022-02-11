#include "stm32f0xx.h"
#include <stdlib.h>
#include "DebugHelper.h"
#include "haptic-piano-tutor-library/src/tty.h"
#include "modules/GlobalVariables.h"
#include "modules/MusicManager.h"
#include "modules/Song.h"

extern const Note_t maryHadALittleLamb1[];
extern const Note_t cMajorScale1[];
extern const Note_t cMajorScale2[];

static const Song_t songs[] = {
    { .defaultTempo = 60, .channels = { maryHadALittleLamb1, NULL, NULL, NULL, NULL, NULL, NULL, NULL } },
    { .defaultTempo = 60, .channels = { cMajorScale1, cMajorScale2, NULL, NULL, NULL, NULL, NULL, NULL } }
};

static const MusicManagerConfig_t config = { .songs = songs };

static MusicManager_t musicManager;

int main(void)
{
    tty_init();
    GlobalVariables_Init();
    MusicManager_Init(&musicManager, &config);

    int i = 0;
    for(;;) {
        DebugHelper_Run();
        i++;
    }
}
