#pragma once

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
/// which do not have a cause to derived from.
class error_without_cause : public base_error {
    public:
        using base_error::base_error;

        const error_ptr &cause() const & noexcept override
        {
                static error_ptr null;
                return null;
        }

        error_ptr cause() && noexcept override
        {
                return nullptr;
        }
};
static_assert(std::is_abstract<error_without_cause>());

}
}
