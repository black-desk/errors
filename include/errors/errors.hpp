#pragma once

#include "errors/error.hpp"
#include "errors/error_ptr.hpp"
#include "errors/impl/base_error.hpp"
#include "errors/impl/code_error.hpp"
#include "errors/impl/error_with_cause.hpp"
#include "errors/impl/exception_error.hpp"
#include "errors/impl/runtime_error.hpp"
#include "errors/impl/system_error.hpp"
#include "errors/impl/wrap_error.hpp"
#include "errors/make.hpp"
#include "errors/source_location.hpp"
#include "errors/utils.hpp"
#include "errors/wrap.hpp"

// Generate main page for the reference documentation.

/// @mainpage
/// This is the reference documentation for the `errors` library.
/// For the user guide, see the [README](https://github.com/black-desk/errors)
