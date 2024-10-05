#pragma once

#include <optional>

#include "errors/error_ptr.hpp"
#include "errors/source_location.hpp"
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

template <>
struct adl_serializer< ::errors::error_ptr> {
        static void to_json(::nlohmann::json &j, const ::errors::error_ptr &err)
        {
                if (!err) {
                        j = {};
                        assert(j.is_null());
                        return;
                }

                if (err->location()) {
                        j["location"] = err->location().value();
                }

                j["message"] = err->what();

                j["caused_by"] = err->cause();
        }
};

#if defined(NLOHMANN_JSON_NAMESPACE_BEGIN) && \
        defined(NLOHMANN_JSON_NAMESPACE_END)
NLOHMANN_JSON_NAMESPACE_END
#else
}
#endif
