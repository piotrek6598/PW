/** @file
 * Future computing interface.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#ifndef FUTURE_H
#define FUTURE_H

#include "threadpool.h"
#include "queue.h"

/** @brief The callable struct is wrapper containing callable function.
 */
typedef struct callable {
  void *(*function)(void *, size_t, size_t *); /* Pointer to callable function. */
  void *arg;                                   /* Pointer to function's arguments. */
  size_t argsz;                                /* Size of arguments. */
} callable_t;

/** @brief The future struct is object containing future result.
  */
typedef struct future {
    void *value;              /* Pointer to result. */
    int resolved;             /* Flag indicates if future is/can be/can't be
                                 resolved. */
    int initiated;            /* Flag indicates if future is initiated. */
    size_t ret_size;          /* Size of result. */
    pthread_mutex_t mutex;    /* Mutex for exclusive access to variables. */
    pthread_cond_t result;    /* Signaled when result was calculated. */
    queue_t *map_calls;       /* Queue of map calls on result. */
} future_t;

/** @brief async Registers task to calculate.
 * @param pool[in, out]     - pointer to threadpool designed to execute the task;
 * @param future[in, out]   - pointer to future which will store the result;
 * @param callable[in]      - task to be executed.
 * @return Value @p 0 in case of success, value @p -1 when future was initiated,
 * but task wasn't registered, value @p -2 when future wasn't initiated.
 */
int async(thread_pool_t *pool, future_t *future, callable_t callable);

/** @brief map Register new task on result.
 * @param pool[in, out]      - pointer to threadpool designed to execute the task;
 * @param future[in, out]    - pointer to future which will store result of given task;
 * @param from[in, out]      - pointer to future storing result;
 * @param function[in, out]  - pointer to function on result.
 * @return Value @p 0 in case of succes, value @p -1 when future was initiated,
 * but task wasn't registered, value @p -2 when future wasn't initiated.
 */
int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *));

/** @brief await Provides result of future when it will be calculated.
 * @param future[in, out]   - pointer to future.
 * @return Pointer to calculated result or NULL if result will be never calculated
 * or future is not initialized.
 */
void *await(future_t *future);

#endif
