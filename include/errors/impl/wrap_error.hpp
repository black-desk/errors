#pragma once

#include <string>

#include "errors/impl/error_with_cause.hpp"

namespace errors
{

namespace impl
{

class wrap_error : public error_with_cause {
    public:
        wrap_error(error_ptr &&cause, source_location loc)
                : error_with_cause(std::move(cause), std::move(loc))
        {
        }

        wrap_error(std::string msg, error_ptr &&cause, source_location loc)
                : error_with_cause(std::move(cause), std::move(loc))
                , message(std::move(msg))
        {
        }

        const char *what() const noexcept override
        {
                return this->message.has_value() ? this->message->c_str() : "";
        }

    private:
        std::optional<std::string> message;
};
static_assert(!std::is_abstract<wrap_error>());

}
}
