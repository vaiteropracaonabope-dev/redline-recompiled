/* Generation-tagged guest handle table primitives.
 *
 * This is deliberately not wired into the Xbox HLE yet: the Xbox ABI boundary must
 * be audited before changing values returned by Nt*/Ps* exports. A handle owns one
 * object reference; callers perform object ref/deref around insert/duplicate/close.
 */
#pragma once
#include <stdint.h>

#define REDLINE_GUEST_HANDLE_SLOT_BITS 10u
#define REDLINE_GUEST_HANDLE_SLOT_MASK ((1u << REDLINE_GUEST_HANDLE_SLOT_BITS) - 1u)
#define REDLINE_GUEST_HANDLE_CAPACITY REDLINE_GUEST_HANDLE_SLOT_MASK
#define REDLINE_GUEST_HANDLE_MAX_GENERATION (UINT32_MAX >> REDLINE_GUEST_HANDLE_SLOT_BITS)
#define REDLINE_GUEST_HANDLE_CURRENT_THREAD 0xFFFFFFFEu

typedef struct {
    uint32_t object;
    uint32_t generation;
    uint8_t type;
    uint8_t live;
    uint8_t retired;
} RedlineGuestHandleEntry;

typedef struct {
    RedlineGuestHandleEntry entries[REDLINE_GUEST_HANDLE_CAPACITY];
} RedlineGuestHandleTable;

static inline void redline_guest_handles_init(RedlineGuestHandleTable *table) {
    if (!table) return;
    for (uint32_t i = 0; i < REDLINE_GUEST_HANDLE_CAPACITY; ++i) {
        table->entries[i].object = 0;
        table->entries[i].generation = 1;
        table->entries[i].type = 0;
        table->entries[i].live = 0;
        table->entries[i].retired = 0;
    }
}

static inline int redline_guest_handle_insert(RedlineGuestHandleTable *table, uint32_t object,
                                               uint8_t type, uint32_t *handle) {
    if (!table || !object || !type || !handle) return 0;
    for (uint32_t i = 0; i < REDLINE_GUEST_HANDLE_CAPACITY; ++i) {
        RedlineGuestHandleEntry *entry = &table->entries[i];
        if (entry->live || entry->retired) continue;
        uint32_t slot = i + 1u;
        if (entry->generation > REDLINE_GUEST_HANDLE_MAX_GENERATION) {
            entry->retired = 1;
            continue;
        }
        uint32_t value = (entry->generation << REDLINE_GUEST_HANDLE_SLOT_BITS) | slot;
        /* Keep the Xbox pseudo current-thread handle and INVALID_HANDLE_VALUE reserved. */
        if (value == REDLINE_GUEST_HANDLE_CURRENT_THREAD || value == UINT32_MAX) {
            entry->retired = 1;
            continue;
        }
        entry->object = object;
        entry->type = type;
        entry->live = 1;
        *handle = value;
        return 1;
    }
    return 0;
}

static inline int redline_guest_handle_lookup(const RedlineGuestHandleTable *table, uint32_t handle,
                                               uint8_t expected_type, uint32_t *object) {
    if (!table || !handle || handle == REDLINE_GUEST_HANDLE_CURRENT_THREAD ||
        handle == UINT32_MAX || !object) return 0;
    uint32_t slot = handle & REDLINE_GUEST_HANDLE_SLOT_MASK;
    uint32_t generation = handle >> REDLINE_GUEST_HANDLE_SLOT_BITS;
    if (!slot || slot > REDLINE_GUEST_HANDLE_CAPACITY || !generation) return 0;
    const RedlineGuestHandleEntry *entry = &table->entries[slot - 1u];
    if (!entry->live || entry->generation != generation ||
        (expected_type && entry->type != expected_type)) return 0;
    *object = entry->object;
    return 1;
}

/* Returns the object and type associated with this handle; does not dereference the object. */
static inline int redline_guest_handle_close(RedlineGuestHandleTable *table, uint32_t handle,
                                              uint32_t *object, uint8_t *type) {
    if (!table || !object || !type || !handle ||
        handle == REDLINE_GUEST_HANDLE_CURRENT_THREAD || handle == UINT32_MAX) return 0;
    uint32_t slot = handle & REDLINE_GUEST_HANDLE_SLOT_MASK;
    uint32_t generation = handle >> REDLINE_GUEST_HANDLE_SLOT_BITS;
    if (!slot || slot > REDLINE_GUEST_HANDLE_CAPACITY || !generation) return 0;
    RedlineGuestHandleEntry *entry = &table->entries[slot - 1u];
    if (!entry->live || entry->generation != generation) return 0;
    *object = entry->object;
    *type = entry->type;
    entry->object = 0;
    entry->type = 0;
    entry->live = 0;
    if (entry->generation == REDLINE_GUEST_HANDLE_MAX_GENERATION)
        entry->retired = 1;
    else
        ++entry->generation;
    return 1;
}

/* Creates a second handle to the same object. The caller must acquire its object reference first. */
static inline int redline_guest_handle_duplicate(RedlineGuestHandleTable *table, uint32_t source,
                                                  uint32_t *target) {
    uint32_t object = 0;
    uint8_t type = 0;
    if (!table || !target || !redline_guest_handle_lookup(table, source, 0, &object)) return 0;
    uint32_t slot = source & REDLINE_GUEST_HANDLE_SLOT_MASK;
    type = table->entries[slot - 1u].type;
    return redline_guest_handle_insert(table, object, type, target);
}
