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
#include <linux/input.h>
#include <termios.h>
#include <pthread.h>
#include <stdatomic.h>
#include "model.h"


void enable_raw_mode(void);
word read_key_as_word(int key_code);
void test_init_input_listener(char *input_device);
void test_write_input(Model *model);
int test_read_error(Model *model);


#endif
