#pragma once

#include <string_view>

#if not defined(ERRORS_SINGLE_HEADER_FILE)
#include "errors/config.hpp"
#endif

namespace errors
{
constexpr std::string_view version = ERRORS_VERSION;
constexpr unsigned int version_major = ERRORS_VERSION_MAJOR;
constexpr unsigned int version_minor = ERRORS_VERSION_MINOR;
constexpr unsigned int version_patch = ERRORS_VERSION_PATCH;
}
