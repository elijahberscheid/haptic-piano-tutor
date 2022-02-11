#include "stm32f0xx.h"
#include "DebugHelper.h"
#include "modules/GlobalVariables.h"
#include "modules/MusicManager.h"
#include "haptic-piano-tutor-library/src/tty.h"

int main(void)
{
    tty_init();
    GlobalVariables_Init();

    int i = 0;
    for(;;) {
        DebugHelper_Run();
        i++;
    }
}
