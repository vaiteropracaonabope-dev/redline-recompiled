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
    assert(redline_mutant_acquire(&signal, &owner, 11));
    assert(signal == 0 && owner == 11);
    assert(!redline_mutant_acquire(&signal, &owner, 12));
    assert(redline_mutant_acquire(&signal, &owner, 11));
    assert(signal == -1 && owner == 11);
    assert(redline_mutant_release(&signal, &owner, 11, &previous));
    assert(previous == -1 && signal == 0 && owner == 11);
    assert(!redline_mutant_release(&signal, &owner, 12, &previous));
    assert(redline_mutant_release(&signal, &owner, 11, &previous));
    assert(previous == 0 && signal == 1 && owner == 0);
    assert(redline_mutant_acquire(&signal, &owner, 12));
    assert(signal == 0 && owner == 12);
}
int main(void) {
    test_semaphore();
    test_mutant_recursion();
    puts("dispatcher object tests passed");
    return 0;
}
