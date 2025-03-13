#ifndef _LFQ_H_
#define _LFQ_H_
#include "list.h"

struct lfq {
    struct list_head head;
    int (*push_back)(struct lfq *, struct list_head *);
    int (*pop)(struct lfq *, struct list_head **);
};

struct lfq *init_lfq();
void del_lfq(struct lfq **queue);
#endif