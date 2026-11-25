#include "host_sync.h"
#include <stdio.h>
int main(void) {
    volatile int32_t value = 0;
    if (redline_sync_load(&value) != 0) return 1;
    if (redline_sync_fetch_add(&value, 3) != 0 || redline_sync_load(&value) != 3) return 2;
    if (redline_sync_exchange(&value, 1) != 3 || redline_sync_load(&value) != 1) return 3;
    if (redline_sync_wait_i32(&value, 0, 0) != 1) return 4;
    if (!redline_sync_monotonic_ns() || !redline_sync_realtime_100ns() || !redline_sync_thread_id()) return 5;
    puts("host_sync Windows tests passed");
    return 0;
}
