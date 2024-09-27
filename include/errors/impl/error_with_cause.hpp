#pragma once

#include "errors/error.hpp"
#include "errors/impl/base_error.hpp"

namespace errors
{
namespace impl
{

class error_with_cause : public base_error {
    public:
        error_with_cause(error_ptr &&cause, source_location loc)
                : base_error(std::move(loc))
                , cause_(std::move(cause))
        {
        }

        const std::unique_ptr<error> &cause() const & override
        {
                return this->cause_;
        }

        std::unique_ptr<error> cause() && override
        {
                return std::move(this->cause_);
        }

    private:
        error_ptr cause_;
};
static_assert(std::is_abstract<error_with_cause>());

}
}
