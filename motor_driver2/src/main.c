#include "stm32f0xx.h"
#include <stdbool.h>
#include <stdint.h> // for uint8_t
#include "motors.h"

int main(void) {
    initializeHapticState();
    for (int i = 0; i < 10; ++i) distanceVector[i] = 88;

    RCC->AHBENR |= RCC_AHBENR_GPIOCEN; // Setup some buttons for basic tests

    setupTimer3();
    enableHapticOutputs(1, 1);

    for(;;) {
        int C6 = (GPIOC->IDR >> 6) & 1;
        int C7 = (GPIOC->IDR >> 7) & 1;
        int C8 = (GPIOC->IDR >> 8) & 1;
        int C9 = (GPIOC->IDR >> 9) & 1;
        int C11 = (GPIOC->IDR >> 11) & 1;
        int C12 = (GPIOC->IDR >> 12) & 1;

        int inputNum = (C6 << 0) | (C7 << 1) | (C8 << 2) | (C9 << 3) | (C12 << 4) | (C11 << 5);

        if (inputNum < NUMBER_OF_FINGERS) {
            for (int i = 0; i < 10; ++i) {
                if (i == inputNum)
                    distanceVector[i] = 0;
                else
                    distanceVector[i] = 88;
            }
        } else {
            distanceVector[0] = 88;
            distanceVector[1] = 88;
            distanceVector[2] = 88;
            distanceVector[3] = 88;
            distanceVector[4] = 88;
            distanceVector[5] = 16;
            distanceVector[6] = 88;
            distanceVector[7] = 88;
            distanceVector[8] = 88;
            distanceVector[9] = 88;
        }
    }
}

