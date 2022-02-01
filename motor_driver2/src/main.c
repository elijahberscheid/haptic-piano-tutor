#include "stm32f0xx.h"
#include <stdint.h> // for uint8_t

// R=right, L=left, H=hand, F=finger
// index mapping for motors
// 0: LH
enum {
    numberOfMotorPairs = 12
};

// FIXME for example
int8_t distanceVector[10] = {5, 5, 5, 5, 5, 5, 5, 5, 5, 5};

typedef struct _hapticState_t {
    uint8_t dutyCycle; // as cycle number when to switch from 1 to 0 (if period=20, then 50% duty cycle is dutyCycle=10)
    uint8_t period; // as multiple of update period (200Hz=5ms)
    uint8_t leftActive; // this represents left side of finger, NOT left hand
    uint8_t rightActive; // this represents right side of finger, NOT right hand
    uint8_t subcycleCount;
} hapticState_t;

hapticState_t fingerHapticStates[10];
hapticState_t wristHapticState[2];


//============================================================================
// Wait for n nanoseconds. (Maximum: 4.294 seconds)
// Credit to Rick and ECE362 staff for this nanosecond timer in embedded assembly.
// I have modified it slightly to work with optimized compilation.
//============================================================================
void nano_wait(unsigned int n) {
    asm("        mov r0,%0\n"
            "1: sub r0,#83\n"
            "bgt 1b\n" : : "r"(n) : "r0", "cc");
}


void initializeHapticState() {
    // This will use Port A, pins 1-8. Don't use PA0 since it's connected to something else on dev board
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // use Port A

    // PA1: SDI  LeftHand
    // PA2: CLK  LeftHand
    // PA3: LE   LeftHand
    // PA4: OE   LeftHand
    // PA5: SDI  RightHand
    // PA6: CLK  RightHand
    // PA7: LE   RightHand
    // PA8: OE   RightHand

    GPIOA->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3
            | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6
            | GPIO_MODER_MODER7 | GPIO_MODER_MODER8);
    GPIOA->MODER |= GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0
            | GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0
            | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0;
    GPIOA->BSRR = GPIO_BSRR_BR_1 | GPIO_BSRR_BR_2 | GPIO_BSRR_BR_3
            | GPIO_BSRR_BR_4 | GPIO_BSRR_BR_5 | GPIO_BSRR_BR_6 | GPIO_BSRR_BR_7
            | GPIO_BSRR_BR_8;

    for (int i = 0; i < 10; ++i) {
        fingerHapticStates[i].dutyCycle = 0;
        fingerHapticStates[i].period = 20;
        fingerHapticStates[i].leftActive = 0;
        fingerHapticStates[i].rightActive = 0;
        fingerHapticStates[i].subcycleCount = 0;
    }
    for (int i = 0; i < 2; ++i) {
        wristHapticState[i].dutyCycle = 0;
        wristHapticState[i].period = 20;
        wristHapticState[i].leftActive = 0;
        wristHapticState[i].rightActive = 0;
        wristHapticState[i].subcycleCount = 0;
    }
}


void pushBitsToHands(uint8_t bitLH, uint8_t bitRH) {
    GPIOA->BSRR = ((bitLH & 1) << 1) | GPIO_BSRR_BR_1 | ((bitRH & 1) << 5) | GPIO_BSRR_BR_5; // write to PA1 and PA5
    //GPIOA->BRR = (1 << 2) | (1 << 6); // falling clock PA2 and PA6
    GPIOA->BRR = GPIO_BRR_BR_2 | GPIO_BRR_BR_6; // falling clock PA2 and PA6
    nano_wait(10); // hold time is 5ns
    GPIOA->BSRR = (1 << 2) | (1 << 6); // rising clock PA2 and PA6
    nano_wait(20); // setup time is 10ns, but minimum pulse width of CLK is 20ns
}

void latchHapticRegisters() {
    GPIOA->BSRR = (1<<3) | (1<<7); // Set LE on PA3 and PA7;
    nano_wait(20); // min pulse width is 20ns
    GPIOA->BRR = (1<<3) | (1<<7); // Reset LE on PA3 and PA7;
}

void enableHapticOutputs(uint8_t left, uint8_t right) {
    GPIOA->BSRR = ((left & 1) << 4) | ((right & 1) << 8) | GPIO_BSRR_BR_4 | GPIO_BSRR_BR_8;
}


