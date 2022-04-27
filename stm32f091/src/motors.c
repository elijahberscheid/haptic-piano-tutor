/**
 *  Welcome to the greatest shift-register haptic motor driver module you've
 *  ever seen! Read on for instructions.
 *
 *  To get everything setup, run this:
 *      initializeHapticState();
 *
 *  You can enable/disable the left-hand and right-hand hardware using two
 *  booleans `leftEn` and `rightEn` as in:
 *      enableHapticOutputs(leftEn, rightEn);
 *
 *  Finally, to control haptic states, you must provide the finger-to-key
 *  distances for every finger. Set each signed value in the `distanceVector[]`
 *  array accordingly. `distanceVector[]` has length NUMBER_OF_FINGERS, defined
 *  as 10. The indices count from left pinky finger to right starting at 0,
 *  with palms facing down.
 *
 *  In summary, call the setup functions once, call the enable function as
 *  needed, and set the values in the distance vector array. The timer3
 *  interrupt will handle the rest.
 *
**/


#include "stm32f0xx.h"
#include <stdbool.h>
#include <stdint.h> // for uint8_t
#include "motors.h"
#include "modules/GlobalVariables.h"

#define NUMBER_OF_FINGERS 10

static hapticState_t fingerHapticStates[NUMBER_OF_FINGERS];
static hapticState_t wristHapticState[2];

// Wait for n nanoseconds. (Maximum: 4.294 seconds)
static void nano_wait(unsigned int n) {
    // Credit to Rick and ECE362 staff for this nanosecond timer in embedded assembly.
    // I have modified it slightly to work with optimized compilation.
    //===========================================================
    asm("        mov r0,%0\n"
            "1: sub r0,#83\n"
            "bgt 1b\n" : : "r"(n) : "r0", "cc");
}


static void pushBitsToHands(uint8_t bitLH, uint8_t bitRH) {
    GPIOA->BSRR = ((bitRH & 1) << 1) | GPIO_BSRR_BR_1 | ((bitLH & 1) << 5) | GPIO_BSRR_BR_5; // write to PA1 and PA5
    GPIOA->BRR = GPIO_BRR_BR_2 | GPIO_BRR_BR_6; // falling clock PA2 and PA6
    nano_wait(20); // hold time is 5ns
    GPIOA->BSRR = (1 << 2) | (1 << 6); // rising clock PA2 and PA6
    nano_wait(20); // setup time is 10ns, but minimum pulse width of CLK is 20ns
}


static void latchHapticRegisters() {
    GPIOA->BSRR = (1<<3) | (1<<7); // Set LE on PA3 and PA7;
    nano_wait(20); // min pulse width is 20ns
    GPIOA->BRR = (1<<3) | (1<<7); // Reset LE on PA3 and PA7;
}


void enableHapticOutputs(uint8_t left, uint8_t right) {
    GPIOA->BSRR = ((~right & 1) << 4) | ((~left & 1) << 8) | GPIO_BSRR_BR_4 | GPIO_BSRR_BR_8;
}


static void setupTimer3() {
    // Setup for 200 Hz
    TIM3->CR1 &= ~TIM_CR1_CEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    TIM3->PSC = 48 - 1;
    TIM3->ARR = 5000 - 1;
    TIM3->DIER |= TIM_DIER_UIE;
    TIM3->CR1 |= TIM_CR1_CEN;
    NVIC->ISER[0] = 1 << 16;
}


int map(int a, int b, int c, int d, int x) {
    int scale = 12;
    int y = (((d-c)<<scale)/(b-a))*((x)-a) + (c<<scale);
    y = y >> scale;
    return y;
}


