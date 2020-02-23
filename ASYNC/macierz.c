/** @file
 * Example of use threadpool's defer.
 *
 * @author Piotr Jasinski <jasinskipiotr99@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "threadpool.h"

int *buffer;
pthread_mutex_t *locks;

void print_buffer(int n){
    for (int i = 0; i < n; i++)
        printf("%d\n", buffer[i]);
}

static void function(void *args, size_t argsz __attribute__((unused))){
    int *arr = (int*)args;
    int time = *arr;
    int row = *(arr+1);
    int value;
    usleep(time * 1000);
    value = *(arr+2);

    mutex_lock(&locks[row]);
    buffer[row] += value;
    mutex_unlock(&locks[row]);
    free(args);
}

int main() {
    int rows;
    int columns;
    thread_pool_t pool;
    runnable_t runnable;

    // Getting matrix size
    scanf("%d", &rows);
    scanf("%d", &columns);

    //printf("Test: %d %d\n", rows, columns);
    int matrix[rows][2 * columns + 1];

    // Getting matrix values and time needed to get value.
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 2 * columns-1; j += 2) {
            scanf("%d %d", &matrix[i][j], &matrix[i][j+1]);
        }
    }

    // Allocating memory for result.
    buffer = (int*)malloc(sizeof(int) * rows);
    locks = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * rows);
    if (buffer == NULL || locks == NULL){
        fprintf(stderr, "Alocating memory error\n");
        return -1;
    }
    for (int i = 0; i < rows; i++) {
        buffer[i] = 0;
        if (pthread_mutex_init(&locks[i], NULL) != 0){
            fprintf(stderr, "Mutex init error\n");
            return -1;
        }
    }

    // Initializing thread pool.
    if (thread_pool_init(&pool, 4) == -1){
        fprintf(stderr, "Thread pool initializing error\n");
        return -1;
    }
    // Deferring work
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < 2 * columns; j += 2){
            int *args = (int*)malloc(sizeof(int) * 3);
            if (args == NULL){
                thread_pool_destroy(&pool);
                return -1;
            }
            args[0] = matrix[i][j+1];
            args[1] = i;
            args[2] = matrix[i][j];
            runnable.function = function;
            runnable.arg = (void*)args;
            runnable.argsz = 3 * sizeof(int);
            defer(&pool, runnable);
        }
    }

    thread_pool_destroy(&pool);

    // Printing result.
    print_buffer(rows);
    free(buffer);
    free(locks);
    return 0;
}
