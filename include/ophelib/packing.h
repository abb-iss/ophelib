#pragma once

#include "ophelib/vector.h"
#include "ophelib/paillier_base.h"

namespace ophelib {

    /**
     * Represents a ciphertext which contains multiple
     * plaintext values packed in a single Integer.
     * Wraps a normal Ciphertext (data).
     */
    class PackedCiphertext {
    public:
        Ciphertext data;

        /**
         * Number of plaintexts packed
         */
        size_t n_plaintexts;

        /**
         * Size in bit of every packed plaintext
         */
        size_t plaintext_bits;

        PackedCiphertext(const Ciphertext &data, const size_t n_plaintexts, const size_t plaintext_bits);
        PackedCiphertext();

        /**
         * Compare data. Encryption moduli are not compared.
         */
        bool operator==(const PackedCiphertext &input) const;

        /**
         * Compare data. Encryption moduli are not compared.
         */
        bool operator!=(const PackedCiphertext &input) const;

        const std::string to_string(const bool brief = true) const;
    };

    std::ostream& operator<<(std::ostream& stream, const PackedCiphertext& c);

    namespace Vector {
        /**
         * How much buffer (bits) to add in front of each number
         * when packing Integers
         */
        static const size_t pack_buffer = 1;

        /**
         * How many plaintexts of a given size can be fit into a single packed
         * ciphertext. Use this to determine the max size of the vector you pass to
         * encrypt_pack().
         * @param plaintext_bits Maximum bit size of every plaintext Integer
         *                       which will later be passed to encrypt_pack()
         * @param pai paillier instance, needed for determining plaintext size
         */
        size_t pack_count(const size_t plaintext_bits, const PaillierBase &pai);

        /**
         * Pack multiple ciphertexts into a single one.
         * This variation takes an iterator instead of a Vector.
         * @param ciphertexts_begin iterator begin
         * @param ciphertexts_end iterator end
         * @param plaintext_bits how many (plaintext) bits each ciphertext contains
         *        at max
         * @param pai paillier instance, needed for determining plaintext size
         */
        PackedCiphertext pack_ciphertexts(const Ciphertext *ciphertexts_begin, const Ciphertext *ciphertexts_end, const size_t plaintext_bits, const PaillierBase &pai);

        /**
         * Pack multiple ciphertexts into a single one.
         * @param ciphertexts vector, must have `length <= pack_count(plaintext_bits)`.
         * @param plaintext_bits how many (plaintext) bits each ciphertext contains
         *        at max
         * @param pai paillier instance, needed for determining plaintext size
         */
        PackedCiphertext pack_ciphertexts(const Vec<Ciphertext> &ciphertexts, const size_t plaintext_bits, const PaillierBase &pai);

        /**
         * Pack a vector of ciphertexts into a vector of packed plaintexts.
         * @param ciphertexts arbitrary size vector
         * @param plaintext_bits how many (plaintext) bits each ciphertext contains
         *        at max
         * @param pai paillier instance, needed for determining plaintext size
         *
         * The difference to pack_ciphertexts() is that this function supports
         * arbitrary length vectors. This means it will split the input vector
         * into chunks which each fit into a packed ciphertext.
         */
        Vec<PackedCiphertext> pack_ciphertexts_vec(const Vec<Ciphertext> &ciphertexts, const size_t plaintext_bits, const PaillierBase &pai);

        /**
         * Encrypt a vector of plaintexts packed in a single ciphertext.
         * This variation takes an iterator instead of a Vector.
         * @param plaintexts_begin iterator begin
         * @param plaintexts_end iterator end
         * @param plaintext_bits maximum bit size each plaintext has
         *        (determined by `plaintexts[i].size_bits()`).
         * @param pai paillier instance, needed for determining plaintext size
         */
        PackedCiphertext encrypt_pack(const Integer *plaintexts_begin, const Integer *plaintexts_end, const size_t plaintext_bits, const PaillierBase &pai);

