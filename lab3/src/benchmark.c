#define _POSIX_C_SOURCE 200809L

#include "p_merge_sort.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static const size_t sizes[] = {1000, 10000, 100000, 1000000};
static const size_t thread_counts[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20
};
static const size_t repetitions = 3;

static uint32_t next_random(uint32_t* state) {
    *state ^= *state << 13;
    *state ^= *state >> 17;
    *state ^= *state << 5;
    return *state;
}

static void fill_array(int* array, size_t size, size_t pattern) {
    uint32_t state = 0x12345678u;

    for (size_t i = 0; i < size; ++i) {
        switch (pattern) {
            case 0:
                array[i] = (int)(next_random(&state) % (size * 2 + 1)) - (int)size;
                break;
            case 1:
                array[i] = (int)i;
                break;
            default:
                array[i] = (int)(size - i);
                break;
        }
    }
}

static double elapsed_milliseconds(
    const struct timespec* start,
    const struct timespec* finish
) {
    const int64_t seconds = (int64_t)finish->tv_sec - start->tv_sec;
    const int64_t nanoseconds = (int64_t)finish->tv_nsec - start->tv_nsec;
    return (double)(seconds * INT64_C(1000000000) + nanoseconds) / 1000000.0;
}

static int is_sorted(const int* array, size_t size) {
    for (size_t i = 1; i < size; ++i) {
        if (array[i - 1] > array[i]) {
            return 0;
        }
    }
    return 1;
}

int main(void) {
    const char* const pattern_names[] = {"random", "sorted", "reverse"};

    printf("pattern,size,threads,average_ms,speedup,efficiency\n");

    for (size_t pattern = 0; pattern < 3; ++pattern) {
        for (size_t size_index = 0; size_index < sizeof(sizes) / sizeof(sizes[0]); ++size_index) {
            const size_t size = sizes[size_index];
            int* input = malloc(size * sizeof(*input));
            if (input == NULL) {
                fprintf(stderr, "Failed to allocate %zu integers\n", size);
                return EXIT_FAILURE;
            }
            fill_array(input, size, pattern);

            double baseline_ms = 0.0;
            for (size_t thread_index = 0;
                 thread_index < sizeof(thread_counts) / sizeof(thread_counts[0]);
                 ++thread_index) {
                const size_t thread_count = thread_counts[thread_index];
                double total_ms = 0.0;

                for (size_t repetition = 0; repetition < repetitions; ++repetition) {
                    struct timespec start;
                    struct timespec finish;

                    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
                        perror("clock_gettime");
                        free(input);
                        return EXIT_FAILURE;
                    }
                    int* sorted = p_merge_sort(input, size, thread_count);
                    if (clock_gettime(CLOCK_MONOTONIC, &finish) != 0) {
                        perror("clock_gettime");
                        free(sorted);
                        free(input);
                        return EXIT_FAILURE;
                    }
                    if (sorted == NULL || !is_sorted(sorted, size)) {
                        fprintf(stderr, "Sorting failed for %s, size %zu, threads %zu\n",
                                pattern_names[pattern], size, thread_count);
                        free(sorted);
                        free(input);
                        return EXIT_FAILURE;
                    }
                    total_ms += elapsed_milliseconds(&start, &finish);
                    free(sorted);
                }

                const double average_ms = total_ms / repetitions;
                if (thread_count == 1) {
                    baseline_ms = average_ms;
                }
                printf("%s,%zu,%zu,%.3f,%.3f,%.3f\n",
                       pattern_names[pattern],
                       size,
                       thread_count,
                       average_ms,
                       baseline_ms / average_ms,
                       baseline_ms / (average_ms * thread_count));
            }
            free(input);
        }
    }

    return EXIT_SUCCESS;
}
