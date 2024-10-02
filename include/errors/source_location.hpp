#pragma once

#if defined(ERRORS_USE_STD_SOURCE_LOCATION)
#include <source_location>
#else

#include <cstdint>

#if defined(__clang__) && !defined(__apple_build_version__) && \
        (__clang_major__ >= 9)
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FILE
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FUNCTION
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_LINE
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_COLUMN
#elif defined(__apple_build_version__) && defined(__clang__) && \
        (__clang_major__ * 10000 + __clang_minor__ * 100 +      \
         __clang_patchlevel__ % 100) >= 110003
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FILE
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FUNCTION
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_LINE
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_COLUMN
// GCC
#elif defined(__GNUC__) && \
        (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FILE
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FUNCTION
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_LINE
#define ERRORS_SOURCE_LOCATION_NO_BUILTIN_COLUMN
// MSVC https://github.com/microsoft/STL/issues/54#issuecomment-616904069 https://learn.microsoft.com/en-us/cpp/overview/compiler-versions?view=msvc-170
#elif defined(_MSC_VER) && !defined(__clang__) && \
        !defined(__INTEL_COMPILER) && (_MSC_VER >= 1926)
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FILE
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FUNCTION
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_LINE
#define ERRORS_SOURCE_LOCATION_HAS_BUILTIN_COLUMN
#endif
#endif

namespace errors
{
/// @brief
/// The source location.
#if defined(ERRORS_USE_STD_SOURCE_LOCATION)
using source_location = std::source_location;
#else
struct source_location {
    public:
        /// @brief
        /// Returns the current source location.
#if defined(ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FILE) &&         \
        defined(ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FUNCTION) && \
        defined(ERRORS_SOURCE_LOCATION_HAS_BUILTIN_LINE) &&     \
        defined(ERRORS_SOURCE_LOCATION_HAS_BUILTIN_COLUMN)
        static constexpr source_location
        current(const char *fileName = __builtin_FILE(),
                const char *functionName = __builtin_FUNCTION(),
                const uint_least32_t lineNumber = __builtin_LINE(),
                const uint_least32_t columnOffset = __builtin_COLUMN()) noexcept
#elif defined(ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FILE) &&       \
        defined(ERRORS_SOURCE_LOCATION_HAS_BUILTIN_FUNCTION) && \
        defined(ERRORS_SOURCE_LOCATION_HAS_BUILTIN_LINE) &&     \
        defined(ERRORS_SOURCE_LOCATION_NO_BUILTIN_COLUMN)
        static constexpr source_location
        current(const char *fileName = __builtin_FILE(),
                const char *functionName = __builtin_FUNCTION(),
                const uint_least32_t lineNumber = __builtin_LINE(),
                const uint_least32_t columnOffset = 0) noexcept
#else
        static constexpr source_location
        current(const char *fileName = "unsupported",
                const char *functionName = "unsupported",
                const uint_least32_t lineNumber = 0,
                const uint_least32_t columnOffset = 0) noexcept
#endif
        {
                return source_location(fileName, functionName, lineNumber,
                                       columnOffset);
        }

        constexpr source_location() noexcept = default;

        /// @brief
        /// Returns the file name,
        /// or "unsupported" if it is not supported by the compiler.
        constexpr const char *file_name() const noexcept
        {
                return file_name_;
        }

        /// @brief
        /// Returns the function name.
        constexpr const char *function_name() const noexcept
        {
                return function_name_;
        }

        /// @brief
        /// Returns the line number.
        constexpr uint_least32_t line() const noexcept
        {
                return line_number;
        }

        /// @brief
        /// Returns the column number,
        /// or 0 if it is not supported by the compiler
        constexpr std::uint_least32_t column() const noexcept
        {
                return column_;
        }

    private:
        constexpr source_location(const char *file, const char *function,
                                  uint_least32_t line,
                                  uint_least32_t column) noexcept
                : file_name_(file)
                , function_name_(function)
                , line_number(line)
                , column_(column)
        {
        }

        const char *file_name_ = "";
        const char *function_name_ = "";
        std::uint_least32_t line_number{};
        std::uint_least32_t column_{};
};
#endif

}

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
struct adl_serializer< ::errors::source_location> {
        static void to_json(::nlohmann::json &j,
                            const ::errors::source_location &loc)
        {
                j["file_name"] = loc.file_name();
                j["function_name"] = loc.function_name();
                j["line"] = loc.line();
                j["column"] = loc.column();
        }
};
#if defined(NLOHMANN_JSON_NAMESPACE_BEGIN) && \
        defined(NLOHMANN_JSON_NAMESPACE_END)
NLOHMANN_JSON_NAMESPACE_END
#else
}
#endif
#endif
