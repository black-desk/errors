# `errors`: a header-only golang-like error wrapping library for c++11

This library aims to bring golang-like error wrapping and handling to
c++11.

------------------------------------------------------------------------

**Table of contents**

- [Documentation](#documentation)
  - [Tutorial](#tutorial)
    - [Basic usage](#basic-usage)
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

------------------------------------------------------------------------

![](https://img.shields.io/github/check-runs/black-desk/errors/master)

# Documentation

The key words “MUST”, “MUST NOT”, “REQUIRED”, “SHALL”, “SHALL NOT”,
“SHOULD”, “SHOULD NOT”, “RECOMMENDED”, “MAY”, and “OPTIONAL” in this
document are to be interpreted as described in RFC 2119.

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
