#include <filesystem>
#include <iostream>

#include "errors/error.hpp"

using errors::make_error;
using errors::wrap;

namespace local_ns
{
// NOTE:
// This is a custom operator for printing the error in a more verbose format.
// But it is not going to
// override the default operator<< outside of this namespace.
inline std::ostream &operator<<(std::ostream &os, const errors::error_ptr &err)
{
        auto current = err.get();

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

                auto current_with_cause =
                        dynamic_cast<const errors::with_cause *>(current);
                if (!current_with_cause) {
                        break;
                }

                current = current_with_cause->cause().get();
        }

        return os;
}

void print_error_in_local_ns()
{
        // Using the custom operator<<
        std::cerr << "Error: " << wrap(wrap(make_error("error"))) << std::endl;
}
}

void print_error_in_global_ns()
{
        using errors::make_error;
        using errors::wrap;

        // Using the default operator<<
        std::cerr << "Error: " << wrap(wrap(make_error("error"))) << std::endl;
}

int main()
{
        print_error_in_global_ns();
        local_ns::print_error_in_local_ns();

        return 0;
}
