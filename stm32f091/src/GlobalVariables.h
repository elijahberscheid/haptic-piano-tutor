#ifndef __GLOBALVARIABLES_H__
#define __GLOBALVARIABLES_H__

#include <stdint.h>
#include <stdbool.h>

//                   Name,                Type, 0 (to have enough arguments for macros)
#define GLOBALS_PRIMITIVE_TABLE(ENTRY) \
    ENTRY(              A,             uint8_t, 0) \
    ENTRY(              B,            uint16_t, 0) \
    ENTRY(              C,            uint32_t, 0) \
    ENTRY(              D,                bool, 0) \

//                   Name,           Base Type,     Length
#define GLOBALS_ARRAY_TABLE(ENTRY) \
    ENTRY(FingerPositions,             uint8_t,       10) \
    ENTRY(FingerDistances,              int8_t,       10)

#define EXPAND_AS_ENUM(_name, _type, _array_length) \
    Global_##_name,

#define EXPAND_AS_PRIMITIVE_VARIABLE(_name, _type, _array_length) \
    _type _name;

#define EXPAND_AS_ARRAY_VARIABLE(_name, _type, _array_length) \
    _type _name[_array_length];

#define EXPAND_AS_PRIMITIVE_ADDRESS(_name, _type, _array_length) \
    &_name,

#define EXPAND_AS_ARRAY_ADDRESS(_name, _type, _array_length) \
    _name,

#define EXPAND_AS_STRING(_name, _type, _array_length) \
    #_name,

#define EXPAND_AS_PRIMITIVE_TYPE(_name, _type, _array_length) \
    #_type,

#define EXPAND_AS_ARRAY_TYPE(_name, _type, _array_length) \
    #_type"["#_array_length"]",

#define EXPAND_AS_LENGTH(_name, _type, _array_length) \
    _array_length,

enum {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_ENUM)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_ENUM)
    GLOBAL_MAX_NUM
};

char *GlobalVariables_GetName(uint8_t id);
char *GlobalVariables_GetType(uint8_t id);
uint16_t GlobalVariables_GetLength(uint8_t id);
void GlobalVariables_Read(uint8_t id, void *dest);
void GlobalVariables_Write(uint8_t id, void *src);
void GlobalVariables_Init(void);

#endif
