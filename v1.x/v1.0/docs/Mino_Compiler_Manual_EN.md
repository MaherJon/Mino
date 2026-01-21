# Mino Compiler User Manual

Version: v0.2.5

## Introduction

Mino is a small educational compiler. This manual explains how to build, install, and use the compiler, and includes debugging and contribution notes.

## Contents

- Installation
- Quick Start
- Command-line Options
- Compilation Pipeline (stages)
- Examples: `examples/simple.mino` and `examples/math.mino`
- Runtime & Libraries
- Debugging & Troubleshooting
- Testing & Contributing
- API / AST Reference (see separate API doc)

## Installation

Build from source:

```bash
make
```

This produces the compiler executable at `bin/minoc`.

Install to system (requires sudo):

```bash
sudo make install
# or
sudo cp bin/minoc /usr/local/bin/minoc
```

Build runtime object (speeds linking / executable generation):

```bash
make runtime
# or via the compiler helper
./bin/minoc --build-runtime
# build static runtime library
./bin/minoc --build-runtime-static
```

## Quick Start

Compile and run the example:

```bash
make
./bin/minoc examples/simple.mino
# The produced executable will be: examples/simple.out
./examples/simple.out
```

If you installed the compiler:

```bash
minoc examples/math.mino
./examples/math.out
```

Output naming: `foo.mino` or `foo.mi` -> `foo.out`.

## Command-line Options

- No arguments: prints usage information.
- `minoc <filename.mino|filename.mi>`: compile a source file, producing `*.out`.
- `minoc --test <test_string>`: run lexer/parser tests on the given string.
- `minoc --lex <filename>`: run lexer and print tokens.
- `minoc --parse <filename>`: run parser, print AST and type-check results.
- `minoc --build-runtime`: build runtime object `lib/minolib/System/System.o`.
- `minoc --build-runtime-static`: build static runtime archive `lib/minolib/libminosys.a`.

You can also use `make test` for the repository's simple test target.

## Compilation Pipeline

Stages implemented by the compiler:

1. Lexer: tokenize source into tokens.
2. Parser: parse tokens into an AST (abstract syntax tree).
3. Semantic analysis: type checking and symbol resolution.
4. Code generation: emit a native executable. The compiler will try to use `lib/minolib/libminosys.a` or `lib/minolib/System/System.o` for runtime support; if missing it invokes `make runtime`.

## Examples

1) Build and run `examples/simple.mino`:

```bash
make
./bin/minoc examples/simple.mino
./examples/simple.out
```

2) Run lexer/parser debug:

```bash
./bin/minoc --lex examples/simple.mino
./bin/minoc --parse examples/simple.mino
```

3) Quick test string:

```bash
./bin/minoc --test "func main() { let x: int = 42; return x; }"
```

## Runtime & Libraries

Runtime code is in `lib/minolib/System/`. Use `make runtime` to generate `lib/minolib/libminosys.a` for faster linking.

## Debugging & Troubleshooting

- File open errors: check path and permissions.
- Parse/type check failures: use `--lex` and `--parse` to inspect intermediate output.
- Link errors / missing runtime: run `make runtime` or `./bin/minoc --build-runtime-static` and verify `lib/minolib/libminosys.a` exists.
- Build failures: ensure `gcc` and development tools are installed.

Useful commands:

```bash
ls -l lib/minolib/libminosys.a lib/minolib/System/System.o
make runtime
./bin/minoc --parse examples/simple.mino
```

## Testing

Add tests under `tests/`. Use:

```bash
make test
```

## Contributing

- Fork, branch, and open PRs.
- Maintain coding style and existing `CFLAGS` (`-Wall -Wextra -g`).
- Add tests to `tests/` for new features.

## API / AST Reference

See `docs/Mino_Compiler_API.md` for detailed AST node descriptions and public APIs in headers.
