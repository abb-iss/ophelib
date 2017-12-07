#pragma once

#include "ophelib/paillier_base.h"

#include <memory>

#include <gmpxx.h>

namespace ophelib {

    /**
     * Fast implementation of the Paillier algorithm, using some
     * mathematical tricks as well as lookup tables (most importantly
     * for fast ciphertext randomization).
     *
     * The table below lists the supported key sizes and different
     * parameters which depend on it. Data from:
     * - `NIST Special Publication 800-57 Part 1, Revision 4`, `Table 2: Comparable strengths`
     * - `Encryption Performance Improvements of the Paillier Cryptosystem`, paper by
     *   Christine Jost, Ha Lam, Alexander Maximov, and Ben Smeets
     * - the `compute_randomizer_params` program included in this source tree
     *
     * | Strength | n bits | α bits | r bits | r lut✝ | r use‡ |
     * |----------|--------|--------|--------|---------|--------|
     * | 80*      | 1024   | 320    | 80     | 256     | 15     |
     * | 112      | 2048   | 512    | 112    | 4096    | 12     |
     * | 128      | 3072   | 512    | 128    | 4096    | 14     |
     * | ~140     | 4096   | 512    | 140    | 8192    | 14     |
     * | 192      | 7680   | 1024   | 192    | 16384   | 18     |
     *
     * - ✝ Size of randomizer lookup table. Can be chosen arbitrarily, a tradeoff
     *   between lookup table size and `r use` has to be considered.
     * - ‡ How many values to select from the lookup table each time. Depends on
     *   `r bits` and `r lut`, compute it using `compute_randomizer_params`.
     * - * **Do not use in production, insufficent security level!**
     *
     * The correct parameters prom this table will be chosen automatically
     * in the constructor when passsing a key size.
     */
    class PaillierFast : public PaillierBase {
    public:

        /**
         * Basic, slow Ciphertext randomizer
         */
        class Randomizer {
        protected:
            const PaillierFast *paillier;
            Integer g_pow_n;
            const Integer r() const;
            bool precomputed = false;

        public:
            Randomizer(const PaillierFast *paillier);

            virtual void precompute();
            /**
             * Get a random value to randomize the ciphertext with
             */
            virtual const Integer get_noise() const;
            virtual const std::string to_string(const bool brief = true) const;
        };

        /**
         * Fast randomizer using a random cache/lookup table
         */
        class FastRandomizer: Randomizer {
            /**
             * Size of lookup table, i.e.
             * number of random `(g^n)^r` to generate
             */
            const size_t r_lut_size;

            /**
             * How many values to select randomly from the
             * lookup table each time get_noise is called
             */
            const size_t r_use_count;

            /**
             * Lookup table
             */
            std::vector<Integer> gn_pow_r;
        public:
            FastRandomizer(const PaillierFast *paillier, const size_t r_lut_size, const size_t r_use_count);

            /**
             * Fill random cache
             */
            void precompute();
            const Integer get_noise() const;
            const std::string to_string(const bool brief = true) const;
        };

    private:
        PaillierFast() = delete;
        PaillierFast(const size_t key_size_bits, const size_t a_bits, const size_t r_bits);

        void check_valid_key_size(size_t key_size_bits) const;
        void check_valid_r_bits(size_t r_bits) const;
        size_t param_a_bits(size_t key_size_bits) const;
        size_t param_r_bits(size_t key_size_bits) const;
        size_t param_r_lut_size(size_t r_bits) const;
        size_t param_r_use_count(size_t r_bits) const;

        void precompute();

        const size_t a_bits;
        const size_t r_bits;
        FastRandomizer randomizer;

        /**
         * pubkey precomputation
         */
        Integer n2;

        /**
         * privkey precomputation
         */
        Integer mu;

        Ciphertext precomputed_zero;

        Integer check_plaintext(const Integer &plaintext) const;

    public:
        /**
         * Initialize a new PaillierFast instance.
         * @param key_size_bits possible values are 1024, 2048, 3072, 4096, 7680.
         *        See PaillierFast detailed description for more
         *        information. **1024 bits should not be used in
         *        production!**
         */
        PaillierFast(const size_t key_size_bits);
        PaillierFast(const PublicKey &pub);
        PaillierFast(const PublicKey &pub, const PrivateKey &priv);
        PaillierFast(const KeyPair &pair);

        void generate_keys() final;
        Integer decrypt(const Ciphertext &ciphertext) const final;
        Ciphertext encrypt(const Integer &plaintext) const final;
        Ciphertext zero_ciphertext() const;

        const std::string to_string(const bool brief = true) const final;
    };
}
