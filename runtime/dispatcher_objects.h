/* Pure guest dispatcher-object rules shared by the HLE and its unit tests. */
#pragma once
#include <stdint.h>

static inline int redline_semaphore_valid_create(int32_t initial_count, int32_t maximum_count) {
    return maximum_count > 0 && initial_count >= 0 && initial_count <= maximum_count;
}

/* Returns 1 on success, 0 if the release would be invalid or exceed the limit. */
static inline int redline_semaphore_release(int32_t current_count, int32_t maximum_count,
                                           int32_t release_count, int32_t *new_count) {
    if (!new_count || release_count <= 0 || current_count < 0 ||
        maximum_count <= 0 || current_count > maximum_count ||
        release_count > maximum_count - current_count) return 0;
    *new_count = current_count + release_count;
    return 1;
}

static inline int redline_mutant_acquire(int32_t *signal, uint32_t *owner, uint32_t thread) {
    if (!signal || !owner || !thread || (*signal <= 0 && *owner != thread)) return 0;
    --*signal; /* negative values represent recursive ownership depth */
    *owner = thread;
    return 1;
}

/* Returns 1 when the current thread owns the mutant and it was released. */
static inline int redline_mutant_release(int32_t *signal, uint32_t *owner, uint32_t thread,
                                         int32_t *previous_count) {
    if (!signal || !owner || !thread || *owner != thread) return 0;
    if (previous_count) *previous_count = *signal;
    if (++*signal > 0) *owner = 0;
    return 1;
}
