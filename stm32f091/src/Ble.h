#ifndef __BLE_H__
#define __BLE_H__

#include <stdint.h>

void Ble_SendString(char *message, uint8_t length);
void Ble_Run(void);
void Ble_Init(void);

#endif
