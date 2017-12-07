#include "ophelib/random.h"
#include "ophelib/error.h"

#include <fstream>

namespace ophelib {
    Random::Random() {
        std::ifstream urandom("/dev/urandom", std::ios::binary);
        unsigned long seed = 0;
        if (urandom.is_open()) {
            urandom.read((char *)&seed, sizeof(seed));
            urandom.close();
        } else {
            error_exit("could not open /dev/urandom");
        }

        gmp_randinit_default(state);
        gmp_randseed_ui(state, seed);
    }

    Random::~Random() {
        gmp_randclear(state);
    }

    Integer Random::rand_int(const Integer &max) {
        if(max < 2)
            error_exit("max must be > 1");

        Integer ret;
        mpz_urandomm(ret.get_mpz_t(), state, max.get_mpz_t());
        return ret;
    }

    Integer Random::rand_int_bits(const size_t n_bits) {
        if(n_bits < 1)
            error_exit("n_bits must be > 0");

        Integer ret;
        mpz_urandomb(ret.get_mpz_t(), state, n_bits);
        return ret;
    }

    Integer Random::rand_prime(const size_t n_bits) {
        if(n_bits < 2)
            error_exit("n_bits must be > 1");

        while(true) {
            //generate a random number in the interval [0, 2^(n_bits - 1))
            Integer ret = rand_int_bits(n_bits - 1);

            //shift number to the interval [2^(n_bits - 1), 2^n_bits)
            mpz_setbit(ret.get_mpz_t(), n_bits - 1);

            if(ret.is_prime()) {
                return ret;
            }
        }
    }
}
