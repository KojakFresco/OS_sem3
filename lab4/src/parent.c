#include "parent.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <bits/signum-arch.h>

#include "shared.h"

static volatile sig_atomic_t sig_received = 0;

static void sig_handler(int sig) {
    if (sig == SIGUSR2) {
        sig_received = 1;
    }
}

int notify_child(const pid_t child_pid, const sigset_t mask, StreamBuffer* buffer) {
    kill(child_pid, SIGUSR1);
    while (sig_received == 0) {
        sigsuspend(&mask);
    }
    sig_received = 0;

    if (buffer->error == ERR_CHILD) {
        return EXIT_FAILURE;
    }

    buffer->data_size = 0;
    buffer->flush = false;
    return EXIT_SUCCESS;
}

int run_parent(void* mapping, const size_t capacity, const pid_t child_pid, const sigset_t mask) {
    StreamBuffer* buffer = mapping;
    const struct sigaction action = {.sa_handler = sig_handler};
    sigaction(SIGUSR2, &action, NULL);

    buffer->error = NO_ERROR;

    char* line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (1) {
        nread = getline(&line, &len, stdin);

        if (nread == -1) {
            buffer->is_eof = true;
            kill(child_pid, SIGUSR1);
            break;
        }

        if (line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        if (nread == 1 && line[0] == '\n') continue;
        if (line[nread - 1] == '\n') line[nread - 1] = '\0';

        char* endptr = line;
        char* prev_ptr = endptr;
        while (*endptr != '\0') {
            errno = 0;
            long val = strtol(prev_ptr, &endptr, 10);

            if (endptr == prev_ptr || (*endptr != '\0' && !isspace(*endptr))) {
                fprintf(stderr, "Error: Wrong symbol '%c' in entry\n", *endptr);
                goto exit_on_err;
            }
            if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
                fprintf(stderr, "Error: Number is too big for int type (should be from %d to %d).\n",
                        INT_MIN, INT_MAX);
                goto exit_on_err;
            }

            while (isspace(*endptr)) ++endptr;

            prev_ptr = endptr;

            buffer->data[buffer->data_size] = (int) val;
            buffer->data_size += 1;
            if (buffer->data_size >= capacity || endptr[0] == '\0') {
                if (notify_child(child_pid, mask, buffer) == EXIT_FAILURE) {
                    free(line);
                    return EXIT_FAILURE;
                }
            }
        }

        buffer->flush = true;
    }
    free(line);

    printf("[Parent] Work is over without exceptions\n");
    return EXIT_SUCCESS;

exit_on_err:
    free(line);
    buffer->error = ERR_PARENT;
    kill(child_pid, SIGUSR1);
    return EXIT_FAILURE;
}
