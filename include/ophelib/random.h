#pragma once

#include "ophelib/integer.h"

namespace ophelib {

    /**
     * Random provider. Wraps GMPs mpz_urandomm(). Is implemented as a singleton,
     * so you can't instantiate it. Get an instance via instance().
     */
    class Random {
    public:
        /**
         * Get an instance.
         */
        static Random& instance() {
            static Random _instance;
            return _instance;
        }
        ~Random();

        /**
         * Get an integer <= max.
         */
        Integer rand_int(const Integer &max);

        /**
         * Generate a random integer which is at most n_bits long.
         * Might be a bit shorter if the first random digits are zero.
         */
        Integer rand_int_bits(const size_t n_bits);

        /**
         * Generate a random prime which is n_bits long.
         * The number is guaranteed to have exactly n_bits bits.
         */
        Integer rand_prime(const size_t n_bits);
    private:
        Random();
        Random( const Random& ) {};
        Random & operator = (const Random &) { return *this; };

        gmp_randstate_t state;
    };
}
