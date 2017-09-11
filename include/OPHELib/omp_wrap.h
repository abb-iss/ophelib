#pragma once

#ifdef _OPENMP

/* Include OpenMP definitions, and
   declare a macro for lock variable
   declaration so we can make this
   dependent on _OPENMP */
#include <omp.h>
#define omp_declare_lock(name) omp_lock_t name;

#else

/* NOP out the OpenMP functions */
#define omp_set_nested(x);
#define omp_declare_lock(x);
#define omp_init_lock(x);
#define omp_set_lock(x);
#define omp_unset_lock(x);
#define omp_destroy_lock(x);

#endif
