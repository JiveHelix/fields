add_library(fields INTERFACE)

# Projects that include this project must #include "fields/<header-name>"
target_include_directories(fields INTERFACE ${PROJECT_SOURCE_DIR})

target_link_libraries(fields INTERFACE jive)

target_sources(
    fields
    INTERFACE
    compare.h
    core.h
    describe.h
    fields.h)
