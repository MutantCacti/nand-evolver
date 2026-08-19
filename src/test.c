/*
 * test.c
 * See definitions in test.h
 * The current task is XOR, which is temporarily hardcoded.
 *
 * Created: 2026-08-18
 *  Author: Maxence Morel Dierckx
 */
#include "test.h"


static _Atomic uint8_t key_states[KEY_MAX + 1] = {0};
static int expected_output = 0;
static struct termios original_termios; // Store original terminal settings


void disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
    printf("\033[?25h"); // Show cursor
    fflush(stdout);
}


void enable_raw_mode(void)
{
    tcgetattr(STDIN_FILENO, &original_termios);
    atexit(disable_raw_mode);

    struct termios raw = original_termios;
    raw.c_lflag &= ~(ECHO | ICANON); // Disable echo and canonical

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    printf("\033[?25l"); // Hide cursor
    fflush(stdout);
}


void* key_listener_thread(void* arg)
{
    const char* input_device = (const char*)arg;
    int fd = open(input_device, O_RDONLY); // Requires input permissions
    if (fd == -1) {
        fprintf(stderr, "read_key_as_word: Error opening input device %s (did you forget sudo?)\n", input_device);
        exit(1);
    }

    struct input_event ev;
    for (;;)
    {
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n == (ssize_t)sizeof(ev)) {
            if (ev.code > KEY_MAX) continue;
            if (ev.type == EV_KEY) {
                if (ev.value == 0) {
                    key_states[ev.code] = 0;
                } else if (ev.value == 1) {
                    key_states[ev.code] = 1;
                }
            }
        } else if (n < 0) {
            fprintf(stderr, "read_key_as_word: Error reading input device %s\n", input_device);
        }
    }
}


word read_key_as_word(int key_code)
{
    if (key_code < 0 || key_code > KEY_MAX) {
        fprintf(stderr, "read_key_as_word: Invalid key code %d\n", key_code);
        return (word)0;
    }
    return key_states[key_code];
}


void test_init_input_listener(char *input_device)
{
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, key_listener_thread, (void*)input_device) != 0) {
        fprintf(stderr, "test_init_input_listener: Failed to initialise key listener thread\n");
        return;
    }
    pthread_detach(thread_id);
}


void test_write_input(Model *model)
{
    int F_key_down = read_key_as_word(33);
    int J_key_down = read_key_as_word(36);

    // Write inputs to bits 0 and 1
    model_arena_set(model, 0, F_key_down);
    model_arena_set(model, 1, J_key_down);

    expected_output = F_key_down ^ J_key_down;
}


int test_read_error(Model *model)
{
    // The expected output for XOR of keys F and J
    // is stored in the static var in test_write_input
    int actual_output = (int)model_arena_get(model, 2);
    return actual_output != expected_output;
}
