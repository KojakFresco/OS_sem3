#include "child.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "shared.h"

static volatile sig_atomic_t sig_received = 0;

static void sig_handler(int sig) {
    if (sig == SIGUSR1) {
        sig_received = 1;
    }
}

int run_child(void* mapping, const pid_t parent_pid, const sigset_t mask, const char* out_path) {
    StreamBuffer* buff = mapping;
    const struct sigaction action = {.sa_handler = sig_handler};
    sigaction(SIGUSR1, &action, NULL);

    FILE* out = fopen(out_path, "w");
    if (out == NULL) {
        perror("fopen");
        buff->error = ERR_CHILD;
        kill(parent_pid, SIGUSR2);
        return EXIT_FAILURE;
    }

    int is_first = 1;
    int res = 0;
    while (1) {
        while (sig_received == 0) {
            sigsuspend(&mask);
        }
        sig_received = 0;

        if (buff->error == ERR_PARENT) {
            fclose(out);
            return EXIT_FAILURE;
        }

        if (buff->flush) {
            is_first = 1;
            fprintf(out, "%d\n", res);
        }
        if (buff->is_eof) {
            printf("[Child] Work is over without exceptions\n");
            break;
        }

        for (size_t i = 0; i < buff->data_size; i++) {
            if (is_first) {
                res = buff->data[i];
                is_first = 0;
            } else {
                if (buff->data[i] == 0) {
                    fprintf(stderr, "Error: Can not divide by zero\n");
                    fclose(out);
                    buff->error = ERR_CHILD;
                    kill(parent_pid, SIGUSR2);
                    return EXIT_FAILURE;
                }
                res /= buff->data[i];
            }
        }

        kill(parent_pid, SIGUSR2);
    }
    return EXIT_SUCCESS;
}
