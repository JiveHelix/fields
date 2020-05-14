include(CheckIncludeFileCxx)

add_subdirectory(jive)

add_library(nlohmann_json INTERFACE)

target_include_directories(
    nlohmann_json
    INTERFACE
    ${PROJECT_SOURCE_DIR}/external/nlohmann_json/include)
