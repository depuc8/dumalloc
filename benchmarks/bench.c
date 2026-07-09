/*
 * bench.c — Performance benchmarks for dumalloc.
 *
 * Measures throughput (ops/sec) and average latency (ns/op) for several
 * real-world allocation patterns.  Each benchmark cleans up after itself so
 * the arena is restored for the next run.
 *
 * Build with -O2 for representative numbers.
 */

#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "../include/heap.h"

/* ── Timing helpers ──────────────────────────────────────────────────────── */

static struct timespec ts_now(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t;
}

static double ts_elapsed_ns(struct timespec start, struct timespec end)
{
    return (double)(end.tv_sec  - start.tv_sec)  * 1e9 +
           (double)(end.tv_nsec - start.tv_nsec);
}

static void print_result(const char *name, long ops, double elapsed_ns)
{
    double ops_per_sec = (double)ops / (elapsed_ns * 1e-9);
    double ns_per_op   = elapsed_ns / (double)ops;
    printf("  %-40s  %8.2f ns/op   %10.0f ops/sec\n",
           name, ns_per_op, ops_per_sec);
}

/* ── Benchmarks ──────────────────────────────────────────────────────────── */

/*
 * Bench 1: malloc/free pairs — allocate one block, free it immediately.
 * Exercises the hot path with maximum reuse.
 */
static void bench_paired(size_t sz, int iters)
{
    char label[64];
    snprintf(label, sizeof(label), "malloc/free pair  sz=%4zu", sz);

    struct timespec t0 = ts_now();
    for (int i = 0; i < iters; i++) {
        void *p = my_malloc(sz);
        my_free(p);
    }
    struct timespec t1 = ts_now();

    print_result(label, iters, ts_elapsed_ns(t0, t1));
}

/*
 * Bench 2: Batch alloc then batch free — allocate N blocks, then free all.
 * Exercises find_fit under fragmentation pressure, and then mass free.
 */
static void bench_batch(size_t sz, int n)
{
    /* We can fit at most PREALLOCATED_HEAP_SIZE / block_total blocks. */
    size_t block_total = ALIGN(sz + SIZE_T_SZ);
    if (block_total < MIN_BLK_SZ) block_total = MIN_BLK_SZ;
    int max_blocks = (int)(PREALLOCATED_HEAP_SIZE / block_total);
    if (n > max_blocks) n = max_blocks;

    void **ptrs = NULL;
    /* Use a small stack array to avoid pulling in stdlib malloc. */
    /* Max slots needed: 16384 / 16 = 1024 for sz=1 */
    static void *buf[1024];
    if (n > 1024) n = 1024;
    ptrs = buf;

    char label[64];
    snprintf(label, sizeof(label), "batch alloc×%d  sz=%4zu", n, sz);

    struct timespec t0 = ts_now();
    for (int i = 0; i < n; i++)
        ptrs[i] = my_malloc(sz);
    struct timespec t1 = ts_now();

    for (int i = 0; i < n; i++)
        my_free(ptrs[i]);

    print_result(label, n, ts_elapsed_ns(t0, t1));
}

/*
 * Bench 3: Sequential sizes — allocate blocks of increasing size (1 .. 128),
 * then free them all.  Exercises the allocator across a variety of sizes in
 * one pass.
 */
static void bench_sequential_sizes(void)
{
    const int steps = 32;
    static void *ptrs[32];

    struct timespec t0 = ts_now();
    for (int i = 0; i < steps; i++)
        ptrs[i] = my_malloc((size_t)(i + 1) * 4);
    struct timespec t1 = ts_now();

    for (int i = 0; i < steps; i++)
        my_free(ptrs[i]);

    print_result("sequential sizes  4..128", steps, ts_elapsed_ns(t0, t1));
}

/*
 * Bench 4: Interleaved alloc/free — keep a sliding window of 4 live blocks,
 * freeing the oldest before each new alloc.  Mimics typical heap traffic.
 */
static void bench_interleaved(size_t sz, int iters)
{
    const int window = 4;
    static void *ring[4];
    char label[64];
    snprintf(label, sizeof(label), "interleaved(w=4)  sz=%4zu", sz);

    /* Prime the ring. */
    for (int i = 0; i < window; i++)
        ring[i] = my_malloc(sz);

    struct timespec t0 = ts_now();
    for (int i = 0; i < iters; i++) {
        int slot = i % window;
        my_free(ring[slot]);
        ring[slot] = my_malloc(sz);
    }
    struct timespec t1 = ts_now();

    for (int i = 0; i < window; i++)
        my_free(ring[i]);

    print_result(label, iters, ts_elapsed_ns(t0, t1));
}

/*
 * Bench 5: Fragmentation stress — alloc 16-byte blocks filling the arena,
 * free every other one, then measure how long it takes to alloc 32-byte
 * blocks into the gaps (requires coalescing).
 */
static void bench_fragmentation(void)
{
    static void *ptrs[512];
    int n = 0;

    /* Fill the arena with 16-byte blocks. */
    void *p;
    while ((p = my_malloc(16)) != NULL && n < 512)
        ptrs[n++] = p;

    /* Free every other block to create gaps. */
    for (int i = 0; i < n; i += 2)
        my_free(ptrs[i]);

    /* Now alloc 32-byte blocks which require adjacent free blocks to merge. */
    int fit = 0;
    struct timespec t0 = ts_now();
    for (int i = 1; i < n; i += 2) {
        my_free(ptrs[i]);           /* expose adjacent free block */
        void *q = my_malloc(32);    /* should coalesce and fit    */
        if (q) { ptrs[fit++] = q; }
    }
    struct timespec t1 = ts_now();

    for (int i = 0; i < fit; i++)
        my_free(ptrs[i]);

    int ops = (n / 2);
    print_result("fragmentation + coalesce  sz=32", ops,
                 ts_elapsed_ns(t0, t1));
}

/* ── Entry point ─────────────────────────────────────────────────────────── */

int main(void)
{
    printf("dumalloc benchmarks\n");
    printf("===================\n");
    printf("  heap size: %d bytes\n", PREALLOCATED_HEAP_SIZE);
    printf("  alignment: %d bytes\n\n", ALIGNMENT);
    printf("  %-40s  %12s   %16s\n", "benchmark", "latency", "throughput");
    printf("  %s\n", "---------------------------------------------------------------------");

    bench_paired(8,   500);
    bench_paired(64,  500);
    bench_paired(256, 200);

    putchar('\n');

    bench_batch(8,  128);
    bench_batch(64, 64);

    putchar('\n');

    bench_sequential_sizes();

    putchar('\n');

    bench_interleaved(16, 400);
    bench_interleaved(64, 200);

    putchar('\n');

    bench_fragmentation();

    printf("\n===================\n");
    printf("  Note: heap is 16 KB; iteration counts are capped accordingly.\n");
    printf("  Build with -O2 for representative numbers.\n");

    return 0;
}
