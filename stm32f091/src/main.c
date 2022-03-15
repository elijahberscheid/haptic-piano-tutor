#include "stm32f0xx.h"
#include <stdlib.h>
#include "DebugHelper.h"
#include "ScreenInterface.h"
#include "haptic-piano-tutor-library/src/tty.h"
#include "InputInterfaces.h"
#include "lcd.h"
#include "motors.h"
#include "modules/FingerDistanceCalculator.h"
#include "modules/GlobalVariables.h"
#include "modules/MusicManager.h"
#include "modules/Song.h"
#include "modules/SystemStateManager.h"

extern const Song_t Song_songs[];
static const MusicManagerConfig_t config = { .songs = Song_songs };

static MusicManager_t musicManager;
static SystemStateManager_t systemStateManger;

int main(void) {
    tty_init();
    GlobalVariables_Init();
    MusicManager_Init(&musicManager, &config);
    FingerDistanceCalculator_Init();
    Ble_Init();
    ButtonInterface_Init();
    initializeHapticState();
    SystemStateManager_Init(&systemStateManger);
    ScreenInterface_Init();

    int i = 0;
    for(;;) {
        DebugHelper_Run();
        Ble_Run();
        ButtonInterface_Run();
        i++;
    }
}
