#!/bin/bash
[ -f Makefile ] && make clean
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile include/ophelib/wire/*_generated.h
