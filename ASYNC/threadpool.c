/** @file
 * Implementation of threadpool's interface.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "threadpool.h"
#include "err.h"
#include "pthread_err_supp.h"

/** @brief catch Handles SIGINT and SIGRTMIN.
 * Handler is used by threads working in active threadpool.
 * Assumes that first SIGRTMIN signal provides information
 * about thread's threadpool and save it.
 * In case of SIGINT, pool is destroyed and process using threadpool
 * is terminated after destroying was finished.
 * Otherwise does nothing.
 * @param sig[in]            - signal number;
 * @param siginfo[in, out]   - pointer to info about signal;
 * @param more[in, out]      - pointer to context of signal.
 */
void catch (int sig, siginfo_t *siginfo, void *more __attribute__((unused))){
    static thread_pool_t *pool = NULL;
    // Saving information about threadpool.
    if (pool == NULL){
        sigval_t sigval = siginfo->si_value;
        pool = (thread_pool_t*)sigval.sival_ptr;
        return;
    }

    if (sig == SIGINT){
        // Setting flag to terminate process after destroying threadpool.
        pool->exitflag = 1;
        // Force destroying threadpool.
        thread_pool_destroy(pool);
    }
}

/** @brief get_work Gets task to do.
 * Assumes that thread using this function owns pool's mutex.
 * @param pool[in, out]   - pointer to thread's threadpool.
 * @return Pointer to the task.
 */
runnable_t* get_work(thread_pool_t *pool){
    runnable_t* task = (runnable_t*)pop_queue(pool->tasks);
    if (task == NULL)
        return NULL;
    pool->waiting_tasks--;
    return task;
}

/** @brief thread Function containing thread code using to create thread.
 * As argument pointer to threadpool is expected.
 * New thread will be part of given threadpool.
 * @param data[in]   - pointer to args.
 */
void *thread (void *data){
    thread_pool_t *pool = (thread_pool_t*)data;
    runnable_t task;
    runnable_t *task_pointer;

    struct sigaction action;
    sigset_t block_mask;
    sigemptyset(&block_mask);
    action.sa_sigaction = catch;
    action.sa_mask = block_mask;
    action.sa_flags = SA_SIGINFO | SA_RESTART;
    union sigval value;
    value.sival_int = SIGRTMIN;
    value.sival_ptr = (void*)pool;

    // Providing handler for SIGRTMIN;
    sigaction_create(SIGRTMIN ,&action, NULL);
    // Sending SIGRTMIN signal with information about threadpool.
    sigqueue_sig(getpid(), SIGRTMIN, value);
    // Providing handler for SIGINT
    sigaction_create(SIGINT, &action, NULL);

    while(1){
        // Taking task if pool isn't shutting down.
        mutex_lock(&pool->mutex);
        while (pool->waiting_tasks == 0 && pool->shutdown == 0){
            condition_wait(&pool->work, &pool->mutex);
        }
        // If pool is shutting down and no more tasks are left thread finishes work.
        if (pool->shutdown == 1 && pool->waiting_tasks == 0)
            break;
        // Getting work.
        task_pointer = get_work(pool);
        mutex_unlock(&pool->mutex);
        if (task_pointer == NULL) {
            fprintf(stderr, "thread taking task from empty queue\n");
        } else {
            // Execute work.
            task = *task_pointer;
            (*(task.function))(task.arg, task.argsz);
        }
        free(task_pointer);
    }
    mutex_unlock(&pool->mutex);
    return NULL;
}

/** @brief create_threads Creates thread in given threadpool.
 * @param pool[in, out]   - pointer to threadpool;
 * @param num_thread      - number of thread in given threadpool.
 */
void create_threads(thread_pool_t *pool, size_t num_thread){
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    thread_create(&(pool->threads[num_thread]), &attr, thread, (void *)pool);
    pthread_attr_destroy(&attr);
}

/** @brief thread_pool_init Initiates given pool argument as threadpool.
 * Behavior of initiating previously initiated pool is undefined.
 * @param pool[in, out]   - pointer to new threadpool;
 * @param num_threads     - maximal number of working threads in threadpool.
 * @return Value @p 0 if initiating succeed, otherwise returns @p -1.
 */
