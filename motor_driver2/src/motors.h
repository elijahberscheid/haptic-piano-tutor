#ifndef __MOTORS_H__
#define __MOTORS_H__

#include "stm32f0xx.h"
#include <stdbool.h>
#include <stdint.h>
#define NUMBER_OF_FINGERS 10

int8_t distanceVector[NUMBER_OF_FINGERS];

typedef struct _hapticState_t {
    uint8_t dutyCycle; // as cycle number when to switch from 1 to 0 (if period=20, then 50% duty cycle is dutyCycle=10)
    uint8_t period; // as multiple of update period (200Hz=5ms)
    uint8_t leftActive; // this represents left side of finger, NOT left hand
    uint8_t rightActive; // this represents right side of finger, NOT right hand
    uint8_t subcycleCount;
} hapticState_t;

void initializeHapticState();
void pushBitsToHands(uint8_t bitLH, uint8_t bitRH);
void latchHapticRegisters();
void enableHapticOutputs(uint8_t left, uint8_t right);
void setupTimer3();
void TIM3_IRQHandler();

#endif

