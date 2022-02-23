/* Thread */

#ifndef THREAD_H
#define THREAD_H

#include "xjdb_platform.h"

#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
#include <pthread.h>
typedef pthread_t xjdb_thread_t;
typedef pthread_mutex_t xjdb_mutex_t;
#elif defined(PLATFORM_WINDOWS)
#include <windows.h>
typedef HANDLE xjdb_thread_t;
typedef HANDLE xjdb_mutex_t;
#endif

typedef void *(*xjdb_thread_routine_t)(void *data);

int thread_init(xjdb_thread_t *thd, xjdb_thread_routine_t routine, void *data);
int thread_cancel(xjdb_thread_t *thd);
int thread_join(xjdb_thread_t *thd, void **retval);

int mutex_init(xjdb_mutex_t *mtx);
int mutex_uninit(xjdb_mutex_t *mtx);
int mutex_lock(xjdb_mutex_t *mtx);
int mutex_trylock(xjdb_mutex_t *mtx);
int mutex_unlock(xjdb_mutex_t *mtx);

#endif

