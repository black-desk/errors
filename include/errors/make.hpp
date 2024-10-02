#pragma once

#include <memory>

#include "errors/error_ptr.hpp"
#include "errors/source_location.hpp"

namespace errors
{
namespace detail
{
/// @cond
template <typename E>
class make {
    private:
        make() = delete;
        make(const make &) = delete;
        make(make &&) = delete;
        make &operator=(const make &) = delete;
        make &operator=(make &&) = delete;

    public:
        template <typename... Args>
        struct with : public error_ptr {
                with(Args... args)
                        : error_ptr(std::make_unique<E>(std::move(args)...))
                {
                }
        };

        /// @cond
        // See https://www.cppstories.com/2021/non-terminal-variadic-args/
        template <typename... Args>
        with(Args...) -> with<Args...>;
        /// @endcond
};
/// @endcond
}

/// @see ::errors::make::with
template <typename E>
class make {
    private:
        make() = delete;
        make(const make &) = delete;
        make(make &&) = delete;
        make &operator=(const make &) = delete;
        make &operator=(make &&) = delete;

    public:
        /// @see with()
        template <typename... Args>
        struct with : public error_ptr {
                /// @brief
                /// Create an error.
                ///
                /// @details
                /// This class and this constructor
                /// is used to quickly create an error.
                /// It captures the source location from where it is called,
                /// and passes it as the last argument
                /// with anything you pass to the error constructor.
                with(Args... args,
                     source_location location = source_location::current())
                        : error_ptr(static_cast<error_ptr>(
                                  typename detail::make<E>::with(
                                          std::move(args)...,
                                          std::move(location))))
                {
                }
        };
        /// @example examples/basic-usage/src/main.cpp

        /// @cond
        // See https://www.cppstories.com/2021/non-terminal-variadic-args/
        template <typename... Args>
        with(Args...) -> with<Args...>;
        /// @endcond
};

}
