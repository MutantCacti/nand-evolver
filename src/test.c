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

static int key_fds[MAX_INPUT_DEVICES];
static size_t num_key_fds = 0;


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



size_t find_keyboard_devices(void)
{
    DIR *dir = opendir(INPUT_DIR);
    if (!dir) {
        fprintf(stderr, "find_keyboard_devices: Could not open directory %s\n", INPUT_DIR);
        return 0;
    }

    size_t found = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL && found < MAX_INPUT_DEVICES)
    {
        if (strncmp(entry->d_name, "event", 5) != 0) continue;

        char path[INPUT_PATH_MAX_LENGTH];
        int wrote = snprintf(path, sizeof(path), "%s/%s", INPUT_DIR, entry->d_name);
        if (wrote < 0 || wrote >= (int)sizeof(path)) continue;

        int fd = open(path, O_RDONLY); // Requires input permissions
        if (fd == -1) continue;

        unsigned long key_bits[NLONGS(KEY_MAX + 1)] = {0};
        if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits) < 0) {
            close(fd);
            continue;
        }

        if (!TEST_BIT(KEY_F, key_bits) || !TEST_BIT(KEY_J, key_bits)) {
            close(fd);
            continue;
        }

        char name[256] = "unknown";
        if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) < 0) {
            snprintf(name, sizeof(name), "unknown");
        }
        //printf("Listening on %s (%s)\n", path, name);

        key_fds[found++] = fd;
    }

    closedir(dir);
    return found;
}


void* key_listener_thread(void* arg)
{
    (void)arg;

    struct pollfd pfds[MAX_INPUT_DEVICES];
    for (size_t i = 0; i < num_key_fds; i++) {
        pfds[i].fd = key_fds[i];
        pfds[i].events = POLLIN;
    }

    for (;;)
    {
        if (poll(pfds, num_key_fds, -1) < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "key_listener_thread: poll failed (%s)\n", strerror(errno));
            return NULL;
        }

        for (size_t i = 0; i < num_key_fds; i++)
        {
            if (pfds[i].fd < 0) continue;

            // A wireless keyboard sleeping or unplugging drops its node
            // Reconnecting when it wakes up is out of scope
            if (pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                close(pfds[i].fd);
                pfds[i].fd = -1;
                continue;
            }

            if (!(pfds[i].revents & POLLIN)) continue;

            struct input_event ev;
            ssize_t n = read(pfds[i].fd, &ev, sizeof(ev));
            if (n != (ssize_t)sizeof(ev)) {
                if (n < 0 && errno != EINTR && errno != EAGAIN) {
                    close(pfds[i].fd);
                    pfds[i].fd = -1;
                }
                continue;
            }

            if (ev.type != EV_KEY) continue;
            if (ev.code > KEY_MAX) continue;

            
            if (ev.value == 0) {
                key_states[ev.code] = 0;
            } else if (ev.value == 1) {
                key_states[ev.code] = 1;
            }
            // 2 is autorepeat, we don't care
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


void test_init_input_listener(void)
{
    // Enumerate on the calling thread so failures surface before the frame
    // loop starts and the terminal is in raw mode
    num_key_fds = find_keyboard_devices();
    if (num_key_fds == 0) {
        fprintf(stderr, "test_init_input_listener: No usable input devices found "
                        "(see README.md for input permissions)\n");
        return;
    }

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, key_listener_thread, NULL) != 0) {
        fprintf(stderr, "test_init_input_listener: Failed to initialise key listener thread\n");
        for (size_t i = 0; i < num_key_fds; i++) close(key_fds[i]);
        num_key_fds = 0;
        return;
    }
    pthread_detach(thread_id);
}


void test_write_input(Model *model)
{
    int F_key_down = read_key_as_word(KEY_F);
    int J_key_down = read_key_as_word(KEY_J);

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
