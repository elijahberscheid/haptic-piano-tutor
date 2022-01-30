#include "MusicManager.h"
#include "GlobalVariables.h"

#include <stdio.h>

static void PositionsChanged(void *data) {
    printf("Callback worked!\n");
    GlobalVariables_Unsubscribe(Global_FingerPositions, PositionsChanged);
}

void MusicManager_Init(void) {
    GlobalVariables_Subscribe(Global_FingerPositions, PositionsChanged);
}
