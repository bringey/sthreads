#include "threads.h"

#include <errno.h>

#include <stdio.h>
#include <stdint.h>

#if   defined(__unix__)
    #include <pthread.h>
    #include <sched.h>
    #include <unistd.h>
#elif defined(_WIN32)
    #include <windows.h>

    // encapsulates a function call to a thrd_start_t
    // used by ThreadProcWrapper
    typedef struct thrd_start_call_s {
        thrd_start_t func;
        void *arg;
        HANDLE wrapperSemaphore;
    } thrd_start_call_t;


    // Since WINAPI's thread start function type has a different signature,
    // A wrapper function is used, which will invoke the start function
    // stored in a thrd_start_call_t pointer
    static DWORD WINAPI ThreadProcWrapper(LPVOID lpParam) {
        thrd_start_call_t *call = (thrd_start_call_t*)lpParam;

        thrd_start_t func = call->func;
        void *arg = call->arg;

        // finished accessing call, notify thrd_create
        ReleaseSemaphore(call->wrapperSemaphore, 1, NULL);


        return func(arg); // invoke the real thread start function
    }

    // converts a pointer of type struct timespec to milleseconds of type DWORD
    #define timespecToMillis(ts) ( ((DWORD)ts->tv_sec * 1000) + ((DWORD)ts->tv_nsec / 1000000) )

#else
    // neither unix nor windows, implementation is undefined
    #error Platform is not compatible with this module
#endif


int thrd_create(thrd_t *thrd, thrd_start_t start, void *arg) {
    int result = thrd_success;

    #ifdef __unix__
        int error = pthread_create(&thrd->thread, NULL, (void*(*)(void*))start, arg);
        if (error)
            result = (error == EAGAIN) ? thrd_nomem : thrd_error;

    #elif defined(_WIN32)

        //thrd->startFunc = start;
        //thrd->arg = arg;

        //thrd_start_call_t *call = (thrd_start_call_t*)malloc(sizeof(thrd_start_call_t));
        thrd_start_call_t call;
        call.func = start;
        call.arg = arg;
        call.wrapperSemaphore = CreateSemaphore(NULL, 0, 1, NULL);

        thrd->thread = CreateThread(NULL, 0, ThreadProcWrapper, (LPVOID)(&call),
                                    0, &thrd->threadId);

        if (thrd->thread == NULL) {
            // todo: check GetLastError to determine if out of memory, etc
            result = thrd_error;
        } else {
            // thrd_create cannot return until the wrapper has finished accessing call (call exists on the stack)
            // so block until any one of these happen:
            // 1. ThreadProcWrapper has accessed call, and sets the semaphore count from 0 -> 1
            // 2. the newly spawned thread has exited before #1 occurs (should not happen)
            HANDLE handles[2];
            handles[0] = call.wrapperSemaphore;
            handles[1] = thrd->thread;
            WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        }

        CloseHandle(call.wrapperSemaphore);

    #endif

    return result;
}

int thrd_equal(thrd_t lhs, thrd_t rhs) {
    #ifdef __unix__
        return pthread_equal(lhs.thread, rhs.thread);
    #elif defined(_WIN32)
        // threads are equal if they both have the same id
        return lhs.threadId == rhs.threadId;
    #endif
}

thrd_t thrd_current() {
    thrd_t result;
    #ifdef __unix__
        result.thread = pthread_self();
    #elif defined(_WIN32)
        result.thread = NULL;
        result.threadId = GetCurrentThreadId();
    #endif

    return result;
}

int thrd_sleep(const struct timespec *timepoint, struct timespec *remaining) {
    int result = 0;

    #ifdef __unix__
        result = nanosleep(timepoint, remaining);
    #elif defined(_WIN32)

        DWORD millis = timespecToMillis(timepoint);

        if (remaining == NULL) {
            Sleep(millis);
        } else {
            LARGE_INTEGER start, end, freq;
            QueryPerformanceFrequency(&freq);
            QueryPerformanceCounter(&start);
            Sleep(millis);
            QueryPerformanceCounter(&end);


            LONGLONG diff = end.QuadPart - start.QuadPart;

            printf("Diff: %lld | freq: %lld\n", diff, freq.QuadPart);

            if (diff >= 0) {
                time_t secs = diff / freq.QuadPart;
                remaining->tv_sec = timepoint->tv_sec - secs;
                remaining->tv_nsec = timepoint->tv_nsec - (((diff - (freq.QuadPart * secs)) * 1000000000L) / freq.QuadPart);
            } else {
                remaining->tv_sec = 0;
                remaining->tv_nsec = 0;
            }

        }

    #endif

    return result;
}

void thrd_yield() {
    #ifdef __unix__
        sched_yield();
    #elif defined(_WIN32)
        SwitchToThread();
    #endif
}

NORETURN void thrd_exit(int result) {
    // destroy all tss
    #ifdef __unix__
        pthread_exit((void*)(intptr_t)result);
    #elif defined(_WIN32)
        ExitThread(result);
    #endif
}

int thrd_detach(thrd_t thr) {
    int result = thrd_success;

    #ifdef __unix__
        // nonzero on error
        if (pthread_detach(thr.thread))
            result = thrd_error;
    #elif defined(_WIN32)
        // zero on error
        if (!CloseHandle(thr.thread))
            result = thrd_error;
    #endif

    return result;
}

