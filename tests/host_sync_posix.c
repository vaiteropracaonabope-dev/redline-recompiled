#include "host_sync.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

static volatile int32_t value;
static void *wake_thread(void *unused) {
    (void)unused;
    redline_sync_sleep_ms(10);
    redline_sync_store(&value, 7);
    redline_sync_wake_all(&value);
    return NULL;
}
int main(void) {
    pthread_t thread;
    if (redline_sync_load(&value) != 0) return 1;
    if (redline_sync_fetch_add(&value, 2) != 0 || redline_sync_load(&value) != 2) return 2;
    if (redline_sync_exchange(&value, 0) != 2 || redline_sync_load(&value) != 0) return 3;
    if (redline_sync_wait_i32(&value, 0, 1) != 0) return 4;
    uint64_t before = redline_sync_monotonic_ns();
    if (redline_sync_wait_i32_ns(&value, 0, 500000) != 0) return 9;
    uint64_t elapsed = redline_sync_monotonic_ns() - before;
    if (elapsed < 500000ull || elapsed > 100000000ull) return 10;
    if (pthread_create(&thread, NULL, wake_thread, NULL) != 0) return 5;
    if (redline_sync_wait_i32(&value, 0, 1000) != 1) return 6;
    pthread_join(thread, NULL);
    if (redline_sync_load(&value) != 7) return 7;
    if (!redline_sync_monotonic_ns() || !redline_sync_realtime_100ns() || !redline_sync_thread_id()) return 8;
    puts("host_sync POSIX tests passed");
    return 0;
}
