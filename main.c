#include <stdio.h>
#include <stdlib.h>
#include "lfq.h"
#include "tp.h"
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define STR_NUM (8)
#define TASK_MAX (1024)

// #define LFQ_TEST
// #define PRODUCER_CONSUMER
// #define NORMAL_TEST


#define TP_TEST

struct foo {
    struct list_head node;
    char str[32];
};

const char some_string[STR_NUM][32] = {
    "123", "456", "789", "246", "369", "357", "257", "235"
};

void* push(void* args)
{
    printf("[INFO] push thread here!\n");
    lfq_t *q = (lfq_t*) args;
    for (int i = 0; i < STR_NUM; i++) {
        struct foo *f = (struct foo*) malloc(sizeof(struct foo));
        strncpy(f->str, some_string[i], strlen(some_string[i]));
        q->push_back(q, &f->node);
    }
    return NULL;
}

void* pop(void* args)
{
    printf("[INFO] pop thread here!\n");
    lfq_t *q = (lfq_t*) args;
    struct list_head *node;

    while (true) {
        while (q->is_empty(q))
            ;
        q->pop_front(q, &node);
        struct foo *element = list_entry(node, struct foo, node);
        printf("[INFO] %s\n", element->str);
    }
    return NULL;
}


void* foo1(void* args)
{
    printf("[INFO] foo1 thread here!\n");
    return NULL;
}

void* foo2(void* args)
{
    printf("[INFO] foo2 thread here!\n");
    return NULL;
}


int main(int argc, char* argv[])
{
#ifdef LFQ_TEST
    lfq_t *q = new_lfq();
#ifdef NORMAL_TEST
    for (int i = 0; i < STR_NUM; i++) {
        struct foo *f = (struct foo*) malloc(sizeof(struct foo));
        strncpy(f->str, some_string[i], strlen(some_string[i]));
        q->push_back(q, &f->node);
    }

    struct list_head *node;

    list_for_each(node, &q->head) {
        struct foo *element = list_entry(node, struct foo, node);
        printf("[INFO] %s\n", element->str);
    }
#endif
#ifdef PRODUCER_CONSUMER
    pthread_t thread[2];
    int tidx[2];

    tidx[0] = 0;
    pthread_create(&thread[0], NULL, pop, (void*)q);

    tidx[1] = 1;
    pthread_create(&thread[1], NULL, push, (void*)q);

    pthread_join(thread[0], NULL);
    pthread_join(thread[1], NULL);
#endif
#endif

#ifdef TP_TEST
    printf("[TEST][THREADPOOL] Start\n");
    lfq_t *q = new_lfq();
    if (q == NULL)
        goto out;
    tp_t *tp = new_thread_pool();
    if (tp == NULL)
        goto out;
    
    sleep(1);
    int err;
    struct thread_task *tk1 = (struct thread_task*) malloc(sizeof(struct thread_task));
    INIT_LIST_HEAD(&tk1->node);
    tk1->args = (void*) q;
    tk1->task_func = pop;
    err = tp->push_work(tp, tk1);
    if (err) {
        free (tk1);
        goto out;
    }

    for (int i = 0; i < 2; i++) {
        struct thread_task *tk2 = (struct thread_task*) malloc(sizeof(struct thread_task));
        INIT_LIST_HEAD(&tk2->node);
        tk2->args = (void*) q;
        tk2->task_func = push;
        err = tp->push_work(tp, tk2);
        if (err) {
            free (tk2);
            goto out;
    }
    }


    //printf("[INFO] %ld\n", tp->task_nbrs);
    sleep(5);
#endif

out:
    return 0;
}