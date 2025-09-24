
find_package(Nlohmann_json REQUIRED)

add_catch2_test(
    NAME fields_tests
    SOURCES
        compare_tests.cpp
        fields_tests.cpp
        marshal_tests.cpp
        byte_swap_tests.cpp
        default_tests.cpp
        diff_tests.cpp
        reflect_tests.cpp
    LINK
        fields
        nlohmann_json::nlohmann_json)
