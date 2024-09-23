#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"
#include "errors/error.hpp"

namespace
{
using ::errors::error_ptr;
using ::errors::make_error;
using ::errors::message_error;
using ::errors::source_location;

class fn_error_t : public message_error {
    public:
        fn_error_t(source_location location, error_ptr &&cause, int depth)
                : message_error(std::move(location), std::move(cause),
                                "[depth=" + std::to_string(depth) + "]")
                , depth(depth)

        {
        }

        int depth;
};

errors::error_ptr fn(unsigned int depth)
{
        if (depth == 0) {
                return make_error<fn_error_t>(make_error("error"), depth);
        }
        auto err = fn(depth - 1);
        return make_error<fn_error_t>(std::move(err), depth);
}
}

TEST_CASE("custom error works", "[errors][source_location]")
{
        using Catch::Matchers::EndsWith;
        using Catch::Matchers::Equals;

        const std::uint_least32_t line_number_1 = 31;
        const std::uint_least32_t line_number_2 = 28;

        auto err = fn(3);
        REQUIRE(err != nullptr);
        REQUIRE(err->what() != nullptr);
        REQUIRE_THAT(err->what(), Equals("[depth=3]"));
        REQUIRE(err->type() == typeid(fn_error_t));
        REQUIRE(err->as<fn_error_t>() != nullptr);
        REQUIRE(err->as<fn_error_t>()->depth == 3);
        {
                const auto &location = err->as<message_error>()->location();
                REQUIRE_THAT(location.function_name(), Equals("fn"));
                REQUIRE_THAT(location.file_name(),
                             EndsWith("custom_error.cpp"));
                REQUIRE(location.line() == line_number_1);
        }

        err = err->cause().value_or(nullptr);
        REQUIRE(err != nullptr);
        REQUIRE(err->what() != nullptr);
        REQUIRE_THAT(err->what(), Equals("[depth=2]"));
        REQUIRE(err->type() == typeid(fn_error_t));
        REQUIRE(err->as<fn_error_t>() != nullptr);
        REQUIRE(err->as<fn_error_t>()->depth == 2);
        {
                const auto &location = err->as<message_error>()->location();
                REQUIRE_THAT(location.function_name(), Equals("fn"));
                REQUIRE_THAT(location.file_name(),
                             EndsWith("custom_error.cpp"));
                REQUIRE(location.line() == line_number_1);
        }

        err = err->cause().value_or(nullptr);
        REQUIRE(err != nullptr);
        REQUIRE(err->what() != nullptr);
        REQUIRE_THAT(err->what(), Equals("[depth=1]"));
        REQUIRE(err->type() == typeid(fn_error_t));
        REQUIRE(err->as<fn_error_t>() != nullptr);
        REQUIRE(err->as<fn_error_t>()->depth == 1);
        {
                const auto &location = err->as<message_error>()->location();
                REQUIRE_THAT(location.function_name(), Equals("fn"));
                REQUIRE_THAT(location.file_name(),
                             EndsWith("custom_error.cpp"));
                REQUIRE(location.line() == line_number_1);
        }

        err = err->cause().value_or(nullptr);
        REQUIRE(err != nullptr);
        REQUIRE(err->what() != nullptr);
        REQUIRE_THAT(err->what(), Equals("[depth=0]"));
        REQUIRE(err->type() == typeid(fn_error_t));
        REQUIRE(err->as<fn_error_t>() != nullptr);
        REQUIRE(err->as<fn_error_t>()->depth == 0);
        {
                const auto &location = err->as<message_error>()->location();
                REQUIRE_THAT(location.function_name(), Equals("fn"));
                REQUIRE_THAT(location.file_name(),
                             EndsWith("custom_error.cpp"));
                REQUIRE(location.line() == line_number_2);
        }

        err = err->cause().value_or(nullptr);
        REQUIRE(err != nullptr);
        REQUIRE(err->what() != nullptr);
        REQUIRE_THAT(err->what(), Equals("error"));
        REQUIRE(err->type() == typeid(message_error));
        REQUIRE(err->as<fn_error_t>() == nullptr);
        {
                const auto &location = err->as<message_error>()->location();
                REQUIRE_THAT(location.function_name(), Equals("fn"));
                REQUIRE_THAT(location.file_name(),
                             EndsWith("custom_error.cpp"));
                REQUIRE(location.line() == line_number_2);
        }
}

#if defined(ERRORS_ENABLE_TO_JSON)

#include "nlohmann/json.hpp"

TEST_CASE("to_json works for custom error", "[errors][json]")
{
        auto err = fn(0);
        nlohmann::json j = err;
        REQUIRE(j["message"] == "error");
}

TEST_CASE("to_json works for nested custom error", "[errors][json]")
{
        nlohmann::json j = fn(3);
        REQUIRE(j["message"] == "[depth=3]");
        REQUIRE(j["caused_by"]["message"] == "[depth=2]");
        REQUIRE(j["caused_by"]["caused_by"]["message"] == "[depth=1]");
        REQUIRE(j["caused_by"]["caused_by"]["caused_by"]["message"] == "error");
        REQUIRE(j["caused_by"]["caused_by"]["caused_by"]["caused_by"].is_null());
}

#endif
