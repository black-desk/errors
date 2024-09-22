#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"
#include "errors/error.hpp"

namespace
{
using errors::common_error;
using errors::make_error;

class fn_error : public ::errors::base_error, public virtual ::errors::error {
    public:
        fn_error(::errors::source_location location,
                 ::errors::error_ptr &&cause, int depth)
                : ::errors::base_error(std::move(location), std::move(cause))
                , depth(depth)

        {
        }

        std::optional<std::string> what() const override
        {
                std::string result;
                result.append("[depth=");
                result.append(std::to_string(this->depth));
                result.append("]");
                return result;
        }

        int depth;
};

errors::error_ptr fn(unsigned int depth)
{
        if (depth == 0) {
                return make_error<fn_error>(make_error("error"), depth);
        }
        auto err = fn(depth - 1);
        return make_error<fn_error>(std::move(err), depth);
}
}

TEST_CASE("custom error works", "[errors][source_location]")
{
        using Catch::Matchers::EndsWith;
        using Catch::Matchers::Equals;

        const std::uint_least32_t line_number_1 = 38;
        const std::uint_least32_t line_number_2 = 35;

        auto err = fn(3);
        REQUIRE(err != nullptr);
        REQUIRE(err->what().has_value());
        REQUIRE_THAT(*err->what(), Equals("[depth=3]"));
        REQUIRE_THAT(err->location().function_name(), Equals("fn"));
        REQUIRE_THAT(err->location().file_name(), EndsWith("custom_error.cpp"));
        REQUIRE(err->location().line() == line_number_1);
        REQUIRE(err->is<fn_error>());

        REQUIRE(err->cause() != nullptr);
        REQUIRE(err->cause()->what().has_value());
        REQUIRE_THAT(*err->cause()->what(), Equals("[depth=2]"));
        REQUIRE_THAT(err->cause()->location().function_name(), Equals("fn"));
        REQUIRE_THAT(err->cause()->location().file_name(),
                     EndsWith("custom_error.cpp"));
        REQUIRE(err->cause()->location().line() == line_number_1);
        REQUIRE(err->cause()->is<fn_error>());

        REQUIRE(err->cause()->cause() != nullptr);
        REQUIRE(err->cause()->cause()->what().has_value());
        REQUIRE_THAT(*err->cause()->cause()->what(), Equals("[depth=1]"));
        REQUIRE_THAT(err->cause()->cause()->location().function_name(),
                     Equals("fn"));
        REQUIRE_THAT(err->cause()->cause()->location().file_name(),
                     EndsWith("custom_error.cpp"));
        REQUIRE(err->cause()->cause()->location().line() == line_number_1);
        REQUIRE(err->cause()->cause()->is<fn_error>());

        REQUIRE(err->cause()->cause()->cause() != nullptr);
        REQUIRE(err->cause()->cause()->cause()->what().has_value());
        REQUIRE_THAT(*err->cause()->cause()->cause()->what(),
                     Equals("[depth=0]"));
        REQUIRE_THAT(err->cause()->cause()->cause()->location().function_name(),
                     Equals("fn"));
        REQUIRE_THAT(err->cause()->cause()->cause()->location().file_name(),
                     EndsWith("custom_error.cpp"));
        REQUIRE(err->cause()->cause()->cause()->location().line() ==
                line_number_2);
        REQUIRE(err->cause()->cause()->cause()->is<fn_error>());

        REQUIRE(err->cause()->cause()->cause()->cause() != nullptr);
        REQUIRE(err->cause()->cause()->cause()->cause()->what().has_value());
        REQUIRE_THAT(*err->cause()->cause()->cause()->cause()->what(),
                     Equals("error"));
        REQUIRE_THAT(err->cause()
                             ->cause()
                             ->cause()
                             ->cause()
                             ->location()
                             .function_name(),
                     Equals("fn"));
        REQUIRE_THAT(
                err->cause()->cause()->cause()->cause()->location().file_name(),
                EndsWith("custom_error.cpp"));
        REQUIRE(err->cause()->cause()->cause()->cause()->location().line() ==
                line_number_2);
        REQUIRE(err->is<errors::common_error>());
        REQUIRE_THAT(*err->as<errors::common_error>()->what(), Equals("error"));
        REQUIRE(err->cause()->cause()->cause()->cause()->cause() == nullptr);
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
