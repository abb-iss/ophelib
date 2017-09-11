#!/bin/bash
[ -f Makefile ] && make clean
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile include/ophelib/wire/*_generated.h
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_RUN_TESTS=ON .
make -j 4 VERBOSE=1

#valgrind --leak-check=yes bin/tests
