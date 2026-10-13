#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/select.h>

int main(void) {
    int p1[2];
    int p2[2];
    if (pipe(p1) < 0 || pipe(p2) < 0) {
        perror("Pipe creation failed");
        exit(-1);
    }

    char buff[2048];
    if (fgets(buff, sizeof(buff), stdin) == NULL) {
        fprintf(stderr, "Failed to read output file path\n");
        return -1;
    }
    buff[strcspn(buff, "\n")] = '\0';

    const int pid = fork();
    if (pid < 0) {
        perror("Forking was not successful");
        exit(-1);
    }
    if (pid == 0) {
        dup2(p1[0], 0);
        dup2(p2[1], 1);
        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);

        char* args[] = {"./child", buff, NULL};
        execve("./child", args, NULL);

        perror("Child execve failed");
        exit(-1);
    }

    close(p1[0]);
    close(p2[1]);

    int stdin_open = 1;
    while (1) {
        fd_set in_fds;
        FD_ZERO(&in_fds);

        if (stdin_open)
            FD_SET(0, &in_fds);
        FD_SET(p2[0], &in_fds);

        const int s = select(p2[0] + 1, &in_fds, NULL, NULL, NULL);

        if (s == -1) {
            perror("select failed");
            exit(-1);
        }
        if (FD_ISSET(p2[0], &in_fds)) {
            char child_buff[256];
            const ssize_t bytes_read = read(p2[0], child_buff, sizeof(child_buff) - 1);

            if (bytes_read > 0) {
                child_buff[bytes_read] = '\0';
                if (strstr(child_buff, "ERR_DIV_ZERO") != NULL) {
                    printf("Child reported division by zero. Exiting...\n");

                    close(p1[1]);
                    close(p2[0]);
                    exit(-1);
                }
            } else if (bytes_read == 0) {
                printf("Child finished processing successfully.\n");
                break;
            } else {
                perror("read from pipe2 failed");
                close(p1[1]);
                close(p2[0]);
                exit(-1);
            }
        }
        if (stdin_open && FD_ISSET(0, &in_fds)) {
            if (fgets(buff, sizeof(buff), stdin) != NULL) {
                write(p1[1], buff, strlen(buff));
            } else {
                stdin_open = 0;
                close(p1[1]);
            }
        }
    }

    close(p1[1]);
    close(p2[0]);

    int status;
    wait(&status);

    printf("Child process finished with exit code %d\n", status);
    return 0;
}
