cmake_minimum_required(VERSION 3.5)
project(OPHELibSample LANGUAGES CXX)

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/../cmake")
find_package(NTL REQUIRED)
find_package(GMP REQUIRED)
find_package(GMPXX REQUIRED)
find_package(OPHELib REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenMP)

add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world OPHELib::ophelib)
