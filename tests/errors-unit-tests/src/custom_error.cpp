#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"
#include "errors/errors.hpp"

namespace
{
using ::errors::error_ptr;
using ::errors::source_location;
using ::errors::impl::runtime_error;
using ::errors::impl::wrap_error;

class fn_error_t : public wrap_error {
    public:
        fn_error_t(int depth, error_ptr &&cause, source_location location)
                : wrap_error("[depth=" + std::to_string(depth) + "]",
                             std::move(cause), std::move(location))
                , depth(depth)

        {
        }

        int depth;
};

errors::error_ptr fn(unsigned int depth)
{
        if (depth == 0) {
                return errors::make<fn_error_t>::with(
                        depth, errors::make<runtime_error>::with("error"));
        }
        auto err = fn(depth - 1);
        if (!err) {
                return nullptr;
        }

        return errors::make<fn_error_t>::with(depth, std::move(err));
}
}

TEST_CASE("custom error works", "[errors][source_location]")
{
        using Catch::Matchers::EndsWith;
        using Catch::Matchers::Equals;

        auto current_err = fn(3);

        REQUIRE(current_err != nullptr);
        REQUIRE(current_err->what() != nullptr);
        REQUIRE_THAT(current_err->what(), Equals("[depth=3]"));
        REQUIRE(current_err.as<fn_error_t>() != nullptr);
        REQUIRE(current_err.as<fn_error_t>()->depth == 3);
        {
                const auto &location = current_err->location();
                REQUIRE_THAT(location->function_name(), Equals("fn"));
                REQUIRE_THAT(location->file_name(),
                             EndsWith("custom_error.cpp"));
        }

        current_err = std::move(*current_err).cause();
        REQUIRE(current_err != nullptr);
        REQUIRE(current_err->what() != nullptr);
        REQUIRE_THAT(current_err->what(), Equals("[depth=2]"));
        REQUIRE(current_err.as<fn_error_t>() != nullptr);
        REQUIRE(current_err.as<fn_error_t>()->depth == 2);
        {
                const auto &location = current_err->location();
                REQUIRE_THAT(location->function_name(), Equals("fn"));
                REQUIRE_THAT(location->file_name(),
                             EndsWith("custom_error.cpp"));
        }

        current_err = std::move(*current_err).cause();
        REQUIRE(current_err != nullptr);
        REQUIRE(current_err->what() != nullptr);
        REQUIRE_THAT(current_err->what(), Equals("[depth=1]"));
        REQUIRE(current_err.as<fn_error_t>() != nullptr);
        REQUIRE(current_err.as<fn_error_t>()->depth == 1);
        {
                const auto &location = current_err->location();
                REQUIRE_THAT(location->function_name(), Equals("fn"));
                REQUIRE_THAT(location->file_name(),
                             EndsWith("custom_error.cpp"));
        }

        current_err = std::move(*current_err).cause();
        REQUIRE(current_err != nullptr);
        REQUIRE(current_err->what() != nullptr);
        REQUIRE_THAT(current_err->what(), Equals("[depth=0]"));
        REQUIRE(current_err.as<fn_error_t>() != nullptr);
        REQUIRE(current_err.as<fn_error_t>()->depth == 0);
        {
                const auto &location = current_err->location();
                REQUIRE_THAT(location->function_name(), Equals("fn"));
                REQUIRE_THAT(location->file_name(),
                             EndsWith("custom_error.cpp"));
        }

        current_err = std::move(*current_err).cause();
        REQUIRE(current_err != nullptr);
        REQUIRE(current_err->what() != nullptr);
        REQUIRE_THAT(current_err->what(), Equals("error"));
        REQUIRE(current_err.as<fn_error_t>() == nullptr);
        {
                const auto &location = current_err->location();
                REQUIRE_THAT(location->function_name(), Equals("fn"));
                REQUIRE_THAT(location->file_name(),
                             EndsWith("custom_error.cpp"));
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
