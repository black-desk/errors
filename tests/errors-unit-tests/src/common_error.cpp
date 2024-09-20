#include "catch2/catch_test_macros.hpp"
#include "errors/error.hpp"
#include "nlohmann/json.hpp"

namespace
{
errors::error_ptr fn1()
{
        return errors::make_error<errors::common_error>("error");
}

errors::error_ptr fn2(unsigned int depth)
{
        if (depth == 0) {
                return errors::make_error<errors::common_error>("error");
        }
        return errors::make_error<errors::common_error>(
                "depth=" + std::to_string(depth), fn2(depth - 1));
}
}

TEST_CASE("common_error works", "[errors]")
{
        auto err = fn1();
        REQUIRE(err != nullptr);
        REQUIRE(err->what() == "error");
}

TEST_CASE("nested common_error works", "[errors]")
{
        auto err = fn2(3);
        REQUIRE(err != nullptr);
        REQUIRE(err->what() == "depth=3");
        REQUIRE(err->cause() != nullptr);
        REQUIRE(err->cause()->what() == "depth=2");
        REQUIRE(err->cause()->cause() != nullptr);
        REQUIRE(err->cause()->cause()->what() == "depth=1");
        REQUIRE(err->cause()->cause()->cause() != nullptr);
        REQUIRE(err->cause()->cause()->cause()->what() == "error");
        REQUIRE(err->cause()->cause()->cause()->cause() == nullptr);
}

#if defined(ERRORS_ENABLE_TO_JSON)

#include "nlohmann/json.hpp"

TEST_CASE("to_json works for common_error", "[errors][json]")
{
        auto err = fn1();
        nlohmann::json j = err;
        REQUIRE(j["message"] == "error");
}

TEST_CASE("to_json works for nested common_error", "[errors][json]")
{
        nlohmann::json j = fn2(3);
        REQUIRE(j["message"] == "depth=3");
        REQUIRE(j["caused_by"]["message"] == "depth=2");
        REQUIRE(j["caused_by"]["caused_by"]["message"] == "depth=1");
        REQUIRE(j["caused_by"]["caused_by"]["caused_by"]["message"] == "error");
        REQUIRE(j["caused_by"]["caused_by"]["caused_by"]["caused_by"].is_null());
}

#endif
