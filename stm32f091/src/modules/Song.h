#ifndef __SONG_H__
#define __SONG_H__

#include <stdint.h>
#include "Note.h"

enum {
    Song_MaxConcurrentNotes = 8,
    Song_NumberOfSongs = 3
};

typedef struct {
    const Note_t *channels[Song_MaxConcurrentNotes];
} Song_t;

#endif
