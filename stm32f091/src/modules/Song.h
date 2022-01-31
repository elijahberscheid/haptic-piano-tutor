#ifndef __SONG_H__
#define __SONG_H__

#include <stdint.h>
#include "Note.h"

enum {
    Song_MaxConcurrentNotes = 8
};

typedef struct {
    uint8_t defaultTempo;
    const Note_t *channels[Song_MaxConcurrentNotes];
} Song_t;

#endif
