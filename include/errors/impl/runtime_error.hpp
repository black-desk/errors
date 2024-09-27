#pragma once

#include <string>

#include "errors/impl/base_error.hpp"

namespace errors
{

namespace impl
{

class runtime_error : public base_error {
    public:
        runtime_error(std::string msg, source_location loc)
                : base_error(std::move(loc))
                , message(std::move(msg))
        {
        }

        const char *what() const noexcept override
        {
                return this->message.c_str();
        }

        std::string message;
};
static_assert(!std::is_abstract<runtime_error>());

}
}
