cmake_minimum_required(VERSION 3.2)
project(ophelib)

# get version and git ref
function(get_version_changelog)
    file(READ "${CMAKE_SOURCE_DIR}/CHANGELOG" CHANGELOG_CONTENTS LIMIT 50)
    string(REGEX MATCH "v ([^\n]+)" CHANGELOG_CONTENTS "${CHANGELOG_CONTENTS}")
    string(REGEX REPLACE "^v " ""  CHANGELOG_CONTENTS "${CHANGELOG_CONTENTS}")
    string(REGEX REPLACE "[()]" ""  CHANGELOG_CONTENTS "${CHANGELOG_CONTENTS}")
    string(REGEX REPLACE " " "-"  CHANGELOG_CONTENTS "${CHANGELOG_CONTENTS}")
    set(VERSION_CHANGELOG "${CHANGELOG_CONTENTS}" PARENT_SCOPE)
endfunction()
function(get_version_git_ref)
    execute_process(COMMAND git rev-parse --revs-only --git-dir="${CMAKE_SOURCE_DIR}/.git" HEAD
            OUTPUT_VARIABLE GIT_REF
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE GIT_ERROR)
    if(${GIT_ERROR} EQUAL 0)
        set(VERSION_GIT_REF "${GIT_REF}" PARENT_SCOPE)
    else()
        set(VERSION_GIT_REF "unknown" PARENT_SCOPE)
    endif()
endfunction()
get_version_changelog()
get_version_git_ref()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPHELIB_VERSION='${VERSION_CHANGELOG}'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPHELIB_GIT_REF='${VERSION_GIT_REF}'")

