if(NOT TARGET tl::expected)
  return()
endif()

pfl_add_executable(
  DISABLE_INSTALL
  OUTPUT_NAME
  error-handling
  SOURCES
  # find -regex '\./\(src\|include\)/.+\.[ch]\(pp\)?\(\.in\)?' -type f | sort
  ./src/main.cpp
  LINK_LIBRARIES
  PRIVATE
  errors::errors
  tl::expected
  COMPILE_OPTIONS
  PRIVATE
  -DERRORS_ENABLE_SOURCE_LOCATION)

add_test(NAME run-examples_error-handling COMMAND error-handling)
