#pragma once

#include "errors/error.hpp"
#include "errors/error_ptr.hpp"

namespace errors
{

template <typename E>
[[nodiscard]]
bool is(const error &err) noexcept
{
        auto current = &err;
        while (current != nullptr) {
                if (dynamic_cast<const E *>(current) != nullptr) {
                        return true;
                }

                current = current->cause().get();
        }
        return false;
}

template <typename E>
[[nodiscard]]
const E *as(const error &err) noexcept
{
        auto current = &err;
        while (current != nullptr) {
                auto result = dynamic_cast<const E *>(current);
                if (result != nullptr) {
                        return result;
                }

                current = current->cause().get();
        }
        return nullptr;
}

}
