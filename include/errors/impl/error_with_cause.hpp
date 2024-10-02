#pragma once

#include "errors/error.hpp"
#include "errors/error_ptr.hpp"
#include "errors/impl/base_error.hpp"

namespace errors
{
namespace impl
{

/// @brief
/// A base error class.
///
/// @details
/// It implements the cause method
/// for all other builtin error types
/// which have a cause to derived from.
class error_with_cause : public base_error {
    public:
        /// @brief
        /// Constructor with source_location.
        ///
        /// @param loc
        /// The source location.
        ///
        /// @param cause
        /// The cause of the error.
        error_with_cause(error_ptr &&cause, source_location loc)
                : base_error(std::move(loc))
                , cause_(std::move(cause))
        {
        }

        const error_ptr &cause() const & noexcept override
        {
                return this->cause_;
        }

        error_ptr cause() && noexcept override
        {
                return std::move(this->cause_);
        }

    private:
        error_ptr cause_;
};
static_assert(std::is_abstract<error_with_cause>());

}
}
