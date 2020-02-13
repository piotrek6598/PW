/** @file
 * Queue implementation based on linked list.
 *
 * @author Piotr Jasinski <jasinsksipiotr99@gmail.com>
 */

#include "queue.h"

/** @brief make_queue Creates empty queue.
 * @return Pointer to the queue or NULL if allocation problem occured.
 */
queue_t* make_queue(){
    queue_t *new_queue = (queue_t*)malloc(sizeof(queue_t));
    if (new_queue == NULL)
        return NULL;
    new_queue->front = NULL;
    new_queue->back = NULL;
    return new_queue;
}

/** @brief add_queue Adds element to queue.
 * @param queue[in, out]   - pointer to queue;
 * @param value[in, out]   - pointer to element.
 * @return Value @p 0 if element was added, otherwise @p -1.
 */
int add_queue(queue_t *queue, void *value){
    queue_node_t *new_node = (queue_node_t*)malloc(sizeof(queue_node_t));
    if (new_node == NULL)
        return -1;
    new_node->value = value;
    new_node->next = NULL;
    if (queue->back == NULL){
        queue->front = new_node;
        queue->back = new_node;
        return 0;
    }
    queue->back->next = new_node;
    queue->back = new_node;
    return 0;
}

/** @brief pop_queue Removes object from front of queue.
 * @param queue[in, out]   - pointer to queue.
 * @return Pointer to element or NULL if queue is empty.
 */
void* pop_queue(queue_t *queue){
    queue_node_t *tmp_node;
    void *value;
    if (queue->front == NULL)
        return NULL;
    tmp_node = queue->front;
    queue->front = queue->front->next;
    if (queue->front == NULL)
        queue->back = NULL;
    value = tmp_node->value;
    free(tmp_node);
    return value;
}

/** @brief delete_queue Deletes queue.
 * @param queue[in]   - pointer to queue.
 */
void delete_queue(queue_t *queue){
    queue_node_t *tmp_node;
    while (queue->front != NULL){
        tmp_node = queue->front;
        queue->front = queue->front->next;
        free(tmp_node);
    }
    free(queue);
}
