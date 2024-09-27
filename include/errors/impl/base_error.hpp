#pragma once

#include "errors/error.hpp"
#include "errors/source_location.hpp"
namespace errors
{
namespace impl
{

// A base class for all other builtin error types to derived from,
// which implement the default cause and location method.
class base_error : public virtual error {
    public:
        base_error(source_location loc)
                : loc(std::move(loc))
        {
        }

        const std::unique_ptr<error> &cause() const & override
        {
                static const error_ptr null = nullptr;
                return null;
        }

        std::unique_ptr<error> cause() && override
        {
                return nullptr;
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
