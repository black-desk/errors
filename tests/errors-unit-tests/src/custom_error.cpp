#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"
#include "errors/error.hpp"

namespace
{
class fn_error : public ::errors::base_error, public virtual ::errors::error {
    public:
        class context : public ::errors::context {
            public:
                context(int depth, ::errors::source_location location =
                                           ::errors::source_location::current())
                        : ::errors::context{ location }
                        , depth(depth)
                {
                }
                int depth;
        };

        fn_error(context context, ::errors::error_ptr &&cause = nullptr)
                : ::errors::base_error(std::move(cause))
                , context_(std::move(context))

        {
        }

        std::optional<std::string> what() const override
        {
                std::string result;
                result.append("[depth=");
                result.append(std::to_string(this->context_.depth));
                result.append("]");
                return result;
        }

        const ::errors::source_location &location() const override
        {
                return this->context_.location;
        }

    private:
        context context_;
};

errors::error_ptr fn(unsigned int depth)
{
        if (depth == 0) {
                return errors::make_error<errors::common_error>("error");
        }
        return errors::make_error<fn_error>(depth, fn(depth - 1));
}
}

TEST_CASE("custom error works", "[errors][source_location]")
{
        using Catch::Matchers::EndsWith;
        using Catch::Matchers::Equals;

        auto err = fn(3);
        REQUIRE(err != nullptr);
        REQUIRE(err->what().has_value());
        REQUIRE_THAT(*err->what(), Equals("[depth=3]"));
        REQUIRE_THAT(err->location().function_name(), Equals("fn"));
        REQUIRE_THAT(err->location().file_name(), EndsWith("custom_error.cpp"));
        REQUIRE(err->location().line() == 50);
        REQUIRE(err->is<fn_error>());

        REQUIRE(err->cause() != nullptr);
        REQUIRE(err->cause()->what().has_value());
        REQUIRE_THAT(*err->cause()->what(), Equals("[depth=2]"));
        REQUIRE_THAT(err->cause()->location().function_name(), Equals("fn"));
        REQUIRE_THAT(err->cause()->location().file_name(),
                     EndsWith("custom_error.cpp"));
        REQUIRE(err->cause()->location().line() == 50);
        REQUIRE(err->cause()->is<fn_error>());

        REQUIRE(err->cause()->cause() != nullptr);
        REQUIRE(err->cause()->cause()->what().has_value());
        REQUIRE_THAT(*err->cause()->cause()->what(), Equals("[depth=1]"));
        REQUIRE_THAT(err->cause()->cause()->location().function_name(),
                     Equals("fn"));
        REQUIRE_THAT(err->cause()->cause()->location().file_name(),
                     EndsWith("custom_error.cpp"));
        REQUIRE(err->cause()->cause()->location().line() == 50);
        REQUIRE(err->cause()->cause()->is<fn_error>());

        REQUIRE(err->cause()->cause()->cause() != nullptr);
        REQUIRE(err->cause()->cause()->cause()->what().has_value());
        REQUIRE_THAT(*err->cause()->cause()->cause()->what(), Equals("error"));
        REQUIRE_THAT(err->cause()->cause()->cause()->location().function_name(),
                     Equals("fn"));
        REQUIRE_THAT(err->cause()->cause()->cause()->location().file_name(),
                     EndsWith("custom_error.cpp"));
        REQUIRE(err->cause()->cause()->cause()->location().line() == 48);
        REQUIRE(err->cause()->cause()->cause()->is<errors::common_error>());
        REQUIRE_THAT(*err->as<errors::common_error>()->what(), Equals("error"));

        REQUIRE(err->cause()->cause()->cause()->cause() == nullptr);
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
