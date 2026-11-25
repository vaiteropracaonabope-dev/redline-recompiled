#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "host_thread.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    RedlineThreadProc proc;
    void *argument;
    int suspended;
} PosixThread;
typedef struct { PosixThread *thread; } StartInfo;
static volatile int32_t g_next_thread_id;

static void *thread_entry(void *opaque) {
    StartInfo *start = opaque;
    PosixThread *thread = start->thread;
    free(start);
    pthread_mutex_lock(&thread->mutex);
    while (thread->suspended) pthread_cond_wait(&thread->condition, &thread->mutex);
    RedlineThreadProc proc = thread->proc;
    void *argument = thread->argument;
    pthread_mutex_unlock(&thread->mutex);
    pthread_cond_destroy(&thread->condition);
    pthread_mutex_destroy(&thread->mutex);
    free(thread);
    proc(argument);
    return NULL;
}

int redline_thread_create(RedlineThreadProc proc, void *argument, size_t stack_size,
                          int suspended, uint32_t *thread_id, RedlineThreadToken *token) {
    if (!proc || !thread_id || !token) return 0;
    PosixThread *thread = calloc(1, sizeof *thread);
    StartInfo *start = malloc(sizeof *start);
    if (!thread || !start) { free(thread); free(start); return 0; }
    pthread_mutex_init(&thread->mutex, NULL);
    pthread_cond_init(&thread->condition, NULL);
    thread->proc = proc;
    thread->argument = argument;
    thread->suspended = suspended != 0;
    start->thread = thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (stack_size >= 16384u) pthread_attr_setstacksize(&attr, stack_size);
    int rc = pthread_create(&(pthread_t){0}, &attr, thread_entry, start);
    pthread_attr_destroy(&attr);
    if (rc != 0) {
        pthread_cond_destroy(&thread->condition);
        pthread_mutex_destroy(&thread->mutex);
        free(thread); free(start);
        return 0;
    }
    *thread_id = (uint32_t)__atomic_add_fetch(&g_next_thread_id, 1, __ATOMIC_RELAXED);
    *token = suspended ? (RedlineThreadToken)(uintptr_t)thread : 0;
    return 1;
}

uint32_t redline_thread_resume(RedlineThreadToken token) {
    PosixThread *thread = (PosixThread *)(uintptr_t)token;
    if (!thread) return 0;
    pthread_mutex_lock(&thread->mutex);
    uint32_t previous = thread->suspended ? 1u : 0u;
    thread->suspended = 0;
    pthread_cond_signal(&thread->condition);
    pthread_mutex_unlock(&thread->mutex);
    return previous;
}

#include <string.h>
#include <sched.h>
void redline_thread_set_name(const char *name) {
#if defined(__linux__) || defined(__ANDROID__)
    if (name) (void)pthread_setname_np(pthread_self(), name);
#else
    (void)name;
#endif
}
void redline_thread_set_priority(int priority) {
    if (priority != REDLINE_THREAD_PRIORITY_TIME_CRITICAL) return;
    /* SCHED_FIFO usually requires elevated privileges; treat this as best effort. */
    int minimum = sched_get_priority_min(SCHED_FIFO);
    if (minimum < 0) return;
    struct sched_param parameter;
    memset(&parameter, 0, sizeof parameter);
    parameter.sched_priority = minimum;
    (void)pthread_setschedparam(pthread_self(), SCHED_FIFO, &parameter);
}
