set(link_libraries PRIVATE Catch2::Catch2WithMain errors::errors)
set(compile_options PRIVATE -DERRORS_ENABLE_SOURCE_LOCATION=1)

if(errors_WITH_NLOHMANN_JSON)
  find_package(nlohmann_json 3.5.0 REQUIRED)
  list(APPEND link_libraries PRIVATE nlohmann_json::nlohmann_json)
  list(APPEND compile_options PRIVATE -DERRORS_ENABLE_NLOHMANN_JSON_SUPPORT)
endif()

pfl_add_executable(
  DISABLE_INSTALL
  OUTPUT_NAME
  errors-unit-tests
  SOURCES
  # find -regex '\./\(src|include\)/.+\.[ch]\(pp\)?\(\.in\)?' -type f | sort
  ./src/common_error.cpp
  ./src/custom_error.cpp
  ./src/source_location.cpp
  LINK_LIBRARIES
  ${link_libraries}
  COMPILE_OPTIONS
  ${compile_options})

pfl_catch_discover_tests(errors::errors::errors-unit-tests)
