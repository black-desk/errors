#pragma once

#include <string>

#include "errors/impl/error_with_cause.hpp"

namespace errors
{
namespace impl
{

/// @brief
/// An error which wraps another error.
///
/// @details
/// This error is used to wrap another error with an optional message.
/// If the message is not provided,
/// the only extra information that
/// this error will records
/// is the source location.
///
/// @see ::errors::wrap
class wrap_error : public error_with_cause {
    public:
        /// @brief
        /// Constructor with cause and source_location.
        ///
        /// @param cause
        /// The cause of the error.
        ///
        /// @param loc
        /// The source location.
        wrap_error(error_ptr &&cause, source_location loc)
                : error_with_cause(std::move(cause), std::move(loc))
        {
        }

        /// @brief
        /// Constructor with message, cause and source_location.
        ///
        /// @param msg
        /// The error message.
        ///
        /// @param cause
        /// The cause of the error.
        ///
        /// @param loc
        /// The source location.
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
