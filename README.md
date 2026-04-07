# dumalloc

A minimal heap allocator implemented in C. Uses an implicit free-list over a statically allocated arena with first-fit placement and lazy coalescing.

## Build

```sh
make lib   # builds libdumalloc.a
make all   # builds the test driver (requires main() in main.c)
make clean
```

Requires `gcc` and GNU `make`. No external dependencies.

## Usage

```c
#include "heap.h"

void *p = my_malloc(64);
// ... use p ...
my_free(p);
```

## Configuration

All tuneable constants are in `heap.h`:

| Constant | Default | Description |
|---|---|---|
| `PREALLOCATED_HEAP_SIZE` | `16384` | Arena size in bytes |
| `ALIGNMENT` | `8` | Allocation alignment |
| `MIN_BLK_SZ` | `ALIGN(2)` | Minimum block size |
| `SPLIT_THRESHOLD_DIVIDER` | `1` | Block split threshold |

## Limitations

- Single-threaded only — no synchronisation.
- Fixed-size arena — cannot grow beyond `PREALLOCATED_HEAP_SIZE`.
- Block splitting not yet implemented.