#pragma once

#include "ophelib/paillier_base.h"

namespace ophelib {

    /**
     * Reference implementation of the classic Paillier algorithm.
     * This is quite slow compared to PaillierFast and should thus not
     * be used except for tests.
     */
    class Paillier : public PaillierBase {

        Paillier() = delete;

        void precompute();

        // pubkey precomputations
        Integer n_minus_one;
        Integer n2;

        // privkey precomputations
        Integer lambda, mu;

        Ciphertext randomize(Ciphertext ciphertext) const;
        Integer randomizer_val() const;
        Ciphertext encrypt_no_rand(const Integer &plaintext) const;

    public:
        Paillier(const size_t key_size_bits) : PaillierBase(key_size_bits) { }
        Paillier(const PublicKey &pub);
        Paillier(const PublicKey &pub, const PrivateKey &priv);
        Paillier(const KeyPair &pair);

        void generate_keys();
        Integer decrypt(const Ciphertext &ciphertext) const;
        Ciphertext encrypt(const Integer &plaintext) const;

        const std::string to_string(const bool brief = true) const;
    };

}
