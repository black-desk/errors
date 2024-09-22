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
        base_error(
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                source_location location,
#endif
                error_ptr &&cause)
                : cause_(std::move(cause))
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                , location_(std::move(location))
#endif
        {
        }
        const std::unique_ptr<error> &cause() const override
        {
                return this->cause_;
        }

#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
        const source_location &location() const override
        {
                return this->location_;
        }
#endif

    private:
        error_ptr cause_;
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
        source_location location_;
#endif
};

class common_error : public base_error, public virtual error {
    public:
        common_error(
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                source_location location,
#endif
                error_ptr &&cause, const char *message)
                : base_error(
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                          std::move(location),
#endif
                          std::move(cause))

        {
                if (!message) {
                        return;
                }
                this->message_ = message;
        }

        common_error(
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                source_location location,
#endif
                error_ptr &&cause, std::string message)
                : base_error(
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                          std::move(location),
#endif
                          std::move(cause))
                , message_(std::move(message))
        {
        }

        std::optional<std::string> what() const override
        {
                return this->message_;
        }

    private:
        std::optional<std::string> message_;
};

// NOTE:
// This is a helper struct to automatically capture the source_location
// for the caller of make_error with implicit conversion
// from T or nullptr_t to capture_location<error_ptr>.
// This trick inspired by
// answers from https://stackoverflow.com/a/57548488
// and https://stackoverflow.com/a/66402319.
// See https://stackoverflow.com/questions/57547273/how-to-use-source-location-in-a-variadic-template-function
template <typename T>
struct capture_location {
        T value;
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
        source_location location;
        capture_location(std::nullptr_t value,
                         source_location location = source_location::current())
                : value(std::move(value))
                , location(std::move(location))
        {
        }
        capture_location(T value,
                         source_location location = source_location::current())
                : value(std::move(value))
                , location(std::move(location))
        {
        }
#else
        capture_location(std::nullptr_t value)
                : value(value)
        {
        }
        capture_location(T &&value)
                : value(std::move(value))
        {
        }
#endif
};

template <typename E, typename... Args>
inline error_ptr make_error(capture_location<error_ptr> &&cause, Args &&...args)
{
        return std::make_unique<E>(
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                std::move(cause.location),
#endif
                std::move(cause.value), std::forward<Args>(args)...);
}

inline error_ptr make_error(capture_location<const char *> message)
{
        return std::make_unique<common_error>(
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                std::move(message.location),
#endif
                nullptr, message.value);
}

inline error_ptr wrap(capture_location<error_ptr> &&cause,
                      const char *message = nullptr)
{
        return make_error<common_error>(std::move(cause), message);
}

inline error_ptr wrap(capture_location<error_ptr> &&cause, std::string message)
{
        return make_error<common_error>(std::move(cause), std::move(message));
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
