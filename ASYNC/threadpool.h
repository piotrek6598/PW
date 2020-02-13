/** @file
 * Threadpool's interface.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stddef.h>
#include <pthread.h>
#include "err.h"
#include "queue.h"
#include "pthread_err_supp.h"

typedef struct runnable {
  void (*function)(void *, size_t);
  void *arg;
  size_t argsz;
} runnable_t;


typedef struct thread_pool {
    int initiated;                 /* flag indicates if threadpool is initialized */
    int exitflag;                  /* flag indicates if process terminates after shut down */
    size_t pool_size;              /* pool_size */
    size_t waiting_tasks;          /* number of waiting tasks */
    pthread_t *threads;            /* array of created threads */
    pthread_mutex_t mutex;         /* mutex for exclusive access to variables */
    pthread_cond_t work;           /* signaled when there is work to do */
    queue_t *tasks;                /* queue of tasks to be done */
    int shutdown;                  /* flag indicates if threadpool is shuttig down */
} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, size_t pool_size);

void thread_pool_destroy(thread_pool_t *pool);

int defer(thread_pool_t *pool, runnable_t runnable);

#endif
