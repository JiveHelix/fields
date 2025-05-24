find_package(Nlohmann_json REQUIRED)
add_executable(fields_demo fields_demo.cpp)
add_executable(precision_demo precision_demo.cpp)
add_executable(byte_swap_demo byte_swap_demo.cpp)

target_link_libraries(
    fields_demo
    PUBLIC
    project_warnings
    project_options
    fields
    nlohmann_json::nlohmann_json)

target_link_libraries(
    precision_demo
    PUBLIC
    project_warnings
    project_options
    fields)

target_link_libraries(
    byte_swap_demo
    PUBLIC
    project_warnings
    project_options
    fields)

install(TARGETS fields_demo DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS precision_demo DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS byte_swap_demo DESTINATION ${CMAKE_INSTALL_BINDIR})
