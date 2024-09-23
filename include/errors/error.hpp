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

#if not defined(ERRORS_SINGLE_HEADER_FILE)
#include "errors/source_location.hpp"
#endif

namespace errors
{

class error;

class with_cause {
    public:
        virtual const std::unique_ptr<error> &cause() const & noexcept = 0;
        virtual std::unique_ptr<error> &cause() & noexcept = 0;
};

class with_source_location {
    public:
        virtual const source_location &location() const noexcept = 0;
};

class error : public std::exception {
    public:
        error() = default;
        error(error &&) = delete;
        error(const error &) = delete;
        error &operator=(error &&) = delete;
        error &operator=(const error &) = delete;
        template <typename E>
        bool is() const
        {
                auto current = this;
                while (current != nullptr) {
                        if (dynamic_cast<const E *>(current) != nullptr) {
                                return true;
                        }

                        auto current_with_cause =
                                dynamic_cast<const with_cause *>(current);
                        if (!current_with_cause) {
                                return false;
                        }

                        current = current_with_cause->cause().get();
                }
                return false;
        }

        template <typename E>
        const E *as() const
        {
                auto current = this;
                while (current != nullptr) {
                        auto result = dynamic_cast<const E *>(current);
                        if (result != nullptr) {
                                return result;
                        }

                        auto current_with_cause =
                                dynamic_cast<const with_cause *>(current);
                        if (!current_with_cause) {
                                return nullptr;
                        }

                        current = current_with_cause->cause().get();
                }
                return nullptr;
        }

        template <typename E>
        E *as()
        {
                auto current = this;
                while (current != nullptr) {
                        auto result = dynamic_cast<E *>(current);
                        if (result != nullptr) {
                                return result;
                        }

                        auto current_with_cause =
                                dynamic_cast<const with_cause *>(current);
                        if (!current_with_cause) {
                                return nullptr;
                        }

                        current = current_with_cause->cause().get();
                }
                return nullptr;
        }

        const std::type_info &type()
        {
                return typeid(*this);
        }

        std::optional<std::unique_ptr<error>> cause()
        {
                auto this_with_cause = dynamic_cast<with_cause *>(this);
                if (!this_with_cause) {
                        return std::nullopt;
                }
                return this_with_cause->cause() == nullptr ?
                               std::nullopt :
                               std::optional(
                                       std::move(this_with_cause->cause()));
        }

        std::optional<source_location> location() const
        {
                auto this_with_source_location =
                        dynamic_cast<const with_source_location *>(this);
                if (!this_with_source_location) {
                        return std::nullopt;
                }
                return this_with_source_location->location();
        }
};

using error_ptr = std::unique_ptr<error>;

class base_error :
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
        public virtual with_source_location,
#endif
        public virtual with_cause,
        public virtual error {
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

        const std::unique_ptr<error> &cause() const & noexcept override
        {
                return this->cause_;
        }
        std::unique_ptr<error> &cause() & noexcept override
        {
                return this->cause_;
        }

#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
        const source_location &location() const noexcept override
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

class message_error : public base_error {
    public:
        message_error(
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

        message_error(
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

        const char *what() const noexcept override
        {
                if (!this->message_) {
                        return "";
                }
                return this->message_->c_str();
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
        return std::make_unique<message_error>(
#if defined(ERRORS_ENABLE_SOURCE_LOCATION)
                std::move(message.location),
#endif
                nullptr, message.value);
}

inline error_ptr wrap(capture_location<error_ptr> &&cause,
                      const char *message = nullptr)
{
        return make_error<message_error>(std::move(cause), message);
}

inline error_ptr wrap(capture_location<error_ptr> &&cause, std::string message)
{
        return make_error<message_error>(std::move(cause), std::move(message));
}

} // namespace errors

#if not defined(ERRORS_DISABLE_OSTREAM)
inline std::ostream &operator<<(std::ostream &os, const errors::error_ptr &err)
{
        auto current = err.get();

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

                auto current_with_cause =
                        dynamic_cast<const errors::with_cause *>(current);
                if (!current_with_cause) {
                        break;
                }

                current = current_with_cause->cause().get();
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
struct adl_serializer<::errors::error_ptr> {
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

                auto cause = err->cause().value_or(nullptr);
                if (!cause) {
                        return;
                }
                j["caused_by"] = cause;
        }
};
#if defined(NLOHMANN_JSON_NAMESPACE_BEGIN) && \
        defined(NLOHMANN_JSON_NAMESPACE_END)
NLOHMANN_JSON_NAMESPACE_END
#else
}
#endif
#endif
