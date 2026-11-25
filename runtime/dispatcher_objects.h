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

/* Acquire result: 0 = unavailable/invalid, 1 = acquired normally, 2 = acquired abandoned. */
static inline int redline_mutant_acquire(int32_t *signal, uint32_t *owner, uint8_t *abandoned,
                                        uint32_t thread) {
    if (!signal || !owner || !abandoned || !thread || (*signal <= 0 && *owner != thread)) return 0;
    int result = *abandoned ? 2 : 1;
    *abandoned = 0;
    --*signal; /* negative values represent recursive ownership depth */
    *owner = thread;
    return result;
}

static inline int redline_mutant_abandon(int32_t *signal, uint32_t *owner, uint8_t *abandoned,
                                         uint32_t exiting_thread) {
    if (!signal || !owner || !abandoned || !exiting_thread || *owner != exiting_thread) return 0;
    *owner = 0;
    *signal = 1;
    *abandoned = 1;
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

/* Raw guest handles are object addresses. Each duplicate/reference owns one count. */
static inline int redline_object_reference(uint32_t *references) {
    if (!references || !*references || *references == UINT32_MAX) return 0;
    ++*references;
    return 1;
}

/* Returns 1 when this release drops the last reference. */
static inline int redline_object_dereference(uint32_t *references) {
    if (!references || !*references) return 0;
    return --*references == 0;
}

/* Remove a released object from a compact array registry without leaving stale slots. */
static inline int redline_object_registry_remove(uint32_t *objects, uint32_t *count,
                                                 uint32_t capacity, uint32_t object) {
    if (!objects || !count || !object || *count > capacity) return 0;
    for (uint32_t i = 0; i < *count; ++i) {
        if (objects[i] != object) continue;
        objects[i] = objects[*count - 1];
        objects[*count - 1] = 0;
        --*count;
        return 1;
    }
    return 0;
}
