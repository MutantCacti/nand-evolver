/*
 * main.c
 * Entry point
 *
 * Created: 2026-08-17
 *  Author: Maxence Morel Dierckx
 */
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include "test.h"


#define TARGET_FRAME_US 16666 // 60 FPS
static volatile sig_atomic_t running = 1;


long long get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000000LL + (long long)tv.tv_usec;
}


void handle_exit_signal(int sig)
{
    (void)sig;
    running = 0;
}


void arena_string(Model *model, char *arena_str)
{
    for (size_t i = 0; i < model->arena_size; i++)
    {
        word w = model->arena[word_index(i)];
        word bit = (w >> word_offset(i)) & (word)1;
        arena_str[i] = bit ? '1' : '0';
    }
    arena_str[model->arena_size] = '\0';
}


int main(void)
{
    signal(SIGINT, handle_exit_signal);
    signal(SIGTERM, handle_exit_signal);

    enable_raw_mode(); // Make terminal input non-echo and non-canonical (line-by-line)
    test_init_input_listener("/dev/input/event3");

    printf("Running at %d TPS\n", 1000000 / TARGET_FRAME_US);

    /* Pre-initialise a model that solves XOR
     * Arena: 8
     * 0    1   2   3   4   5       6               7
     * A    B   !A  !B  AnB !An!B   (AnB)n(!An!B)   !((AnB)n(!An!B))
     *
     * Nands: 6
     * 0    1   2   3   4   5
     * 003  114 015 346 567 772
     *
     * Diagram:
     * A -------->NAND---.
     * |.---------^      |
     * .\                |==NAND--<NAND--C
     * | '-<NAND-.       |
     * |         |==NAND-'
     * B---<NAND-'
     *
     * 4 layers requires 4 ticks propagation
     */

    Model *m = model_init(8);
    model_add_nand(m, 0, 0, 3);
    model_add_nand(m, 1, 1, 4);
    model_add_nand(m, 0, 1, 5);
    model_add_nand(m, 3, 4, 6);
    model_add_nand(m, 5, 6, 7);
    model_add_nand(m, 7, 7, 2);
    const int propagation_ticks = 4;

    printf("Initialised model (arena_size=%zu, num_nands=%zu)\n", m->arena_size, m->num_nands);

    char arena_str[m->arena_size + 1];
    int error = 0;
    while (running)
    {
        long long start_time = get_time_us();

        test_write_input(m);
        model_compute_ticks(m, propagation_ticks);

        error = test_read_error(m);

        arena_string(m, arena_str);
        printf("\r[arena: %s] error: %d\033[K", arena_str, error);

        fflush(stdout);

        long long work_time = get_time_us() - start_time;
        if (work_time < TARGET_FRAME_US) {
            usleep(TARGET_FRAME_US - work_time);
        } else {
            long long work_time_ms = work_time / 1000;
            fprintf(stderr, "\nmain: Frame dropped (work time: %lld ms)\n", work_time_ms);
        }
    }

    printf("\nExiting...\n");
    model_free(m);
    return 0;
}
