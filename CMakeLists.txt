cmake_minimum_required(VERSION 3.15)


# detect whether fields has been added as a subproject
if(NOT DEFINED PROJECT_NAME)
    set(FIELDS_NOT_SUBPROJECT ON)
endif()

include(cmake/DefineProjectOnce.cmake)

define_project_once(fields)

project(fields CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # This is the top-level project
    include(cmake/Standalone.cmake)
endif()

add_subdirectory(external)
add_subdirectory(fields)

if(FIELDS_NOT_SUBPROJECT)
    add_subdirectory(examples)
    add_subdirectory(test)
endif()

option(ENABLE_UNITY "Enable Unity builds of projects" OFF)

if(ENABLE_UNITY)
  # Add for any project you want to apply unity builds for
  set_target_properties(fields PROPERTIES UNITY_BUILD ON)
endif()
