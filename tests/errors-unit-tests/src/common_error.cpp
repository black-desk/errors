#include <fcntl.h>

#include <cstdlib>
#include <sstream>

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"
#include "errors/errors.hpp"

using ::Catch::Matchers::Equals;
using ::errors::error_ptr;
using ::errors::impl::code_error;
using ::errors::impl::exception_error;
using ::errors::impl::runtime_error;
using ::errors::impl::system_error;

namespace
{

error_ptr fn1()
{
        return errors::make<runtime_error>::with("error");
}

error_ptr fn2(unsigned int depth)
{
        if (depth == 0) {
                return errors::make<runtime_error>::with("error");
        }
        auto err = fn2(depth - 1);
        return errors::wrap("depth=" + std::to_string(depth), std::move(err));
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
        auto current = err.get();
        REQUIRE(current != nullptr);
        REQUIRE_THAT(current->what(), Equals("depth=3"));

        current = current->cause().get();
        REQUIRE(current != nullptr);
        REQUIRE_THAT(current->what(), Equals("depth=2"));

        current = current->cause().get();
        REQUIRE(current != nullptr);
        REQUIRE_THAT(current->what(), Equals("depth=1"));

        current = current->cause().get();
        REQUIRE(current != nullptr);
        REQUIRE_THAT(current->what(), Equals("error"));

        current = current->cause().get();
        REQUIRE(!current);

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

        auto err = errors::make<code_error<int>>::with(
                "open " + std::string(path), code);
        REQUIRE(err != nullptr);
        REQUIRE_THAT(err->what(), Equals("open /file/not/exist [code=" +
                                         std::to_string(ENOENT) + "]"));
}

TEST_CASE("exception_error works", "[errors]")
{
        try {
                throw std::runtime_error("error");
        } catch (...) {
                auto err = errors::make<exception_error>::with();
                REQUIRE(err != nullptr);
                REQUIRE_THAT(err->what(), Equals("error"));
                REQUIRE(err->cause() == nullptr);
                REQUIRE(std::move(*err).cause() == nullptr);
        }

        try {
                throw "error";
        } catch (...) {
                auto err = errors::make<exception_error>::with();
                REQUIRE(err != nullptr);
                REQUIRE_THAT(err->what(), Equals("Unknown exception"));
        }

        auto err = errors::make<exception_error>::with();
        REQUIRE(err != nullptr);
        REQUIRE_THAT(err->what(), Equals(""));
}

TEST_CASE("system_error works", "[errors]")
{
        auto path = "/file/not/exist";
        auto fd = open(path, O_RDONLY);
        assert(fd < 0);
        auto code = errno;

        auto err = errors::make<system_error>::with("open " + std::string(path),
                                                    code);
        REQUIRE(err != nullptr);
        REQUIRE_THAT(
                err->what(),
                Equals("open /file/not/exist: No such file or directory [code=" +
                       std::to_string(ENOENT) + "]"));
}

#if defined(ERRORS_ENABLE_NLOHMANN_JSON_SUPPORT)

#include "nlohmann/json.hpp"

TEST_CASE("to_json works for runtime_error", "[errors][json]")
{
        auto err = fn1();
        nlohmann::json j = err;
        REQUIRE(j["message"] == "error");
}

TEST_CASE("to_json works for nested error", "[errors][json]")
{
        nlohmann::json j = fn2(3);
        REQUIRE(j["message"] == "depth=3");
        REQUIRE(j["caused_by"]["message"] == "depth=2");
        REQUIRE(j["caused_by"]["caused_by"]["message"] == "depth=1");
        REQUIRE(j["caused_by"]["caused_by"]["caused_by"]["message"] == "error");
        REQUIRE(j["caused_by"]["caused_by"]["caused_by"]["caused_by"].is_null());
}

#endif
