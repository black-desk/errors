#include <filesystem>
#include <iostream>

// If you want to disable the default ostream support
// to customize it for your own program,
// please define ERRORS_DISABLE_OSTREAM
// before include any header files from `errors`
#define ERRORS_DISABLE_OSTREAM
#include "errors/error.hpp"

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

int main()
{
        using errors::make_error;
        using errors::message_error;
        using errors::wrap;

        std::cerr << "Error: "
                  << wrap(wrap(make_error<message_error>(nullptr, "error")))
                  << std::endl;

        return 0;
}
