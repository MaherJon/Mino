# Mino Compiler v0.2.5

## Summary
- What: Lightweight Mino language compiler that parses, type-checks and emits native x86_64 executables (experimental).
- This release: Adds member-access and call AST nodes, a basic x86_64 backend, variable slot mapping, and English comments across modified files.

## Highlights
- Parser: supports dotted member access (e.g. `sys.IO.print.PrintInt`) and `let/var` declarations.
- Semantic: basic type inference, function pre-declaration and call/type checking.
- Backend: emits AT&T x86_64 assembly and links with runtime helpers to produce native executables.
- Examples: sample programs in `examples/` demonstrate features.

## Changelog (short)
- Added `CALL_EXPR` and `GET_EXPR` AST nodes.
- Implemented call/name resolution in semantic analysis.
- Simple code generator in `src/codegen/` → produces `build/out.s` and links via `gcc -no-pie`.
- Replaced Chinese comments with English across modified files.

## Installation / Build
- Prerequisites: `gcc`, `make`.
- Build the compiler (compile only):
```bash
make
# result: bin/minoc
```

## Compiler Usage
- General: `bin/minoc <file.mino>` compiles and links to `<file.mino>.out`.
- Quick options:
  - Lex only: `bin/minoc --lex examples/simple.mino`
  - Parse + print AST + typecheck: `bin/minoc --parse examples/simple.mino`
  - Test string: `bin/minoc --test "source code here"`

## Build & Run Example
- Build the compiler:
```bash
make
```
- Compile an example to native binary:
```bash
./bin/minoc examples/simple.mino
# produces: examples/simple.mino.out
```
- Run the generated executable:
```bash
./examples/simple.mino.out
```

## Important Notes
- The code generator writes assembly to `build/out.s`. Running `make clean` will remove `build/out.s`; if you want to keep the generated assembly, copy it before cleaning:
```bash
cp build/out.s build/out.s.saved
make clean
```
or rebuild after cleaning:
```bash
make
./bin/minoc examples/simple.mino
```
- The backend is minimal and experimental: supports integer arithmetic, simple calls (up to 6 register args), and a basic variable layout. Expect limitations for complex programs.

## Troubleshooting
- If compilation fails with implicit-declaration errors, ensure headers in `include/` are fresh and run `make` from repo root.
- If a generated program segfaults, inspect `build/out.s` (copy it before cleaning) and test with a simpler example like `examples/no_include.mino`.

## Contact / Next Steps
- For issues or feature requests, open a GitHub Issue.
- Planned improvements: more robust ABI handling, stack frame layout, broader type support, and extended runtime.

---
