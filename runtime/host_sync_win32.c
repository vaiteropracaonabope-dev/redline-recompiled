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
int redline_sync_wait_i32(const volatile int32_t *value, int32_t expected, int timeout_ms) {
    DWORD timeout = timeout_ms < 0 ? INFINITE : (DWORD)timeout_ms;
    return WaitOnAddress((volatile VOID *)value, &expected, sizeof expected, timeout) ? 1 :
           GetLastError() == ERROR_TIMEOUT ? 0 : -1;
}
void redline_sync_wake_all(const volatile int32_t *value) { WakeByAddressAll((PVOID)value); }
uint64_t redline_sync_monotonic_ns(void) {
    LARGE_INTEGER now, frequency;
    if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&now)) return 0;
    return (uint64_t)((now.QuadPart * 1000000000ull) / (uint64_t)frequency.QuadPart);
}
uint64_t redline_sync_realtime_100ns(void) {
    FILETIME ft;
    ULARGE_INTEGER value;
    GetSystemTimeAsFileTime(&ft);
    value.LowPart = ft.dwLowDateTime;
    value.HighPart = ft.dwHighDateTime;
    return value.QuadPart;
}
void redline_sync_sleep_ms(uint32_t milliseconds) { Sleep((DWORD)milliseconds); }
void redline_sync_yield(void) { SwitchToThread(); }
uint64_t redline_sync_thread_id(void) { return (uint64_t)GetCurrentThreadId(); }
