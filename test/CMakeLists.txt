# automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

add_library(fields_catch_main STATIC catch_main.cpp)
target_link_libraries(fields_catch_main PUBLIC CONAN_PKG::catch2)
target_link_libraries(fields_catch_main PRIVATE project_options)

add_executable(
    fields_tests
    marshal_tests.cpp)

target_link_libraries(
    fields_tests
    PRIVATE
    project_warnings
    project_options
    fields_catch_main
    fields)

# automatically discover tests that are defined in catch based test files.
catch_discover_tests(
    fields_tests
    TEST_PREFIX
    "unittests."
    EXTRA_ARGS)
