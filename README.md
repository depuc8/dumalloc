# dumalloc

A minimal heap allocator implemented in C. Uses an implicit free-list over a statically allocated arena with first-fit placement, block splitting, and lazy forward coalescing.

## Build

```sh
make        # builds bin/dumalloc (test driver)
make lib    # builds bin/libdumalloc.a
make test   # builds and runs the unit test suite
make bench  # builds and runs the performance benchmarks
make clean  # removes all build artefacts
```

Requires `gcc` and GNU `make`. No external dependencies.

## Project Structure

```
dumalloc/
├── include/        # Public headers
│   └── heap.h
├── src/            # Allocator implementation + test driver
│   ├── heap.c
│   └── main.c
├── tests/          # Unit test suite
│   └── test_alloc.c
├── benchmarks/     # Performance benchmarks (AI-generated)
│   └── bench.c
├── bin/            # Compiled binaries (created by make)
└── build/          # Object files (created by make)
```

## Usage

```c
#include "heap.h"

void *p = my_malloc(64);
/* ... use p ... */
my_free(p);
```

## Configuration

All tuneable constants are in `heap.h`:

| Constant | Default | Description |
|---|---|---|
| `PREALLOCATED_HEAP_SIZE` | `16384` | Arena size in bytes |
| `ALIGNMENT` | `8` | Allocation alignment |
| `MIN_BLK_SZ` | `SIZE_T_SZ + ALIGNMENT` | Minimum splittable block size |
| `SPLIT_THRESHOLD_DIVIDER` | `1` | Block split threshold |

## Limitations

- Single-threaded only — no synchronisation.
- Fixed-size arena — cannot grow beyond `PREALLOCATED_HEAP_SIZE`.
- Linear-time `find_fit` — performance degrades under heavy fragmentation.