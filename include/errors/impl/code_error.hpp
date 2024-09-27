#pragma once

#include <cassert>
#include <cstring>
#include <string>

#include "errors/impl/runtime_error.hpp"

namespace errors
{

namespace impl
{

template <typename Code>
class code_error : public runtime_error {
    public:
        code_error(const std::string &message, Code code,
                   source_location location)
                : runtime_error(message, std::move(location))
                , code(code)
        {
                this->message += " [code=" + std::to_string(code) + "]";
        }

        const char *what() const noexcept override
        {
                return this->message.c_str();
        }

        Code code;
};

static_assert(!std::is_abstract<code_error<unsigned int>>());
}
}
