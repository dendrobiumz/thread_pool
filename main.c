#include <stdio.h>
#include <stdlib.h>
#include "lfq.h"
#include "string.h"

#define STR_NUM (8)


struct foo {
    struct list_head node;
    char str[32];
};

char some_string[STR_NUM][32] = {
    "123", "456", "789", "246", "369", "357", "257", "235"
};


int main(int argc, char* argv[])
{
    lfq_t *q = new_lfq();

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

    return 0;
}