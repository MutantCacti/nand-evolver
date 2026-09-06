/*
 * core/arena.h
 * A simple memory arena as an array of words.
 *
 * Created: 2026-09-06
 *  Author: Maxence Morel Dierckx
 */
#ifndef ARENA_H_
#define ARENA_H_


#include "word.h"


#include <stddef.h>


typedef struct
{
    word * words;
    size_t num_words;
}
Arena;


// Zero-initialise an arena of size size
Arena * arena_init(size_t num_words);


// Set the arena to a new size, zero-initialising new memory
int arena_resize(Arena * arena, size_t new_num_words);


// Free the arena allocation
void arena_free(Arena * arena);


#endif
