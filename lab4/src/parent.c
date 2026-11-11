#include "parent.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>

#include "shared.h"

static volatile sig_atomic_t sig_received = 0;

static void sig_handler(int sig) {
    if (sig == SIGUSR2) {
        sig_received = 1;
    }
}

int notify_child(const pid_t child_pid, const sigset_t mask, StreamBuffer* buffer) {
    if (kill(child_pid, SIGUSR1) == -1) {
        perror("kill");
        return EXIT_FAILURE;
    }

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
    if (sigaction(SIGUSR2, &action, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }

    char* line = NULL;
    size_t len = 0;
    ssize_t nread;

    while (1) {
        nread = getline(&line, &len, stdin);

        if (nread == -1) {
            buffer->is_eof = true;
            if (kill(child_pid, SIGUSR1) == -1) {
                perror("kill");
                free(line);
                return EXIT_FAILURE;
            }
            break;
        }

        char* endptr = line;

        while (isspace((unsigned char)*endptr)) {
            ++endptr;
        }
        if (*endptr == '\0') {
            continue;
        }

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

            while (isspace((unsigned char)*endptr)) ++endptr;

            prev_ptr = endptr;

            buffer->data[buffer->data_size] = (int) val;
            buffer->data_size += 1;
            if (buffer->data_size >= capacity || endptr[0] == '\0') {
                if (endptr[0] == '\0') buffer->flush = true;
                if (notify_child(child_pid, mask, buffer) == EXIT_FAILURE) {
                    free(line);
                    return EXIT_FAILURE;
                }
            }
        }
    }
    free(line);

    printf("[Parent PID: %d] Work is over without exceptions\n", getpid());
    return EXIT_SUCCESS;

exit_on_err:
    free(line);
    buffer->error = ERR_PARENT;
    if (kill(child_pid, SIGUSR1) == -1) {
        perror("kill");
    }
    return EXIT_FAILURE;
}
