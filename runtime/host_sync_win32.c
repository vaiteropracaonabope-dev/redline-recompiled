#include <windows.h>
#include "host_sync.h"

int32_t redline_sync_load(const volatile int32_t *value) {
    return (int32_t)InterlockedCompareExchange((volatile LONG *)value, 0, 0);
}
void redline_sync_store(volatile int32_t *value, int32_t desired) {
    InterlockedExchange((volatile LONG *)value, (LONG)desired);
}
int32_t redline_sync_fetch_add(volatile int32_t *value, int32_t amount) {
    return (int32_t)InterlockedExchangeAdd((volatile LONG *)value, (LONG)amount);
}
int32_t redline_sync_exchange(volatile int32_t *value, int32_t desired) {
    return (int32_t)InterlockedExchange((volatile LONG *)value, (LONG)desired);
}
int redline_sync_wait_i32_ns(const volatile int32_t *value, int32_t expected, int64_t timeout_ns) {
    DWORD timeout;
    if (timeout_ns < 0) timeout = INFINITE;
    else if (timeout_ns == 0) timeout = 0;
    else {
        uint64_t milliseconds = ((uint64_t)timeout_ns + 999999ull) / 1000000ull;
        timeout = milliseconds >= (uint64_t)INFINITE ? INFINITE - 1 : (DWORD)milliseconds;
    }
    return WaitOnAddress((volatile VOID *)value, &expected, sizeof expected, timeout) ? 1 :
           GetLastError() == ERROR_TIMEOUT ? 0 : -1;
}
int redline_sync_wait_i32(const volatile int32_t *value, int32_t expected, int timeout_ms) {
    return redline_sync_wait_i32_ns(value, expected, timeout_ms < 0 ? -1 : (int64_t)timeout_ms * 1000000ll);
}
void redline_sync_wake_all(const volatile int32_t *value) { WakeByAddressAll((PVOID)value); }
uint64_t redline_sync_monotonic_ns(void) {
    LARGE_INTEGER now, frequency;
    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&now)) return 0;
    return (uint64_t)(now.QuadPart / frequency.QuadPart) * 1000000000ull +
           (uint64_t)(now.QuadPart % frequency.QuadPart) * 1000000000ull / (uint64_t)frequency.QuadPart;
}
uint64_t redline_sync_realtime_100ns(void) {
    FILETIME ft;
    ULARGE_INTEGER value;
    GetSystemTimeAsFileTime(&ft);
    value.LowPart = ft.dwLowDateTime;
    value.HighPart = ft.dwHighDateTime;
    return value.QuadPart - 116444736000000000ull;
}
void redline_sync_sleep_ms(uint32_t milliseconds) { Sleep((DWORD)milliseconds); }
void redline_sync_yield(void) { SwitchToThread(); }
uint64_t redline_sync_thread_id(void) { return (uint64_t)GetCurrentThreadId(); }
