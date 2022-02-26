#ifndef __GLOBALVARIABLES_H__
#define __GLOBALVARIABLES_H__

#include <stdint.h>
#include <stdbool.h>

//        Name,                     Type,       0 (to have enough arguments for macros)
#define GLOBALS_PRIMITIVE_TABLE(ENTRY) \
    ENTRY(SoundDetectedSignal,      uint8_t,    0) \
    ENTRY(IsPlayingSong,            bool,       0) \
    ENTRY(SongIndex,                uint8_t,    0) \
    ENTRY(SystemState,              uint8_t,    0) \
    ENTRY(DebugButtonSignal,        uint8_t,    0) \
    ENTRY(RightButtonSignal,        uint8_t,    0) \
    ENTRY(StartButtonSignal,        uint8_t,    0) \
    ENTRY(NoteForwardSignal,        uint8_t,    0) \
    ENTRY(NoteBackwardSignal,       uint8_t,    0) \

//        Name,                     Base Type,  Length
#define GLOBALS_ARRAY_TABLE(ENTRY) \
    ENTRY(FingerPositions,          uint8_t,    10) \
    ENTRY(DesiredFingerPositions,   uint8_t,    10) \
    ENTRY(FingerDistances,          int8_t,     10) \

#define EXPAND_AS_ENUM(_name, _type, _array_length) \
    Global_##_name,

#define EXPAND_AS_PRIMITIVE_VARIABLE(_name, _type, _array_length) \
    _type _name;

#define EXPAND_AS_ARRAY_VARIABLE(_name, _type, _array_length) \
    _type _name[_array_length];

#define EXPAND_AS_PRIMITIVE_ADDRESS(_name, _type, _array_length) \
    &(_name),

#define EXPAND_AS_ARRAY_ADDRESS(_name, _type, _array_length) \
    (_name),

#define EXPAND_AS_STRING(_name, _type, _array_length) \
    #_name,

#define EXPAND_AS_PRIMITIVE_TYPE(_name, _type, _array_length) \
    #_type,

#define EXPAND_AS_ARRAY_TYPE(_name, _type, _array_length) \
    #_type"["#_array_length"]",

#define EXPAND_AS_LENGTH(_name, _type, _array_length) \
    (_array_length),

#define EXPAND_AS_PRIMITIVE_INITIALIZATION(_name,_type, _array_size) \
    memset(&_name, 0, sizeof(_name));

#define EXPAND_AS_ARRAY_INITIALIZATION(_name,_type, _array_size) \
    memset(_name, 0, sizeof(_name));

enum {
    GLOBALS_PRIMITIVE_TABLE(EXPAND_AS_ENUM)
    GLOBALS_ARRAY_TABLE(EXPAND_AS_ENUM)
    Global_NumberOfIds
};

typedef void (*Callback)(void * context, const void *data);

typedef struct {
    void *context;
    Callback callback;
} GlobalVariables_Subscription_t;

char *GlobalVariables_GetName(uint8_t id);
char *GlobalVariables_GetType(uint8_t id);
uint16_t GlobalVariables_GetLength(uint8_t id);
void GlobalVariables_SetWriteEnable(uint8_t id, bool enable);
void GlobalVariables_Read(uint8_t id, void *dest);
void GlobalVariables_Write(uint8_t id, void *src);
void GlobalVariables_DebugWrite(uint8_t id, void *src);
void GlobalVariables_Subscribe(uint8_t id, const GlobalVariables_Subscription_t *subscription);
void GlobalVariables_Unsubscribe(uint8_t id, const GlobalVariables_Subscription_t *subscription);
void GlobalVariables_Init(void);

#endif
