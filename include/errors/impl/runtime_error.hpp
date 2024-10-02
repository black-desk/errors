#pragma once

#include <string>

#include "errors/impl/error_without_cause.hpp"

namespace errors
{

namespace impl
{

/// @brief
/// An error that has a message.
///
/// @details
/// This class is used to create an error that has a message.
class runtime_error : public error_without_cause {
    public:
        /// @brief
        /// Constructor with message and source_location.
        ///
        /// @param msg
        /// The message.
        ///
        /// @param loc
        /// The source location.
        runtime_error(std::string msg, source_location loc)
                : error_without_cause(std::move(loc))
                , message(std::move(msg))
        {
        }

        const char *what() const noexcept override
        {
                return this->message.c_str();
        }

        /// @brief
        /// The error message.
        std::string message;
};
static_assert(!std::is_abstract<runtime_error>());

}
}
