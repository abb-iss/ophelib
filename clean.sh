#!/bin/bash
[ -f Makefile ] && make clean
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile include/ophelib/wire/*_generated.h demo/hello_world demo/rand.txt bin/rand.txt
