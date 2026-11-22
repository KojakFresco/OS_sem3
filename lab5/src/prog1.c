#include "contracts.h"

#include <stdio.h>
#include <stdlib.h>

int main() {
    char line[256];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        int cmd;
        if (sscanf(line, "%d", &cmd) != 1) {
            fprintf(stderr, "Invalid command! Enter a number.\n");
            continue;
        }

        float A, B, e;
        int a, b;
        switch (cmd) {
            case 1:
                if (sscanf(line, "%*d %f %f %f", &A, &B, &e) != 3) {
                    fprintf(stderr, "Usage: 1 <A> <B> <e>\n");
                    break;
                }
                printf("%f\n", SinIntegral(A, B, e));
                break;
            case 2:
                if (sscanf(line, "%*d %d %d", &a, &b) != 2) {
                    fprintf(stderr, "Usage: 2 <A> <B>\n");
                    break;
                }
                int const result = GCF(a, b);
                if (result <= 0) {
                    fprintf(stderr, "Only natural numbers are supported\n");
                } else {
                    printf("%d\n", result);
                }
                break;
            default:
                fprintf(stderr, "Wrong command!\nUsage:\n    1 <A> <B> <e>\n    2 <A> <B>\n");
                break;
        }
    }
    return EXIT_SUCCESS;
}
