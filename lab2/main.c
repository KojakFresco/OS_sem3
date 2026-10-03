#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

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
        close(p1[0]); close(p1[1]);
        close(p2[0]); close(p2[1]);

        char* args[] = {"./child", buff, NULL};
        execve("./child", args, NULL);

        perror("Child execv failed");
        exit(-1);
    }

    close(p1[0]);
    close(p2[1]);

    while (fgets(buff, sizeof(buff), stdin) != NULL) {
        write(p1[1], buff, strlen(buff));
    }
    close(p1[1]);
    close(p2[0]);

    int status;
    wait(&status);

    printf("Child process finished with exit code %d\n", status);
    return 0;
}
