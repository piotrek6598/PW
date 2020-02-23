/** @file
 * Threadpool's interface.
 * In case of receiving SIGINT threadpool finishes all deferred tasks
 * and terminates process.
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

/** @brief The runnable struct is wrapper containing runnable function.
  */
typedef struct runnable {
  void (*function)(void *, size_t);
  void *arg;
  size_t argsz;
} runnable_t;

/** @brief The thread_pool struct is object representing threadpool.
  */
typedef struct thread_pool {
    int initiated;                 /* Flag indicates if threadpool is initiated. */
    int exitflag;                  /* Flag indicates if process terminates after shut down. */
    size_t pool_size;              /* Pool_size. */
    size_t waiting_tasks;          /* Number of waiting tasks. */
    pthread_t *threads;            /* Array of created threads. */
    pthread_mutex_t mutex;         /* Mutex for exclusive access to variables. */
    pthread_cond_t work;           /* Signaled when there is work to do. */
    queue_t *tasks;                /* Queue of tasks to be done. */
    int shutdown;                  /* Flag indicates if threadpool is shutting down. */
} thread_pool_t;

/** @brief thread_pool_init Initiates given pool argument as threadpool.
 * Behavior of initiating previously initiated pool is undefined.
 * @param pool[in, out]   - pointer to new threadpool;
 * @param num_threads     - maximal number of working threads in threadpool.
 * @return Value @p 0 if initiating successed, otherwise returns @p -1.
 */
int thread_pool_init(thread_pool_t *pool, size_t pool_size);

/** @brief thread_pool_destroy Destroys given threadpool.
 * All task already registered will be performed.
 * In case of destroying threadpool with SIGINT process will be terminated.
 * After destroying pool is marked as uninitiated.
 * Do nothing on uninitiated threadpool.
 * @param pool[in]   - pointer to threadpool.
 */
void thread_pool_destroy(thread_pool_t *pool);

/** @brief defer Registers task to do.
 * Task can be only registered in initiated not shutdowning pool.
 * @param pool[in, out]   - pointer to threadpool
 * @param runnable[in]    - task to do.
 * @return Value @p 0 if task was registered, otherwise returns @p -1.
 */
int defer(thread_pool_t *pool, runnable_t runnable);

#endif