void setupTimer3() {
    // Setup for 200 Hz
    TIM3->CR1 &= ~TIM_CR1_CEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->PSC = 48 - 1;
    TIM3->ARR = 5000 - 1;
    TIM3->DIER |= TIM_DIER_UIE;
    TIM3->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] = 1 << 16;
}


void TIM3_IRQHandler() {
    TIM3->SR &= ~TIM_SR_UIF; // acknowledge interrupt

    for (int i = 0; i < 10; ++i) {
        if (distanceVector[i] < 0) { // if distance is negative, need right motor to push finger left
            distanceVector[i] = -distanceVector[i]; // take absolute value
            fingerHapticStates[i].leftActive = 0;
            fingerHapticStates[i].rightActive = 1;
        } else { // if distance is positive, need left motor to push finger right
            fingerHapticStates[i].leftActive = 1;
            fingerHapticStates[i].rightActive = 0;
        }

        // period = desiredFreq/updateFrequency
        if (distanceVector[i] >= 20) { // Far from key
            fingerHapticStates[i].dutyCycle = 0; // off
            fingerHapticStates[i].period = 20; // arbitrary
        } else if (distanceVector[i] >= 2) { // Nearby key
            fingerHapticStates[i].dutyCycle = 10; // 50%
            fingerHapticStates[i].period = 20; // 20 * period of update
        } else { // On top of key
            fingerHapticStates[i].dutyCycle = 2; // 10%
            fingerHapticStates[i].period = 20;
            // Activate both motors
            fingerHapticStates[i].leftActive = 1;
            fingerHapticStates[i].rightActive = 1;
        }
    }

    // write both hands simultaneously
    int i = 0;
    while (i < 5) {
        // left side of finger on each hand
        pushBitsToHands((fingerHapticStates[i].subcycleCount < fingerHapticStates[i].dutyCycle) && fingerHapticStates[i].leftActive,
                        (fingerHapticStates[i+5].subcycleCount < fingerHapticStates[i+5].dutyCycle) && fingerHapticStates[i+5].leftActive);
        // right side of finger on each hand
        pushBitsToHands((fingerHapticStates[i].subcycleCount < fingerHapticStates[i].dutyCycle) && fingerHapticStates[i].rightActive,
                        (fingerHapticStates[i+5].subcycleCount < fingerHapticStates[i+5].dutyCycle) && fingerHapticStates[i+5].rightActive);
        fingerHapticStates[i].subcycleCount += 1;
        fingerHapticStates[i + 5].subcycleCount += 1;

        if (fingerHapticStates[i].subcycleCount > fingerHapticStates[i].period) {
            fingerHapticStates[i].subcycleCount = 0;
        }
        if (fingerHapticStates[i + 5].subcycleCount > fingerHapticStates[i + 5].period) {
            fingerHapticStates[i + 5].subcycleCount = 0;
        }
        ++i;
    }

    // left side of both wrists
    pushBitsToHands((wristHapticState[0].subcycleCount < wristHapticState[0].dutyCycle) && wristHapticState[0].leftActive,
                    (wristHapticState[1].subcycleCount < wristHapticState[1].dutyCycle) && wristHapticState[1].leftActive);
    // right side of both wrists
    pushBitsToHands((wristHapticState[0].subcycleCount < wristHapticState[0].dutyCycle) && wristHapticState[0].rightActive,
                    (wristHapticState[1].subcycleCount < wristHapticState[1].dutyCycle) && wristHapticState[1].rightActive);
    wristHapticState[0].subcycleCount += 1;
    wristHapticState[1].subcycleCount += 1;
    if (wristHapticState[0].subcycleCount > wristHapticState[0].period) {
        wristHapticState[0].subcycleCount = 0;
    }
    if (wristHapticState[1].subcycleCount > wristHapticState[1].period) {
        wristHapticState[1].subcycleCount = 0;
    }
    i += 2;

    // finish filling shift registers
    while (i < 16) {
        pushBitsToHands(0, 0);
        ++i;
    }
    latchHapticRegisters();
}


int main(void) {
    initializeHapticState();
    setupTimer3();
    enableHapticOutputs(1, 1);

    for(;;);
}
