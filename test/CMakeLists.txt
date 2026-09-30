
find_package(Nlohmann_json REQUIRED)

add_catch2_test(
    NAME fields_tests
    SOURCES
        aliases_tests.cpp
        byte_swap_tests.cpp
        compare_tests.cpp
        default_tests.cpp
        diff_tests.cpp
        fields_tests.cpp
        hidden_tests.cpp
        marshal_tests.cpp
        reflect_tests.cpp
    LINK
        fields
        nlohmann_json::nlohmann_json)
