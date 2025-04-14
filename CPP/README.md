# Simple C++ Compiler

This is a simple C++ compiler project that implements the front end of a compiler: lexical analysis, syntax analysis, and semantic analysis.

## Features Supported

* Basic C++ syntax processing
* Variable declarations and assignments (`int x = 10;`, `char c = 'a';`)
* Arithmetic expressions with operator precedence (`x = 5 + y * 2;`)
* Function definitions, including the main function
* Conditional statements (if/else/else if)
* Loop statements (for, while)
* Include directives (`#include <iostream>`)
* Type checking for variables

## Project Structure

* **lexer.h**: Tokenizes the input source code into tokens
* **parser.h**: Parses the tokens into an Abstract Syntax Tree (AST)
* **semantic.h**: Performs semantic analysis on the AST (type checking, etc.)
* **main.cpp**: Main entry point for the compiler
* **Makefile**: Build system for the project

## Building and Running

To build the compiler:
```
make
```

To run the compiler against a test input file:
```
./compiler test_input.cpp
```

Or use the test target:
```
make test
```

To clean up build artifacts:
```
make clean
```

## Example Code

The compiler can process C++ code like:

```cpp
#include <iostream>

int main() {
    int x = 10;
    char c = 'a';
    
    if (x > 5) {
        x = 20;
    } else {
        x = 0;
    }
    
    while (x > 0) {
        x = x - 1;
    }
    
    for (int i = 0; i < 5; i = i + 1) {
        // Loop code
    }
    
    return 0;
}
```

## Compiler Phases

1. **Lexical Analysis**: Converts source code into tokens
2. **Syntax Analysis**: Builds an Abstract Syntax Tree from tokens
3. **Semantic Analysis**: Validates the AST for semantic correctness

## Limitations

* No code generation (this is only the front-end)
* Limited type support (only int and char)
* No support for classes, templates, or other advanced C++ features
* No support for preprocessing other than basic #include
* Limited error reporting and recovery 