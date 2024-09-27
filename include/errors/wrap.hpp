
#include <memory>

#include "errors/impl/wrap_error.hpp"
#include "errors/source_location.hpp"

namespace errors
{
class wrap : public error_ptr {
    public:
        wrap(std::string message, error_ptr &&cause,
             source_location location = source_location::current())
                : error_ptr(cause != nullptr ?
                                    std::make_unique<impl::wrap_error>(
                                            message, std::move(cause),
                                            std::move(location)) :
                                    nullptr)
        {
        }

        wrap(error_ptr &&cause,
             source_location location = source_location::current())
                : error_ptr(cause != nullptr ?
                                    std::make_unique<impl::wrap_error>(
                                            std::move(cause),
                                            std::move(location)) :
                                    nullptr)
        {
        }
};
}
