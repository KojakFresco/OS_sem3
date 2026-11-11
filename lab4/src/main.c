#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/mman.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#include "child.h"
#include "parent.h"
#include "shared.h"

int main() {
    sigset_t mask, old_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    if (sigprocmask(SIG_BLOCK, &mask, &old_mask) == -1) {
        perror("sigprocmask");
        return EXIT_FAILURE;
    }

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("sysconf");
        return EXIT_FAILURE;
    }

    void* map = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    StreamBuffer* buf = map;
    buf->error = NO_ERROR;
    buf->flush = false;
    buf->is_eof = false;
    buf->data_size = 0;

    char* out_path = NULL;
    size_t len = 0;
    ssize_t nread;
    nread = getline(&out_path, &len, stdin);

    if (nread == -1) {
        fprintf(stderr, "Failed to read filename from stdin\n");
        goto clean_on_err;
    }
    out_path[strcspn(out_path, "\r\n")] = '\0';

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        goto clean_on_err;
    }

    int status;
    if (pid == 0) {
        status = run_child(map, getppid(), old_mask, out_path);
    } else {
        status = run_parent(map, BUFFER_CAPACITY(page_size), pid, old_mask);
        if (waitpid(pid, NULL, 0) == -1) {
            perror("waitpid");
            goto clean_on_err;
        }
    }

    if (munmap(map, page_size) == -1) {
        perror("munmap");
        free(out_path);
        return EXIT_FAILURE;
    }

    free(out_path);

    if (status == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

clean_on_err:
    if (munmap(map, page_size) == -1) {
        perror("munmap");
    }
    free(out_path);
    return EXIT_FAILURE;
}
