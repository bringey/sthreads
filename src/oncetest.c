#include "threads.h"
#include <stdio.h>

static int counter = 0;

static once_flag onceTestFlag = ONCE_FLAG_INIT;

static void onceTest(void);

static void onceTestFunc(void);

static int onceTestThread(void *arg);

#define NUM_THREADS 100


int main(void) {

    onceTest();
    
    puts("Resetting flag and counter");
    
    onceTestFlag = ONCE_FLAG_INIT;
    counter = 0;
    
    onceTest();

    return 0;


}

static void onceTest() {
    thrd_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; ++i)
        thrd_create(threads + i, onceTestThread, NULL);

    for (int i = 0; i < NUM_THREADS; ++i)
        thrd_join(threads[i], NULL);

    printf("Counter: %d (expecting: 1)\n", counter);
}

static void onceTestFunc(void) {
    puts("This should only appear once");
    counter++;
}

static int onceTestThread(void *arg) {
    (void)arg;
    call_once(&onceTestFlag, onceTestFunc);

    return 0;
}