# enable all the compiler warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -D_GLIBCXX_USE_CXX11_ABI=0 -fPIC")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable OpenMP. The unknown pragma warning is disabled
# because gcc shows it despite compiling correctly.
option(ENABLE_OPENMP "parallelize using OpenMP, default ON"
       ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
if(ENABLE_OPENMP)
    find_package(OpenMP)
    if(OPENMP_FOUND)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
    else()
        message(FATAL_ERROR "OpenMP not found!")
    endif()
else()
    message(WARNING "OpenMP disabled by command line option!")
endif()

# pthread
find_package(Threads)
if(NOT Threads_FOUND)
    message(FATAL_ERROR "Threads not found!")
endif()

# debug build
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDEBUG")
endif()

# profiling build
option(PROFILING_BUILD "enable profiling information in binaries")
if(PROFILING_BUILD)
    message(STATUS "Profiling build enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPERF_N_ITER=100")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -fPIC")
    set(PROFILING_LINKER_FLAGS "-static-libgcc -static-libstdc++ -fPIC")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PROFILING_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${PROFILING_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${PROFILING_LINKER_FLAGS}")
endif()

# deps prefix path passed from ENV
if(DEFINED ENV{BUILD_DEPS_PREFIX})
    set(BUILD_DEPS_PREFIX $ENV{BUILD_DEPS_PREFIX})
    message(STATUS "BUILD_DEPS_PREFIX=${BUILD_DEPS_PREFIX}")
else()
    set(BUILD_DEPS_PREFIX ${CMAKE_SOURCE_DIR}/lib_)
endif()

# option to disable usage of exceptions, and stack traces
option(ENABLE_EXCEPTIONS "enable exceptions and stack traces, default ON"
       ON)
if(ENABLE_EXCEPTIONS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPHELIB_ENABLE_EXCEPTIONS")
else()
    message(WARNING "Exceptions disabled by command line option!")
endif()

# add gmp
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
find_library(GMP gmp
        PATHS ${BUILD_DEPS_PREFIX}/install/lib)
find_library(GMP_STATIC libgmp.a
        PATHS ${BUILD_DEPS_PREFIX}/install/lib)
include_directories(${BUILD_DEPS_PREFIX}/install/include)
set(GMP ${GMP_LIBRARIES})
set(GMP_STATIC ${GMP_STATIC_LIBRARIES})
if("${GMP}" STREQUAL "GMP-NOTFOUND")
    message(FATAL_ERROR "libgmp not found at ${BUILD_DEPS_PREFIX}/install/lib!")
endif()
if("${GMP_STATIC}" STREQUAL "GMP_STATIC-NOTFOUND")
    message(FATAL_ERROR "libgmp.a not found at ${BUILD_DEPS_PREFIX}/install/lib!")
endif()

# add ntl
find_library(NTL ntl
        PATHS ${BUILD_DEPS_PREFIX}/install/lib)
find_library(NTL_STATIC libntl.a
        PATHS ${BUILD_DEPS_PREFIX}/install/lib)

set(NTL ${NTL_LIBRARIES})
set(NTL_STATIC ${NTL_STATIC_LIBRARIES})
if("${NTL}" STREQUAL "NTL-NOTFOUND")
    message(FATAL_ERROR "libntl not found at ${BUILD_DEPS_PREFIX}/install/lib!")
endif()
if("${NTL_STATIC}" STREQUAL "NTL_STATIC-NOTFOUND")
    message(FATAL_ERROR "libntl.a not found at ${BUILD_DEPS_PREFIX}/install/lib!")
endif()

# flatbuffers
set(FLATC_BINARY ${BUILD_DEPS_PREFIX}/flatbuffers/flatc)
if(EXISTS ${FLATC_BINARY})
    message(STATUS "Found flatbuffers compiler '${FLATC_BINARY}'")
else()
    message(FATAL_ERROR "Unable to find flatbuffers compiler!")
endif()
include_directories(${BUILD_DEPS_PREFIX}/flatbuffers/include)
function(compile_flatbuffers_schema_to_cpp SRC_FBS GEN_FILE_NAME_VAR)
    get_filename_component(SRC_FBS_DIR ${SRC_FBS} PATH)
    string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER ${SRC_FBS})
    add_custom_command(
            OUTPUT ${GEN_HEADER}
            DEPENDS ${SRC_FBS}
            DEPENDS ${FLATC_BINARY}
            COMMAND "${FLATC_BINARY}" -c --no-includes --gen-mutable -o "${SRC_FBS_DIR}" "${SRC_FBS}"
    )
    set("${GEN_FILE_NAME_VAR}" "${GEN_HEADER}" PARENT_SCOPE)
endfunction()

# allow to pass additional compiler and linker flags from outside
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADDITIONAL_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ADDITIONAL_LD_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${ADDITIONAL_LD_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ADDITIONAL_LD_FLAGS}")

# ophelib library
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/wire)
aux_source_directory(${PROJECT_SOURCE_DIR}/src LIBOPHELIB_SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/include LIBOPHELIB_HEADERS)
list(REMOVE_ITEM LIBOPHELIB_SOURCES ${PROJECT_SOURCE_DIR}/src/compute_randomizer_params.cpp)
if(NOT ENABLE_EXCEPTIONS)
    # do not include backtrace code if exceptions are enabled
    list(REMOVE_ITEM LIBOPHELIB_SOURCES ${PROJECT_SOURCE_DIR}/src/backtrace.cpp)
endif()

set(OPHELIB_WIRE_DIR ${PROJECT_SOURCE_DIR}/include/ophelib/wire)
file(GLOB FBS_FILES ${OPHELIB_WIRE_DIR}/*.fbs)
foreach(file ${FBS_FILES})
    compile_flatbuffers_schema_to_cpp("${file}" GENERATED_FILENAME)
    message(STATUS ${GENERATED_FILENAME})
    list(APPEND LIBOPHELIB_SOURCES ${GENERATED_FILENAME})
endforeach()

add_library(ophelib SHARED ${LIBOPHELIB_SOURCES} ${LIBOPHELIB_HEADERS})
target_link_libraries(ophelib ${NTL} ${GMP} ${CMAKE_THREAD_LIBS_INIT})
add_library(ophelib_static STATIC ${LIBOPHELIB_SOURCES} ${LIBOPHELIB_HEADERS})
target_link_libraries(ophelib_static ${NTL_STATIC} ${GMP_STATIC} ${CMAKE_THREAD_LIBS_INIT})
set_target_properties(ophelib_static PROPERTIES OUTPUT_NAME ophelib)

# ophelib tests
aux_source_directory(${PROJECT_SOURCE_DIR}/test TEST_SOURCES)
list(REMOVE_ITEM TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/perf_base_ops.cpp)
list(REMOVE_ITEM TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/perf_packing.cpp)
list(REMOVE_ITEM TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/hello_world.cpp)
list(REMOVE_ITEM TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/ophelib_dump.cpp)
list(REMOVE_ITEM TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/perf_vector_parallel.cpp)
add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests ${CMAKE_THREAD_LIBS_INIT} ophelib_static)

# ophelib parallel vector tests
add_executable(perf_vector_parallel
        ${PROJECT_SOURCE_DIR}/test/perf_vector_parallel.cpp
        ${PROJECT_SOURCE_DIR}/test/catch.hpp
        ${PROJECT_SOURCE_DIR}/test/run_tests.cpp)
target_link_libraries(perf_vector_parallel ophelib_static)

option(BUILD_RUN_TESTS "run tests after build")
if(BUILD_RUN_TESTS)
    set(TEST_PROG_ARGS --durations yes)
    if(${CMAKE_CROSSCOMPILING} STREQUAL TRUE)
        add_custom_command(TARGET tests POST_BUILD COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests.js ${TEST_PROG_ARGS})
    else()
        add_custom_command(TARGET tests POST_BUILD COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests ${TEST_PROG_ARGS})
    endif()
endif()

# ophelib perf_base_ops
include_directories(${PROJECT_SOURCE_DIR}/src)
add_executable(perf_base_ops ${PROJECT_SOURCE_DIR}/test/perf_base_ops.cpp)
target_link_libraries(perf_base_ops ophelib_static)

# ophelib perf_packing
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_executable(perf_packing ${PROJECT_SOURCE_DIR}/test/perf_packing.cpp)
target_link_libraries(perf_packing ophelib_static)

# ophelib compute_randomizer_params
add_executable(compute_randomizer_params src/compute_randomizer_params.cpp)
target_link_libraries(compute_randomizer_params ophelib_static)

# ophelib hello_world
add_executable(hello_world ${PROJECT_SOURCE_DIR}/test/hello_world.cpp)
target_link_libraries(hello_world ophelib_static)

# ophelib dump helper
add_executable(ophelib_dump ${PROJECT_SOURCE_DIR}/test/ophelib_dump.cpp)
target_link_libraries(ophelib_dump ophelib_static)
