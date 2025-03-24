#include <linux/futex.h>

#include <sys/syscall.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "tp.h"

static void futex_wait(atomic_long *addr, int expected) {
    syscall(SYS_futex, addr, FUTEX_WAIT, expected, NULL, NULL, 0);
}

static void futex_wake(atomic_long *addr) {
    syscall(SYS_futex, addr, FUTEX_WAKE, 1, NULL, NULL, 0);
}

typedef struct worker_args{
    int thread_idx;
    tp_t *tp;
} worker_args_t;


static void* worker_func(void *args)
{
    worker_args_t *worker_args = (worker_args_t*) args;
    tp_t *tp = worker_args->tp;
    const int thread_idx = worker_args->thread_idx;
    printf("[INFO] init worker %d\n", thread_idx + 1);
    lfq_t *q = tp->worker_queues[thread_idx];
    
    struct list_head *node = NULL;
    struct thread_task *task = NULL;
    int err;
    while (true) {
        if (q->is_empty(q))
            futex_wait(&q->sz, 1);

        // while (q->is_empty(q))
        //     ;
        
        err = q->pop_front(q, &node);
        if (err)
            continue;
        // if (err == -2) {
        //     printf("[INFO] empty queue.\n");
        //     continue;
        // } else if (err)
        //     continue;
        

        task = list_entry(node, struct thread_task, node);
        printf("[WORKER %d] Executing task\n", thread_idx + 1);
        task->task_func(task->args);
        node = NULL;
        free(task);
        task = NULL;
    }
    free(worker_args);
    return NULL;
}

static int push_work(tp_t *self, struct thread_task *task)
{
    int thread_idx = self->task_nbrs++ % THREAD_NBRS;
    int err;
    err = self->worker_queues[thread_idx]->push_back(self->worker_queues[thread_idx], &task->node);
    if (err)
        return -1;
    futex_wake(&self->worker_queues[thread_idx]->sz);
    printf("[INFO] push work to thread %d\n", thread_idx + 1);
    return 0;
}


tp_t* new_thread_pool()
{
    tp_t* tp = (tp_t*) malloc(sizeof(tp_t));
    if (tp == NULL)
        return NULL;

    tp->task_nbrs = 0;
    tp->push_work = push_work;
    int err;
    for (int idx = 0; idx < THREAD_NBRS; idx++) {
        tp->worker_queues[idx] = new_lfq();
        if (tp->worker_queues[idx] == NULL) {
            for (int i = 0; i < idx; i++) {
                pthread_cancel(tp->threads[i]);
                free(tp->worker_queues[idx]);
            }
            free (tp);
            tp = NULL;
            goto out;
        }
        worker_args_t *args = (worker_args_t*) malloc(sizeof(worker_args_t));
        args->thread_idx = idx;
        args->tp = tp;
        err = pthread_create(&tp->threads[idx], NULL, worker_func, (void*)args);
        if (err) {
            for (int i = 0; i < idx; i++)
                pthread_cancel(tp->threads[i]);
            
            for (int i = 0; i <= idx; i++)
                free(tp->worker_queues[idx]);

            free (tp);
            tp = NULL;
            goto out;
        }
        pthread_detach(tp->threads[idx]);
    }
    printf("[INFO] new thread_pool successfully\n");
out:
    return tp;
}

void init_thread_pool(tp_t* tp)
{
    tp->task_nbrs = 0;
    tp->push_work = push_work;
    int err;
    for (int idx = 0; idx < THREAD_NBRS; idx++) {
        tp->worker_queues[idx] = new_lfq();
        if (tp->worker_queues[idx] == NULL) {
            for (int i = 0; i < idx; i++) {
                pthread_cancel(tp->threads[i]);
                free(tp->worker_queues[idx]);
            }
            free (tp);
            tp = NULL;
            goto out;
        }
        worker_args_t *args = (worker_args_t*) malloc(sizeof(worker_args_t));
        args->thread_idx = idx;
        args->tp = tp;
        err = pthread_create(&tp->threads[idx], NULL, worker_func, (void*)args);
        if (err) {
            for (int i = 0; i < idx; i++)
                pthread_cancel(tp->threads[i]);
            
            for (int i = 0; i <= idx; i++)
                free(tp->worker_queues[idx]);

            free (tp);
            tp = NULL;
            goto out;
        }
        pthread_detach(tp->threads[idx]);
    }
out:
    return;
}