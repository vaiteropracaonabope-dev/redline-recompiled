#include "guest_handle_table.h"
#include <assert.h>
#include <stdio.h>

static void test_insert_lookup_and_type(void) {
    RedlineGuestHandleTable table;
    uint32_t handle = 0, object = 0;
    redline_guest_handles_init(&table);
    assert(redline_guest_handle_insert(&table, 0xB0001020u, 6, &handle));
    assert(handle && handle != 0xB0001020u && handle != REDLINE_GUEST_HANDLE_CURRENT_THREAD);
    assert(redline_guest_handle_lookup(&table, handle, 0, &object) && object == 0xB0001020u);
    assert(redline_guest_handle_lookup(&table, handle, 6, &object));
    assert(!redline_guest_handle_lookup(&table, handle, 5, &object));
    assert(!redline_guest_handle_lookup(&table, 0, 0, &object));
    assert(!redline_guest_handle_lookup(&table, REDLINE_GUEST_HANDLE_CURRENT_THREAD, 0, &object));
}

static void test_duplicate_and_close(void) {
    RedlineGuestHandleTable table;
    uint32_t first = 0, second = 0, object = 0;
    uint8_t type = 0;
    redline_guest_handles_init(&table);
    assert(redline_guest_handle_insert(&table, 0xB0002000u, 0xF0, &first));
    assert(redline_guest_handle_duplicate(&table, first, &second));
    assert(first != second);
    assert(redline_guest_handle_lookup(&table, second, 0xF0, &object) && object == 0xB0002000u);
    assert(redline_guest_handle_close(&table, first, &object, &type));
    assert(object == 0xB0002000u && type == 0xF0);
    assert(!redline_guest_handle_lookup(&table, first, 0, &object));
    assert(redline_guest_handle_lookup(&table, second, 0, &object));
    assert(!redline_guest_handle_close(&table, first, &object, &type));
    assert(redline_guest_handle_close(&table, second, &object, &type));
}

static void test_stale_generation_after_slot_reuse(void) {
    RedlineGuestHandleTable table;
    uint32_t stale = 0, current = 0, object = 0;
    uint8_t type = 0;
    redline_guest_handles_init(&table);
    assert(redline_guest_handle_insert(&table, 0xB0003000u, 2, &stale));
    assert(redline_guest_handle_close(&table, stale, &object, &type));
    assert(redline_guest_handle_insert(&table, 0xB0004000u, 2, &current));
    assert((stale & REDLINE_GUEST_HANDLE_SLOT_MASK) == (current & REDLINE_GUEST_HANDLE_SLOT_MASK));
    assert(stale != current);
    assert(!redline_guest_handle_lookup(&table, stale, 0, &object));
    assert(redline_guest_handle_lookup(&table, current, 0, &object) && object == 0xB0004000u);
}

static void test_special_namespace_is_reserved(void) {
    RedlineGuestHandleTable table;
    uint32_t handle = 0, object = 0;
    redline_guest_handles_init(&table);
    table.entries[0].generation = 0x4C000u;
    assert(redline_guest_handle_insert(&table, 0xB0005000u, 1, &handle));
    assert((handle & REDLINE_GUEST_HANDLE_SPECIAL_MASK) != REDLINE_GUEST_HANDLE_SYMLINK_TAG);
    assert(!redline_guest_handle_lookup(&table, REDLINE_GUEST_HANDLE_SYMLINK_TAG | 1u, 0, &object));
}

static void test_capacity_and_invalid_inputs(void) {
    static RedlineGuestHandleTable table;
    uint32_t handles[REDLINE_GUEST_HANDLE_CAPACITY], object = 0;
    uint8_t type = 0;
    redline_guest_handles_init(&table);
    assert(!redline_guest_handle_insert(&table, 0, 1, &handles[0]));
    assert(!redline_guest_handle_insert(&table, 1, 0, &handles[0]));
    assert(!redline_guest_handle_insert(&table, 1, 1, NULL));
    for (uint32_t i = 0; i < REDLINE_GUEST_HANDLE_CAPACITY; ++i)
        assert(redline_guest_handle_insert(&table, 0xB0010000u + i * 0x20u, 1, &handles[i]));
    uint32_t extra = 0;
    assert(!redline_guest_handle_insert(&table, 0xB0020000u, 1, &extra));
    assert(!redline_guest_handle_close(&table, UINT32_MAX, &object, &type));
    assert(!redline_guest_handle_close(&table, REDLINE_GUEST_HANDLE_CURRENT_THREAD, &object, &type));
}

int main(void) {
    test_insert_lookup_and_type();
    test_duplicate_and_close();
    test_stale_generation_after_slot_reuse();
    test_special_namespace_is_reserved();
    test_capacity_and_invalid_inputs();
    puts("generation-tagged guest handle table tests passed");
    return 0;
}
