/* Portable host-thread creation, metadata, and one-shot resume. */
#pragma once
#include <stdint.h>
#include <stddef.h>
typedef void (*RedlineThreadProc)(void *argument);
typedef uintptr_t RedlineThreadToken;
enum { REDLINE_THREAD_PRIORITY_NORMAL = 0, REDLINE_THREAD_PRIORITY_TIME_CRITICAL = 1 };
int redline_thread_create(RedlineThreadProc proc, void *argument, size_t stack_size,
                          int suspended, uint32_t *thread_id, RedlineThreadToken *token);
uint32_t redline_thread_resume(RedlineThreadToken token);
/* Best effort: unsupported priority changes are ignored by the backend. */
void redline_thread_set_name(const char *name);
void redline_thread_set_priority(int priority);
