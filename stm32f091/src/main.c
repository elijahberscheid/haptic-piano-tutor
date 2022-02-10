#include "stm32f0xx.h"
#include "DebugHelper.h"
#include "GlobalVariables.h"
#include "haptic-piano-tutor-library/src/tty.h"

int main(void)
{
    tty_init();
    GlobalVariables_Init();

    int i = 0;
    for(;;) {
        DebugHelper_Run();
        uint8_t foo = 10;
        GlobalVariables_Write(Global_A, &foo);
        i++;
    }
}
