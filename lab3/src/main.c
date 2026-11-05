#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

#include "p_merge_sort.h"

#define min(a, b) ((a) > (b) ? (b) : (a))

static int parse_size_t(const char* str, size_t* out) {
    while (isspace((unsigned char)*str)) {
        str++;
    }
    if (*str == '-') {
        return -1;
    }

    char* endptr;
    errno = 0;

    unsigned long long res = strtoull(str, &endptr, 10);

    if (errno == ERANGE) return -1;
    if (endptr == str) return -1;
    if (res > SIZE_MAX) return -1;

    *out = (size_t) res;
    return 0;
}

static int* resize_array(int* arr, size_t* const capacity) {
    if (*capacity == SIZE_MAX) {
        fprintf(stderr, "Failed to resize array");
        free(arr);
        return NULL;
    }
    if (*capacity > SIZE_MAX / 2) {
        *capacity = SIZE_MAX;
    } else {
        *capacity *= 2;
    }
    int* new_arr = realloc(arr, *capacity * sizeof(int));
    if (new_arr == NULL) {
        perror("Memory allocation failed");
        free(arr);
        return NULL;
    }
    return new_arr;
}

static size_t numbers_from_line(char* line, int** out_arr) {
    size_t capacity = 1;
    size_t count = 0;
    int* arr = malloc(sizeof(int) * capacity);
    if (arr == NULL) return 0;

    char* endptr = line;

    while (*endptr != '\0') {
        while (isspace((unsigned char)*endptr)) {
            ++endptr;
        }
        if (*endptr == '\0') break;

        errno = 0;
        char* next_ptr;
        long val = strtol(endptr, &next_ptr, 10);
        if (next_ptr == endptr) {
            free(arr);
            fprintf(stderr, "Entry must contain only valid numbers\n");
            return 0;
        }
        if (errno == ERANGE || val > INT_MAX) {
            free(arr);
            fprintf(stderr, "Entry contains a too large number\n");
            return 0;
        }

        if (count > capacity - 1) {
            arr = resize_array(arr, &capacity);
            if (arr == NULL) {
                return 0;
            }
        }

        arr[count++] = (int)val;
        endptr = next_ptr;
    }

    *out_arr = arr;
    return count;
}

int main(const int argc, char* argv[]) {
    size_t th_count;

    if (argc == 1) {
        th_count = 1;
    } else {
        if (parse_size_t(argv[1], &th_count) != 0 || th_count == 0) {
            printf("Threads count must be a valid positive number");
            return EXIT_FAILURE;
        }
    }

    char* line = NULL;
    size_t line_size = 0;
    ssize_t nread = 0;
    nread = getline(&line, &line_size, stdin);

    if (nread == -1) {
        free(line);
        return EXIT_FAILURE;
    }

    int* arr = NULL;
    size_t size = numbers_from_line(line, &arr);
    free(line);

    if (size == 0) {
        free(arr);
        return EXIT_FAILURE;
    }

    int* sorted = p_merge_sort(arr, size, th_count);
    free(arr);
    if (sorted == NULL) {
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < size; ++i) {
        printf("%d ", sorted[i]);
    }
    printf("\n");
    free(sorted);
    return EXIT_SUCCESS;
}
