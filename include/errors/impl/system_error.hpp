#pragma once

#include <cassert>
#include <cstring>
#include <string>

#include "errors/impl/code_error.hpp"

namespace errors
{

namespace impl
{

class system_error : public code_error<int> {
    public:
        system_error(const std::string &message, int code,
                     source_location location)
                : code_error(message + ": " + strerror(code), code,
                             std::move(location))
        {
        }

        system_error(const std::string &message, source_location location)
                : code_error(message + ": " + strerror(errno), errno,
                             std::move(location))
        {
        }
};

static_assert(!std::is_abstract<code_error<unsigned int>>());
}
}
