cmake_minimum_required(VERSION 3.15)

project(fields CXX)

include(${CMAKE_CURRENT_LIST_DIR}/cmake_includes/setup_project.cmake)
setup_project()

add_subdirectory(fields)

if (ENABLE_TESTING)
    if (${PROJECT_IS_TOP_LEVEL} OR ${RECURSIVE_BUILD_TESTS})
        add_subdirectory(test)
    endif ()
endif ()

if (${PROJECT_IS_TOP_LEVEL})
    option(
        BUILD_EXAMPLES
        "Build the example targets"
        ${CONAN_USER_INFO_BUILD_EXAMPLES})

    if (${BUILD_EXAMPLES})
        add_subdirectory(examples)
    endif ()
endif ()
