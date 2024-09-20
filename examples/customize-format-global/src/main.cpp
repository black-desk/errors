#include <filesystem>
#include <iostream>

// If you want to disable the default ostream support
// to customize it for your own program,
// please define ERRORS_DISABLE_OSTREAM
// before include any header files from `errors`
#define ERRORS_DISABLE_OSTREAM
#include "errors/error.hpp"

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

int main()
{
        using errors::common_error;
        using errors::make_error;
        using errors::wrap;

        std::cerr << wrap(wrap(make_error<common_error>("error"))) << std::endl;

        return 0;
}
