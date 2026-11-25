#define _POSIX_C_SOURCE 200809L
#include "host_sync.h"

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>

#define WAIT_BUCKETS 64
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} WaitBucket;
static WaitBucket g_wait_buckets[WAIT_BUCKETS];
static pthread_once_t g_wait_once = PTHREAD_ONCE_INIT;

static void init_wait_buckets(void) {
    for (int i = 0; i < WAIT_BUCKETS; ++i) {
        pthread_mutex_init(&g_wait_buckets[i].mutex, NULL);
        pthread_cond_init(&g_wait_buckets[i].condition, NULL);
    }
}
static WaitBucket *bucket_for(const volatile int32_t *value) {
    uintptr_t key = (uintptr_t)value;
    return &g_wait_buckets[(key >> 2) % WAIT_BUCKETS];
}
int32_t redline_sync_load(const volatile int32_t *value) {
    return __atomic_load_n(value, __ATOMIC_SEQ_CST);
}
void redline_sync_store(volatile int32_t *value, int32_t desired) {
    __atomic_store_n(value, desired, __ATOMIC_SEQ_CST);
}
int32_t redline_sync_fetch_add(volatile int32_t *value, int32_t amount) {
    return __atomic_fetch_add(value, amount, __ATOMIC_SEQ_CST);
}
int32_t redline_sync_exchange(volatile int32_t *value, int32_t desired) {
    return __atomic_exchange_n(value, desired, __ATOMIC_SEQ_CST);
}
int redline_sync_wait_i32(const volatile int32_t *value, int32_t expected, int timeout_ms) {
    pthread_once(&g_wait_once, init_wait_buckets);
    WaitBucket *bucket = bucket_for(value);
    struct timespec deadline = {0, 0};
    if (timeout_ms >= 0) {
        clock_gettime(CLOCK_REALTIME, &deadline);
        deadline.tv_sec += timeout_ms / 1000;
        deadline.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
        if (deadline.tv_nsec >= 1000000000L) deadline.tv_sec++, deadline.tv_nsec -= 1000000000L;
    }
    pthread_mutex_lock(&bucket->mutex);
    int result = 1;
    while (redline_sync_load(value) == expected) {
        int rc = timeout_ms < 0 ? pthread_cond_wait(&bucket->condition, &bucket->mutex)
                                : pthread_cond_timedwait(&bucket->condition, &bucket->mutex, &deadline);
        if (rc == ETIMEDOUT) { result = 0; break; }
        if (rc != 0) { result = -1; break; }
    }
    pthread_mutex_unlock(&bucket->mutex);
    return result;
}
void redline_sync_wake_all(const volatile int32_t *value) {
    pthread_once(&g_wait_once, init_wait_buckets);
    WaitBucket *bucket = bucket_for(value);
    pthread_mutex_lock(&bucket->mutex);
    pthread_cond_broadcast(&bucket->condition);
    pthread_mutex_unlock(&bucket->mutex);
}
uint64_t redline_sync_monotonic_ns(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) return 0;
    return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec;
}
uint64_t redline_sync_realtime_100ns(void) {
    struct timespec t;
    if (clock_gettime(CLOCK_REALTIME, &t) != 0) return 0;
    return (uint64_t)t.tv_sec * 10000000ull + (uint64_t)t.tv_nsec / 100ull;
}
void redline_sync_sleep_ms(uint32_t milliseconds) {
    struct timespec req = {(time_t)(milliseconds / 1000), (long)(milliseconds % 1000) * 1000000L};
    while (nanosleep(&req, &req) != 0 && errno == EINTR) {}
}
void redline_sync_yield(void) { sched_yield(); }
uint64_t redline_sync_thread_id(void) {
    pthread_t self = pthread_self();
    uint64_t id = 0;
    __builtin_memcpy(&id, &self, sizeof self < sizeof id ? sizeof self : sizeof id);
    return id;
}
