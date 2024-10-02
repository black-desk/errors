
#pragma once

#include <cassert>

#include "errors/impl/error_without_cause.hpp"

namespace errors
{
namespace impl
{

/// @brief
/// An error that has an exception.
///
/// @details
/// This class is used to create an error from an exception.
class exception_error : public error_without_cause {
    public:
        /// @brief
        /// Create an exception error from an exception pointer.
        ///
        /// @param exception
        /// The exception pointer.
        ///
        /// @param location
        /// The source location.
        ///
        /// @details
        /// The exception pointer is stored in the error.
        exception_error(std::exception_ptr exception, source_location location)
                : error_without_cause(std::move(location))
                , exception_ptr(std::move(exception))
        {
        }

        /// @brief
        /// Create an exception error from the current exception.
        ///
        /// @param location
        /// The source location.
        exception_error(source_location location)
                : error_without_cause(std::move(location))
                , exception_ptr(std::current_exception())
        {
        }

        /// @brief
        /// Get the exception message.
        ///
        /// @return
        /// The exception message.
        /// If the exception is not set, an empty string is returned.
        const char *what() const noexcept override
        {
                try {
                        if (!this->exception_ptr) {
                                return "";
                        }
                        std::rethrow_exception(this->exception_ptr);
                } catch (const std::exception &e) {
                        return e.what();
                } catch (...) {
                        return "Unknown exception";
                }
        }

        /// @brief
        /// The exception pointer.
        std::exception_ptr exception_ptr;
};
static_assert(!std::is_abstract<exception_error>());

}
}
