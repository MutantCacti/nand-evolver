/*
 * test.c
 * See definitions in test.h
 * The current task is XOR, which is temporarily hardcoded.
 *
 * Created: 2026-08-18
 *  Author: Maxence Morel Dierckx
 */
#include "test.h"


static volatile uint8_t key_states[KEY_MAX + 1] = {0};


void* key_listener_thread(void* arg)
{
    const char* input_device = (const char*)arg;
    int fd = open(input_device, O_RDONLY); // Requires sudo
    if (fd == -1) {
        fprintf(stderr, "read_key_as_word: Error opening input device %s (did you forget sudo?)\n", input_device);
        exit(1);
    }

    struct input_event ev;
    for (;;)
    {
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n == (ssize_t)sizeof(ev)) {
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


word read_key_as_word(int key)
{
    if (key < 0 || key > KEY_MAX) {
        fprintf(stderr, "read_key_as_word: Invalid key code %d\n", key);
        return (word)0;
    }
    return key_states[key];
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
    word input_mask = (word)3; // 0..011;
    model->arena[0] &= ~input_mask;
    model->arena[0] |= read_key_as_word(33) << 0;
    model->arena[0] |= read_key_as_word(36) << 1;
}


int test_read_output(Model *model)
{
    word output_value = model->arena[0] >> 2 & (word)1;
    return (int)output_value;
}

int test_read_error(Model *model)
{
    // The expected output for XOR of keys F and J
    int expected_output = (read_key_as_word(33) ^ read_key_as_word(36)) & 1;
    int actual_output = test_read_output(model);
    return actual_output != expected_output;
}
