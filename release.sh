#!/bin/bash
[ -f Makefile ] && make clean
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile include/ophelib/wire/*_generated.h
cmake -DCMAKE_BUILD_TYPE=Release .
make -j 4 VERBOSE=1
