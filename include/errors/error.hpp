#pragma once

#include <memory>
#include <optional>
#include <utility>

#include "errors/detail/interface.hpp"
#include "errors/source_location.hpp"

namespace errors
{

class error;

template <typename E>
[[nodiscard]]
bool is(const error &err) noexcept;

template <typename E>
[[nodiscard]]
const E *as(const error &err) noexcept;

class error_ptr;

class error : public virtual detail::interface {
    public:
        [[nodiscard]]
        virtual const char *what() const noexcept = 0;
        [[nodiscard]]
        virtual const error_ptr &cause() const & = 0;
        [[nodiscard]]
        virtual error_ptr cause() && = 0;
        [[nodiscard]]
        virtual std::optional<source_location> location() const = 0;
};
static_assert(std::is_abstract<error>());

class error_ptr : public std::unique_ptr<error> {
    public:
        using std::unique_ptr<error>::unique_ptr;

        [[nodiscard]]
        const char *what() const noexcept
        {
                return this->get()->what();
        }

        [[nodiscard]]
        const error_ptr &cause() const &
        {
                return this->get()->cause();
        }

        [[nodiscard]]
        error_ptr cause() &&
        {
                return std::move(**this).cause();
        }

        [[nodiscard]]
        std::optional<source_location> location() const
        {
                return this->get()->location();
        }

        template <typename E>
        [[nodiscard]]
        bool is() const
        {
                return ::errors::is<E>(**this);
        }

        template <typename E>
        [[nodiscard]]
        const E *as() const
        {
                return ::errors::as<E>(**this);
        }

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

#if defined(ERRORS_ENABLE_NLOHMANN_JSON_SUPPORT)
#include "nlohmann/json.hpp"
#if defined(NLOHMANN_JSON_NAMESPACE_BEGIN) && \
        defined(NLOHMANN_JSON_NAMESPACE_END)
NLOHMANN_JSON_NAMESPACE_BEGIN
#else
namespace nlohmann
{
#endif
template <>
struct adl_serializer< ::errors::error_ptr> {
        static void to_json(::nlohmann::json &j, const ::errors::error_ptr &err)
        {
                if (!err) {
                        j = {};
                        assert(j.is_null());
                        return;
                }

                if (err->location()) {
                        j["location"] = err->location().value();
                }

                j["message"] = err->what();

                j["caused_by"] = err->cause();
        }
};
#if defined(NLOHMANN_JSON_NAMESPACE_BEGIN) && \
        defined(NLOHMANN_JSON_NAMESPACE_END)
NLOHMANN_JSON_NAMESPACE_END
#else
}
#endif
#endif
