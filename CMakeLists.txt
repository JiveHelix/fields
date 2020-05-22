cmake_minimum_required(VERSION 3.15)

# detect whether fields has been added as a subproject
if(NOT DEFINED PROJECT_NAME)
    set(FIELDS_NOT_SUBPROJECT ON)
endif()

include(cmake/DefineProjectOnce.cmake)

define_project_once(fields)

project(fields CXX)

if(FIELDS_NOT_SUBPROJECT)
    # This is the top-level project
    set(CONAN_EXTRA_REQUIRES "nlohmann_json/3.7.3")
    include(cmake/Standalone.cmake)
    option(ENABLE_UNITY "Enable Unity builds of projects" OFF)
endif()

add_subdirectory(external)
add_subdirectory(fields)

if(FIELDS_NOT_SUBPROJECT)

    add_subdirectory(examples)

    if(ENABLE_TESTING)
        enable_testing()
        add_subdirectory(test)
    endif()

elseif(RECURSIVE_BUILD_TESTS AND ENABLE_TESTING)

    # A parent project has requested tests.
    add_subdirectory(test)

endif()

if(ENABLE_UNITY)
  # Add for any project you want to apply unity builds for
  set_target_properties(fields PROPERTIES UNITY_BUILD ON)
endif()
