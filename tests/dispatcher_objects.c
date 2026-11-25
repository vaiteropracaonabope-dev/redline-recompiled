#include "dispatcher_objects.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void test_semaphore(void) {
    int32_t next = -1;
    assert(redline_semaphore_valid_create(0, 1));
    assert(redline_semaphore_valid_create(3, 3));
    assert(!redline_semaphore_valid_create(-1, 3));
    assert(!redline_semaphore_valid_create(4, 3));
    assert(!redline_semaphore_valid_create(0, 0));
    assert(redline_semaphore_release(1, 3, 2, &next) && next == 3);
    assert(!redline_semaphore_release(2, 3, 2, &next));
    assert(!redline_semaphore_release(1, 3, 0, &next));
    assert(!redline_semaphore_release(1, 3, -1, &next));
    assert(!redline_semaphore_release(INT32_MAX, INT32_MAX, INT32_MAX, &next));
}
static void test_mutant_recursion(void) {
    int32_t signal = 1, previous = 0;
    uint32_t owner = 0;
    uint8_t abandoned = 0;
    assert(redline_mutant_acquire(&signal, &owner, &abandoned, 11) == 1);
    assert(signal == 0 && owner == 11);
    assert(redline_mutant_acquire(&signal, &owner, &abandoned, 12) == 0);
    assert(redline_mutant_acquire(&signal, &owner, &abandoned, 11) == 1);
    assert(signal == -1 && owner == 11);
    assert(redline_mutant_release(&signal, &owner, 11, &previous));
    assert(previous == -1 && signal == 0 && owner == 11);
    assert(!redline_mutant_release(&signal, &owner, 12, &previous));
    assert(redline_mutant_release(&signal, &owner, 11, &previous));
    assert(previous == 0 && signal == 1 && owner == 0);
    assert(redline_mutant_acquire(&signal, &owner, &abandoned, 12) == 1);
    assert(signal == 0 && owner == 12);
}

static void test_mutant_abandonment(void) {
    int32_t signal = 1;
    uint32_t owner = 0;
    uint8_t abandoned = 0;
    assert(redline_mutant_acquire(&signal, &owner, &abandoned, 21) == 1);
    assert(redline_mutant_acquire(&signal, &owner, &abandoned, 21) == 1);
    assert(redline_mutant_abandon(&signal, &owner, &abandoned, 21));
    assert(signal == 1 && owner == 0 && abandoned == 1);
    assert(redline_mutant_acquire(&signal, &owner, &abandoned, 22) == 2);
    assert(signal == 0 && owner == 22 && abandoned == 0);
    assert(!redline_mutant_abandon(&signal, &owner, &abandoned, 21));
    assert(redline_mutant_release(&signal, &owner, 22, NULL));
    assert(signal == 1 && owner == 0);
}

static void test_object_references(void) {
    uint32_t refs = 1;
    assert(redline_object_reference(&refs) && refs == 2);
    assert(redline_object_reference(&refs) && refs == 3);
    assert(!redline_object_dereference(NULL));
    assert(!redline_object_dereference(&(uint32_t){0}));
    assert(!redline_object_dereference(&refs) && refs == 2);
    assert(!redline_object_dereference(&refs) && refs == 1);
    assert(redline_object_dereference(&refs) && refs == 0);
    assert(!redline_object_reference(&refs));
    refs = UINT32_MAX;
    assert(!redline_object_reference(&refs) && refs == UINT32_MAX);
}
\nint main(void) {
    test_semaphore();
    test_mutant_recursion();
    test_mutant_abandonment();\n    test_object_references();
    puts("dispatcher object tests passed");
    return 0;
}
