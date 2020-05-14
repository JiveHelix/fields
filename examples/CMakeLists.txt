add_executable(fields_demo fields_demo.cpp)

target_link_libraries(
    fields_demo
    PUBLIC
    project_warnings
    project_options
    fields
    nlohmann_json)
