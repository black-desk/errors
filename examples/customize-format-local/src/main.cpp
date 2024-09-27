#include <filesystem>
#include <iostream>

#include "errors/errors.hpp"

using errors::impl::runtime_error;
using errors::wrap;

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
