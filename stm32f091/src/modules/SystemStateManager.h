#ifndef __SYSTEMSTATEMANAGER_H__
#define __SYSTEMSTATEMANAGER_H__

#include <stdint.h>

enum {
    SystemState_Idle,
    SystemState_Running,
    SystemState_Paused,
    SystemState_CalibrationError,
    SystemState_NumberOfStates
};
typedef uint8_t SystemState_t; // assumed to be uint8_t in GlobalVariables.h

enum {
    HandedMode_Left,
    HandedMode_Right,
    HandedMode_Both,
    HandedMode_NumberOfModes
};
typedef uint8_t HandedMode_t; // assumed to be uint8_t in GlobalVariables.h

void SystemStateManager_Init(void);

#endif
