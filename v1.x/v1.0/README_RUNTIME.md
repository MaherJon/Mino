Mino Runtime Library — Usage Guide

概览
- 位置: `lib/minolib/System/`（实现）和 `include/System.h`（头文件）。
- 已生成的构件: `lib/minolib/System/System.o` 和静态库 `lib/minolib/libminosys.a`（可由 `make runtime` 生成）。

构建运行时
Mino Runtime Library — Usage Guide

Overview

Location: `lib/minolib/System/` (implementation) and `include/System.h` (header).

Build artifacts: `lib/minolib/System/System.o` and the static library `lib/minolib/libminosys.a` (created by `make runtime`).

Building the runtime

```bash
# from repository root
make runtime
# Produces: lib/minolib/System/System.o and lib/minolib/libminosys.a
```

Runtime API summary (common functions)

Initialization

`void initSystem()` — Initialize runtime modules (also called automatically by library constructor).

Printing / I/O

`void sys_print(const char* s)` — Print a string without newline.

`void sys_PrintStringLn(const char* s)` — Print a string followed by newline.

`void sys_println(const char* s)` — Simple println wrapper.

`void sys_printlnf(const char* fmt, ...)` — Formatted println.

Flattened exports (for compiler-generated calls), e.g. `void sys_IO_print_PrintInt(int)`.

`void sys_IO_print_PrintIntLn(int)` — Print integer and newline (provided).

Input

`char* sys_readline(void)` — Read a line; returns a malloc'd buffer (caller must `sys_free`).

Low-level scanner wrappers such as `sys_IO_scanner_scanInt(int*)` are available.

Memory & strings

`void* sys_malloc(size_t)`, `void sys_free(void*)`, `char* sys_strdup(const char*)`.

`char* sys_itoa(int)` — integer to string (malloc'd result).

Math

Floating: `sys_sin`, `sys_cos`, `sys_sqrt`, `sys_pow`, `sys_floor`, `sys_ceil`, `sys_abs`.

Integer helpers: `sys_Math_absInt(int)`, `sys_Math_powInt(int a, int b)`.

File I/O

`FILE* sys_fopen(const char* path, const char* mode)`

`int sys_fclose(FILE* f)`

`size_t sys_fread(void* ptr, size_t size, size_t nmemb, FILE* f)`

`size_t sys_fwrite(const void* ptr, size_t size, size_t nmemb, FILE* f)`

`char* sys_fgets(char* s, int size, FILE* f)` — returns `s` or `NULL`.

`int sys_remove(const char* path)`

Time & random

`double sys_time_seconds(void)` — seconds since the epoch (high-resolution when available).

`int sys_rand_int(void)`, `void sys_srand_seed(unsigned int)`.

Linking notes (compiler integration)

The compiler prefers linking against `lib/minolib/libminosys.a`. If the archive is not present it will try `lib/minolib/System/System.o`, and as a last resort it will compile and link `lib/minolib/System/System.c` directly.

To link runtime from your own C program:

```bash
gcc your.o -Llib/minolib -lminosys -o yourprog
# or
gcc your.o lib/minolib/System/System.o -o yourprog
```

Examples

Language-level example (see `examples/math.mino`):

```
sys.IO.print.PrintIntLn(42);
sys.IO.print.PrintStringLn("hello");
```

C usage:

```c
#include "System.h"

int main(void) {
  initSystem();
  sys_PrintStringLn("Hello from runtime");
  return 0;
}
```

Testing & examples

Included example: `examples/math.mino` demonstrates integer math and `PrintIntLn`.

Consider adding examples for file I/O, `sys_readline`, `sys_printlnf`, and time/random APIs.

Notes & extensions

Naming convention: the compiler flattens dotted names (e.g. `sys.IO.print.PrintInt` -> `sys_IO_print_PrintInt`). The runtime provides both flattened exports and nicer C wrappers in `System.c`.

Thread-safety: current runtime is not thread-safe. Add synchronization if needed.

Memory ownership: returned strings are malloc'd; callers must free with `sys_free`.