void TIM3_IRQHandler() {
    TIM3->SR &= ~TIM_SR_UIF; // acknowledge interrupt

    int8_t distanceVector[NUMBER_OF_FINGERS];
    GlobalVariables_Read(Global_ResolvedFingerDistances, distanceVector);

    // Track how many fingers on each hand are "far" from accurate position. If
    // whole hand is far from target, use wrist motors instead of finger motors
    uint8_t numberOfFarLeftFingers = 0;
    uint8_t numberOfFarRightFingers = 0;

    for (int i = 0; i < NUMBER_OF_FINGERS; ++i) {
        uint8_t absoluteDistance;
        if (distanceVector[i] < 0) { // if distance is negative, need right motor to push finger left
            absoluteDistance = -distanceVector[i];
            fingerHapticStates[i].leftActive = 0;
            fingerHapticStates[i].rightActive = 1;
        } else { // if distance is positive, need left motor to push finger right
            absoluteDistance = distanceVector[i];
            fingerHapticStates[i].leftActive = 1;
            fingerHapticStates[i].rightActive = 0;
        }

        // period = desiredFreq/updateFrequency
        if (absoluteDistance >= 50) { // finger is far from key
            fingerHapticStates[i].dutyCycle = 0;  // Turn off motors for this finger
            //fingerHapticStates[i].period = 30;  // Arbitrary
            if (absoluteDistance < 88) { // this block is for wrist motors, but only if all fingers in 20-87 range
                if (i < NUMBER_OF_FINGERS/2) {
                  ++numberOfFarLeftFingers;
                } else {
                  ++numberOfFarRightFingers;
                }
            }
        } else if (absoluteDistance >= 1) { // finger is nearby key
            fingerHapticStates[i].dutyCycle = map(1, 50, 10, 20, absoluteDistance);
            fingerHapticStates[i].period = map(1, 50, 25, 30, absoluteDistance);
        } else { // finger is basically on top of key
            fingerHapticStates[i].dutyCycle = 4;
            fingerHapticStates[i].period = 10;
            // Activate both motors
            fingerHapticStates[i].leftActive = 1;
            fingerHapticStates[i].rightActive = 1;
        }
    }

    if (numberOfFarLeftFingers == NUMBER_OF_FINGERS/2) {
        wristHapticState[0].dutyCycle = 40;
        wristHapticState[0].period = 60;
        wristHapticState[0].leftActive = fingerHapticStates[4].leftActive;  // set direction based on left thumb
        wristHapticState[0].rightActive = fingerHapticStates[4].rightActive;
    } else {
        wristHapticState[0].dutyCycle = 0;
    }

    if (numberOfFarRightFingers == NUMBER_OF_FINGERS/2) {
        wristHapticState[1].dutyCycle = 40;
        wristHapticState[1].period = 60;
        wristHapticState[1].leftActive = fingerHapticStates[5].leftActive;  // set direction based on right thumb
        wristHapticState[1].rightActive = fingerHapticStates[5].rightActive;
    } else {
        wristHapticState[1].dutyCycle = 0;
    }

    // Now that the haptic state structures are configured, we write the data
    // into the shift registers. The left and right hands are written in
    // parallel since they each have their own shift register.

    for (int i = 0; i < 16-12; ++i) {
        pushBitsToHands(0, 0);
    }

    for (int i = 0; i < NUMBER_OF_FINGERS/2; ++i) {
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

    latchHapticRegisters();
}

void initializeHapticState() {
    // This will use Port A, pins 1-8. Don't use PA0 since it's connected to something else on dev board
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN; // use Port A

    // PA1: SDI  RightHand
    // PA2: CLK  RightHand
    // PA3: LE   RightHand
    // PA4: OE   RightHand
    // PA5: SDI  LeftHand
    // PA6: CLK  LeftHand
    // PA7: LE   LeftHand
    // PA8: OE   LeftHand

    GPIOA->MODER &= ~(GPIO_MODER_MODER1 | GPIO_MODER_MODER2 | GPIO_MODER_MODER3
            | GPIO_MODER_MODER4 | GPIO_MODER_MODER5 | GPIO_MODER_MODER6
            | GPIO_MODER_MODER7 | GPIO_MODER_MODER8);
    GPIOA->MODER |= GPIO_MODER_MODER1_0 | GPIO_MODER_MODER2_0
            | GPIO_MODER_MODER3_0 | GPIO_MODER_MODER4_0 | GPIO_MODER_MODER5_0
            | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0;
    GPIOA->BSRR = GPIO_BSRR_BR_1 | GPIO_BSRR_BR_2 | GPIO_BSRR_BR_3
            | GPIO_BSRR_BR_4 | GPIO_BSRR_BR_5 | GPIO_BSRR_BR_6 | GPIO_BSRR_BR_7
            | GPIO_BSRR_BR_8;

    for (int i = 0; i < NUMBER_OF_FINGERS; ++i) {
        fingerHapticStates[i].dutyCycle = 0;
        fingerHapticStates[i].period = 20;
        fingerHapticStates[i].leftActive = 0;
        fingerHapticStates[i].rightActive = 0;
        fingerHapticStates[i].subcycleCount = 0;
    }
    for (int i = 0; i < 2; ++i) {
        wristHapticState[i].dutyCycle = 0;
        wristHapticState[i].period = 20;
        wristHapticState[i].leftActive = 1;
        wristHapticState[i].rightActive = 1;
        wristHapticState[i].subcycleCount = 0;
    }

    setupTimer3();
    enableHapticOutputs(1, 1);
}
