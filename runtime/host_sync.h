/* Platform-neutral synchronization and clock primitives for the runtime. */
#pragma once
#include <stdint.h>

int32_t redline_sync_load(const volatile int32_t *value);
void redline_sync_store(volatile int32_t *value, int32_t desired);
int32_t redline_sync_fetch_add(volatile int32_t *value, int32_t amount);
int32_t redline_sync_exchange(volatile int32_t *value, int32_t desired);

/* Wait while *value equals expected. timeout_ms < 0 means infinite.
 * Returns 1 when the value changed, 0 on timeout, and -1 on error. */
int redline_sync_wait_i32(const volatile int32_t *value, int32_t expected, int timeout_ms);
void redline_sync_wake_all(const volatile int32_t *value);

uint64_t redline_sync_monotonic_ns(void);
uint64_t redline_sync_realtime_100ns(void);
void redline_sync_sleep_ms(uint32_t milliseconds);
void redline_sync_yield(void);
uint64_t redline_sync_thread_id(void);
