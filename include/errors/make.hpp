#pragma once

#include <memory>

#include "errors/error.hpp"

namespace errors
{

template <typename E>
class make {
    public:
        template <typename... Args>
        struct with : public error_ptr {
                with(Args... args,
                     source_location location = source_location::current())
                        : error_ptr(std::make_unique<E>(std::move(args)...,
                                                        std::move(location)))
                {
                }
        };

        template <typename... Args>
        with(Args...) -> with<Args...>;

    private:
        make() = default;
};

}
