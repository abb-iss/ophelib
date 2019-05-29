# helper function to extract version from changelog
function(get_version_changelog)
    file(READ "${CMAKE_SOURCE_DIR}/CHANGELOG" CHANGELOG_CONTENTS LIMIT 50)
    string(REGEX MATCH "v ([^\n]+)" CHANGELOG_CONTENTS "${CHANGELOG_CONTENTS}")
    string(REGEX REPLACE "^v " ""  CHANGELOG_CONTENTS "${CHANGELOG_CONTENTS}")
    string(REGEX REPLACE "[()]" ""  CHANGELOG_CONTENTS "${CHANGELOG_CONTENTS}")
    string(REGEX REPLACE " " "-"  CHANGELOG_CONTENTS "${CHANGELOG_CONTENTS}")
    set(VERSION_CHANGELOG "${CHANGELOG_CONTENTS}" PARENT_SCOPE)
endfunction()

# helper function to extract git ref from repo
function(get_version_git_ref)
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --revs-only HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_REF
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE GIT_ERROR)
        if(${GIT_ERROR} EQUAL 0)
            set(VERSION_GIT_REF "${GIT_REF}" PARENT_SCOPE)
            set(VERSION_GIT_REF "${GIT_REF}")
        endif()
    endif()

    if(NOT DEFINED VERSION_GIT_REF)
        set(VERSION_GIT_REF "unknown" PARENT_SCOPE)
    endif()
endfunction()
