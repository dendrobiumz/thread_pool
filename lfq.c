#include <stdatomic.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "lfq.h"

enum ERR {
    NULLPTR=1, 
};


// static bool _cmpxchg(uintptr_t volatile *ptr, uintptr_t old, uintptr_t new)
// {
//     if (*ptr != old)
//         return false;

//     *ptr = new;
//     return true;
// }


// static int _push_back(struct lfq *self, struct list_head *node)
// {
//     if (!self || !node)
//         return -NULLPTR;

//     struct list_head *tail;
//     atomic_uintptr_t q_head_prev;

//     do {
//         tail = self->head.prev;      
//         node->next = &self->head;        
//     } while (!__sync_bool_compare_and_swap(&(self->head.prev), tail, node));
//     // do {
//     //     q_head_prev = (uintptr_t) self->head.prev;
//     //     tail = self->head.prev;      
//     //     node->next = &self->head;        
//     // } while (!atomic_compare_exchange_weak(&(q_head_prev), &tail, node));

//     node->prev = tail;
//     tail->next = node;
//     return 0;
// }

// static int _pop(struct lfq *self, struct list_head **target)
// {
//     if (!self || !target)
//         return -NULLPTR;

//     struct list_head *first = NULL, *second = NULL;
//     //atomic_uintptr_t q_head_next;
//     // do {
//     //     q_head_next = (uintptr_t) self->head.next;
//     //     first = self->head.next;
//     // } while (!atomic_compare_exchange_weak(&q_head_next, &first, first->next));

//     do {
//         //q_head_next = (uintptr_t) self->head.next;
//         first = self->head.next;
//         if (first == &self->head) {
//             *target = NULL;
//             goto out;
//         }

//         second = first->next;
//     } while (!__sync_bool_compare_and_swap(&self->head.next, first, second));

//     second->prev = &self->head;
//     *target = first;
// out:
//     return 0;
// }

static int pop_front(struct lfq *self, struct list_head **target)
{
    if (self == NULL)
        return -1;

    atomic_uintptr_t *first = (atomic_uintptr_t *) &self->head.next;
    uintptr_t expected, desired;
    do {
        expected = atomic_load(first);
        if (expected == (uintptr_t)&self->head)
            return -2;
        desired = atomic_load((atomic_uintptr_t *)&((struct list_head*)expected)->next);
    } while (!atomic_compare_exchange_strong(first, &expected, desired));

    ((struct list_head*) desired)->prev = &self->head;
    *target = (struct list_head*) expected;
    return 0;
}

static int pop_back(struct lfq *self, struct list_head **target)
{
    if (self == NULL)
        return -1;

    atomic_uintptr_t *prev_first = (atomic_uintptr_t *) &self->head.prev;
    uintptr_t expected, desired;
    do {
        expected = atomic_load(prev_first);
        if (expected == (uintptr_t)&self->head)
            return -2;
        desired = atomic_load((atomic_uintptr_t *)&((struct list_head*)expected)->prev);
    } while (!atomic_compare_exchange_strong(prev_first, &expected, desired));

    ((struct list_head*) desired)->next = &self->head;
    *target = (struct list_head*) expected;    
    return 0;
}

static int push_back(struct lfq *self, struct list_head *node)
{
    if (self == NULL || node == NULL)
        return -1;

    node->next = &self->head;
    atomic_uintptr_t *last = (atomic_uintptr_t *)&self->head.prev;
    uintptr_t desired = (uintptr_t)node, expected;

    do {
        expected = atomic_load(last);
    } while (!atomic_compare_exchange_strong(last, &expected, desired));

    node->prev = (struct list_head*)expected;
    node->prev->next = node;

    return 0;
}


static int is_empty(struct lfq *q)
{
    atomic_uintptr_t *next = (atomic_uintptr_t *) &q->head.next;
    uintptr_t expected;

    return atomic_load(next) == (uintptr_t)&q->head;
}


lfq_t *new_lfq()
{
    lfq_t *queue = (lfq_t*) malloc(sizeof(lfq_t));
    if (!queue)
        return NULL;
    INIT_LIST_HEAD(&queue->head);
    queue->pop_back = pop_back;
    queue->pop_front = pop_front;
    queue->push_back = push_back;
    queue->is_empty = is_empty;
    return queue;
}

int init_lfq(lfq_t* queue)
{
    if (!queue)
        return -1;

    INIT_LIST_HEAD(&queue->head);
    queue->pop_back = pop_back;
    queue->pop_front = pop_front;
    queue->push_back = push_back;
    queue->is_empty = is_empty;
    return 0;
}

void del_lfq(struct lfq **queue)
{
    free(*queue);
    *queue = NULL;
}