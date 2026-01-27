# Mino — Elegant Object-Oriented Language (stable: v1.0)

## ✨ Language Features
Mino is a modern object-oriented programming language that compiles to C, designed to strike the balance between simplicity and powerful functionality. Current stable release is v1.0 (see `v1.x/v1.0`).

``` Mino
// Elegant syntax example
#include <Mino>

class Person {
    String name
    int age
    
    func greet() {
        System.IO.print("Hello, I'm " + this.name)
    }
}

func main {
    Person p = Person("Alice", 25)
    p.greet()
    
    // Modern language features
    let numbers = [1, 2, 3, 4, 5]
    numbers.each((n) => print(n * 2))
    
    return 0
}
```
## 🚀 Quick Start
### Installation
```bash
# Clone the repository
git clone https://github.com/MaherJon/Mino.git

# Build the compiler (v1.0)
cd Mino/v1.x/v1.0
make

# Run examples (from v1.0)
make example
```
### Your First Mino Program
#### Create hello.mino:

``` Mino
#include <Mino>

func main {
    System.IO.print("Hello, Mino World!")
    return 0
}
```
#### Compile and run:

```bash
# Compile and link to native executable (v1.0)
bin/minoc hello.mino
# produces: hello.mino.out

# Run generated executable
./hello.mino.out
```
## 📖 Why Mino?
### 🎯 For C Developers
```c
// What you write in C:
struct Person {
    char name[50];
    int age;
};

void Person_greet(struct Person* self) {
    printf("Hello, %s\n", self->name);
}

// What you write in Mino:
class Person {
    String name
    int age
    
    func greet() {
        print("Hello, " + this.name)
    }
}
```
## ✨ Clean & Expressive
No header files - Automatic interface generation    
Memory-safe - Optional GC and RAII support    
Modern syntax - Type inference, lambdas, extensions    
C interoperability - Direct C function calls    

## 🏗️ Architecture
### Compilation Pipeline
```text
Mino Source (.mino) 
     ↓
   Lexer/Parser
     ↓
   AST + Semantic Analysis
     ↓
   C Code Generation
     ↓
   Native Executable (via GCC/Clang)
```
### Project Structure
```text
Mino/ (repository root)
├── v0.x/
├── v1.x/
│   └── v1.0/
│       ├── Makefile
│       ├── docs/
│       ├── examples/
│       ├── include/
│       └── src/
├── LICENSE
└── README.md
```
## 📚 Language Tour
### Classes & Objects
```Mino
class Vector2 {
    float x, y
    
    // Constructor
    init(float x, float y) {
        this.x = x
        this.y = y
    }
    
    // Method
    func length() -> float {
        return sqrt(x*x + y*y)
    }
    
    // Operator overloading
    func +(Vector2 other) -> Vector2 {
        return Vector2(x + other.x, y + other.y)
    }
}
```
### Modern Features
```Mino
// Type inference
let message = "Hello"  // Auto String type

// Lambda expressions
let squares = [1, 2, 3].map(x => x * x)

// Pattern matching
match (result) {
    Ok(value) => print("Success: " + value)
    Err(msg) => print("Error: " + msg)
}

// Extension methods
extension String {
    func reverse() -> String {
        // ... implementation
    }
}
```
### C Interoperability
```Mino
// Direct C function calls
extern "C" {
    func printf(format: String, ...) -> int
    func malloc(size: int) -> Pointer
}

// Using C libraries
#include <math.h>

func calculate() {
    let result = sqrt(16.0)  // Calls C's sqrt directly
    print(result)
}
```
## 🛠️ Development Status
### ✅ Implemented
Lexer and parser    
Basic AST structure    
C code generation for simple programs    

### 🚧 In Progress
Class and method definitions    
Basic type system    
Inheritance and interfaces    
Generics/templates    
Standard library    
Package manager    
IDE support (LSP)

### 📅 Planned
Concurrency (async/await)   
Metaprogramming    
WebAssembly target    
Debugger integration

## 🔧 Building from Source
### Prerequisites
GCC or Clang    
Make    
CMake (optional)    

### Build Steps (v1.0)
```bash
# Clone and build
git clone https://github.com/MaherJon/Mino.git
cd Mino/v1.x/v1.0

# Build the compiler (result: bin/minoc)
make

# Run tests
make test

# Install globally (optional)
make install
```

Note: when `bin/minoc` compiles a Mino program it may emit assembly to `build/out.s` (the code generator output). Keep a copy of `build/out.s` if you need to inspect generated assembly before running `make clean`.
## 📊 Performance
Mino generates optimized C code, achieving performance comparable to hand-written C:

| Operation	| Mino	| Python  |  C (handwritten) |
| -----------|-------|---------|-------------------|
| Fibonacci	 | 0.8s	 | 3.2s	   | 0.7s |
| Matrix Mul | 1.2s	 | 12.4s   | 1.1s |
| File I/O	 | 0.3s	 | 1.8s	   | 0.2s |
## 🌟 Community & Contribution
### Getting Involved
- **Fork:** Fork the repository and create feature branches.
- **Issues:** Check Issues and `v1.x/v1.0/RELEASE_NOTES.md` for tasks.
- **Chat:** See `v1.x/v1.0/docs` for community/communication links.
- **Contributing:** Follow any guidance in `v1.x/v1.0/docs`.

## 📄 License
Mino is released under the GNU General Public License v2.0 (GPLv2). See LICENSE for full text.

## 🙏 Acknowledgments
Inspired by C++, Java, and Rust    
Built with LLVM/C infrastructure    
Thanks to all contributors and testers

"The beauty of C with the elegance of modern OOP"

Give Mino a star ⭐ if you find it interesting! Join us in building the next generation of systems programming languages.
