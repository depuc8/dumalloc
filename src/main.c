#include <stdio.h>
#include <string.h>
#include "heap.h"

static int passed = 0;
static int failed = 0;

static void check(const char *name, int cond)
{
    if (cond) {
        printf("  PASS  %s\n", name);
        passed++;
    } else {
        printf("  FAIL  %s\n", name);
        failed++;
    }
}

static void test_malloc_zero(void)
{
    void *p = my_malloc(0);
    check("malloc(0) returns NULL", p == NULL);
}

static void test_malloc_basic(void)
{
    void *p = my_malloc(32);
    check("malloc(32) returns non-NULL", p != NULL);
    my_free(p);
}

static void test_alignment(void)
{
    void *p = my_malloc(1);
    check("payload is aligned", p != NULL && ((size_t)p % ALIGNMENT) == 0);
    my_free(p);
}

static void test_write_read(void)
{
    const char msg[] = "dumalloc works";
    char *p = my_malloc(sizeof(msg));
    if (!p) { check("write/read roundtrip", 0); return; }
    memcpy(p, msg, sizeof(msg));
    check("write/read roundtrip", memcmp(p, msg, sizeof(msg)) == 0);
    my_free(p);
}

static void test_multiple_allocs(void)
{
    void *a = my_malloc(16);
    void *b = my_malloc(16);
    void *c = my_malloc(16);
    check("multiple allocs return distinct pointers",
          a != NULL && b != NULL && c != NULL && a != b && b != c);
    my_free(a);
    my_free(b);
    my_free(c);
}

static void test_reuse_after_free(void)
{
    void *a = my_malloc(128);
    my_free(a);
    void *b = my_malloc(128);
    check("freed block is reused", b != NULL);
    my_free(b);
}

static void test_free_null(void)
{
    my_free(NULL);
    check("free(NULL) is a no-op", 1);
}

static void test_heap_exhaustion(void)
{
    while (my_malloc(ALIGNMENT))
        ;
    void *p = my_malloc(ALIGNMENT);
    check("malloc returns NULL when heap is full", p == NULL);
}

int main(void)
{
    printf("dumalloc test suite\n");
    printf("-------------------\n");

    test_malloc_zero();
    test_malloc_basic();
    test_alignment();
    test_write_read();
    test_multiple_allocs();
    test_reuse_after_free();
    test_free_null();
    test_heap_exhaustion();

    printf("-------------------\n");
    printf("  %d passed, %d failed\n", passed, failed);

    return failed == 0 ? 0 : 1;
}
