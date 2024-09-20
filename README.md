# A header only golang-like error wrapping library for c++11

![](https://img.shields.io/github/check-runs/black-desk/errors/master)

This library aims to bring golang-like error wrapping and handling to
c++11.

## Install

### CMake

#### [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)

1.  Include CPM.cmake to your project

2.  Use CPM.cmake to add this library:

    ``` cmake
    CPMFindPackage("gh:black-desk/errors@0.1.0")
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

### Install to system

1.  Install from source:

    ``` bash
    git clone https://github.com/black-desk/errors && \
      cd errors && \
      cmake -B build && \
      cmake --build build \
      && cmake --install build
    ```

2.  Include header file like this:

    ``` cpp
    #include "errors/error.hpp"
    ```

3.  Start write codes

After install to system, you can also use `find_package` of CMake, then
link this library to your target like this:

``` cmake
find_package(
  errors 0.1.0 # or a later release
  REQUIRED)

add_executable(demo main.cpp)
target_link_libraries(demo PRIVATE errors::errors)
```

### Single header mode

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
