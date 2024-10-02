#pragma once

#include <optional>

#include "errors/detail/interface.hpp"

namespace errors
{

class error_ptr;
struct source_location;

/// @brief
/// Interface representing an error.
///
/// @details
/// This interface class represents an error
/// with an optional source location where it was generated
/// and an optional cause.
class error : public virtual detail::interface {
    public:
        /// @brief
        /// Get error message.
        ///
        /// @return
        /// A c-style string,
        /// which is an error message.
        /// It should never be `NULL`,
        /// but it can be an empty string.
        ///
        /// @details
        ///
        /// This method works just like `std::exception::what`,
        /// you should print all the context
        /// which is useful for debugging in this method
        /// except for the source location.
        [[nodiscard]]
        virtual const char *what() const noexcept = 0;
        /// @brief
        /// Get cause of this error,
        /// if any.
        ///
        /// @return
        /// Reference to a smart pointer
        /// which holds the cause of current error,
        /// or a `nullptr` if there is no cause.
        [[nodiscard]]
        virtual const error_ptr &cause() const & noexcept = 0;
        /// @brief
        /// Transfer the ownership of
        /// cause of this error,
        /// if any.
        ///
        /// @return
        /// A smart pointer holds the cause of current error,
        /// or a `nullptr` if there is no cause
        ///
        /// @details
        ///
        /// This method is used to transfer the ownership of the cause,
        /// **use with caution**.
        [[nodiscard]]
        virtual error_ptr cause() && noexcept = 0;
        /// @brief
        /// Get the source location of this error, if any.
        ///
        /// @return
        /// An optional source location.
        ///
        /// @details
        ///
        /// This method returns the source location
        /// where this error is generated,
        /// if it is available.
        [[nodiscard]]
        virtual std::optional<source_location> location() const noexcept = 0;
};
static_assert(std::is_abstract<error>());
}
