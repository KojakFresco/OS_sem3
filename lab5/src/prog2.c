#include <stdlib.h>
#include <stdio.h>

#include <dlfcn.h>
#include <string.h>

static void* handle = NULL;
static float (*SinIntegral)(float A, float B, float e) = NULL;
static int (*GCF)(int A, int B) = NULL;

static int switch_lib(char const* new_lib) {
    if (handle != NULL) {
        if (dlclose(handle) != 0) {
            fprintf(stderr, "Can't close library: %s\n", dlerror());
            return EXIT_FAILURE;
        }
    }

    handle = dlopen(new_lib, RTLD_LAZY);
    if (handle == NULL) {
        fprintf(stderr, "Can't open library %s: %s\n", new_lib, dlerror());
        return EXIT_FAILURE;
    }

    SinIntegral = dlsym(handle, "SinIntegral");
    if (SinIntegral == NULL) {
        fprintf(stderr, "Can't find SinIntegral in %s: %s\n", new_lib, dlerror());
        return EXIT_FAILURE;
    }

    GCF = dlsym(handle, "GCF");
    if (GCF == NULL) {
        fprintf(stderr, "Can't find GCF in %s: %s\n", new_lib, dlerror());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static char* make_path(char const* str) {
    static char path[32];
    snprintf(path, sizeof(path), "./%s.so", str);
    return path;
}

int main() {
    char* lib_name = "lib1";

    if (switch_lib(make_path(lib_name)) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }


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
            case 0:
                if (strcmp(lib_name, "lib1") == 0) {
                    lib_name = "lib2";
                } else {
                    lib_name = "lib1";
                }
                if (switch_lib(make_path(lib_name)) == EXIT_FAILURE) {
                    return EXIT_FAILURE;
                }
                printf("Realization switched to %s\n", lib_name);
                break;
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
    if (handle != NULL) {
        dlclose(handle);
    }
    return EXIT_SUCCESS;
}
