
#include "threads.h"

#include <stdio.h>

//static int testThread(void* arg);


int main() {

	/*thrd_t thrd;

	thrd_create(&thrd, testThread, NULL);

	int result;
	thrd_join(thrd, &result);

	printf("Thread Result: %d\n", (DWORD)result);*/

	printf("Sleeping for 2500 ms\n");

	struct timespec remaining;
	struct timespec timepoint = { .tv_sec = 2, .tv_nsec = 500000000 };
	thrd_sleep(&timepoint, &remaining);

	printf("Remaining: seconds: %d | nanoseconds %d\n", remaining.tv_sec, remaining.tv_nsec);


    

    return 0;
}

//static int testThread(void *arg) {
//    (void)arg;
//
//    for (int i = 0; i < 10; ++i) {
//        puts("Doing work!");
//        Sleep(100);
//    }
//
//    return -1;
//}
