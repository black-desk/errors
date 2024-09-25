#include <fcntl.h>

#include <cstdlib>
#include <exception>
#include <sstream>

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"
#include "errors/error.hpp"

using ::Catch::Matchers::Equals;
using ::errors::error_ptr;
using ::errors::make_error;
using ::errors::message_error;
using ::errors::wrap;

namespace
{

error_ptr fn1()
{
        return make_error<message_error>(nullptr, "error");
}

error_ptr fn2(unsigned int depth)
{
        if (depth == 0) {
                return make_error<message_error>(nullptr, "error");
        }
        auto err = fn2(depth - 1);
        return wrap(std::move(err), "depth=" + std::to_string(depth));
}
}

TEST_CASE("message_error works", "[errors]")
{
        auto err = fn1();
        REQUIRE(err != nullptr);
        REQUIRE_THAT(err->what(), Equals("error"));

        std::stringstream ss;
        ss << err;
        REQUIRE_THAT(ss.str(), Equals("error"));
}

TEST_CASE("nested message_error works", "[errors]")
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

TEST_CASE("code_error works", "[errors]")
{
        auto path = "/file/not/exist";
        auto fd = open(path, O_RDONLY);
        assert(fd < 0);
        auto code = errno;
        auto err = make_error<errors::code_error>(
                nullptr, "open " + std::string(path) + ": " + strerror(code),
                code);
        REQUIRE(err != nullptr);
        REQUIRE_THAT(err->what(),
                     Equals("open /file/not/exist: No such file or directory"));
}

TEST_CASE("exception_error works", "[errors]")
{
        try {
                throw std::runtime_error("error");
        } catch (...) {
                auto err = make_error<errors::exception_error>(nullptr);
                REQUIRE(err != nullptr);
                REQUIRE_THAT(err->what(), Equals("error"));
        }
}

#if defined(ERRORS_ENABLE_NLOHMANN_JSON_SUPPORT)

#include "nlohmann/json.hpp"

TEST_CASE("to_json works for message_error", "[errors][json]")
{
        auto err = fn1();
        nlohmann::json j = err;
        REQUIRE(j["message"] == "error");
}

TEST_CASE("to_json works for nested message_error", "[errors][json]")
{
        nlohmann::json j = fn2(3);
        REQUIRE(j["message"] == "depth=3");
        REQUIRE(j["caused_by"]["message"] == "depth=2");
        REQUIRE(j["caused_by"]["caused_by"]["message"] == "depth=1");
        REQUIRE(j["caused_by"]["caused_by"]["caused_by"]["message"] == "error");
        REQUIRE(j["caused_by"]["caused_by"]["caused_by"]["caused_by"].is_null());
}

#endif
