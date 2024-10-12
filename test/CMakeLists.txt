add_library(fields_catch_main STATIC catch_main.cpp)
target_link_libraries(fields_catch_main PUBLIC CONAN_PKG::catch2)
target_link_libraries(fields_catch_main PRIVATE project_options)

add_executable(
    fields_tests
    compare_tests.cpp
    fields_tests.cpp
    marshal_tests.cpp
    byte_swap_tests.cpp
    default_tests.cpp)

target_link_libraries(
    fields_tests
    PRIVATE
    fields_catch_main
    fields
    CONAN_PKG::nlohmann_json)

add_test(NAME FieldsTests COMMAND ${CMAKE_BINARY_DIR}/bin/fields_tests)
