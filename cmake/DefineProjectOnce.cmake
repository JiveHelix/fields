# Define this as a macro so that the return statement, if necessary, will return control to the parent directory.
macro(define_project_once)
    if((DEFINED ARGV0_SOURCE_DIR)
        AND (NOT ARGV0_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR))
        message(
            "Ignoring ARGV0 in ${CMAKE_CURRENT_SOURCE_DIR}. "
            "Already added: ${ARGV0_SOURCE_DIR}")
        return()
    endif()

    message("Allowing project definition to proceed for ${ARGV0}")
endmacro()