        /**
         * Encrypt a vector of plaintexts packed in a single ciphertext.
         * @param plaintexts vector, must have `length <= pack_count(plaintext_bits)`.
         * @param plaintext_bits maximum bit size each plaintext has
         *        (determined by `plaintexts[i].size_bits()`).
         * @param pai paillier instance, needed for determining plaintext size
         */
        PackedCiphertext encrypt_pack(const Vec<Integer> &plaintexts, const size_t plaintext_bits, const PaillierBase &pai);

        /**
         * Encrypt a vector of plaintexts into a vector of packed plaintexts.
         * @param plaintexts arbitrary size vector
         * @param plaintext_bits maximum bit size each plaintext has
         *        (determined by `plaintexts[i].size_bits()`).
         * @param pai paillier instance, needed for determining plaintext size
         *
         * The difference to encrypt_pack() is that this function supports arbitrary
         * length vectors. This means it will split the plaintext vector
         * into chunks which each fit into a packed ciphertext.
         *
         * @TODO
         * Note that we could also determine plaintext_bits dynamically, even for each
         * separate chunk, to save space. However, this leaks information as it could
         * then be seen in plaintext how many bits are used e.g. during a certain period
         * in time series data. However if we wrapped the size information into the encrypted
         * value too, this could be avoided and possible some space be saved.
         */
        Vec<PackedCiphertext> encrypt_pack_vec(const Vec<Integer> &plaintexts, const size_t plaintext_bits, const PaillierBase &pai);

        /**
         * Decrypt packed ciphertext and put results in the plaintext vector.
         * This variation takes an iterator instead of a Vector. So, no
         * resizing is going to happen and it's the callers duty.
         * @param plaintexts_begin iterator begin
         * @param plaintexts_end iterator end
         * @param pai paillier instance, needed for determining plaintext size
         */
        void decrypt_pack(const PackedCiphertext &ciphertext, Integer *plaintexts_begin, Integer *plaintexts_end, const PaillierBase &pai);

        /**
         * Decrypt packed ciphertext and return plaintext vector.
         * Counterpart to pack_ciphertexts() and encrypt_pack().
         */
        Vec<Integer> decrypt_pack(const PackedCiphertext &ciphertext, const PaillierBase &pai);

        /**
         * Decrypt packed ciphertext and put results in the plaintext vector.
         * The vector will be reset to size 0 before filling.
         * Counterpart to pack_ciphertexts() and encrypt_pack()
         */
        void decrypt_pack(const PackedCiphertext &ciphertext, Vec<Integer> &plaintexts, const PaillierBase &pai);

        /**
         * Decrypt packed ciphertexts and return plaintext vector.
         * Counterpart to pack_ciphertexts_vec() and encrypt_pack_vec()
         */
        Vec<Integer> decrypt_pack(const Vec<PackedCiphertext> &ciphertexts, const PaillierBase &pai);

        /**
         * Decrypt packed ciphertexts and put results in the plaintext vector
         * The vector will be reset to size 0 before filling.
         * Counterpart to pack_ciphertexts_vec() and encrypt_pack_vec()
         */
        void decrypt_pack(const Vec<PackedCiphertext> &ciphertexts, Vec<Integer> &plaintexts, const PaillierBase &pai);

        /**
         * Decrypt each ciphertext in a vector. Uses packing before decryption
         * for speedup.
         * @param cipher ciphertext vector
         * @param pai paillier instance used for packing and decryption
         * @param plaintext_bits how many bits of plaintext each ciphertext
         *        maximally contains
         */
        Vec<Integer> decrypt_fast(const Vec<Ciphertext> &cipher, const PaillierBase &pai, const size_t plaintext_bits);

        /**
         * Decrypt each ciphertext in a matrix. Uses packing before decryption
         * for speedup.
         * @param cipher ciphertext matrix
         * @param pai paillier instance used for packing and decryption
         * @param plaintext_bits how many bits of plaintext each ciphertext
         *        maximally contains
         */
        Mat<Integer> decrypt_fast(const Mat<Ciphertext> &cipher, const PaillierBase &pai, const size_t plaintext_bits);
    }
}
