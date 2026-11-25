#define _POSIX_C_SOURCE 200809L
#include "host_thread.h"
#include "host_sync.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static volatile int32_t g_started;
static void worker(void *unused) {
    (void)unused;
    redline_thread_set_name("redline-test");
    redline_sync_store(&g_started, 1);
}
int main(void) {
    redline_thread_set_name("redline-main");
    redline_thread_set_priority(REDLINE_THREAD_PRIORITY_NORMAL);
    uint32_t id = 0;
    RedlineThreadToken token = 0;
    assert(redline_thread_create(worker, NULL, 65536, 1, &id, &token));
    assert(id != 0 && token != 0);
    redline_sync_sleep_ms(20);
    assert(redline_sync_load(&g_started) == 0);
    assert(redline_thread_resume(token) == 1);
    for (int i = 0; i < 200 && !redline_sync_load(&g_started); ++i)
        redline_sync_sleep_ms(1);
    assert(redline_sync_load(&g_started) == 1);
    puts("host thread tests passed");
    return 0;
}
