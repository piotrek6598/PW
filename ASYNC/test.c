#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "future.h"
#include "threadpool.h"

int multiply_by;
static thread_pool_t pool;

static void *function(void *arg, size_t args, size_t *argsz){
    int n = *(int*)arg;
    int *result = malloc(sizeof(int));
    *result = n * multiply_by;
    multiply_by++;
    if (multiply_by == 2)
        sleep(3);
    else
        sleep(5);
    printf("Finished work, result is: %d\n", *result);
    return result;
}

static void *function2(void *arg, size_t args, size_t *argsz){
    sleep(6);
    printf("Work finished\n");
}

int main(){
    int err;
    int base = 10;
    multiply_by = 1;
    future_t *futures = (future_t*)malloc(sizeof(future_t) * 6);
    callable_t callable;
    callable.function = function;
    callable.arg = (void*)&base;
    callable.argsz = sizeof(int);
    err = async(&pool, &futures[0], callable);
    printf("%d\n", err);
    err = thread_pool_init(&pool, 3);
    printf("%d\n", err);
    err = async(&pool, &futures[0], callable);
    for (int i = 1; i < 6; i++){
        err = map(&pool, &futures[i], &futures[0], function);
        printf("%d\n", err);
    }
    int *result = (int*)await(&futures[0]);
    printf("Future[0]: %d\n", *result);
    for (int i = 1; i < 6; i++){
        result = (int*)await(&futures[i]);
        printf("Future[%d]: %d\n", i, *result);
    }
    free(futures);
    result = await(&futures[0]);
    if (result == NULL){
        printf("OK\n");
    }
    else {
        printf("ZLE\n");
    }
    future_t next;
    callable.function = function2;
    async(&pool, &next, callable);
    sleep(1);
    raise(SIGINT);
    return 0;
}
