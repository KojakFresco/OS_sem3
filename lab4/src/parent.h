#ifndef OS_SEM3_PARENT_H
#define OS_SEM3_PARENT_H
#include <stddef.h>
#include <sys/types.h>

int run_parent(void* mapping, size_t capacity, pid_t child_pid, sigset_t mask);

#endif //OS_SEM3_PARENT_H
