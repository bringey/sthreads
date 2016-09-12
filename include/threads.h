// file: threads.h
//
// Simple implementation of C11's threads.h
// This implementation either wraps the pthreads library or the windows api
// depending on the platform the module is compiled for.
// For unix systems the __unix__ macro should be defined
// For Windows systems the _WIN32 macro should be defined
//

#ifndef _THREADS_H
#define _THREADS_H

#include <time.h>

#ifdef __unix__
#include <pthread.h>
#elif defined(_WIN32)
#include <windows.h>
#endif


// NORETURN macro defines the function attribute noreturn
#if defined(_MSC_VER)
#define NORETURN __declspec(noreturn)  // Visual Studio compiler
#else
#define NORETURN _Noreturn             // C11 standard
#endif

#if defined _MSC_VER
#define RESTRICT __declspec(restrict)
#else
#define RESTRICT restrict
#endif



// type definitions -----------------------------------------------------------

typedef int (*thrd_start_t)(void*);

typedef struct thrd_t_s {
    #ifdef __unix__
        pthread_t thread;
    #elif defined(_WIN32)
        HANDLE thread;
        DWORD threadId;
    #endif
} thrd_t;

enum {
    thrd_success,
    thrd_timedout,
    thrd_busy,
    thrd_nomem,
    thrd_error
};

//typedef struct mtx_t_s {
//
//} mtx_t;

#ifdef __unix__
    typedef pthread_mutex_t mtx_t;
#elif defined(_WIN32)
    typedef HANDLE mtx_t;
#endif

// 2 bit type specifier
// bit 0: 0 for plain, 1 for recursive
// bit 1: 1 for timed
enum {
    mtx_plain     = 0,
    mtx_recursive = 1,
    mtx_timed     = 2
};

#ifdef __unix__
    typedef pthread_once_t once_flag;
    #define ONCE_FLAG_INIT PTHREAD_ONCE_INIT
#elif defined _WIN32
    typedef LONG once_flag;
    #define ONCE_READY 1    // function has not been called
    #define ONCE_DONE 0     // function was called
    #define ONCE_FLAG_INIT ONCE_READY
#endif

#ifdef __unix__
    typedef pthread_cond_t cnd_t;
#elif defined(_WIN32)
    typedef CONDITION_VARIABLE cnd_t;
#endif

// thread local storage specifier
#ifdef _MSC_VER
#define thread_local __declspec(thread)
#else
#define thread_local _Thread_local
#endif

#ifdef __unix__
    typedef pthread_key_t tss_t;
#elif defined(_WIN32)
    typedef DWORD tss_t;
#endif

typedef void (*tss_dtor_t)(void*);

// thread functions -----------------------------------------------------------

int thrd_create(thrd_t*, thrd_start_t, void*);

int thrd_equal(thrd_t lhs, thrd_t rhs);

thrd_t thrd_current();

int thrd_sleep(const struct timespec* time_point, struct timespec* remaining);

void thrd_yield();

NORETURN void thrd_exit(int result);

int thrd_detach(thrd_t thr);

int thrd_join(thrd_t thr, int *res);

// mutex functions ------------------------------------------------------------

int mtx_init(mtx_t *mutex, int type);

int mtx_lock(mtx_t *mutex);

int mtx_timedlock(mtx_t *mutex, const struct timespec *time_point);

int mtx_trylock(mtx_t *mutex);

int mtx_unlock(mtx_t *mutex);

void mtx_destroy(mtx_t *mutex);

// call once ------------------------------------------------------------------

void call_once(once_flag *flag, void (*func)(void));

// condition variables --------------------------------------------------------

int cnd_init(cnd_t *cond);

int cnd_signal(cnd_t *cond);

int cnd_broadcast(cnd_t *cond);

int cnd_wait(cnd_t *cond, mtx_t *mutex);

int cnd_timedwait(cnd_t *cond, mtx_t *mutex, const struct timespec *time_point);

void cnd_destroy(cnd_t *cond);

// thread specific storage (tss) ----------------------------------------------

int tss_create(tss_t *tss_key, tss_dtor_t destructor);

void* tss_get(tss_t tss_key);

int tss_set(tss_t tss_key, void *val);

void tss_delete(tss_t tss_key);

#endif
