#include <string.h>
#include "GlobalVariables.h"

#include <stdio.h>

#define EXPAND_AS_PRIMITIVE_INITIALIZATION(_name,_type, _array_size) \
    memset(&_name, 0, sizeof(_name));

#define EXPAND_AS_ARRAY_INITIALIZATION(_name,_type, _array_size) \
    memset(_name, 0, sizeof(_name));

GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_PRIMITIVE_VARIABLE)
GLOBALS_ARRAY_TABLE(EXPAND_AS_ARRAY_VARIABLE)

static void *globalsList[Global_NumberOfIds] = {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_PRIMITIVE_ADDRESS)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_ARRAY_ADDRESS)
};

static char *globalsNames[Global_NumberOfIds] = {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_STRING)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_STRING)
};

static char *globalsTypes[Global_NumberOfIds] = {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_PRIMITIVE_TYPE)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_ARRAY_TYPE)
};

static uint16_t globalsLengths[Global_NumberOfIds] = {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_LENGTH)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_LENGTH)
};

static bool globalsWriteEnable[Global_NumberOfIds] = { 0 };

char *GlobalVariables_GetName(uint8_t id) {
    return globalsNames[id];
}

char *GlobalVariables_GetType(uint8_t id) {
    return globalsTypes[id];
}

uint16_t GlobalVariables_GetLength(uint8_t id) {
    return globalsLengths[id];
}

void GlobalVariables_SetWriteEnable(uint8_t id, bool enable) {
    globalsWriteEnable[id] = enable;
}

void GlobalVariables_Read(uint8_t id, void *dest) {
    if (strcmp("uint8_t", globalsTypes[id]) == 0) {
        *(uint8_t *)dest = *(uint8_t *)globalsList[id];
    }
    else if (strcmp("uint16_t", globalsTypes[id]) == 0) {
        *(uint16_t *)dest = *(uint16_t *)globalsList[id];
    }
    else if (strcmp("uint32_t", globalsTypes[id]) == 0) {
        *(uint32_t *)dest = *(uint32_t *)globalsList[id];
    }
    else if (strcmp("bool", globalsTypes[id]) == 0) {
        *(bool *)dest = *(bool *)globalsList[id];
    }
    else if (strncmp("uint8_t[", globalsTypes[id], strlen("uint8_t[")) == 0) {
        memcpy(dest, globalsList[id], globalsLengths[id] * sizeof(uint8_t));
    }
    else if (strncmp("int8_t[", globalsTypes[id], strlen("int8_t[")) == 0) {
        memcpy(dest, globalsList[id], globalsLengths[id] * sizeof(int8_t));
    }
}

static void WriteVariable(uint8_t id, void *src) {
    if (strcmp("uint8_t", globalsTypes[id]) == 0) {
        *(uint8_t *)globalsList[id] = *(uint8_t *)src;
    }
    else if (strcmp("uint16_t", globalsTypes[id]) == 0) {
        *(uint16_t *)globalsList[id] = *(uint16_t *)src;
    }
    else if (strcmp("uint32_t", globalsTypes[id]) == 0) {
        *(uint32_t *)globalsList[id] = *(uint32_t *)src;
    }
    else if (strcmp("bool", globalsTypes[id]) == 0) {
        *(bool *)globalsList[id] = *(bool *)src;
    }
    else if (strncmp("uint8_t[", globalsTypes[id], strlen("uint8_t[")) == 0) {
        memcpy(globalsList[id], src, globalsLengths[id] * sizeof(uint8_t));
    }
    else if (strncmp("int8_t[", globalsTypes[id], strlen("int8_t[")) == 0) {
        memcpy(globalsList[id], src, globalsLengths[id] * sizeof(int8_t));
    }
}

void GlobalVariables_Write(uint8_t id, void *src) {
    if (globalsWriteEnable[id]) {
        WriteVariable(id, src);
    }
}

void GlobalVariables_DebugWrite(uint8_t id, void *src) {
    WriteVariable(id, src);
}

void GlobalVariables_Init(void) {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_PRIMITIVE_INITIALIZATION)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_ARRAY_INITIALIZATION)

    for (uint8_t id = 0; id < Global_NumberOfIds; id++) {
        globalsWriteEnable[id] = true;
    }
}
