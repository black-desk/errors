
#pragma once

#include <cassert>

#include "errors/impl/base_error.hpp"

namespace errors
{

namespace impl
{
class exception_error : public base_error {
    public:
        exception_error(std::exception_ptr exception, source_location location)
                : base_error(std::move(location))
                , exception_ptr(std::move(exception))
        {
        }

        exception_error(source_location location)
                : base_error(std::move(location))
                , exception_ptr(std::current_exception())
        {
        }

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

        std::exception_ptr exception_ptr;
};
static_assert(!std::is_abstract<exception_error>());
}
}
