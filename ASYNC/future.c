/** @file
 * Implementation of futures computing interface.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#include <stdio.h>
#include "future.h"

/** @brief function_wrapper Provide wrapper for function that can be used
 * in runnable_t.
 */
void function_wrapper(void*, size_t);

/** @brief The callback struct is wrapper containing callable function, future
 * to fullfill and threadpool which will perform calculus.
 */
typedef struct callback{
    void *(*function)(void *, size_t, size_t *); /* Pointer to callable function. */
    void *function_arg;                          /* Pointer to arguments' of callable's function. */
    size_t function_argsz;                       /* Size of function's arguments. */
    future_t *future;                            /* Future where the result will be stored. */
    thread_pool_t *pool;                         /* Threadpool which will execute the function.
                                                    Initiated only in callback created by map. */
} callback_t;

/** @brief execute_map_call Defer single callback to threadpool.
 * Using to defer callbacks from map when result was calculated.
 * @param map_call[in, out]   - pointer to callback.
 */
void execute_map_call(callback_t *map_call){
    runnable_t runnable;
    runnable.function = function_wrapper;
    runnable.arg = map_call;
    runnable.argsz = sizeof(map_call);

    if (defer(map_call->pool, runnable) != 0){
        fprintf(stderr, "can't defer map_call\n");
    }
}

/** @brief execute_map_cals Defers all map callbacks to threadpool.
 * @param future[in, out]   - pointer to future.
 */
void execute_map_calls (future_t *future){
    queue_t *map_calls = future->map_calls;
    if (map_calls == NULL){
        return;
    }
    queue_node_t *tmp_node;
    while (map_calls->front != NULL){
        tmp_node = map_calls->front;
        callback_t *callback = (callback_t*)tmp_node->value;

        // Setting future's result as function argument.
        callback->function_arg = future->value;
        callback->function_argsz = sizeof(future->value);
        map_calls->front = map_calls->front->next;

        execute_map_call(callback);
        free(tmp_node);
    }
}

/** @brief function_wrapper Provide wrapper for function that can be used in runnable_t.
 * Function executes given function, save result and execute map calls.
 * Works only if args is pointer to callback.
 * @param args[in,out]   - pointer to callback.
 */
void function_wrapper(void *args, size_t argsz __attribute__((unused))){
    callback_t *callback = (callback_t*)args;
    void *result;

    result = (*(callback->function))
            (callback->function_arg, callback->function_argsz, &callback->future->ret_size);
    mutex_lock(&callback->future->mutex);

    // Setting result and marking future as resolved.
    callback->future->value = result;
    callback->future->resolved = 1;

    // Notifying that result was calculated.
    condition_broadcast(&callback->future->result);

    // Execute map_calls.
    execute_map_calls(callback->future);

    mutex_unlock(&callback->future->mutex);
    free(args);
}

/** @brief future_init Initiating given future.
 * @param future[in, out]   - pointer to future.
 * @return Value @p 0 if initiating succeed, otherwise value @p -1.
 */
int future_init(future_t *future){
    if (future == NULL)
        return -1;
    future->initiated = 0;
    future->resolved = -1;
    future->value = NULL;
    future->ret_size = 0;
    future->map_calls = make_queue();

    if (future->map_calls == NULL)
        return -1;

    if (pthread_mutex_init(&future->mutex, NULL) != 0){
        delete_queue(future->map_calls);
        return -1;
    }

    if (pthread_cond_init(&future->result, NULL) != 0){
        delete_queue(future->map_calls);
        pthread_mutex_destroy(&future->mutex);
        return -1;
    }

    future->initiated = 1;
    return 0;
}

/** @brief async Registers task to calculate.
 * @param pool[in, out]     - pointer to threadpool designed to execute the task;
 * @param future[in, out]   - pointer to future which will store the result;
 * @param callable[in]      - task to be executed.
 * @return Value @p 0 in case of success, value @p -1 when future was initiated,
 * but task wasn't registered, value @p -2 when future wasn't initiated.
 */
