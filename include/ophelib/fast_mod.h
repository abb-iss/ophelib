#pragma once

#include "ophelib/integer.h"

namespace ophelib {

    /*
     * Reduction of modulo `p^2*q^2` in the first step is easy to do – simply
     * compute everything modulo `p^2` and `q^2` in parallel then use the Chinese
     * Remainder Theorem (CRT) to combine the result. However, we found it
     * much faster to further split `p^2` (and `q^2`) as well and represent a
     * value A as `a0+a1*p modulo p^2`. Then, computing `A*B` mod `p^2` is better
     * (faster) done with such a representation, where you convert A into a
     * base p number => E.g.
     *   `A * B = a0 * b0 + p * (a0 * b1 + a1 * b0) + p^2 * (a1 * b1) mod p^2
     *   = a0 * b0 + p * (a0 * b1 + a1 * b0)`, not to forget the carry bit.
     * This is much faster than plain mod p^2 computation.
     *
     * IF YOU PLAN ON MODIFYING THIS CODE TO MAKE IT FASTER, FIRST TAKE
     * A LOOK AT THE CODE DELETED IN COMMIT 1e70dd1.
     */
    class FastMod {
        const Integer p, q, p2, q2, n, n2;

    public:

        /**
         * @param p from private key
         * @param q from private key
         */
        FastMod(const Integer &p, const Integer &q);

        /**
         * Variant of the constructor where you can pass more
         * precomputed values to save some CPU time
         */
        FastMod(const Integer &p, const Integer &q, const Integer &p2, const Integer &q2, const Integer &n, const Integer &n2);

        const Integer &get_n2() const;

        /**
         * First part of the acceleration: split the (mod n^2) calculation
         * into two calculations (mod p^2) and (mod q^2). The use the
         * chinese reminder theorem to combine the results.
         * Is about 1.5x faster than "classic" pow_mod_n2.
         *
         * * fast pow completed in 1.54122s
         * * pow completed in 2.55037s
         */
        Integer pow_mod_n2(const Integer &base, const Integer &exp) const;

        /**
         * Same as pow_mod_n2 but parallelized.
         * Is about 3x faster than "classic" pow_mod_n2.
         *
         * * fast pow parallel completed in 0.820312s
         * * pow completed in 2.55037s
         */
        Integer pow_mod_n2_par(const Integer &base, const Integer &exp) const;
    };
}
