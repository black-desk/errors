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
        std::cerr << wrap(wrap(make_error("error"))) << std::endl;
}
}

void print_error_in_global_ns()
{
        using errors::common_error;
        using errors::make_error;
        using errors::wrap;

        // Using the default operator<<
        std::cerr << wrap(wrap(make_error("error"))) << std::endl;
}

int main()
{
        print_error_in_global_ns();
        local_ns::print_error_in_local_ns();

        return 0;
}
