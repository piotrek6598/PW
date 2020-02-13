/** @file
 * Example of use future computing.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include "future.h"
#include "threadpool.h"


int multiply_by;
static thread_pool_t pool;
pthread_mutex_t mutex;

// Multiply given argument by multiply_by, increase multiply_by by 1 and save result.
static void *function(void *arg, size_t argsz __attribute__((unused)),
                      size_t *retsz __attribute__((unused))){
    long long int n = *(long long int*)arg;
    long long int *result = malloc(sizeof(long long int));
    mutex_lock(&mutex);
    *result = n * multiply_by;
    multiply_by++;
    mutex_unlock(&mutex);
    return result;
}

int main() {
    int err;
    long long int n;
    long long int base = 1;
    multiply_by = 1;
    callable_t callable;

    scanf ("%lld", &n);
    // There's nothing to do if n is less than 1.
    if (n < 1){
        printf("%d\n", 1);
        return 0;
    }

    future_t futures[n];

    // Initiating threadpool.
    if ((err = thread_pool_init(&pool, 3)) != 0){
        return err;
    }

    if ((err = pthread_mutex_init(&mutex, NULL)) != 0){
        return err;
    }

    // Creating callable.
    callable.function = function;
    callable.arg = (void*)&base;
    callable.argsz = sizeof(long long int);

    async(&pool, &futures[0], callable);

    // Mapping partial result.
    for (int i = 0; i < n - 1; i++){
        map(&pool, &futures[i + 1], &futures[i], function);
    }

    // Waiting for result.
    long long int *result = (long long int*)await(&futures[n - 1]);
    printf("%lld\n", *result);

    // Destroying
    thread_pool_destroy(&pool);
    pthread_mutex_destroy(&mutex);
    return 0;
}
