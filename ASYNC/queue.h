#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdlib.h>

typedef struct queue_node{
    void *value;
    struct queue_node *next;
} queue_node_t;

typedef struct queue{
    queue_node_t *front, *back;
} queue_t;

queue_t *make_queue();
int add_queue(queue_t *queue, void *value);
void *pop_queue(queue_t *queue);
void delete_queue(queue_t *queue);

#endif
