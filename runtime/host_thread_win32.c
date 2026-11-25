#include <windows.h>
#include <stdlib.h>
#include "host_thread.h"
typedef struct { HANDLE handle; } WinThread;
typedef struct { RedlineThreadProc proc; void *argument; } StartInfo;
static DWORD WINAPI thread_entry(void *opaque) {
    StartInfo *start = opaque;
    RedlineThreadProc proc = start->proc;
    void *argument = start->argument;
    free(start);
    proc(argument);
    return 0;
}
int redline_thread_create(RedlineThreadProc proc, void *argument, size_t stack_size,
                          int suspended, uint32_t *thread_id, RedlineThreadToken *token) {
    if (!proc || !thread_id || !token) return 0;
    StartInfo *start = malloc(sizeof *start);
    WinThread *thread = suspended ? malloc(sizeof *thread) : NULL;
    if (!start || (suspended && !thread)) { free(start); free(thread); return 0; }
    start->proc = proc;
    start->argument = argument;
    DWORD id = 0;
    HANDLE handle = CreateThread(NULL, stack_size, thread_entry, start,
                                 suspended ? CREATE_SUSPENDED : 0, &id);
    if (!handle) { free(start); free(thread); return 0; }
    *thread_id = (uint32_t)id;
    if (suspended) { thread->handle = handle; *token = (RedlineThreadToken)(uintptr_t)thread; }
    else { CloseHandle(handle); *token = 0; }
    return 1;
}
uint32_t redline_thread_resume(RedlineThreadToken token) {
    WinThread *thread = (WinThread *)(uintptr_t)token;
    if (!thread) return 0;
    DWORD previous = ResumeThread(thread->handle);
    CloseHandle(thread->handle);
    free(thread);
    return previous == (DWORD)-1 ? 0 : (uint32_t)previous;
}

void redline_thread_set_name(const char *name) {
    typedef HRESULT (WINAPI *SetDescriptionFn)(HANDLE, PCWSTR);
    static SetDescriptionFn set_description;
    static int initialized;
    if (!initialized) {
        set_description = (SetDescriptionFn)(void *)GetProcAddress(
            GetModuleHandleA("kernel32.dll"), "SetThreadDescription");
        initialized = 1;
    }
    if (set_description && name) {
        wchar_t wide_name[64];
        if (MultiByteToWideChar(CP_UTF8, 0, name, -1, wide_name, 64))
            (void)set_description(GetCurrentThread(), wide_name);
    }
}
void redline_thread_set_priority(int priority) {
    if (priority == REDLINE_THREAD_PRIORITY_TIME_CRITICAL)
        (void)SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
}
