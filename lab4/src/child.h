#ifndef OS_SEM3_CHILD_H
#define OS_SEM3_CHILD_H
#include <fcntl.h>
#include <bits/types/sigset_t.h>

int run_child(void* mapping, pid_t parent_pid, sigset_t mask, const char* out_path);
#endif //OS_SEM3_CHILD_H
