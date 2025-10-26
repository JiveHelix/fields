add_library(fields INTERFACE)

find_package(Jive REQUIRED)

# Projects that use this project must #include "fields/<header-name>"
target_include_directories(fields INTERFACE ${PROJECT_SOURCE_DIR})

target_link_libraries(fields INTERFACE jive::jive)

target_sources(
    fields
    INTERFACE
    compare.h
    comparisons.h
    core.h
    describe.h
    enum_field.h
    fields.h
    marshal.h
    network_byte_order.h
    serialize.h)

install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/fields
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
