#include "threads.h"
#include <stdio.h>

static int counter = 0;

static once_flag onceTestFlag = ONCE_FLAG_INIT;

static void onceTest(void);

static int onceTestThread(void *arg);

#define NUM_THREADS 100


int main(void) {

    thrd_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i)
        thrd_create(threads + i, onceTestThread, NULL);

    for (int i = 0; i < NUM_THREADS; ++i)
        thrd_join(threads[i], NULL);

    printf("Counter: %d (expecting: 1)\n", counter);

    return 0;


}

static void onceTest(void) {
    puts("This should only appear once");
    counter++;
}

static int onceTestThread(void *arg) {
    call_once(&onceTestFlag, onceTest);

    return 0;
}