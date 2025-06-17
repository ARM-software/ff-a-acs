/*
 * Copyright (c) 2025, Arm Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "val.h"
#include "val_malloc.h"
#include "val_misc.h"

// Replace with your custom malloc interface
void *val_aligned_alloc(size_t align, size_t size);
void *val_malloc(size_t size);
void *val_calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
size_t val_free(void *ptr);

// Memory region for verifying over/underruns
typedef struct {
    size_t size;
    uint32_t guard_start;
    char data[0];  // flexible array member
} test_block;

#define GUARD_MAGIC 0xDEADBEEF
#define MAX_ALLOC 1000

static bool is_aligned(void *ptr, size_t align)
{
    return ((uintptr_t)ptr % align) == 0;
}

static void verify_guard(test_block *tb)
{
    assert(tb->guard_start == GUARD_MAGIC);
}

static void fill_pattern(void *p, size_t size, char val)
{
    val_memset(p, val, size);
}

static void test_malloc_free_basic(void)
{
    LOG(INFO, "Test: val_malloc and val_free");
    void *p = val_malloc(128);

    assert(p);
    fill_pattern(p, 128, 0xAB);
    val_free(p);
}

static void test_calloc_init(void)
{
    LOG(INFO, "Test: val_calloc zero init");
    int *p = val_calloc(16, sizeof(int));

    assert(p);
    for (int i = 0; i < 16; ++i) assert(p[i] == 0);
    val_free(p);
}

static void test_realloc(void)
{
    LOG(INFO, "Test: val_realloc grow/shrink");
    char *p = val_malloc(32);

    val_strcpy(p, "GrowShrink");
    p = val_realloc(p, 64);
    assert(val_strcmp(p, "GrowShrink") == 0);
    p = val_realloc(p, 16);
    assert(val_strcmp(p, "GrowShrink") == 0);
    val_free(p);
}

static void test_malloc_align(void)
{
    size_t sizes[] = {8, 16, 32, 64, 128, 4096};

    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); ++i) {
        void *p = val_aligned_alloc(sizes[i], sizes[i]*2);
        assert(p && is_aligned(p, sizes[i]));
        fill_pattern(p, sizes[i]*2, 0xCD);
        val_free(p);
    }

}

static void test_full_heap_cycle(void)
{
    LOG(INFO, "Test: heap exhaustion and reuse");
    void *blocks[MAX_ALLOC];
    size_t sz = 1024;
    int count = 0;
    while ((blocks[count] = val_malloc(sz)) && count < MAX_ALLOC) count++;
    for (int i = 0; i < count; ++i) val_free(blocks[i]);
}

static void test_coalescing(void)
{
    LOG(INFO, "Test: coalescing adjacent frees");
    void *a = val_malloc(256);
    void *b = val_malloc(256);
    void *c = val_malloc(256);
    val_free(b);
    val_free(a);
    void *d = val_malloc(512);
    assert(d == a);
    val_free(c);
    val_free(d);
}

static void test_edge_cases(void)
{
    LOG(INFO, "Test: edge conditions");
    assert(val_malloc(0) == NULL);
    assert(val_calloc(0, 0) == NULL);
    void *p = val_realloc(NULL, 64);
    assert(p);
    val_free(p);
    p = val_malloc(64);
    assert(val_realloc(p, 0) == NULL);
}

static void test_memory_isolation(void)
{
    LOG(INFO, "Test: allocation isolation");
    char *a = val_malloc(64);
    char *b = val_malloc(64);
    assert(a && b && a != b);
    val_memset(a, 'X', 64);
    val_memset(b, 'Y', 64);
    for (int i = 0; i < 64; ++i) {
        assert(a[i] == 'X');
        assert(b[i] == 'Y');
    }
    val_free(a);
    val_free(b);
}

static void test_valgrind_style_guards(void)
{
    LOG(INFO, "Test: valgrind-style overrun detection");
    size_t sz = 128;
    test_block *tb = val_malloc(sizeof(test_block) + sz + sizeof(uint32_t));
    assert(tb);
    tb->guard_start = GUARD_MAGIC;
    char *payload = tb->data;
    assert(((uintptr_t)(payload + sz)) % 4 == 0);
    uint32_t *guard_end = (uint32_t *)(uintptr_t)(payload + sz);
    *guard_end = GUARD_MAGIC;

    fill_pattern(payload, sz, 0xAB);
    verify_guard(tb);
    assert(*guard_end == GUARD_MAGIC);

    // Simulate user memory overrun
    // payload[128] = 0xFF; // Uncomment to test failure
    assert(*guard_end == GUARD_MAGIC);  // Should still be intact
    val_free(tb);
}

int malloc_test_main(void)
{

    test_malloc_free_basic();
    val_malloclib_visualize();
    test_calloc_init();
    test_realloc();
    test_malloc_align();
    test_full_heap_cycle();
    test_coalescing();
    test_edge_cases();
    test_memory_isolation();
    test_valgrind_style_guards();
    val_malloclib_visualize();

    LOG(INFO, "All tests passed.");
    return 0;
}
