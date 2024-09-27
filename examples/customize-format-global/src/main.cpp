#include <filesystem>
#include <iostream>

// If you want to disable the default ostream support
// to customize it for your own program,
// please define ERRORS_DISABLE_OSTREAM
// before include any header files from `errors`
#define ERRORS_DISABLE_OSTREAM
#include "errors/errors.hpp"

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

int main()
{
        using errors::impl::runtime_error;
        using errors::wrap;

        std::cerr << "Error: "
                  << wrap(wrap(errors::make<runtime_error>::with("error")))
                  << std::endl;

        return 0;
}
