# Basic usage

This library defined an abstract class `errors::error`
(see [header file](../../include/errors/error.hpp))
to represent an error,
which has some method to display error message
and the source_location where this error is created.

A function or method want to return an error
should return an `errors::error_ptr`,
which is a `std::unique_ptr<error>`.

```cpp{include=./src/main.cpp}
// See ./src/main.cpp
```
