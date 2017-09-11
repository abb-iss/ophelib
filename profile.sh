#!/bin/bash
[ -f Makefile ] && make clean
rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile include/ophelib/wire/*_generated.h
cmake -DPROFILING_BUILD=ON .
make -j 4

# gprof
bin/perf_base_ops
gprof bin/perf_base_ops gmon.out > analysis.txt

# callgrind
rm callgrind.out.*
valgrind --tool=callgrind bin/perf_base_ops
callgrind_annotate callgrind.out.* > analysis_kg.txt
