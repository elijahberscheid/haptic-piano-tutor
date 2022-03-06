#include "stm32f0xx.h"
#include "ButtonInterface.h"
#include "DebugHelper.h"
#include "ScreenInterface.h"
#include "haptic-piano-tutor-library/src/tty.h"
#include "lcd.h"
#include "modules/GlobalVariables.h"
#include "modules/MusicManager.h"
#include "modules/Song.h"
#include "modules/SystemStateManager.h"

extern const Song_t Song_songs[];
static const MusicManagerConfig_t config = { .songs = Song_songs };

static MusicManager_t musicManager;

int main(void) {
    tty_init();
    GlobalVariables_Init();
    MusicManager_Init(&musicManager, &config);
    ButtonInterface_Init();
    SystemStateManager_Init();
    ScreenInterface_Init();

    int i = 0;
    for(;;) {
        DebugHelper_Run();
        ButtonInterface_Run();
        i++;
    }
}
