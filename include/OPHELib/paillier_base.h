#pragma once

#include "OPHELib/integer.h"
#include "OPHELib/fast_mod.h"
#include "OPHELib/error.h"

#include <memory>
#include <vector>

namespace OPHELib {

    /**
     * An encrypted Integer value
     */
    class Ciphertext {
    public:
        /**
         * This contains the actual encrypted data
         */
        Integer data;

        /**
         * Shared pointer to the modulus, so we can do operations
         * without need for the Paillier class.
         */
        std::shared_ptr<Integer> n2_shared;

        /**
         * Shared pointer to the FastMod. An instance is tied to a
         * specific modulus (n^2), so it makes sense to keep one around
         * for every pub/priv key.
         */
        std::shared_ptr<FastMod> fast_mod;

        Ciphertext(const Integer &data, const std::shared_ptr<Integer> &n2_shared, const std::shared_ptr<FastMod> &fast_mod);
        Ciphertext(const Integer &data, const std::shared_ptr<Integer> &n2_shared);
        Ciphertext(const Integer &data);
        Ciphertext();

        /**
         * Compare data. Encryption moduli are not compared.
         */
        bool operator==(const Ciphertext &input) const;

        /**
         * Compare data. Encryption moduli are not compared.
         */
        bool operator!=(const Ciphertext &input) const;

        /**
         * Unary -
         */
        Ciphertext operator-() const;

        /**
         * Add ciphertexts
         */
        Ciphertext operator+(const Ciphertext &other) const;
        void operator+=(const Ciphertext &other);

        /**
         * Binary -. This is quite a bit slower than
         * +, as we first have to negate the second ciphertext
         * and then add the two.
         */
        Ciphertext operator-(const Ciphertext &other) const;
        void operator-=(const Ciphertext &other);

        /**
         * Scalar multiplication
         */
        Ciphertext operator*(const Integer &other) const;
        void operator*=(const Integer &other);

        /**
         * String representation
         */
        const std::string to_string(const bool brief = true) const;
    };

    std::ostream& operator<<(std::ostream& stream, const Ciphertext& c);

    /**
     * Paillier private key. p^2 and p^q are not stored here,
     * they are precomputed in the Paillier class if needed.
     */
    class PrivateKey {
    public:
        size_t key_size_bits;
        size_t a_bits;

        Integer p;
        Integer q;
        /**
         * Not all implementations use this
         */
        Integer a;

        PrivateKey(const size_t key_size_bits, const Integer &p, const Integer &q);
        PrivateKey(const size_t key_size_bits, const size_t a_bits, const Integer &p, const Integer &q, const Integer &a);
        PrivateKey();

        bool operator==(const PrivateKey &input) const;

        const std::string to_string(const bool brief = true) const;
    };

    class PublicKey {
    public:
        size_t key_size_bits;

        Integer n;
        Integer g;

        PublicKey(const size_t keysize, const Integer &n, const Integer &g);
        PublicKey();

        bool operator==(const PublicKey &input) const;

        const std::string to_string(const bool brief = true) const;
    };

    /**
     * Wrapper helper class
     */
    class KeyPair {
    public:
        PublicKey pub;
        PrivateKey priv;

        KeyPair(const PublicKey &pub, const PrivateKey &priv);
        KeyPair();

        bool operator==(const KeyPair &input) const;

        const std::string to_string(const bool brief = true) const;
    };

    /**
     * Base class from which all Paillier implementations are derived.
     * Contains some of the basic data fields which every implementation
     * needs, and functions which can be shared between implementations
     * (such as packing). This class is abstract and cannot be used directly.
     *
     * @TODO
     * Split this into PaillierBasePub and PaillierBasePriv, so we can
     * have static type checking on the fact if we have a private key or not.
     */
    class PaillierBase {
    protected:
        /**
         * Size of key in bits
         */
        const size_t key_size_bits;

        /**
         * Maximum size of bits a plaintext to encrypt can have.
         * Usually same as key size.
         */
        const size_t plaintxt_size_bits;

        PrivateKey priv;
        PublicKey pub;

        bool have_priv = false;
        bool have_pub = false;

        std::shared_ptr<Integer> n2_shared;
        std::shared_ptr<FastMod> fast_mod;

        /**
         * Before encryption, negative numbers will be converted to
         * positive numbers by transfering them to the space above
         * this boundary.
         */
        Integer pos_neg_boundary;

        /**
         * Minimum value plaintext numbers to encrypt can have
         */
        Integer plaintxt_lower_boundary;

        /**
         * Maximum value plaintext numbers to encrypt can have
         */
        Integer plaintxt_upper_boundary;

    public:
        PaillierBase(const size_t key_size_bits);
        PaillierBase(const PublicKey &pub);
        PaillierBase(const PublicKey &pub, const PrivateKey &priv);
        PaillierBase(const KeyPair &pair);

        const PublicKey &get_pub() const;
        const PrivateKey &get_priv() const;
        const KeyPair get_keypair() const;

        /**
         * Minimum value plaintext numbers to encrypt can have
         */
        virtual const Integer plaintext_lower_boundary() const;

        /**
         * Maximum value plaintext numbers to encrypt can have
         */
        virtual const Integer plaintext_upper_boundary() const;

        /**
         * How many bits a ciphertext will need at max.
         */
        size_t ciphertext_size_bits() const;

        /**
         * Maximum size of bits a plaintext to encrypt can have.
         */
        size_t plaintext_size_bits() const;

        /**
         * Get a shared pointer to the FastMod class for the n^2 currently
         * used.
         */
        const std::shared_ptr<FastMod> get_fast_mod() const;

        /**
         * Get a shared pointer to the modulus
         */
        const std::shared_ptr<Integer> get_n2() const;

        /**
         * Generate a pub/priv keypair
         */
        virtual void generate_keys() = 0;

        /**
         * Decrypt ciphertext
         */
        virtual Integer decrypt(const Ciphertext &ciphertext) const = 0;

        /**
         * Encrypt ciphertext
         */
        virtual Ciphertext encrypt(const Integer &plaintext) const = 0;

        virtual const std::string to_string(const bool brief = true) const;
    };
}
