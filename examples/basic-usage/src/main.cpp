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
