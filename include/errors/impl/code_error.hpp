#pragma once

#include <cassert>
#include <cstring>
#include <string>

#include "errors/impl/runtime_error.hpp"

namespace errors
{

namespace impl
{

/// @brief
/// An error that has a code.
///
/// @tparam Code
/// The type of the code.
///
/// @details
/// This class is used to create an error that has a code.
template <typename Code>
class code_error : public runtime_error {
    public:
        /// @brief
        /// Constructor with message, code and source_location.
        ///
        /// @param message
        /// The message.
        ///
        /// @param code
        /// The code.
        ///
        /// @param location
        /// The source location.
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

        /// @brief
        /// The error code.
        Code code;
};

static_assert(!std::is_abstract<code_error<unsigned int>>());
}
}
