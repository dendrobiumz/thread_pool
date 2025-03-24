#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <pthread.h>
#include "lfq.h"

#define THREAD_NBRS (8)

typedef struct thread_task {
    void *args;
    void* (*task_func)(void*);
    struct list_head node;
} thread_task_t;



typedef struct thread_pool {
    pthread_t threads[THREAD_NBRS];
    lfq_t *worker_queues[THREAD_NBRS];
    size_t task_nbrs;
    int (*push_work)(struct thread_pool *self, struct thread_task *task);
} tp_t;


tp_t* new_thread_pool();
void init_thread_pool(tp_t* tp);


#endif
