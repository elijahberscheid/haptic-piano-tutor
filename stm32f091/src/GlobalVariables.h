#ifndef __GLOBALVARIABLES_H__
#define __GLOBALVARIABLES_H__

#include <stdint.h>
#include <stdbool.h>

//                   Name,     ID,                Type
#define GLOBALS_TABLE(ENTRY) \
    ENTRY(              A,      0,             uint8_t) \
    ENTRY(              B,      1,            uint16_t) \
    ENTRY(              C,      2,            uint32_t) \
    ENTRY(              D,      3,                bool)

#define EXPAND_AS_ENUM(_name, _id, _type) \
    Global_##_name = _id,

#define EXPAND_AS_STRING(_name, _id, _type) \
    #_name,

#define EXPAND_AS_TYPE(_name, _id, _type) \
    #_type,

#define EXPAND_AS_VARIABLE(_name, _id, _type) \
    _type _name;

#define EXPAND_AS_ADDRESS(_name, _id, _type) \
    &_name,

enum {
    GLOBALS_TABLE(EXPAND_AS_ENUM)
    GLOBAL_MAX_NUM
};

char *GlobalVariables_GetName(uint8_t id);
char *GlobalVariables_GetType(uint8_t id);
void GlobalVariables_Read(uint8_t id, void *dest);
void GlobalVariables_Write(uint8_t id, void *src);
void GlobalVariables_Init(void);

#endif
