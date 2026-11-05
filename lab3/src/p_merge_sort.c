#include "p_merge_sort.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#define min(a, b) ((a) > (b) ? (b) : (a))

static void* allocate(const size_t size) {
    void* res = malloc(size);
    if (res == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }
    return res;
}


typedef struct {
    int* start;
    size_t size;
} Batch;

static int* merge(
    const int* const left,
    const size_t left_size,
    const int* const right,
    const size_t right_size
) {
    int l_p = 0, r_p = 0, i = 0;
    int* res = allocate((left_size + right_size) * sizeof(int));
    if (res == NULL) {
        return NULL;
    }
    while (l_p < left_size && r_p < right_size) {
        if (left[l_p] <= right[r_p]) {
            res[i++] = left[l_p];
            ++l_p;
        } else {
            res[i++] = right[r_p];
            ++r_p;
        }
    }
    if (l_p < left_size) {
        while (l_p != left_size) {
            res[i++] = left[l_p++];
        }
    }
    if (r_p < right_size) {
        while (r_p != right_size) {
            res[i++] = right[r_p++];
        }
    }
    return res;
}

static int* thread_merge_sort(int* const arr, const size_t size) {
    if (size == 1) {
        int* res = allocate(sizeof(int));
        if (res != NULL) {
            res[0] = arr[0];
        }
        return res;
    }

    int* left = thread_merge_sort(arr, size / 2);
    if (left == NULL) return NULL;
    int* right = thread_merge_sort(arr + size / 2, size - size / 2);
    if (right == NULL) {
        free(left);
        return NULL;
    }

    int* res = merge(left, size / 2, right, size - size / 2);
    free(right);
    free(left);
    return res;
}

static void* thread_func(void* arg) {
    // returns not NULL if error
    const Batch* batch = arg;
    int* res = thread_merge_sort(batch->start, batch->size);
    if (res == NULL) return batch->start;
    for (size_t i = 0; i < batch->size; ++i) {
        batch->start[i] = res[i];
    }
    free(res);
    return NULL;
}

int* p_merge_sort(const int* arr, const size_t size, const size_t th_count) {
    int* copy = allocate(size * sizeof(int));
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, arr, size * sizeof(int));

    pthread_t* threads = allocate(sizeof(pthread_t) * th_count);
    if (threads == NULL) {
        free(copy);
        return NULL;
    }

    Batch* batches = allocate(sizeof(Batch) * th_count);
    if (batches == NULL) {
        free(threads);
        free(copy);
        return NULL;
    }

    const size_t batch_size = (size - 1) / th_count + 1;
    size_t active_threads = 0;
    for (size_t i = 0; i < th_count; ++i) {
        if (size <= batch_size * i) {
            break;
        }
        batches[i].start = copy + batch_size * i;
        batches[i].size = min(batch_size, size - batch_size * i);
        if (pthread_create(&threads[i], NULL, thread_func, &batches[i]) != 0) {
            perror("Thread creation failed");
            free(threads);
            free(batches);
            free(copy);
            return NULL;
        }
        ++active_threads;
    }

    bool thread_error = false;
    for (int i = 0; i < active_threads; ++i) {
        void* res;
        pthread_join(threads[i], &res);
        if (res != NULL) {
            thread_error = true;
        }
    }
    free(threads);
    if (thread_error) {
        free(copy);
        free(batches);
        return NULL;
    }

    int* sorted = batches[0].start;
    size_t sorted_size = batches[0].size;
    for (int i = 1; i < active_threads; ++i) {
        int* old = sorted;
        sorted = merge(sorted, sorted_size, batches[i].start, batches[i].size);
        sorted_size += batches[i].size;
        if (sorted == NULL) {
            break;
        }
        if (i != 1) free(old);
    }

    if (copy != sorted) {
        free(copy);
    }
    free(batches);
    return sorted;
}
