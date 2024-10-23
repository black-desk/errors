# `errors`: a header-only golang-like error wrapping library for c++

[![codecov](https://codecov.io/github/black-desk/errors/graph/badge.svg?token=QM9XK2OX15)](https://codecov.io/github/black-desk/errors)
![](https://img.shields.io/github/check-runs/black-desk/errors/master)
![](https://img.shields.io/github/commit-activity/w/black-desk/errors/master)
![](https://img.shields.io/github/contributors/black-desk/errors)
![](https://img.shields.io/github/release-date/black-desk/errors)
![](https://img.shields.io/github/commits-since/black-desk/errors/latest/master)
![](https://img.shields.io/github/license/black-desk/errors)

This library aims to bring golang-like error wrapping and handling to
c++.

---------------------------------------------------------------------

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT",
"SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this
document are to be interpreted as described in RFC 2119.

## Table of Contents

- [Overview](#overview)
- [Reference](#reference)
- [Tutorial](#tutorial)
  - [Basic usage](#basic-usage)
  - [Use `errors` with `expected`](#use-errors-with-expected)
  - [Advanced usage](#advanced-usage)
  - [How to](#how-to)
    - [Customize output format of error
      globally](#customize-output-format-of-error-globally)
    - [Customize output format of error
      locally](#customize-output-format-of-error-locally)
- [Install](#install)
  - [CMake](#cmake)
    - [CPM.cmake](#cpm.cmake)
    - [`FetchContent`](#fetchcontent)
    - [Install to system](#install-to-system)
  - [Vendor](#vendor)
- [Contributing to `errors`](#contributing-to-errors)
  - [Coding Rules](#coding-rules)
  - [Build](#build)

## Overview

When you are writing C++ code in a project where exceptions are not
allowed, error reporting often becomes a problem. In the traditional
model, in such cases, developers almost only have error codes as the
error handling mechanism. However, handling errors based on error codes
is relatively primitive and not very user-friendly, as the reported
errors lack specific contextual information. Troubleshooting requires
detailed log inspection, which is inconvenient.

The error handling mechanism based on returned errors in Golang
(https://pkg.go.dev/errors) has become quite usable after some time of
development.

So let’s bring the mechanism to c++.

## Reference

See <https://black-desk.github.io/errors>.

## Tutorial

### Basic usage

This library defined an abstract class `errors::error` (see [header
file](docs/../examples/basic-usage/../../include/errors/error.hpp)) to
represent an error, which has some method to display error message and
the source_location where this error is created.

A function or method want to return an error should return an
`errors::error_ptr`, which is a `std::unique_ptr<error>`.

``` cpp
#include <iostream>

#include "errors/errors.hpp"

using errors::error_ptr;
using errors::impl::runtime_error;

// NOTE:
// If you have a function which might goes wrong,
// but you don't want to or can not to throw an exception,
// you can return an error_ptr.
error_ptr fn() noexcept
{
        return errors::make<runtime_error>::with("error occurs");
};

// NOTE:
// You can return an wrapped error_ptr using the `wrap` function.
error_ptr fn2() noexcept
{
        auto err = fn();
        return errors::wrap(fn());
}

int main()
{
        auto err = fn2();

        // NOTE:
        // This will print something like:
        // ```
        // Error: error occurs
        // ```
        // The wrap() call in fn2 with no message will be omitted
        // in default error printer function.
        std::cerr << "Error: " << err << std::endl;

        // NOTE:
        // But the wrapped error does record the wrap() source_location to err.
        // You can get it by calling err->location()
        auto loc = err->location();
        assert(loc.has_value());
        std::cerr << "Error function: " << loc->function_name() << std::endl
                  << "Error file_name: " << loc->file_name() << std::endl
                  << "Error line: " << loc->line() << std::endl
                  << "Error column: " << loc->column() << std::endl;

        // NOTE:
        // Or use json output.
#if defined(ERRORS_ENABLE_NLOHMANN_JSON_SUPPORT)
        std::cerr << "Error in JSON:" << std::endl
                  << nlohmann::json(err) << std::endl;
#endif

        // NOTE:
        // This will print something like:
        // ```
        // Error: no error
        // ```
        err = nullptr;
        std::cerr << "Error: " << err << std::endl;

        return 0;
}
```

### Use `errors` with `expected`

``` cpp
#include <iostream>

#include "errors/errors.hpp"
#include "tl/expected.hpp"

using errors::error_ptr;
using errors::impl::runtime_error;
using tl::expected;
using tl::unexpected;

// NOTE:
// If you have a `Stack` class that can push and pop integer elements.
class stack_t {
    public:
        // NOTE:
        // You will have some functions that has return value in a semantic way,
        // like the `pop` function.
        // It is **RECOMMEND** use `std::expected` (or `tl::expected` for c++11)
        // to return the expected value when function return correctly
        // and the error_ptr when something goes wrong.
        expected<int, error_ptr> pop() noexcept;

        // NOTE:
        // You can also use `std::expected` (or `tl::expected` for c++11)
        // for the `push` function which returns nothing.
        // But its somekind of wired,
        // because you can just simply return `error_ptr`.
        error_ptr push(int value) noexcept;

    private:
        static const int MAX_SIZE = 3;
        int data[MAX_SIZE];
        int top = 0;
};

expected<int, error_ptr> stack_t::pop() noexcept
{
        if (top == 0) {
                return unexpected(
                        errors::make<runtime_error>::with("underflow"));
        }

        return data[--top];
}

error_ptr stack_t::push(int value) noexcept
{
        if (top == MAX_SIZE) {
                return errors::make<runtime_error>::with("overflow");
        }

        data[top++] = value;
        return nullptr;
}

int main()
{
        stack_t stack;

        auto value = stack.pop();
        assert(!value);
        std::cerr << "Failed to pop element from stack: " << value.error()
                  << std::endl;

        auto err = stack.push(1);
        assert(err == nullptr);
        err = stack.push(2);
        assert(err == nullptr);
        err = stack.push(3);
        assert(err == nullptr);

        err = stack.push(4);
        assert(err != nullptr);
        std::cerr << "Failed to push element to stack: " << err << std::endl;
        return 0;
}
```

### Advanced usage

``` cpp
#include <iostream>

#include "errors/errors.hpp"
#include "tl/expected.hpp"

using errors::error_ptr;
using errors::wrap;
using errors::impl::runtime_error;
using tl::expected;
using tl::unexpected;

class stack_t {
    public:
        expected<int, error_ptr> pop() noexcept;
        error_ptr push(int value) noexcept;

        static const int MAX_SIZE = 3;

    private:
        int data[MAX_SIZE];
        int top = 0;
};

// NOTE:
// A new error type for errors reported by stack_t class.
// An error type should records
// the machine friendly error context information.
// It can be used to generate developer and
// user friendly error message later.
struct stack_error_t : public runtime_error {
    public:
        // NOTE:
        // Write a constructor whichs last argument is `source_location`
        // to make this error type compatible with errors::make.
        stack_error_t(int top, errors::source_location location)
                : runtime_error("stack error [top=" + std::to_string(top) + "]",
                                std::move(location))

                , top(top)
        {
        }

        int top;
};

expected<int, error_ptr> stack_t::pop() noexcept
{
        if (top == 0) {
                return unexpected(errors::make<stack_error_t>::with(top));
        }

        return data[--top];
}

error_ptr stack_t::push(int value) noexcept
{
        if (top == MAX_SIZE) {
                return errors::make<stack_error_t>::with(top);
        }

        data[top++] = value;
        return nullptr;
}

// NOTE:
// This function prints the error message for stack_error_t.
// It is used to generate user friendly error messages.
void print_stack_error(const error_ptr &err)
{
        if (!err) {
                return;
        }

        if (!err.is<stack_error_t>()) {
                return;
        }

        std::cout << "Stack error occurs" << std::endl;

        auto stack_error = err.as<stack_error_t>();
        assert(stack_error != nullptr);

        if (stack_error->top == 0) {
                std::cout << "Stack underflow" << std::endl;
                return;
        }

        if (stack_error->top == stack_t::MAX_SIZE) {
                std::cout << "Stack overflow" << std::endl;
                return;
        }

        std::cout << "Unknown stack error" << std::endl;
}

error_ptr fn(stack_t &stack)
{
        auto value = stack.pop();
        assert(!value);
        return errors::wrap("something goes wrong",
                            wrap(wrap(std::move(value.error()))));
}

int main()
{
        stack_t stack;

        auto err = fn(stack);
        // NOTE:
        // Print logs for developer here.
        std::cerr << "Error: " << err << std::endl;

        // NOTE:
        // Print errors for user here.
        print_stack_error(err);

        err = stack.push(1);
        assert(err == nullptr);
        err = stack.push(2);
        assert(err == nullptr);
        err = stack.push(3);
        assert(err == nullptr);

        err = stack.push(4);
        assert(err != nullptr);
        print_stack_error(err);

        err = errors::make<stack_error_t>::with(1);
        print_stack_error(err);

        return 0;
}
```

### How to

#### Customize output format of error globally

``` cpp
#include <filesystem>
#include <iostream>

// If you want to disable the default ostream support
// to customize it for your own program,
// please define ERRORS_DISABLE_OSTREAM
// before include any header files from `errors`
#define ERRORS_DISABLE_OSTREAM
#include "errors/errors.hpp"

inline std::ostream &operator<<(std::ostream &os, const errors::error_ptr &err)
{
        const auto *current = err.get();

        if (!current) {
                os << "no error";
                return os;
        }

        while (current != nullptr) {
                os << std::endl;

                auto location = current->location();
                if (!location) {
                        os << "[source location not available] ";
                } else {
                        os << "[function " << location->function_name()
                           << " at "
                           << std::filesystem::path(location->file_name())
                                        .filename()
                                        .string()
                           << " " << location->line() << ":"
                           << location->column() << "] ";
                }

                auto what = current->what();
                assert(what);
                os << what;

                current = current->cause().get();
        }

        return os;
}

int main()
{
        using errors::wrap;
        using errors::impl::runtime_error;

        std::cerr << "Error: "
                  << wrap(wrap(errors::make<runtime_error>::with("error")))
                  << std::endl;

        return 0;
}
```

#### Customize output format of error locally

``` cpp
#include <filesystem>
#include <iostream>

#include "errors/errors.hpp"

using errors::wrap;
using errors::impl::runtime_error;

namespace local_ns
{
// NOTE:
// This is a custom operator for printing the error in a more verbose format.
// But it is not going to
// override the default operator<< outside of this namespace.
inline std::ostream &operator<<(std::ostream &os, const errors::error_ptr &err)
{
        const auto *current = err.get();

        if (!current) {
                os << "no error";
                return os;
        }

        while (current != nullptr) {
                os << std::endl;

                auto location = current->location();
                if (!location) {
                        os << "[source location not available] ";
                } else {
                        os << "[function " << location->function_name()
                           << " at "
                           << std::filesystem::path(location->file_name())
                                        .filename()
                                        .string()
                           << " " << location->line() << ":"
                           << location->column() << "] ";
                }

                auto what = current->what();
                assert(what);
                os << what;

                current = current->cause().get();
        }

        return os;
}

void print_error_in_local_ns()
{
        // Using the custom operator<<
        std::cerr << "Error: "
                  << wrap(wrap(errors::make<runtime_error>::with("error")))
                  << std::endl;
}
}

void print_error_in_global_ns()
{
        using errors::wrap;

        // Using the default operator<<
        std::cerr << "Error: "
                  << wrap(wrap(errors::make<runtime_error>::with("error")))
                  << std::endl;
}

int main()
{
        print_error_in_global_ns();
        local_ns::print_error_in_local_ns();

        return 0;
}
```

## Install

This library is a simple header-only c++11 library, which means that you
have so many ways to make it available to your project.

### CMake

You can use this library via the CMake build system.

#### [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)

It’s **RECOMMEND** to use this library via CPM.cmake, the CMake package
manager.

1.  Include CPM.cmake to your project

2.  Use CPM.cmake to add this library:

    ``` cmake
    CPMFindPackage("gh:black-desk/errors@0.2.0") # or a later release
    ```

3.  Link your target with `errors::errors` like this:

    ``` cmake
    add_executable(demo main.cpp)
    target_link_libraries(demo PRIVATE errors::errors)
    ```

4.  Include header file like this:

    ``` cpp
    #include "errors/error.hpp"
    ```

5.  Start write codes

#### [`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html)

As CPM.cmake is a wrapper of CMake’s `FetchContent` feature, you can
also use this library directly via raw `FetchContent`.

1.  Use `FetchContent` to add this library:

    ``` cmake
    find_package(errors 0.2.0 QUIET)
    if (NOT errors_FOUND)
      include(FetchContent)

      FetchContent_Declare(
        errors
        GIT_REPOSITORY https://github.com/black-desk/errors.git
        GIT_TAG        v0.2.0 # or a later release
      )

      FetchContent_MakeAvailable(errors)
    endif()
    ```

2.  Link your target with `errors::errors` like this:

    ``` cmake
    add_executable(demo main.cpp)
    target_link_libraries(demo PRIVATE errors::errors)
    ```

3.  Include header file like this:

    ``` cpp
    #include "errors/error.hpp"
    ```

4.  Start write codes

#### Install to system

You can also use this library in the old way, which means to install
this library into your development environment, then use it via CMake’s
`find_package` function.

1.  Install from source:

    ``` bash
    git clone https://github.com/black-desk/errors && \
      cd errors && \
      cmake -B build && \
      cmake --build build && \
      cmake --install build
    ```

2.  Use `find_package` to import this library, then link this library to
    your target like this:

    ``` cmake
    find_package(errors 0.2.0 REQUIRED) # or a later release

    add_executable(demo main.cpp)
    target_link_libraries(demo PRIVATE errors::errors)
    ```

3.  Include header file like this:

    ``` cpp
    #include "errors/error.hpp"
    ```

4.  Start write codes

### Vendor

If you want to vendor this library to your project, it’s **RECOMMEND**
to use the single-file version header.

1.  Download
    [`errors.hpp`](https://github.com/black-desk/errors/releases/latest/download/errors.hpp)
    from [latest github
    release](https://github.com/black-desk/errors/releases/latest)

2.  Place `errors.hpp` in the include directory of your project

3.  Include header file like this:

    ``` cpp
    #include "errors.hpp"
    ```

4.  Start write codes

## Contributing to `errors`

### Coding Rules

Check coding ruels [here](https://github.com/black-desk/coding-rules)

### Build

**NOTE**: This is the build guide for developers want to build and test
this project.

It is **RECOMMEND** to build and test this project using
[cmake-presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
with the `default` preset.

``` bash
cmake --workflow --preset=default
```

You can use environment variables to customize CMake workflow like this:

``` bash
# Use [ccache](https://ccache.dev/)
export CMAKE_CXX_LAUNCHER=ccache
# Use clang++
export CXX=clang++

# Run clang-tidy
# NOTE:
# This is not the standard CMake environment variable,
# this project support this variable by using this env to initialize
# the CMake cache variable of the same name in preset.
export CMAKE_CXX_CLANG_TIDY=clang-tidy

cmake --workflow --preset=default --fresh
```

The configuration of `default` preset is:

``` json
{
        "version": 6,
        "cmakeMinimumRequired": {
                "major": 3,
                "minor": 25,
                "patch": 0
        },
        "configurePresets": [
                {
                        "name": "default",
                        "displayName": "Default configuration",
                        "description": "The default configuration for developers of `errors`",
                        "binaryDir": "${sourceDir}/build",
                        "cacheVariables": {
                                "CMAKE_CXX_CLANG_TIDY": "$env{CMAKE_CXX_CLANG_TIDY}",
                                "CMAKE_CXX_FLAGS": "-Wall -Wextra -Wpedantic -Werror -O0 -g3 -fsanitize=address,undefined",
                                "CMAKE_EXPORT_COMPILE_COMMANDS": true,
                                "CMAKE_COLOR_DIAGNOSTICS": true
                        }
                },
                {
                        "name": "CI",
                        "inherits": "default",
                        "cacheVariables": {
                                "errors_COVERAGE": true
                        }
                }
        ],
        "buildPresets": [
                {
                        "name": "default",
                        "displayName": "Default build",
                        "description": "Use default configuration to build `errors` for developers.",
                        "configurePreset": "default"
                },
                {
                        "name": "CI",
                        "configurePreset": "CI"
                }
        ],
        "testPresets": [
                {
                        "name": "default",
                        "displayName": "Default tests",
                        "description": "Use default configuration to test `errors` for developers.",
                        "configurePreset": "default",
                        "output": {
                                "outputOnFailure": true
                        },
                        "execution": {
                                "noTestsAction": "error",
                                "stopOnFailure": true
                        }
                },
                {
                        "name": "CI",
                        "inherits": "default",
                        "configurePreset": "CI"
                }
        ],
        "workflowPresets": [
                {
                        "name": "default",
                        "displayName": "Default workflow for developers",
                        "description": "Configure, build then test `errors` for developers.",
                        "steps": [
                                {
                                        "type": "configure",
                                        "name": "default"
                                },
                                {
                                        "type": "build",
                                        "name": "default"
                                },
                                {
                                        "type": "test",
                                        "name": "default"
                                }
                        ]
                },
                {
                        "name": "CI",
                        "displayName": "CI workflow",
                        "description": "Configure, build then test `errors` for CI.",
                        "steps": [
                                {
                                        "type": "configure",
                                        "name": "CI"
                                },
                                {
                                        "type": "build",
                                        "name": "CI"
                                },
                                {
                                        "type": "test",
                                        "name": "CI"
                                }
                        ]
                }
        ]
}
```
