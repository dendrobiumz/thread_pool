#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <pthread.h>
#include "lfq.h"

#define THREAD_NBRS (8)

struct thread_task {
    void *args;
    void* (*task_func)(void*);
};


struct thread_pool {
    pthread_t threads[THREAD_NBRS];
    size_t thread_task_nbrs;
    
};


struct thread_pool* init_thread_pool();



#endif