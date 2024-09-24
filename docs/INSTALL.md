# Install

This library is a simple header-only c++11 library,
which means that you have so many ways to make it available to your project.

## CMake

You can use this library via the CMake build system.

### [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)

It's **RECOMMEND** to use this library via CPM.cmake, the CMake package manager.

1. Include CPM.cmake to your project
2. Use CPM.cmake to add this library:

   ```cmake
   CPMFindPackage("gh:black-desk/errors@0.2.0") # or a later release
   ```

3. Link your target with `errors::errors` like this:

   ```cmake
   add_executable(demo main.cpp)
   target_link_libraries(demo PRIVATE errors::errors)
   ```

4. Include header file like this:

   ```cpp
   #include "errors/error.hpp"
   ```

5. Start write codes

### [`FetchContent`](https://cmake.org/cmake/help/latest/module/FetchContent.html)

As CPM.cmake is a wrapper of CMake's `FetchContent` feature,
you can also use this library directly via raw `FetchContent`.

1. Use `FetchContent` to add this library:

   ```cmake
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

2. Link your target with `errors::errors` like this:

   ```cmake
   add_executable(demo main.cpp)
   target_link_libraries(demo PRIVATE errors::errors)
   ```

3. Include header file like this:

   ```cpp
   #include "errors/error.hpp"
   ```

4. Start write codes

### Install to system

You can also use this library in the old way,
which means to install this library into your development environment,
then use it via CMake's `find_package` function.

1. Install from source:

   ```bash
   git clone https://github.com/black-desk/errors && \
     cd errors && \
     cmake -B build && \
     cmake --build build && \
     cmake --install build
   ```

2. Use `find_package` to import this library,
   then link this library to your target like this:

   ```cmake
   find_package(errors 0.2.0 REQUIRED) # or a later release

   add_executable(demo main.cpp)
   target_link_libraries(demo PRIVATE errors::errors)
   ```

3. Include header file like this:

   ```cpp
   #include "errors/error.hpp"
   ```

4. Start write codes

## Vendor

If you want to vendor this library to your project,
it's **RECOMMEND** to use the single-file version header.

1. Download [`errors.hpp`](https://github.com/black-desk/errors/releases/latest/download/errors.hpp)
   from [latest github release](https://github.com/black-desk/errors/releases/latest)

2. Place `errors.hpp` in the include directory of your project

3. Include header file like this:

   ```cpp
   #include "errors.hpp"
   ```

4. Start write codes
