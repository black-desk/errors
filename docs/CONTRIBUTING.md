# Contributing to `errors`

## Coding Rules

Check coding ruels [here](https://github.com/black-desk/coding-rules)

## Build

**NOTE**:
This is the build guide for developers want to build and test this project.

It is **RECOMMEND** to build and test this project using
[cmake-presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
with the `default` preset.

```bash
cmake --workflow --preset=default
```

You can use environment variables to customize CMake workflow like this:

```bash
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

```json {include=../CMakePresets.json}
{ "comments": "See ../CMakePresets.json" }
```
