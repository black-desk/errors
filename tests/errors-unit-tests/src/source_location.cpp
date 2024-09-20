#include "errors/source_location.hpp"

namespace
{
auto fn()
{
        return errors::source_location::current();
}
}

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"

TEST_CASE("source_location works", "[source_location]")
{
        using Catch::Matchers::EndsWith;
        using Catch::Matchers::Equals;

        auto location = fn();
        REQUIRE_THAT(location.file_name(), EndsWith("source_location.cpp"));
        REQUIRE(location.line() == 7);
        REQUIRE_THAT(location.function_name(), Equals("fn"));
}
