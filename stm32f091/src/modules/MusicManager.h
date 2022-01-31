#ifndef __MUSICMANAGER_H__
#define __MUSICMANAGER_H__

#include <stdint.h>
#include "Song.h"

typedef struct {
    const Song_t *songs;
} MusicManagerConfig_t;

typedef struct {
    const MusicManagerConfig_t *config;
    uint32_t ticks;
} MusicManager_t;

void MusicManager_Init(MusicManager_t *instance, const MusicManagerConfig_t *config);

#endif
