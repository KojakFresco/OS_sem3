#ifndef OS_SEM3_SHARED_H
#define OS_SEM3_SHARED_H
#include <stdbool.h>
#include <stddef.h>
#include <unistd.h>

#define NO_ERROR      0
#define ERR_CHILD     1
#define ERR_PARENT    2

typedef struct StreamBuffer {
    int error;
    bool flush; // is this batch the last batch of one entry
    bool is_eof; // was previous batch the absolute final batch
    size_t data_size;
    int data[];
} StreamBuffer;

#define BUFFER_CAPACITY(page_size) ((page_size - sizeof(struct StreamBuffer)) / sizeof(int))

#endif //OS_SEM3_SHARED_H
