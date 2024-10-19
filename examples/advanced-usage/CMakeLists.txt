if(NOT TARGET tl::expected)
  return()
endif()

pfl_add_executable(
  DISABLE_INSTALL
  OUTPUT_NAME
  advanced-usage
  SOURCES
  # find -regex '\./\(src\|include\)/.+\.[ch]\(pp\)?\(\.in\)?' -type f | sort
  ./src/main.cpp
  LINK_LIBRARIES
  PRIVATE
  errors::errors
  tl::expected
  COMPILE_OPTIONS
  PRIVATE
  -DERRORS_ENABLE_SOURCE_LOCATION
  PROPERTIES
  CXX_STANDARD
  17
  CXX_STANDARD_REQUIRED
  ON
  CXX_EXTENSIONS
  OFF)

add_test(NAME run-examples_advanced-usage COMMAND advanced-usage)
