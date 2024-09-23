#include <sstream>

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"
#include "errors/error.hpp"

using ::Catch::Matchers::Equals;
using ::errors::error_ptr;
using ::errors::make_error;
using ::errors::wrap;

namespace
{

error_ptr fn1()
{
        return make_error("error");
}

error_ptr fn2(unsigned int depth)
{
        if (depth == 0) {
                return make_error("error");
        }
        auto err = fn2(depth - 1);
        return wrap(std::move(err), "depth=" + std::to_string(depth));
}
}

TEST_CASE("common_error works", "[errors]")
{
        auto err = fn1();
        REQUIRE(err != nullptr);
        REQUIRE_THAT(err->what(), Equals("error"));

        std::stringstream ss;
        ss << err;
        REQUIRE_THAT(ss.str(), Equals("error"));
}

TEST_CASE("nested common_error works", "[errors]")
{
        auto err = fn2(3);
        REQUIRE(err != nullptr);
        REQUIRE_THAT(err->what(), Equals("depth=3"));

        err = err->cause().value_or(nullptr);
        REQUIRE(err != nullptr);
        REQUIRE_THAT(err->what(), Equals("depth=2"));

        err = err->cause().value_or(nullptr);
        REQUIRE(err != nullptr);
        REQUIRE_THAT(err->what(), Equals("depth=1"));

        err = err->cause().value_or(nullptr);
        REQUIRE(err != nullptr);
        REQUIRE_THAT(err->what(), Equals("error"));
        REQUIRE(!err->cause().has_value());

        std::stringstream ss;
        ss << fn2(3);
        REQUIRE_THAT(ss.str(), Equals("depth=3: depth=2: depth=1: error"));
}

#if defined(ERRORS_ENABLE_NLOHMANN_JSON_SUPPORT)

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