int thread_pool_init(thread_pool_t *pool, size_t num_threads) {
    // Initiating pool variables and allocating memory for arrays.
    if (pool == NULL)
        return -1;
    pool->initiated = 0;
    pool->exitflag = 0;
    pool->pool_size = num_threads;
    pool->waiting_tasks = 0;
    pool->shutdown = 0;
    pool->tasks = make_queue();
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * num_threads);

    if (pool->tasks == NULL)
        return -1;
    if (pool->threads == NULL)
        return -1;

    // Initiating mutex for variables.
    if (pthread_mutex_init(&pool->mutex, NULL) != 0)
        return -1;
    if (pthread_cond_init(&pool->work, NULL) != 0)
        return -1;
    for (size_t i = 0; i < pool->pool_size; i++)
        create_threads(pool, i);
    mutex_lock(&pool->mutex);
    // Marking pool as successfully initiated.
    pool->initiated = 1;
    mutex_unlock(&pool->mutex);
    return 0;
}

/** @brief thread_pool_destroy Destroys given threadpool.
 * All task already registered will be performed.
 * In case of destroying threadpool with SIGINT process will be terminated.
 * After destroying pool is marked as uninitiated.
 * Do nothing on uninitiated thredpool.
 * @param pool[in]   - pointer to threadpool.
 */
void thread_pool_destroy(thread_pool_t *pool) {
    if (pool == NULL)
        return;
    if (pool->initiated == 0)
        return;
    mutex_lock(&pool->mutex);
    // Setting shutdown flag.
    if (pool->shutdown == 1){
        mutex_unlock(&pool->mutex);
        return;
    }
    pool->shutdown = 1;
    // Waking up all sleeping threads
    for (size_t i = 0; i < pool->pool_size; i++)
        condition_signal(&pool->work);
    mutex_unlock(&pool->mutex);
    for (size_t i = 0; i < pool->pool_size; i++)
      thread_join(pool->threads[i], NULL);
    mutex_lock(&pool->mutex);
    // Setting pool as uninitiated
    pool->initiated = 0;
    mutex_unlock(&pool->mutex);
    free(pool->threads);
    delete_queue(pool->tasks);
    mutex_destroy(&pool->mutex);
    condition_destroy(&pool->work);
    // Terminating process if needed.
    if (pool->exitflag == 1)
        exit(130);
}

/** @brief defer Registers task to do.
 * Task can be only registered in initiated not shutdowning pool.
 * @param pool[in, out]   - pointer to threadpool
 * @param runnable[in]    - task to do.
 * @return Value @p 0 if task was registered, otherwise returns @p -1.
 */
int defer(struct thread_pool *pool, runnable_t runnable) {
    runnable_t *runnable_copy;

    if (pool == NULL || pool->initiated == 0){
        fprintf(stderr, "given threadpool doesn't exist or is uninitiated\n");
        return -1;
    }
    // If pool shuts down, new task can't be added.
    mutex_lock(&pool->mutex);
    if (pool->shutdown > 0) {
        mutex_unlock(&pool->mutex);
        fprintf(stderr, "adding task to shutdowning threadpool\n");
        return -1;
    }

    runnable_copy = (runnable_t*)malloc(sizeof(runnable_t));
    if (runnable_copy == NULL){
        fprintf(stderr, "adding task error\n");
        return -1;
    }
    runnable_copy->function = runnable.function;
    runnable_copy->arg = runnable.arg;
    runnable_copy->argsz = runnable.argsz;

    // If task doesn't added to queue, it isn't registered.
    if (add_queue(pool->tasks, (void*)runnable_copy) == -1){
        mutex_unlock(&pool->mutex);
        fprintf(stderr, "adding task error\n");
        return -1;
    }
    pool->waiting_tasks++;
    // Signals threads that there's work to do.
    condition_signal(&pool->work);
    mutex_unlock(&pool->mutex);
    return 0;
}
