set(link_libraries PRIVATE errors::errors)
set(compile_options PRIVATE -DERRORS_ENABLE_SOURCE_LOCATION)

if(errors_WITH_NLOHMANN_JSON)
  find_package(nlohmann_json 3.5.0 REQUIRED)
  list(APPEND link_libraries PRIVATE nlohmann_json::nlohmann_json)
  list(APPEND compile_options PRIVATE -DERRORS_ENABLE_NLOHMANN_JSON_SUPPORT)
endif()

pfl_add_executable(
  DISABLE_INSTALL
  OUTPUT_NAME
  customize-format-local
  SOURCES
  # find -regex '\./\(src\|include\)/.+\.[ch]\(pp\)?\(\.in\)?' -type f | sort
  ./src/main.cpp
  LINK_LIBRARIES
  ${link_libraries}
  COMPILE_OPTIONS
  ${compile_options})

add_test(NAME run-examples_customize-format-local
         COMMAND customize-format-local)
