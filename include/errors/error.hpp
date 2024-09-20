#pragma once
#include <cassert>
#include <memory>
#include <optional>
#include <ostream>
#include <string>

#if not defined(ERRORS_SINGLE_HEADER_FILE)
#include "errors/config.hpp"
#include "errors/version.hpp"
#endif

#if defined(ERRORS_ENABLE_SOURCE_LOCATION) and \
        not defined(ERRORS_SINGLE_HEADER_FILE)
#include "errors/source_location.hpp"
#endif

namespace errors
{

struct context {
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
        context(source_location location = source_location::current())
                : location{ location }
        {
        }
        source_location location;
#endif
};

class error {
    public:
        error() = default;
        error(error &&) = delete;
        error(const error &) = delete;
        error &operator=(error &&) = delete;
        error &operator=(const error &) = delete;
        virtual ~error() = default;

#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
        virtual const source_location &location() const = 0;
#endif
        virtual std::optional<std::string> what() const = 0;
        virtual const std::unique_ptr<error> &cause() const = 0;

        template <typename E>
        bool is() const
        {
                for (auto current = this; current != nullptr;
                     current = current->cause().get()) {
                        if (dynamic_cast<const E *>(current) == nullptr) {
                                continue;
                        }
                        return true;
                }
                return false;
        }

        template <typename E>
        const E *as() const
        {
                for (auto current = this; current != nullptr;
                     current = current->cause().get()) {
                        if (dynamic_cast<const E *>(current) == nullptr) {
                                continue;
                        }
                        return dynamic_cast<const E *>(current);
                }
                return nullptr;
        }
};
using error_ptr = std::unique_ptr<error>;

class base_error : public virtual error {
    public:
        base_error(error_ptr &&cause = nullptr)
                : cause_(std::move(cause))
        {
        }
        const std::unique_ptr<error> &cause() const override
        {
                return this->cause_;
        }

    private:
        error_ptr cause_;
};

class common_error : public base_error, public virtual error {
    public:
        struct context_t : public errors::context {
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                context_t(char const *message,
                          source_location location = source_location::current())
                        : errors::context{ location }
                {
                        if (message != nullptr) {
                                this->message = message;
                        }
                }
                context_t(std::string message,
                          source_location location = source_location::current())
                        : errors::context{ location }
                        , message{ std::move(message) }
                {
                }
#else
                context_t(char const *message)
                        : errors::context{}
                {
                        if (message != nullptr) {
                                this->message = message;
                        }
                }
                context_t(std::string message)
                        : errors::context{}
                        , message{ std::move(message) }
                {
                }
#endif
                std::optional<std::string> message;
        };

        common_error(context_t context, error_ptr &&cause = nullptr)
                : base_error(std::move(cause))
                , context(std::move(context))
        {
        }

        std::optional<std::string> what() const override
        {
                return this->context.message;
        }

#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
        const source_location &location() const override
        {
                return this->context.location;
        }
#endif

    private:
        context_t context;
};

template <typename E, typename... Args>
inline error_ptr make_error(typename E::context_t ctx, Args &&...args)
{
        return std::make_unique<E>(std::move(ctx), std::forward<Args>(args)...);
}

inline error_ptr wrap(errors::error_ptr &&cause,
                      common_error::context_t ctx = { nullptr })
{
        return make_error<common_error>(std::move(ctx), std::move(cause));
}

} // namespace errors

#if not defined(ERRORS_DISABLE_OSTREAM)
inline std::ostream &operator<<(std::ostream &os, const errors::error_ptr &err)
{
        bool printed = false;
        for (auto current_err = err.get(); current_err != nullptr;
             current_err = current_err->cause().get()) {
                if (!current_err) {
                        os << "success";
                        return os;
                }
                if (printed) {
                        os << ": ";
                        printed = false;
                }
                if (!current_err->what()) {
                        continue;
                }
                os << *current_err->what();
                printed = true;
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

                j["location"] = err->location();
                if (err->what()) {
                        j["message"] = *err->what();
                }
                if (err->cause()) {
                        j["caused_by"] = err->cause();
                }
        }
};
#if defined(NLOHMANN_JSON_NAMESPACE_BEGIN) && \
        defined(NLOHMANN_JSON_NAMESPACE_END)
NLOHMANN_JSON_NAMESPACE_END
#else
}
#endif
#endif
