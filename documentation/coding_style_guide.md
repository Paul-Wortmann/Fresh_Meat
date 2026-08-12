
---

```markdown
# C/C++ Coding Style Guide

**Version:** 1.0  
**Status:** Draft  


## 1. Introduction

This document defines the coding style and conventions for all C and C++ code within the project. The primary goals are to:

- Promote **consistency** across the entire codebase.
- Improve **readability** and **maintainability**.
- Facilitate code **sharing** and **reuse**.
- Encourage **best practices** and reduce the likelihood of errors.

> **Guiding Principle:** Code is read much more often than it is written. Optimize for the reader, not the writer.

This guide is largely inspired by the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html), the [LLVM Coding Standards](https://llvm.org/docs/CodingStandards.html), and the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines). For topics not explicitly covered here, refer to these primary sources.


## 2. Language Standards

- **C:** Target **C11** or newer.
- **C++:** Target **ISO C++20** or newer. C++17, C++14, and C++11 are also acceptable if required by the target platform.
- Avoid non‑standard compiler extensions unless absolutely required by a platform boundary.
- Prefer standard, modern, and portable C++.


## 3. File Organisation

### 3.1 File Names

- All file names shall be **lowercase** and may include underscores (`_`).
- **C++** header files: `.hpp`; implementation files: `.cpp`.
- **C** header files: `.h`; implementation files: `.c`.
- Examples: `my_class.hpp`, `my_class.cpp`, `utils.h`, `thread_pool.c`

### 3.2 Header Guards

- Every header must have a guard to prevent multiple inclusions.
- The guard symbol shall be of the form:  
  `<PROJECT>_<PATH>_<FILE>_HPP_` for C++ headers, or `_H_` for C headers.
- Example: for `src/core/network_system.hpp` in project `MyProject`:

  ```cpp
  #ifndef MYPROJECT_CORE_NETWORK_SYSTEM_HPP_
  #define MYPROJECT_CORE_NETWORK_SYSTEM_HPP_
  // ... contents ...
  #endif  // MYPROJECT_CORE_NETWORK_SYSTEM_HPP_
  ```

### 3.3 Include Order

To minimise hidden dependencies, includes shall be ordered as follows (with a blank line between groups):

1. The associated header file (for `.cpp`/`.c` files).
2. C system headers (e.g., `<stdio.h>`).
3. C++ system headers (e.g., `<vector>`, `<string>`).
4. Other libraries’ headers.
5. Project headers.

### 3.4 Mixing C and C++

- When a C function is to be called from C++, or vice versa, declare it inside an `extern "C"` block in the C++ header:

  ```cpp
  #ifdef __cplusplus
  extern "C" {
  #endif

  void c_api_function(int param);

  #ifdef __cplusplus
  }
  #endif
  ```


## 4. Naming Conventions

Consistent naming is crucial for readability.

| Category | Convention | Examples |
| :--- | :--- | :--- |
| **Files** | `lowercase_with_underscores` | `my_class.cpp`, `network_utils.hpp` |
| **Classes / Types** | `UpperCamelCase` | `MyClass`, `NetworkManager`, `HttpRequest` |
| **Functions / Methods** | `lowerCamelCase` | `getData()`, `parseInput()`, `sendRequest()` |
| **Variables** (local, parameters) | `lowerCamelCase` | `userName`, `totalCount`, `isValid` |
| **Member variables** | `m_` + `lowerCamelCase` | `m_userName`, `m_totalCount`, `m_isValid` |
| **Constants** (compile‑time) | `k` + `UpperCamelCase` | `kMaxBufferSize`, `kDefaultTimeout` |
| **Enumerators** | `UpperCamelCase` (no prefix) | `Red`, `Green`, `Blue` |
| **Namespaces** | `lowercase` | `namespace myproject`, `namespace core` |
| **Macros** | `UPPER_CASE_WITH_UNDERSCORES` | `#define MY_PROJECT_VERSION 1` |

- **Important:** Never use names that begin with `__` (double underscore) or `_` (underscore followed by an uppercase letter) – these are reserved for the implementation.

**Pointer / reference placement:**  
Place the `*` or `&` adjacent to the **type**, not the name:  
`int* ptr;` and `const std::string& str;` (not `int *ptr;`).


## 5. Formatting

### 5.1 Indentation and Whitespace

- Use **spaces**, not tabs.
- Indent by **4 spaces** per level.
- Exactly **one statement per line**.
- A space between a keyword and an opening parenthesis: `if (condition)`, not `if(condition)`.
- Spaces around binary operators: `a + b`, `x = y`, not `a+b`, `x=y`.

### 5.2 Braces (Allman style – mandatory)

- Braces are required for all control statements (`if`, `else`, `for`, `while`, `do`), even for single‑statement blocks.
- The opening brace shall appear on the **next line** after the statement.

  ```cpp
  // Good
  if (condition)
  {
      doSomething();
  }

  // Good – single line still needs braces
  if (condition)
  {
      return;
  }

  // Bad – opening brace on same line (not Allman)
  if (condition) {
      doSomething();
  }

  // Bad – missing braces
  if (condition)
      doSomething();
  ```

