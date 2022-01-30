#include <stdio.h>
#include <string.h>
#include "GlobalVariables.h"

enum {
    MAX_NUM_SUBSCRIPTIONS = 16
};

GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_PRIMITIVE_VARIABLE)
GLOBALS_ARRAY_TABLE(EXPAND_AS_ARRAY_VARIABLE)

static void * const globalsList[GLOBAL_MAX_NUM] = {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_PRIMITIVE_ADDRESS)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_ARRAY_ADDRESS)
};

static char * const globalsNames[GLOBAL_MAX_NUM] = {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_STRING)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_STRING)
};

static char * const globalsTypes[GLOBAL_MAX_NUM] = {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_PRIMITIVE_TYPE)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_ARRAY_TYPE)
};

static uint16_t const globalsLengths[GLOBAL_MAX_NUM] = {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_LENGTH)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_LENGTH)
};

typedef void (*Callback)(void *);
static Callback subscriptionList[GLOBAL_MAX_NUM][MAX_NUM_SUBSCRIPTIONS] = { NULL };

char *GlobalVariables_GetName(uint8_t id) {
    return globalsNames[id];
}

char *GlobalVariables_GetType(uint8_t id) {
    return globalsTypes[id];
}

uint16_t GlobalVariables_GetLength(uint8_t id) {
    return globalsLengths[id];
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

void GlobalVariables_Write(uint8_t id, void *src) {
    bool valueWritten = false;
    bool valueChanged = false;

    if (strcmp("uint8_t", globalsTypes[id]) == 0) {
        if (memcmp(globalsList[id], src, sizeof(uint8_t)) != 0) {
            memcpy(globalsList[id], src, sizeof(uint8_t));
            valueChanged = true;
        }
        valueWritten = true;
    }
    else if (strcmp("uint16_t", globalsTypes[id]) == 0) {
        if (memcmp(globalsList[id], src, sizeof(uint16_t)) != 0) {
            memcpy(globalsList[id], src, sizeof(uint16_t));
            valueChanged = true;
        }
        valueWritten = true;
    }
    else if (strcmp("uint32_t", globalsTypes[id]) == 0) {
        if (memcmp(globalsList[id], src, sizeof(uint32_t)) != 0) {
            memcpy(globalsList[id], src, sizeof(uint32_t));
            valueChanged = true;
        }
        valueWritten = true;
    }
    else if (strcmp("bool", globalsTypes[id]) == 0) {
        if (memcmp(globalsList[id], src, sizeof(bool)) != 0) {
            memcpy(globalsList[id], src, sizeof(bool));
            valueChanged = true;
        }
        valueWritten = true;
    }
    else if (strncmp("uint8_t[", globalsTypes[id], strlen("uint8_t[")) == 0) {
        if (memcmp(globalsList[id], src, globalsLengths[id] * sizeof(uint8_t)) != 0) {
            memcpy(globalsList[id], src, globalsLengths[id] * sizeof(uint8_t));
            valueChanged = true;
        }
        valueWritten = true;
    }
    else if (strncmp("int8_t[", globalsTypes[id], strlen("int8_t[")) == 0) {
        if (memcmp(globalsList[id], src, globalsLengths[id] * sizeof(int8_t)) != 0) {
            memcpy(globalsList[id], src, globalsLengths[id] * sizeof(int8_t));
            valueChanged = true;
        }
        valueWritten = true;
    }

    if (!valueWritten) {
        printf("Write to id = %u not successful, check that its type has been added to GlobalVariables_Write()\n", id);
    }
    else if (valueChanged) {
        for (uint8_t i = 0; i < MAX_NUM_SUBSCRIPTIONS; i++) {
            if (subscriptionList[id][i] != NULL) {
                (*subscriptionList[id][i])(src);
            }
        }
    }
}

void GlobalVariables_Subscribe(uint8_t id, void (*callback)(void *)) {
    bool subscribed = false;
    for (uint8_t i = 0; i < MAX_NUM_SUBSCRIPTIONS; i++) {
        if (subscriptionList[id][i] == NULL) {
            subscriptionList[id][i] = callback;
            subscribed = true;
            break;
        }
    }
    if (!subscribed) {
        printf("Could not subscribe to id = %u\n", id);
    }
}

void GlobalVariables_Unsubscribe(uint8_t id, void (*callback)(void *)) {
    bool unsubscribed = false;
    for (uint8_t i = 0; i < MAX_NUM_SUBSCRIPTIONS; i++) {
        if (subscriptionList[id][i] == callback) {
            subscriptionList[id][i] = NULL;
            unsubscribed = true;
            break;
        }
    }
    if (!unsubscribed) {
        printf("Could not unsubscribe from id = %u\n", id);
    }
}

void GlobalVariables_Init(void) {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_PRIMITIVE_INITIALIZATION)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_ARRAY_INITIALIZATION)
}