int async(thread_pool_t *pool, future_t *future, callable_t callable) {
    if (future_init(future) != 0)
        return -2;

    // Creating callback.
    callback_t *callback = (callback_t*)malloc(sizeof(callback_t));
    if (callback == NULL)
        return -1;
    callback->function = callable.function;
    callback->function_arg = callable.arg;
    callback->function_argsz = callable.argsz;
    callback->future = future;

    runnable_t runnable;
    runnable.function = function_wrapper;
    runnable.arg = (void*)callback;
    runnable.argsz = sizeof(callback);

    // Defering work to threadpool.
    if (defer(pool, runnable) != 0){
        free(callback);
        return -1;
    }

    mutex_lock(&future->mutex);
    // Marking future that result is expected.
    if (future->resolved == -1)
        future->resolved = 0;
    mutex_unlock(&future->mutex);
    return 0;
}

/** @brief map Register new task on result.
 * @param pool[in, out]      - pointer to threadpool designed to execute the task;
 * @param future[in, out]    - pointer to future which will store result of given task;
 * @param from[in, out]      - pointer to future storing result;
 * @param function[in, out]  - pointer to function on result.
 * @return Value @p 0 in case of succes, value @p -1 when future was initiated,
 * but task wasn't registered, value @p -2 when future wasn't initiated.
 */
int map(thread_pool_t *pool, future_t *future, future_t *from,
        void *(*function)(void *, size_t, size_t *)) {
    // Initiating future.
    if (future_init(future) != 0)
        return -2;

    // Registering map call.
    if (from == NULL)
        return -1;
    if (from->initiated == 0){
        if (future_init(from) != 0)
            return -1;
    }

    mutex_lock(&from->mutex);
    // Creating callback.
    callback_t *new_callback = (callback_t*)malloc(sizeof(callback_t));
    if (new_callback == NULL)
        return -1;
    new_callback->function = function;
    new_callback->future = future;
    new_callback->pool = pool;

    // If result is expected but not calculated, callback will be executed when
    // result comes out.
    if (from->resolved != 1){
        if (add_queue(from->map_calls, (void*)new_callback) != 0){
            mutex_unlock(&from->mutex);
            free(new_callback);
            return -1;
        }
        // Marking future that result is expected.
        mutex_lock(&future->mutex);
        if (future->resolved == -1)
            future->resolved = 0;
        mutex_unlock(&future->mutex);
        mutex_unlock(&from->mutex);
        return 0;
    }
    // If result is calculated, callback is executed.
    new_callback->function_arg = from->value;
    new_callback->function_argsz = sizeof(from->result);
    runnable_t runnable;
    runnable.function = function_wrapper;
    runnable.arg = (void*)new_callback;
    runnable.argsz = sizeof(new_callback);

    if (defer(pool, runnable) != 0){
        free(new_callback);
        return -1;
    }

    mutex_lock(&future->mutex);
    // Marking future that result is expected.
    if (future->resolved == -1)
        future->resolved = 0;
    mutex_unlock(&future->mutex);
    mutex_unlock(&from->mutex);
    return 0;
}

/** @brief await Provides result of future when it will be calculated.
 * @param future[in, out]   - pointer to future.
 * @return Pointer to calculated result or NULL if result will be never calculated
 * or future is not initialized.
 */
void *await(future_t *future) {
    void *result;
    if (future == NULL)
        return NULL;
    if (future->initiated == 0)
        return NULL;
    mutex_lock(&future->mutex);
    // Checking possibility of getting result.
    if (future->resolved == -1){
        mutex_unlock(&future->mutex);
        return NULL;
    }
    // Waiting for the result.
    while (future->resolved == 0)
        condition_wait(&future->result, &future->mutex);
    result = future->value;
    mutex_unlock(&future->mutex);
    return result;
}