int thrd_join(thrd_t thr, int *res) {
    int result = thrd_success;

    #ifdef __unix__
        void *threadReturn;
        if (pthread_join(thr.thread, &threadReturn)) {
            result = thrd_error;
        } else {
            if (res != NULL)
                *res = (intptr_t)threadReturn;
        }
    #elif defined(_WIN32)
        if (WaitForSingleObject(thr.thread, INFINITE) == WAIT_OBJECT_0) {
            if (res != NULL) {
                DWORD exitCode;
                if (GetExitCodeThread(thr.thread, &exitCode)) {
                    *res = exitCode;
                }
            }
            CloseHandle(thr.thread);
        } else {
            result = thrd_error;
        }
    #endif

    return result;
}

// mutex

int mtx_init(mtx_t *mutex, int type) {
    int result = thrd_success;

    #ifdef __unix__

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);

        int ptype = ((type & mtx_recursive) == mtx_recursive) ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL;
        pthread_mutexattr_settype(&attr, ptype);

        if (pthread_mutex_init(mutex, &attr))
            result = thrd_error;

        pthread_mutexattr_destroy(&attr);

    #elif defined(_WIN32)
        (void)type; // type not needed since winapi mutexes are recursive and support timeouts
        HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
        if (hMutex == NULL)
            result = thrd_error;
        else
            *mutex = hMutex;


    #endif

    return result;
}

int mtx_lock(mtx_t *mutex) {
    int result = thrd_success;

    #ifdef __unix__

        if (pthread_mutex_lock(mutex))
            result = thrd_error;

    #elif defined(_WIN32)
        // same behavior for plain and recursive mutexes
        if (WaitForSingleObject(*mutex, INFINITE) != WAIT_OBJECT_0)
            result = thrd_error;
    #endif

    return result;
}

int mtx_timedlock(mtx_t *mutex, const struct timespec *time_point) {
    int result = thrd_success;

    #ifdef __unix__

        int error = pthread_mutex_timedlock(mutex, time_point);
        if (error)
            result = (error == ETIMEDOUT) ? thrd_timedout : thrd_error;

    #elif defined(_WIN32)
        //DWORD ms = (time_point->tv_sec * 1000) + (time_point->tv_nsec / 10000000L);
		DWORD ms = timespecToMillis(time_point);
        DWORD waitResult = WaitForSingleObject(*mutex, ms);
        if (waitResult != WAIT_OBJECT_0)
            result = (waitResult == WAIT_TIMEOUT) ? thrd_timedout : thrd_error;

    #endif

    return result;
}

int mtx_trylock(mtx_t *mutex) {
    int result = thrd_success;

    #ifdef __unix__

        int error = pthread_mutex_trylock(mutex);
        if (error)
            result = (error == EBUSY) ? thrd_busy : thrd_error;

    #elif defined(_WIN32)

        DWORD waitResult = WaitForSingleObject(*mutex, 0);
        if (waitResult != WAIT_OBJECT_0)
            result = (waitResult == WAIT_TIMEOUT) ? thrd_busy : thrd_error;

    #endif

    return result;
}

int mtx_unlock(mtx_t *mutex) {
    int result = thrd_success;

    #ifdef __unix__
        if (pthread_mutex_unlock(mutex))
            result = thrd_error;
    #elif defined(_WIN32)
        if (!ReleaseMutex(*mutex))
            result = thrd_error;
    #endif

    return result;
}

void mtx_destroy(mtx_t *mutex) {

    #ifdef __unix__
        pthread_mutex_destroy(mutex);
    #elif defined(_WIN32)
        CloseHandle(*mutex);
    #endif

}

// call once ------------------------------------------------------------------


void call_once(once_flag *flag, void(*func)(void)) {

    #ifdef __unix__
        pthread_once(flag, func);
    #elif defined _WIN32

        // whoever gets ONCE_READY first gets to call the function
        if (InterlockedExchange(flag, ONCE_DONE) == ONCE_READY)
            func();
        

    #endif

}

// condition variables --------------------------------------------------------

int cnd_init(cnd_t *cond) {
    (void)cond;
    return 0;
}

int cnd_signal(cnd_t *cond) {
    (void)cond;
    return 0;
}

int cnd_broadcast(cnd_t *cond) {
    (void)cond;
    return 0;
}

int cnd_wait(cnd_t *cond, mtx_t *mutex) {
    (void)cond; (void)mutex;
    return 0;
}

int cnd_timedwait(cnd_t *cond, mtx_t *mutex, const struct timespec *time_point) {
    (void)cond; (void)mutex; (void)time_point;
    return 0;
}

void cnd_destroy(cnd_t *cond) {
    (void)cond;
}

// thread specific storage ----------------------------------------------------

int tss_create(tss_t *tss_key, tss_dtor_t destructor) {
    (void)tss_key; (void)destructor;
    return 0;
}

void* tss_get(tss_t tss_key) {
    (void)tss_key;
    return NULL;
}

int tss_set(tss_t tss_key, void *val) {
    (void)tss_key; (void)val;
    return 0;
}

void tss_delete(tss_t tss_key) {
    (void)tss_key;
}

