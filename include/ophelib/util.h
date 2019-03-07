#pragma once

#include "ophelib/integer.h"

#include <vector>
#include <iostream>
#include <chrono>

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

namespace ophelib {
    /**
     * Binomial coefficient
     */
    const Integer nCr(const Integer n_, const Integer r_);

    /**
     * Binomial coefficient
     */
    long long nCr(const long &n, const long &r);

    /**
     * Get a temp file name. **NOT SECURE**! Only use for tests.
     * The file will not yet exist.
     */
    std::string temp_name();

    /**
     * Git ref string of the current build
     */
    std::string ophelib_ref();

    /**
     * Version of the current build from the changelog
     */
    std::string ophelib_version();

    /**
     * If was compiled with OpenMP support
     */
    bool ophelib_openmp_enabled();

    /**
     * Helper class for performance measurements.
     * All output is printed to stderr.
     */
    class StopWatch {
        std::chrono::high_resolution_clock::time_point t0, t1;
        long long d;
        const std::string name;
        const int n_iter;

    public:

        /**
         * Initialize a new stopwatch.
         * @param name to use when printing. Must not contain a semicolon.
         * @param n_iter divisor to use when calculating
         *        time per iteration
         */
        StopWatch(std::string name, int n_iter);

        /**
         * Print a CSV (actually semicolons are used as separators)
         * header which shows the columns printed by stop(). Columns:
         *
         * * name: as passed to constructor
         * * n_iter_: as passed to constructor
         * * t_s: total time in seconds
         * * dt_ms: time in milliseconds per iteration
         */
        static void header();

        /**
         * Start measuring
         */
        void start();

        /**
         * Stop measuring, and optionally print a CSV column
         * containing the measurements
         *
         * @return measured total time in microseconds
         */
        long long stop(const bool print = true);
    };
}
