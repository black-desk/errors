
#include <memory>

#include "errors/impl/wrap_error.hpp"
#include "errors/make.hpp"
#include "errors/source_location.hpp"

namespace errors
{

/// @brief
/// A function like class to wrap an error.
/// @see wrap()
class wrap : public error_ptr {
    public:
        /// @brief
        /// Wraps an error with an optional message.
        ///
        /// @details
        /// This class is used to quickly wrap an error with an optional message.
        /// It is equivalent to `errors::make<wrap_error>::with(...)`.
        ///
        /// @see ::errors::impl::wrap_error
        /// @see ::errors::make::with
        wrap(std::string message, error_ptr &&cause,
             source_location location = source_location::current())
                : error_ptr(
                          cause != nullptr ?
                                  static_cast<error_ptr>(
                                          detail::make<impl::wrap_error>::with(
                                                  std::move(message),
                                                  std::move(cause),
                                                  std::move(location))) :
                                  nullptr)
        {
        }

        /// @brief
        /// Wraps an error.
        wrap(error_ptr &&cause,
             source_location location = source_location::current())
                : error_ptr(
                          cause != nullptr ?
                                  static_cast<error_ptr>(
                                          detail::make<impl::wrap_error>::with(
                                                  std::move(cause),
                                                  std::move(location))) :
                                  nullptr)
        {
        }
};
}
