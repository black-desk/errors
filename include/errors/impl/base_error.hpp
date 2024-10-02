#pragma once

#include "errors/error.hpp"
#include "errors/source_location.hpp"
namespace errors
{
namespace impl
{

/// @brief
/// A base error class.
///
/// @details
/// It implements the location method
/// for all other builtin error types to derived from.
class base_error : public virtual error {
    public:
        /// @brief
        /// Constructor with source_location.
        ///
        /// @param loc
        /// The source location.
        base_error(source_location loc)
                : loc(std::move(loc))
        {
        }

        std::optional<source_location> location() const noexcept override
        {
                return this->loc;
        }

    private:
        source_location loc;
};
static_assert(std::is_abstract<base_error>());

}
}
