#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <signal.h>
#include <string.h>

#include "child.h"
#include "parent.h"
#include "shared.h"

int main() {
    void* map = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }

    sigset_t mask, old_mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigprocmask(SIG_BLOCK, &mask, &old_mask);

    char* out_path = NULL;
    size_t len = 0;
    ssize_t nread;
    nread = getline(&out_path, &len, stdin);

    if (nread == -1) {
        munmap(map, PAGE_SIZE);
        fprintf(stderr, "Failed to read filename from stdin\n");
        return EXIT_FAILURE;
    }
    out_path[strcspn(out_path, "\r\n")] = '\0';

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        munmap(map, PAGE_SIZE);
        free(out_path);
        return EXIT_FAILURE;
    }
    if (pid == 0) {
        run_child(map, getppid(), old_mask, out_path);
    } else {
        run_parent(map, BUFFER_CAPACITY, pid, old_mask);
    }
    munmap(map, PAGE_SIZE);
    free(out_path);
    return EXIT_SUCCESS;
}
