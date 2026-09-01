/*
 * test.h
 * Input handler and output parser
 * Measures error against known solutions
 *
 * Created: 2026-08-18
 *  Author: Maxence Morel Dierckx
 */
#ifndef TEST_H_
#define TEST_H_


#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <termios.h>
#include <pthread.h>
#include <stdatomic.h>
#include "model.h"


#define INPUT_DIR "/dev/input"
#define INPUT_PATH_MAX_LENGTH 512
#define MAX_INPUT_DEVICES 16
#define LONG_BITS (8 * sizeof(size_t))
#define NLONGS(bits) (((bits) + LONG_BITS - 1) / LONG_BITS)
#define TEST_BIT(bit, arr) (((arr)[(bit) / LONG_BITS] >> ((bit) % LONG_BITS)) & 1UL)


// Helpers
void enable_raw_mode(void);
word read_key_as_word(int key_code);
size_t find_keyboard_devices(void);

// Test interface
void test_init_input_listener(void);
void test_write_input(Model *model);
int test_read_error(Model *model);


#endif
