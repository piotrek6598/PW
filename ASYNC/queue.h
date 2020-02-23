/** @file
 * Queue interface.
 * Queue enable storing objects of any type.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>
#include <stdlib.h>

/** @brief The queue_node struct represents one object in queue.
 */
typedef struct queue_node {
    void *value;               /* Pointer to object in queue. */
    struct queue_node *next;   /* Pointer to next object in queue */
} queue_node_t;

/** @brief The queue struct represents queue.
  */
typedef struct queue {
    queue_node_t *front, *back; /* Pointer to first and last object in queue. */
} queue_t;

/** @brief make_queue Creates empty queue.
 * @return Pointer to the queue or NULL if allocation problem occurred.
 */
queue_t *make_queue();

/** @brief add_queue Adds element to queue.
 * @param queue[in, out]   - pointer to queue;
 * @param value[in, out]   - pointer to element.
 * @return Value @p 0 if element was added, otherwise @p -1.
 */
int add_queue(queue_t *queue, void *value);

/** @brief pop_queue Removes object from front of queue.
 * @param queue[in, out]   - pointer to queue.
 * @return Pointer to element or NULL if queue is empty.
 */
void *pop_queue(queue_t *queue);

/** @brief delete_queue Deletes queue.
 * @param queue[in]   - pointer to queue.
 */
void delete_queue(queue_t *queue);

#endif
