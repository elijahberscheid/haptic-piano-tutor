#include <string.h>
#include "GlobalVariables.h"

#define EXPAND_AS_INITIALIZATION(_name, _id, _type) \
    memset(&_name, 0, sizeof(_name));

GLOBALS_TABLE(EXPAND_AS_VARIABLE)

static void *globalsList[GLOBAL_MAX_NUM] = {
    GLOBALS_TABLE(EXPAND_AS_ADDRESS)
};

static char *globalsNames[GLOBAL_MAX_NUM] = {
    GLOBALS_TABLE(EXPAND_AS_STRING)
};

static char *globalsTypes[GLOBAL_MAX_NUM] = {
    GLOBALS_TABLE(EXPAND_AS_TYPE)
};

char *GlobalVariables_GetName(uint8_t id) {
    return globalsNames[id];
}

char *GlobalVariables_GetType(uint8_t id) {
    return globalsTypes[id];
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
}

void GlobalVariables_Write(uint8_t id, void *src) {
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
}

void GlobalVariables_Init(void) {
    GLOBALS_TABLE(EXPAND_AS_INITIALIZATION)
}