### 5.3 Line Length

- Maximum **80 characters** per line.

### 5.4 Vertical Spacing

- Use blank lines to group logically related sections of code.
- Separate function definitions with a blank line.
- Use blank lines inside a function to separate logical blocks.

### 5.5 Line Endings and Trailing Whitespace

- Use **LF** (Unix) line endings.
- Strip trailing whitespace.
- End each file with a single newline.

### 5.6 Automatic Formatting

All code **must** be formatted with `clang-format` before committing. A project‑specific `.clang-format` file shall be provided, with settings matching this guide (especially 4‑space indent, Allman braces, and pointer alignment).


## 6. Comments

### 6.1 General

- Write comments to explain **why** something is done, not **what** (the code itself should be clear).
- Keep comments up‑to‑date with code changes.
- Prefer **C++‑style comments** (`//`) for ordinary comments.
- Use C‑style comments (`/* */`) only for multi‑line comments when necessary.

### 6.2 Documentation Comments

- Use **Doxygen**‑style comments for all public interfaces (classes, functions, enums, etc.).
- For C++ use `///`; for C use `/** ... */`.

  ```cpp
  /// @brief Fetches data from the remote server.
  ///
  /// @param url The endpoint URL.
  /// @param timeout Maximum time to wait in milliseconds.
  /// @return A string containing the server's response.
  std::string fetchData(const std::string& url, int timeout);
  ```


## 7. Class Organisation (C++)

For class definitions, members shall be ordered as follows:

1. **Public** section first (most important for users).
2. **Protected** section (if any).
3. **Private** section last.

Within each section:

- Types and aliases (`using`, `typedef`).
- Static constants.
- Constructors and destructor.
- Static member functions.
- Non‑static member functions.
- Data members (preferably at the end).

Example:

```cpp
class MyClass
{
public:
    using ValueType = int;

    MyClass();
    explicit MyClass(int value);
    ~MyClass();

    void doWork();

protected:
    void helper();

private:
    int value_;
    std::string name_;
};
```


## 8. Best Practices and Key Rules

### 8.1 Resource Management (C++)

- **Avoid** raw `new`/`delete` and `malloc`/`free` in application code.
- Prefer **RAII** and smart pointers (`std::unique_ptr`, `std::shared_ptr`).
- Model ownership explicitly (e.g., `std::unique_ptr` for exclusive ownership).

### 8.2 Casting

- **Avoid C‑style casts** (`(int)x`). They are unsafe and hard to find.
- Use C++‑style casts: `static_cast`, `const_cast`, `reinterpret_cast`, and `dynamic_cast`.

### 8.3 Pointers and References

- Use `nullptr` instead of `NULL` or `0`.
- Always check pointers for `nullptr` before dereferencing.
- Use `const` wherever possible (parameters, methods, local variables).
- Mark overriding virtual functions with `override`; use `final` when appropriate.
- Explicitly delete or default special member functions where needed (`= delete`, `= default`).

### 8.4 Error and Exception Handling

- **Exceptions** are **allowed** for error handling.
- Throw exceptions only for truly exceptional errors – not for regular control flow.
- All thrown exceptions must be documented in the function’s Doxygen comment.
- C code (or C‑compatible APIs) shall return error codes instead of throwing.

### 8.5 General

- Declare variables at the point of first use, not at the top of the function.
- Always check the return values of functions that can fail.
- Functions with no parameters in C shall be declared with `(void)`.
- Keep functions small and focused on a single task.
- Prefer standard algorithms (`std::ranges`, `std::algorithm`) over raw loops.
- Use `auto` only when the type is obvious from the context (e.g., iterator declarations, `make_unique`). Avoid `auto` when it obscures the type for the reader.
- Use `constexpr` for values that can be computed at compile time.
- Prefer `std::optional`, `std::variant`, and `std::any` over raw unions or pointer‑based “maybe” patterns.


## 9. Code Review Checklist

Reviewers shall check at least the following:

- [ ] Header guard exists and follows the naming rule.
- [ ] All includes are ordered correctly.
- [ ] Naming conventions are followed (no `e` prefix, no Hungarian, members have trailing `_`).
- [ ] Allman braces are used consistently.
- [ ] Indentation is 4 spaces (no tabs).
- [ ] Line length does not exceed 80 characters.
- [ ] Public interfaces are documented with Doxygen.
- [ ] No raw `new`/`delete` (C++).
- [ ] `const` correctness applied where possible.
- [ ] `override`/`final` used where applicable.
- [ ] Exceptions are documented.
- [ ] No C‑style casts.
- [ ] Variables are declared as late as possible.


## 10. Tools

The following tools shall be used to enforce this style guide automatically:

- **`clang-format`** – automatic code formatting (provide a `.clang-format` file).
- **`clang-tidy`** – static analysis and style checks (provide a `.clang-tidy` file).
- **EditorConfig** – to maintain consistent editor settings (indentation, line endings, etc.).

---

*This document is a living guideline and may be updated as the project evolves.*
```

---


