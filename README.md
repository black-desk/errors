# `errors`: a header-only golang-like error wrapping library for c++11

This library aims to bring golang-like error wrapping and handling to c++11.

![](https://img.shields.io/github/check-runs/black-desk/errors/master)

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD",
"SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be
interpreted as described in RFC 2119.

## Table of Contents

- [Tutorial](#tutorial)
  - [Basic usage](#basic-usage)
  - [Use `errors` with `expected`](#use-errors-with-expected)
  - [Handle error with code](#handle-error-with-code)
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

## Tutorial

### Basic usage

``` cpp
#include <filesystem>
#include <iostream>

#include "errors/error.hpp"

using errors::common_error;
using errors::error_ptr;
using errors::make_error;
using errors::wrap;

// NOTE:
// If you have a function which might goes wrong,
// but you don't want to or can not to throw an exception,
// you can return an error_ptr.
error_ptr fn() noexcept
{
        return make_error<common_error>("error occurs");
};

// NOTE:
// You can return an wrapped error_ptr using the `wrap` function.
error_ptr fn2() noexcept
{
        return wrap(fn());
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
        std::cerr << "Error function: " << loc.function_name() << std::endl
                  << "Error file_name: " << loc.file_name() << std::endl
                  << "Error line: " << loc.line() << std::endl
                  << "Error column: " << loc.column() << std::endl;

        // NOTE:
        // Or use json output.
#if defined(ERRORS_ENABLE_NLOHMANN_JSON_SUPPORT)
        std::cerr << "Error in JSON:" << std::endl
                  << nlohmann::json(err) << std::endl;
#endif

        return 0;
}
```

### Use `errors` with `expected`

``` cpp
#include <iostream>

#include "errors/error.hpp"
#include "tl/expected.hpp"

using errors::common_error;
using errors::error_ptr;
using errors::make_error;
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
                return unexpected(make_error<common_error>("underflow"));
        }

        return data[--top];
}

error_ptr stack_t::push(int value) noexcept
{
        if (top == MAX_SIZE) {
                return make_error<common_error>("overflow");
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

### Handle error with code

``` cpp
#include <iostream>

#include "errors/error.hpp"
#include "tl/expected.hpp"

using errors::common_error;
using errors::error_ptr;
using errors::make_error;
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
class stack_error_t : public errors::base_error {
    public:
        // NOTE:
        // An error context class records
        // the machine friendly error context information.
        // It can be used to generate developer and
        // user friendly error message later.
        struct context_t : public errors::context {
                context_t(int top, errors::source_location location =
                                           errors::source_location::current())
                        : ::errors::context(location)
                        , top(top)
                {
                }
                int top;
        };

        stack_error_t(context_t context, error_ptr &&cause = nullptr)
                : base_error(std::move(cause))
                , context(std::move(context))
        {
        }

        const errors::source_location &location() const override
        {
                return this->context.location;
        }

        // NOTE:
        // This method is used to generate error messages
        // meant to be printed in logs,
        // which should be developer friendly.
        std::optional<std::string> what() const override
        {
                std::string result;
                result.append("stack error [top=");
                result.append(std::to_string(context.top));
                result.append("]");
                return result;
        }

        context_t context;
};

expected<int, error_ptr> stack_t::pop() noexcept
{
        if (top == 0) {
                return unexpected(make_error<stack_error_t>(top));
        }

        return data[--top];
}

error_ptr stack_t::push(int value) noexcept
{
        if (top == MAX_SIZE) {
                return make_error<stack_error_t>(top);
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

        if (!err->is<stack_error_t>()) {
                return;
        }

        std::cout << "Stack error occurs" << std::endl;

        auto stack_error = err->as<stack_error_t>();
        assert(stack_error != nullptr);

        if (stack_error->context.top == 0) {
                std::cout << "Stack underflow" << std::endl;
                return;
        }

        if (stack_error->context.top == stack_t::MAX_SIZE) {
                std::cout << "Stack overflow" << std::endl;
                return;
        }

        std::cout << "Unknown stack error" << std::endl;
}

error_ptr fn(stack_t &stack)
{
        auto value = stack.pop();
        assert(!value);
        return errors::wrap(
                errors::wrap(errors::wrap(std::move(value).error())),
                "something goes wrong");
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

        err = make_error<stack_error_t>(1);
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
#include "errors/error.hpp"

std::ostream &operator<<(std::ostream &os, const errors::error_ptr &err)
{
        auto current_err = err.get();
        if (!current_err) {
                os << "success";
                return os;
        }

        for (; current_err != nullptr;
             current_err = current_err->cause().get()) {
                os << std::endl
                   << "[function " << current_err->location().function_name()
                   << " at "
                   << std::filesystem::path(current_err->location().file_name())
                                .filename()
                                .string()
                   << " " << current_err->location().line() << ":"
                   << current_err->location().column() << "] "
                   << current_err->what().value_or("");
        }
        return os;
}

int main()
{
        using errors::common_error;
        using errors::make_error;
        using errors::wrap;

        std::cerr << wrap(wrap(make_error<common_error>("error"))) << std::endl;

        return 0;
}
```

#### Customize output format of error locally

``` cpp
#include <filesystem>
#include <iostream>

#include "errors/error.hpp"

using errors::common_error;
using errors::make_error;
using errors::wrap;

namespace local_ns
{
// NOTE:
// This is a custom operator for printing the error in a more verbose format.
// But it is not going to
// override the default operator<< outside of this namespace.
std::ostream &operator<<(std::ostream &os, const errors::error_ptr &err)
{
        auto current_err = err.get();
        if (!current_err) {
                os << "success";
                return os;
        }

        for (; current_err != nullptr;
             current_err = current_err->cause().get()) {
                os << std::endl
                   << "[function " << current_err->location().function_name()
                   << " at "
                   << std::filesystem::path(current_err->location().file_name())
                                .filename()
                                .string()
                   << " " << current_err->location().line() << ":"
                   << current_err->location().column() << "] "
                   << current_err->what().value_or("");
        }
        return os;
}
void print_error_in_local_ns()
{
        // Using the custom operator<<
        std::cerr << wrap(wrap(make_error<common_error>("error"))) << std::endl;
}
}

void print_error_in_global_ns()
{
        using errors::common_error;
        using errors::make_error;
        using errors::wrap;

        // Using the default operator<<
        std::cerr << wrap(wrap(make_error<common_error>("error"))) << std::endl;
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
    CPMFindPackage("gh:black-desk/errors@0.1.0") # or a later release
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
    include(FetchContent)

    FetchContent_Declare(
      errors
      GIT_REPOSITORY https://github.com/black-desk/errors.git
      GIT_TAG        v0.1.0 # or a later release
    )

    FetchContent_MakeAvailable(errors)
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
    find_package( errors 0.1.0 REQUIRED) # or a later release

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
