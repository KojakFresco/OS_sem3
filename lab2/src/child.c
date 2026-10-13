#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(const int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output_file>\n", argv[0]);
        return -1;
    }

    char buff[2048];
    const int fd = open(argv[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Failed to open file");
        return -1;
    }

    while (fgets(buff, sizeof(buff), stdin) != NULL) {
        const char* token = strtok(buff, " \n");
        if (token == NULL)
            continue;
        long n = strtol(token, NULL, 10);

        token = strtok(NULL, " \n");
        while (token != NULL) {
            const long k = strtol(token, NULL, 10);
            if (k == 0) {
                fprintf(stderr, "Division by zero is not allowed\n");

                printf("ERR_DIV_ZERO\n");
                fflush(stdout);

                close(fd);
                exit(-1);
            }
            n /= k;
            token = strtok(NULL, " \n");
        }

        char out_buff[64];
        const int len = snprintf(out_buff, sizeof(out_buff), "%ld\n", n);
        write(fd, out_buff, len);
    }

    close(fd);
    return 0;
}
