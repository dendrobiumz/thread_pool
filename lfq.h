#ifndef _LFQ_H_
#define _LFQ_H_

#include "list.h"
#include <stdatomic.h>


typedef struct lfq {
    struct list_head head;
    atomic_long sz;
    int (*push_back)(struct lfq *, struct list_head *);
    int (*pop_front)(struct lfq *, struct list_head **);
    int (*pop_back)(struct lfq *, struct list_head **);
    int (*is_empty)(struct lfq*);
} lfq_t;

lfq_t *new_lfq();
int init_lfq(lfq_t* q);
void del_lfq(struct lfq **queue);

#endif