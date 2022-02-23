/* Thread */

#include <stdlib.h>
#include "thread.h"
#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
#include <errno.h>
#endif

#if defined(PLATFORM_WINDOWS)
struct ntthread_bridge_stub
{
	void *(*proto_callback)(void *);
	void *proto_data;
};

typedef struct ntthread_bridge_stub ntthread_bridge_stub_t;

static ntthread_bridge_stub_t *ntthread_bridge_stub_new(\
	void *(*proto_callback)(void *), void *proto_data)
{
	ntthread_bridge_stub_t *new_stub = (ntthread_bridge_stub_t *)malloc(\
		sizeof(ntthread_bridge_stub_t));
	if (new_stub == NULL) return NULL;
	new_stub->proto_callback = proto_callback;
	new_stub->proto_data = proto_data;
	return new_stub;
}

static void ntthread_bridge_stub_destroy(ntthread_bridge_stub_t *stub)
{
	free(stub);
}

static DWORD ntthread_bridge_stub_start_routine(void *bridge_data)
{
	ntthread_bridge_stub_t *stub = bridge_data;

	stub->proto_callback(stub->proto_data);

	ntthread_bridge_stub_destroy(stub);

	return 0;
}

#endif

int thread_init(xjdb_thread_t *thd, xjdb_thread_routine_t routine, void *data)
{
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0) { return -1; }
    if (pthread_create(thd, &attr, routine, data) != 0) { return -1; }
    if (pthread_attr_destroy(&attr) != 0) { return -1; }
    return 0;
#elif defined(PLATFORM_WINDOWS)
	ntthread_bridge_stub_t *new_stub = NULL;
	HANDLE new_thread;
	if ((new_stub = ntthread_bridge_stub_new(routine, data)) == NULL)
	{ return -1; }
	new_thread = CreateThread(NULL, 0, \
            (LPTHREAD_START_ROUTINE)ntthread_bridge_stub_start_routine, new_stub, 0, NULL);
	if (new_thread == INVALID_HANDLE_VALUE)
	{
		ntthread_bridge_stub_destroy(new_stub);
		return -1;
	}
	*thd = new_thread;
	return 0;
#endif
}

int thread_cancel(xjdb_thread_t *thd)
{
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    return pthread_cancel(*thd);
#elif defined(PLATFORM_WINDOWS)
    (void)thd;
    return 0;
#endif
}

int thread_join(xjdb_thread_t *thd, void **retval)
{
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    return pthread_join(*thd, retval);
#elif defined(PLATFORM_WINDOWS)
	WaitForSingleObject(*thd, INFINITE);
    if (retval != NULL) *retval = 0;
	return 0;
#endif
}

int mutex_init(xjdb_mutex_t *mtx)
{
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    int ret = 0;
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    ret = pthread_mutex_init(mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    return ret;
#elif defined(PLATFORM_WINDOWS)
	*mtx = CreateMutex(0, FALSE, NULL);
	return 0;
#endif
}

int mutex_uninit(xjdb_mutex_t *mtx)
{
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    return pthread_mutex_destroy(mtx);
#elif defined(PLATFORM_WINDOWS)
	CloseHandle(*mtx);
    return 0;
#endif
}

int mutex_lock(xjdb_mutex_t *mtx)
{
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    return pthread_mutex_lock(mtx);
#elif defined(PLATFORM_WINDOWS)
    WaitForSingleObject(*mtx, INFINITE);
    return 0;
#endif
}

int mutex_trylock(xjdb_mutex_t *mtx)
{
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    if (pthread_mutex_trylock(mtx) == EBUSY)
    { return 1; }
    return 0;
#elif defined(PLATFORM_WINDOWS)
	if (WaitForSingleObject(*mtx, 1) == WAIT_TIMEOUT)
	{ return 0; }
	return 1;
#endif
}

int mutex_unlock(xjdb_mutex_t *mtx)
{
#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) || defined(PLATFORM_MACOS)
    return pthread_mutex_unlock(mtx);
#elif defined(PLATFORM_WINDOWS)
	ReleaseMutex(*mtx);
	return 0;
#endif
}

