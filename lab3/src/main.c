#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "p_merge_sort.h"

#define max(a, b) ((a) > (b) ? (a) : (b))
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

int main(const int argc, char* argv[]) {
    size_t th_count;

    if (argc == 1) {
        th_count = 1;
    } else {
        if (parse_size_t(argv[1], &th_count) != 0 || th_count == 0) {
            printf("Threads count must be a valid positive number");
            return 1;
        }
    }

    int* arr = malloc(sizeof(int));
    size_t capacity = 1, size = 0;
    int k;
    while (scanf("%d", &k) == 1) {
        if (size > capacity - 1) {
            arr = resize_array(arr, &capacity);
            if (arr == NULL) {
                return 1;
            }
        }
        arr[size] = k;
        ++size;
    }
    if (size == 0) {
        free(arr);
        return 0;
    }

    int* sorted = p_merge_sort(arr, size, th_count);
    free(arr);
    if (sorted == NULL) {
        return 1;
    }
    for (size_t i = 0; i < size; ++i) {
        printf("%d ", sorted[i]);
    }
    printf("\n");
    free(sorted);
    return 0;
}
