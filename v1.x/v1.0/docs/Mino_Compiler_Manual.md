# Mino 编译器使用手册

版本：v0.2.5

## 简介

Mino 是一个小型教学用编译器，源代码位于仓库根目录。该手册介绍如何构建、安装、使用编译器，并包含调试与贡献指南。

## 目录

- 安装
- 快速开始
- 命令行选项
- 编译流程（阶段说明）
- 示例：编译 `examples/simple.mino` 和 `examples/math.mino`
- 运行时与库
- 调试与故障排查
- 测试与贡献

## 安装

从源码构建（推荐）

1. 进入仓库根目录。
2. 运行：

```bash
make
```

构建完成后，生成可执行文件：`bin/minoc`。

安装到系统目录（需要 sudo）

```bash
sudo make install
# 或者手动复制
sudo cp bin/minoc /usr/local/bin/minoc
```

构建运行时对象（可加速链接/生成可执行文件）

```bash
make runtime
# 或者使用编译器内建命令
./bin/minoc --build-runtime
# 构建静态运行时库
./bin/minoc --build-runtime-static
```

## 快速开始

- 编译示例文件：

```bash
make
./bin/minoc examples/simple.mino
# 生成的可执行文件位于：examples/simple.out（默认基于源文件名）
./examples/simple.out
```

- 使用安装后的可执行文件（如果已经运行 `make install`）：

```bash
minoc examples/math.mino
./examples/math.out
```

默认输出文件名规则：编译 `foo.mino` 或 `foo.mi` 将生成 `foo.out`。

## 命令行选项

编译器主程序 `bin/minoc` 支持下列用法：

- 无参数：显示帮助和用法说明。
- `minoc <filename.mino|filename.mi>`：编译指定源文件，生成 `*.out` 可执行文件。
- `minoc --test <test_string>`：对给定字符串运行词法与句法测试（用于快速验证 lexer/parser）。
- `minoc --lex <filename>`：只运行词法分析并打印 token 列表。
- `minoc --parse <filename>`：只运行解析器并打印 AST 与类型检测结果。
- `minoc --build-runtime`：构建运行时对象 `lib/minolib/System/System.o`。
- `minoc --build-runtime-static`：构建静态运行时库 `lib/minolib/libminosys.a`。

你也可以使用 `make test` 运行仓库中定义的简单测试目标。

## 编译流程（阶段说明）

编译器分为若干阶段：

1. 词法分析（Lexer）
   - 将源代码拆分为 token。
   - 错误报告通常在此阶段给出无法识别的字符或字符串。

2. 语法分析（Parser）
   - 将 token 解析为 AST（抽象语法树）。

3. 语义分析（Semantic / Type checking）
   - 检查类型一致性、符号表与作用域。

4. 代码生成（Codegen）
   - 将 AST 转换为目标可执行文件（当前实现会生成本地可执行文件）。
   - 生成可执行文件时，编译器会尝试使用 `lib/minolib/libminosys.a` 或 `lib/minolib/System/System.o` 作为运行时支持；若不存在，会自动调用 `make runtime` 来构建。

## 示例

1) 编译并运行 `examples/simple.mino`

```bash
make
./bin/minoc examples/simple.mino
# 运行生成的可执行
./examples/simple.out
```

2) 使用词法/解析调试示例

```bash
./bin/minoc --lex examples/simple.mino
./bin/minoc --parse examples/simple.mino
```

3) 在命令行直接测试字符串（快速验证 lexer/parser）

```bash
./bin/minoc --test "func main() { let x: int = 42; return x; }"
```

## 运行时与库

运行时实现位于 `lib/minolib/System/` 中。为了减少每次链接的开销，项目提供 `make runtime` 目标来生成 `lib/minolib/libminosys.a`。

构建静态运行时（推荐）：

```bash
make runtime
```

或者使用编译器内置命令：

```bash
./bin/minoc --build-runtime
./bin/minoc --build-runtime-static
```

## 调试与故障排查

- 无法打开源文件：确认文件路径正确，且有读权限。
- 解析失败或类型检测失败：先使用 `--lex` 与 `--parse` 检查词法与语法输出；查看 `examples/` 中的参考写法。
- 运行时找不到符号或链接错误：确保已运行 `make runtime` 或 `./bin/minoc --build-runtime-static`，并检查 `lib/minolib/libminosys.a` 是否存在。
- 若 `make` 失败：查看 `Makefile` 中的 `CC` 与 `CFLAGS`，确认系统已安装 `gcc` 与标准开发工具链。

常用诊断命令：

```bash
# 查看是否生成运行时库
ls -l lib/minolib/libminosys.a lib/minolib/System/System.o
# 手动构建运行时
make runtime
# 运行解析调试
./bin/minoc --parse examples/simple.mino
```

## 测试

仓库包含 `tests/` 目录，请在后续开发中补充测试用例。当前可使用 `make test` 运行 `Makefile` 中定义的简单测试目标：

```bash
make test
```

## 贡献指南

欢迎贡献：bug 修复、语言特性、代码生成优化、测试用例、文档改进等。建议：

- Fork 仓库并在 feature 分支上提交变更。
- 保持代码风格一致，使用仓库中已有的 `CFLAGS`（`-Wall -Wextra -g`）。
- 添加或更新 `tests/` 中的测试用例以覆盖新功能。

## 常见问题（FAQ）

- Q：生成的可执行为什么叫 `*.out`？
  A：当前实现通过 `getOutputPath` 将源文件扩展名去掉并追加 `.out`，以避免覆盖源文件。

- Q：如何更改默认安装路径？
  A：使用 `make` 生成 `bin/minoc` 后手动复制到任意目录，或修改 `Makefile` 中 `install` 目标。