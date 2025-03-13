#include <stdatomic.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "lfq.h"

enum ERR {
    NULLPTR=1, 
};


static bool _cmpxchg(uintptr_t volatile *ptr, uintptr_t old, uintptr_t new)
{
    if (*ptr != old)
        return false;

    *ptr = new;
    return true;
}


static int _push_back(struct lfq *self, struct list_head *node)
{
    if (!self || !node)
        return -NULLPTR;

    struct list_head *tail;
    atomic_uintptr_t q_head_prev;

    do {
        tail = self->head.prev;      
        node->next = &self->head;        
    } while (!__sync_bool_compare_and_swap(&(self->head.prev), tail, node));
    // do {
    //     q_head_prev = (uintptr_t) self->head.prev;
    //     tail = self->head.prev;      
    //     node->next = &self->head;        
    // } while (!atomic_compare_exchange_weak(&(q_head_prev), &tail, node));

    node->prev = tail;
    tail->next = node;
    return 0;
}

static int _pop(struct lfq *self, struct list_head **target)
{
    if (!self || !target)
        return -NULLPTR;

    struct list_head *first = NULL, *second = NULL;
    //atomic_uintptr_t q_head_next;
    // do {
    //     q_head_next = (uintptr_t) self->head.next;
    //     first = self->head.next;
    // } while (!atomic_compare_exchange_weak(&q_head_next, &first, first->next));

    do {
        //q_head_next = (uintptr_t) self->head.next;
        first = self->head.next;
        if (first == &self->head) {
            *target = NULL;
            goto out;
        }

        second = first->next;
    } while (!__sync_bool_compare_and_swap(&self->head.next, first, second));

    second->prev = &self->head;
    *target = first;
out:
    return 0;
}

struct lfq *init_lfq()
{
    struct lfq *queue = (struct lfq*) malloc(sizeof(struct lfq));
    if (!queue)
        return NULL;
    INIT_LIST_HEAD(&queue->head);
    queue->pop = _pop;
    queue->push_back = _push_back;
    return queue;
}

void del_lfq(struct lfq **queue)
{
    free(*queue);
    *queue = NULL;
}