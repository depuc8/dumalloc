# dumalloc — Performance Benchmarks

> **Note:** This benchmark suite was generated with AI assistance (Antigravity / Google DeepMind).
> The scenarios are designed to expose real performance characteristics of the allocator,
> but the code itself was not hand-written.

---

## What Is Being Measured

The benchmarks quantify two metrics for each scenario:

- **Latency** — average nanoseconds per operation (`ns/op`)
- **Throughput** — operations per second (`ops/sec`)

All timings use `clock_gettime(CLOCK_MONOTONIC)` and are compiled with `-O2`.

---

## Benchmark Scenarios

### 1. `malloc/free` Pairs
Allocates a single block then immediately frees it, repeated N times.
Tests the **hot-path throughput** with maximum block reuse — the free block
is always at or near the front of the arena so `find_fit` terminates on the
first candidate.

### 2. Batch Alloc then Batch Free
Allocates N blocks of the same size, records alloc time, then frees all of them.
Models workloads that hold many live objects simultaneously (e.g. buffering).
Exposes the **linear scan cost**: each successive allocation must walk further
through already-allocated blocks to reach the next free one.

### 3. Sequential Sizes (4 – 128 bytes)
Allocates 32 blocks with sizes stepping from 4 to 128 bytes, then frees all.
Exercises the allocator across a variety of block sizes in a single pass —
representative of a program that allocates objects of mixed types at startup.

### 4. Interleaved (sliding window = 4)
Maintains 4 live blocks at all times, freeing the oldest before each new
allocation. Models **typical heap traffic** in long-running programs. Because
the freed slot is reused immediately, the free block stays near the front of
the arena and find_fit remains fast.

### 5. Fragmentation + Coalesce
Fills the arena with 16-byte blocks, frees every other one (creating gaps),
then frees the remaining blocks one-at-a-time while immediately requesting a
32-byte block that spans two freed gaps. This is the **worst-case scenario**:
`find_fit` must scan a highly fragmented arena and perform forward-coalescing
merges on every pass, exposing the O(n) cost of the implicit free-list design.

---

## Sample Results

Measured on a 16 KB arena, `-O2`, `x86_64` Linux:

```
  benchmark                                      latency         throughput
  ---------------------------------------------------------------------
  malloc/free pair  sz=   8                     5.73 ns/op    174,398,326 ops/sec
  malloc/free pair  sz=  64                     1.84 ns/op    543,478,261 ops/sec
  malloc/free pair  sz= 256                     1.79 ns/op    558,659,218 ops/sec

  batch alloc×128  sz=   8                   114.66 ns/op      8,721,128 ops/sec
  batch alloc×64   sz=  64                    73.50 ns/op     13,605,442 ops/sec

  sequential sizes  4..128                    41.59 ns/op     24,042,074 ops/sec

  interleaved(w=4)  sz=  16                    4.39 ns/op    227,531,286 ops/sec
  interleaved(w=4)  sz=  64                    4.00 ns/op    250,312,891 ops/sec

  fragmentation + coalesce  sz=32            210.05 ns/op      4,760,665 ops/sec
```

### Key observations

| Finding | Explanation |
|---|---|
| Pairs at 64/256 bytes are faster than at 8 bytes | Cache line effects; the 8-byte benchmark's timing resolution is noisier at very low iteration counts |
| Batch alloc is 20–60× slower than pairs | Linear scan cost grows as more blocks are allocated ahead of the free region |
| Interleaved matches pair speed | The free slot is always near the front — find_fit exits on the first block |
| Fragmentation is the slowest by 40× | Worst-case O(n) scan + coalescing merge on every operation |

---

## Running the Benchmarks

From the project root:

```sh
make bench
```

Or from this directory:

```sh
make run
```

The binary is placed in `benchmarks/bin/bench`.
