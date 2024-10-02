#pragma once

#include <cassert>
#include <cstring>
#include <string>

#include "errors/impl/code_error.hpp"

namespace errors
{
namespace impl
{

/// @brief
/// An error that has a system error code.
class system_error : public code_error<int> {
    public:
        /// @brief
        /// Create a system error from a message and a code.
        /// The message will be pretended to the error message from `strerror`.
        ///
        /// @param message
        /// The extra message.
        ///
        /// @param code
        /// The error code.
        ///
        /// @param location
        /// The source location.
        system_error(const std::string &message, int code,
                     source_location location)
                : code_error(message + ": " + strerror(code), code,
                             std::move(location))
        {
        }

        /// @brief
        /// Create a system error from a code.
        /// The message will be set to the error message from `strerror`.
        ///
        /// @param code
        /// The error code.
        ///
        /// @param location
        /// The source location.
        system_error(int code, source_location location)
                : code_error(strerror(code), code, std::move(location))
        {
        }

        /// @brief
        /// Create a system error from errno.
        /// The message will be pretended to the error message from `strerror`.
        ///
        /// @param message
        /// The extra message.
        ///
        /// @param location
        /// The source location.
        system_error(const std::string &message, source_location location)
                : code_error(message + ": " + strerror(errno), errno,
                             std::move(location))
        {
        }

        /// @brief
        /// Create a system error from errno.
        /// The message will be set to the error message from `strerror`.
        ///
        /// @param location
        /// The source location.
        system_error(source_location location)
                : code_error(strerror(errno), errno, std::move(location))
        {
        }
};

static_assert(!std::is_abstract<code_error<unsigned int>>());
}
}
