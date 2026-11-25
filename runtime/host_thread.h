/* Portable host-thread creation and one-shot resume for the runtime. */
#pragma once
#include <stdint.h>
#include <stddef.h>
typedef void (*RedlineThreadProc)(void *argument);
typedef uintptr_t RedlineThreadToken;
/* Detached host thread. A suspended thread's token remains valid until resume. */
int redline_thread_create(RedlineThreadProc proc, void *argument, size_t stack_size,
                          int suspended, uint32_t *thread_id, RedlineThreadToken *token);
uint32_t redline_thread_resume(RedlineThreadToken token);
