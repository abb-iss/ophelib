cmake_minimum_required(VERSION 3.5)
project(OPHELib LANGUAGES CXX)

#
# Dependencies
#

# find dependencies
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")
find_package(NTL REQUIRED)
find_package(GMP REQUIRED)
find_package(GMPXX REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenMP)

# sync submodules
include(UpdateSubmodules)
sync_submodules()

# add flatbuffers from subdirectory
set(FLATBUFFERS_SRC_DIR "${CMAKE_SOURCE_DIR}/extern/flatbuffers")
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "")
add_subdirectory("${FLATBUFFERS_SRC_DIR}"
                 "${CMAKE_CURRENT_BINARY_DIR}/flatbuffers-build"
                 EXCLUDE_FROM_ALL)

# generate C++ headers from flatbuffer schemas
set(ophelib_flatbuffers_generated_includes_dir
    "${CMAKE_CURRENT_BINARY_DIR}/include/ophelib/schemas")
file(GLOB_RECURSE ophelib_flatbuffers_schemas
                  "${CMAKE_CURRENT_LIST_DIR}/schemas/*.fbs")
set(ophelib_schema_include_dirs "${CMAKE_CURRENT_LIST_DIR}/schemas")
build_flatbuffers("${ophelib_flatbuffers_schemas}"
                  "${ophelib_schema_include_dirs}"
                  "ophelib_generated_includes"
                  ""
                  "${ophelib_flatbuffers_generated_includes_dir}"
                  ""
                  "")

#
# Library definition
#

# add OPHELib target
aux_source_directory("${PROJECT_SOURCE_DIR}/src" LIBOPHELIB_SOURCES)
add_library(ophelib "${LIBOPHELIB_SOURCES}")
add_library(OPHELib::ophelib ALIAS ophelib)
target_compile_features(ophelib PRIVATE cxx_std_11)
target_compile_options(
  ophelib PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -pedantic>)
target_compile_options(
  ophelib PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -pedantic>)

# add all dependencies to target
add_dependencies(ophelib ophelib_generated_includes)
target_include_directories(
  ophelib
  PUBLIC
  "$<INSTALL_INTERFACE:include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
  "$<BUILD_INTERFACE:${FLATBUFFERS_SRC_DIR}/include>"
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(ophelib
                      PRIVATE
                      ${LIBNTL_LIBRARIES}
                      ${LIBGMP_LIBRARIES}
                      ${LIBGMPXX_LIBRARIES}
                      Threads::Threads)
if(OpenMP_CXX_FOUND)
  target_compile_options(ophelib PRIVATE
                         $<$<COMPILE_LANGUAGE:CXX>:-DOPHELIB_OPENMP_ENABLED>)
  target_link_libraries(ophelib PUBLIC OpenMP::OpenMP_CXX)
endif()

# compile version and git ref into library
include(GetVersion)
get_version_changelog()
get_version_git_ref()
message(STATUS "Version: ${VERSION_CHANGELOG}, Git ref: ${VERSION_GIT_REF}")
target_compile_options(
  ophelib PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-DOPHELIB_VERSION='${VERSION_CHANGELOG}'>)
target_compile_options(
  ophelib PRIVATE
  $<$<COMPILE_LANGUAGE:CXX>:-DOPHELIB_GIT_REF='${VERSION_GIT_REF}'>)

#
# Binaries definition
#

add_executable(ophelib_compute_randomizer_params
               bin/ophelib_compute_randomizer_params.cpp)
target_link_libraries(ophelib_compute_randomizer_params ophelib)

add_executable(ophelib_dump bin/ophelib_dump.cpp)
target_link_libraries(ophelib_dump ophelib)

add_executable(ophelib_version bin/ophelib_version.cpp)
target_link_libraries(ophelib_version ophelib)

#
# Installing
#

include(GNUInstallDirs)

install(TARGETS ophelib_compute_randomizer_params ophelib_dump ophelib_version
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/OPHELib)

install(TARGETS ophelib
        EXPORT OPHELibTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(EXPORT OPHELibTargets
               FILE
               OPHELibTargets.cmake
               NAMESPACE
               OPHELib::
               DESTINATION
               ${INSTALL_CONFIGDIR})

install(DIRECTORY "include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h*")
install(DIRECTORY "extern/flatbuffers/include/" DESTINATION
                  "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h*")
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/" DESTINATION
                  "${CMAKE_INSTALL_INCLUDEDIR}"
        FILES_MATCHING PATTERN "*.h*")

#
# Export
#

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/OPHELibConfigVersion.cmake
  VERSION
  ${VERSION_CHANGELOG}
  COMPATIBILITY
  AnyNewerVersion)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/OPHELibConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/OPHELibConfig.cmake
  INSTALL_DESTINATION
  ${INSTALL_CONFIGDIR})

# install helper files
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/OPHELibConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/OPHELibConfigVersion.cmake
              DESTINATION
              ${INSTALL_CONFIGDIR})

#
# Tests
#

add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE "${PROJECT_SOURCE_DIR}/test")

# main test
add_executable(ophelib_test
               "${PROJECT_SOURCE_DIR}/test/run_tests.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_fastmod.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_integer.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_ml.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_ntl_conv.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_packing.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_paillier.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_paillier_fast.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_random.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_util.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_vector.cpp"
               "${PROJECT_SOURCE_DIR}/test/test_wire.cpp")
target_link_libraries(ophelib_test ophelib Catch)

# parallel vector performance
add_executable(ophelib_perf_vector_parallel
               "${PROJECT_SOURCE_DIR}/test/perf_vector_parallel.cpp"
               "${PROJECT_SOURCE_DIR}/test/catch.hpp"
               "${PROJECT_SOURCE_DIR}/test/run_tests.cpp")
target_link_libraries(ophelib_perf_vector_parallel ophelib Catch)

# base ops performance
add_executable(ophelib_perf_base_ops
               ${PROJECT_SOURCE_DIR}/test/perf_base_ops.cpp)
target_link_libraries(ophelib_perf_base_ops ophelib Catch)

# packing performance
add_executable(ophelib_perf_packing ${PROJECT_SOURCE_DIR}/test/perf_packing.cpp)
target_link_libraries(ophelib_perf_packing ophelib Catch)

enable_testing()
add_test(NAME ophelib_test COMMAND ophelib_test)
add_test(NAME ophelib_perf_vector_parallel COMMAND ophelib_perf_vector_parallel)
add_test(NAME ophelib_perf_base_ops COMMAND ophelib_perf_base_ops)
add_test(NAME ophelib_perf_packing COMMAND ophelib_perf_packing)
