#include "stm32f0xx.h"
#include <stdbool.h>
#include <stdint.h> // for uint8_t
#include "motors.h"

int main(void) {
    initializeHapticState();

    // Setup some buttons for basic tests
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    setupTimer3();
    enableHapticOutputs(1, 0); // enable left hand

    for(;;) {
        int C6 = GPIOC->IDR & (1<<6);
        int C7 = GPIOC->IDR & (1<<7);
        int C8 = GPIOC->IDR & (1<<8);
        int C9 = GPIOC->IDR & (1<<9);
        int C11 = GPIOC->IDR & (1<<11);
        int C12 = GPIOC->IDR & (1<<12);

        int inputNum = (C6 << 0) | (C7 << 1) | (C8 << 2) | (C9 << 3) | (C12 << 4) | (C11 << 5);

        for (int i = 0; i < NUMBER_OF_FINGERS; ++i) distanceVector[i] = 88;

        if (inputNum < NUMBER_OF_FINGERS) {
          distanceVector[inputNum] = 0;
        }
    }
}
