#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "haptic-piano-tutor-library/src/tty.h"
#include "DebugHelper.h"
#include "GlobalVariables.h"

enum {
    CHAR_BUFFER_SIZE = 128, // must match value in tty.c
    MAX_NUM_WORDS = 64
};

static bool ProcessRead(char **words) {
    bool actionTaken = false;
    for (uint8_t id = 0; id < GLOBAL_MAX_NUM; id++) {
        if (strcasecmp(words[1], GlobalVariables_GetName(id)) == 0) {
            if (strcmp("uint8_t", GlobalVariables_GetType(id)) == 0) {
                uint8_t value = 0;
                GlobalVariables_Read(id, &value);
                printf("%s = %u\n", GlobalVariables_GetName(id), value);
            }
            else if (strcmp("uint16_t", GlobalVariables_GetType(id)) == 0) {
                uint16_t value = 0;
                GlobalVariables_Read(id, &value);
                printf("%s = %u\n", GlobalVariables_GetName(id), value);
            }
            else if (strcmp("uint32_t", GlobalVariables_GetType(id)) == 0) {
                uint32_t value = 0;
                GlobalVariables_Read(id, &value);
                printf("%s = %lu\n", GlobalVariables_GetName(id), value);
            }
            else if (strcmp("bool", GlobalVariables_GetType(id)) == 0) {
                bool value = false;
                GlobalVariables_Read(id, &value);
                printf("%s = %s\n", GlobalVariables_GetName(id), value? "true" : "false");
            }
            actionTaken = true;
        }
    }
    return actionTaken;
}

static bool ProcessWrite(char **words) {
    bool actionTaken = false;
    for (uint8_t id = 0; id < GLOBAL_MAX_NUM; id++) {
        if (strcasecmp(words[1], GlobalVariables_GetName(id)) == 0) {
            if (strcmp("uint8_t", GlobalVariables_GetType(id)) == 0) {
                uint8_t value = (uint8_t) strtol(words[2], NULL, 10);
                GlobalVariables_Write(id, &value);
            }
            else if (strcmp("uint16_t", GlobalVariables_GetType(id)) == 0) {
                uint16_t value = (uint16_t) strtol(words[2], NULL, 10);
                GlobalVariables_Write(id, &value);
            }
            else if (strcmp("uint32_t", GlobalVariables_GetType(id)) == 0) {
                uint32_t value = (uint32_t) strtol(words[2], NULL, 10);
                GlobalVariables_Write(id, &value);
            }
            else if (strcmp("bool", GlobalVariables_GetType(id)) == 0) {
                // writes false for anything entered except "true"
                bool value = (strcasecmp(words[2], "true") == 0)? true : false;
                GlobalVariables_Write(id, &value);
            }
            actionTaken = true;
        }
    }
    return actionTaken;
}

static void action(char **words) {
    bool actionTaken = false;
    if (words[0] != 0) {
        if (strcasecmp(words[0], "help") == 0) {
            printf("\nAvailable commands:\n");
            printf("read <var>\n");
            printf("    prints the value of <var>\n");
            printf("write <var> <value>\n");
            printf("    writes <value> into <var>\n");
            printf("    write <var> without a value will set <var> to 0\n");
            actionTaken = true;
        }
        else if (strcasecmp(words[0], "read") == 0) {
            actionTaken = ProcessRead(words);
        }
        else if (strcasecmp(words[0], "write") == 0) {
            actionTaken = ProcessWrite(words);
        }

        if (!actionTaken){
            printf("Unrecognized command: %s\n", words[0]);
        }
    }
}

void DebugHelper_Run(void) {
    char buf[CHAR_BUFFER_SIZE] = { 0 };
    tty_nonBlockingGetString(buf);
    if (strlen(buf) > 0)
        buf[strlen(buf)-1] = '\0'; // remove \n

    char *words[MAX_NUM_WORDS] = { 0 };
    char *cp = buf;
    for(int i = 0; i < MAX_NUM_WORDS; i++) {
        // strtok tokenizes a string, splitting it up into words that
        // are divided by any characters in the second argument.
        words[i] = strtok(cp," \t");
        // Once strtok() is initialized with the buffer,
        // subsequent calls should be made with NULL.
        cp = 0;
        if (words[i] == 0)
            break;
    }

    action(words);
}

