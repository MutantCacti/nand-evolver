/*
 * train/arena.c
 * A simple memory arena as an array of words.
 *
 * Created: 2026-09-06
 *  Author: Maxence Morel Dierckx
 */
#include "../core/arena.h"


#include <stdlib.h>
#include <string.h>


// MARK: arena_init

Arena * arena_init(size_t num_words)
{
    word * words = calloc(num_words, sizeof(word));
    if (!words) return NULL;

    Arena * arena = calloc(1, sizeof(Arena));
    if (!arena)
    {
        free(words);
        return NULL;
    }

    arena->words = words;
    arena->num_words = num_words;

    return arena;
}


// MARK: arena_resize

int arena_resize(Arena * arena, size_t new_num_words)
{
    if (new_num_words == arena->num_words) return 0;

    word * new_words = realloc(arena->words, new_num_words * sizeof(word));
    if (!new_words) return -1;

    if (new_num_words > arena->num_words)
    {
        memset(new_words + arena->num_words, 0, (new_num_words - arena->num_words) * sizeof(word));
    }

    arena->words = new_words;
    arena->num_words = new_num_words;

    return 0;
}


// MARK: arena_free

void arena_free(Arena * arena)
{
    if (!arena) return;
    if (arena->words) free(arena->words);
    free(arena);
}
