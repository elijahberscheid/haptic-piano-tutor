#ifndef __INPUTINTERFACES_H__
#define __INPUTINTERFACES_H__

#include <stdint.h>

void Ble_SendString(char *message, uint8_t length);
void Ble_Run(void);
void Ble_Init(void);

void ButtonInterface_Run(void);
void ButtonInterface_Init(void);

#endif
