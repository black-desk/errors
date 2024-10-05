#pragma once

#include <memory>
#include <utility>

#include "errors/error.hpp"

namespace errors
{

/// @brief
/// Smart pointer to ::errors::error with some utility member functions
class error_ptr : public std::unique_ptr<error> {
    public:
        using std::unique_ptr<error>::unique_ptr;

        /// @brief
        /// Check if the error is caused by
        /// some kind of ::errors::error recursively.
        /// @tparam E
        /// Type of error to check.
        /// @return
        /// `true` if the error is caused by an instance of error `E`,
        /// `false` otherwise.
        template <typename E>
        [[nodiscard]]
        bool is() const
        {
                static_assert(std::is_base_of_v<error, E>);

                auto current = this->get();
                while (current != nullptr) {
                        if (dynamic_cast<const E *>(current) != nullptr) {
                                return true;
                        }

                        current = current->cause().get();
                }
                return false;
        }

        /// @brief
        /// Looking up error in causes.
        /// @tparam E
        /// Type of error to looking for.
        /// @return
        /// A pointer to `E` if this error is caused by an instance of error `E`,
        /// `nullptr` otherwise.
        /// @details
        /// Examines the cause link list, looking for the first error
        /// which can be referenced by `E`.
        template <typename E>
        [[nodiscard]]
        const E *as() const
        {
                static_assert(std::is_base_of_v<error, E>);

                auto current = this->get();
                while (current != nullptr) {
                        auto result = dynamic_cast<const E *>(current);
                        if (result != nullptr) {
                                return result;
                        }

                        current = current->cause().get();
                }
                return nullptr;
        }

        /// @copydoc as() const
        template <typename E>
        [[nodiscard]]
        E *as()
        {
                return const_cast<E *>(std::as_const(*this).as<E>());
        }
};
static_assert(!std::is_abstract<error_ptr>());
}

#if not defined(ERRORS_DISABLE_OSTREAM)

#include <cassert>
#include <ostream>

inline std::ostream &operator<<(std::ostream &os, const errors::error_ptr &err)
{
        const auto *current = err.get();

        if (!current) {
                os << "no error";
                return os;
        }

        bool printed = false;

        while (current != nullptr) {
                if (printed) {
                        os << ": ";
                        printed = false;
                }

                auto what = current->what();
                assert(what);
                if (what[0] != '\0') {
                        os << what;
                        printed = true;
                }

                current = current->cause().get();
        }

        return os;
}
#endif
