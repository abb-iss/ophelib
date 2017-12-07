#include "ophelib/packing.h"

namespace ophelib {

    PackedCiphertext::PackedCiphertext(const Ciphertext &data_, const size_t n_plaintexts_, const size_t plaintext_bits_)
            : data(data_),
              n_plaintexts(n_plaintexts_),
              plaintext_bits(plaintext_bits_) { }

    PackedCiphertext::PackedCiphertext() { }

    bool PackedCiphertext::operator==(const PackedCiphertext &input) const {
        return plaintext_bits == input.plaintext_bits &&
               n_plaintexts == input.n_plaintexts &&
               data == input.data;
    }

    bool PackedCiphertext::operator!=(const PackedCiphertext &input) const {
        return plaintext_bits != input.plaintext_bits ||
               n_plaintexts != input.n_plaintexts ||
               data != input.data;
    }

    const std::string PackedCiphertext::to_string(const bool brief) const {
        std::ostringstream o("");

        o << "<PackedCiphertext";
        o << " data=" << data.to_string(brief);
        o << " plaintext_bits=" << plaintext_bits;
        o << " n_plaintexts=" << n_plaintexts;
        o << ">";

        return o.str();
    }

    std::ostream &operator<<(std::ostream &stream, const PackedCiphertext &c) {
        stream << c.to_string(false);
        return stream;
    }

    namespace Vector {

        size_t pack_count(const size_t plaintext_bits, const PaillierBase &pai) {
            return pai.plaintext_size_bits() / (plaintext_bits + pack_buffer);
        }

        PackedCiphertext pack_ciphertexts(const Ciphertext *ciphertexts_begin, const Ciphertext *ciphertexts_end, const size_t plaintext_bits, const PaillierBase &pai) {
            const size_t shift = plaintext_bits + pack_buffer;
            const size_t n_ciphertexts = (size_t) (ciphertexts_end - ciphertexts_begin);
            if(n_ciphertexts < 1)
                error_exit("not enough ciphertexts!");
            if(n_ciphertexts > pack_count(plaintext_bits, pai))
                error_exit("too many ciphertexts!");

            const Integer mul = Integer(1) << shift;

            Ciphertext sum = *ciphertexts_begin;
            for(auto iter = ciphertexts_begin + 1; iter < ciphertexts_end; iter++) {
                sum *= mul;
                sum += *iter;
            }

            return PackedCiphertext(
                    sum,
                    n_ciphertexts,
                    plaintext_bits
            );
        }

        PackedCiphertext pack_ciphertexts(const Vec<Ciphertext> &ciphertexts, const size_t plaintext_bits, const PaillierBase &pai) {
            return pack_ciphertexts(ciphertexts.begin(), ciphertexts.end(), plaintext_bits, pai);
        }

        Vec<PackedCiphertext> pack_ciphertexts_vec(const Vec<Ciphertext> &ciphertexts, const size_t plaintext_bits, const PaillierBase &pai) {
            const auto plaintexts_per_pack = pack_count(plaintext_bits, pai);
            const auto n_vectors = ciphertexts.length() / plaintexts_per_pack;
            const auto last_vector = ciphertexts.length() % plaintexts_per_pack;

            Vec<PackedCiphertext> ret;
            ret.SetLength(n_vectors + (last_vector > 0 ? 1 : 0));
            for(size_t i = 0; i < n_vectors; i++) {
                const auto begin = ciphertexts.begin() + i * plaintexts_per_pack;
                const auto end = begin + plaintexts_per_pack;
                ret[i] = pack_ciphertexts(begin, end, plaintext_bits, pai);
            }

            if(last_vector > 0) {
                ret[n_vectors] = pack_ciphertexts(ciphertexts.end() - last_vector,
                                                  ciphertexts.end(),
                                                  plaintext_bits,
                                                  pai);
            }

            return ret;
        }

        PackedCiphertext encrypt_pack(const Integer *plaintexts_begin, const Integer *plaintexts_end, const size_t plaintext_bits, const PaillierBase &pai) {
            const size_t shift = plaintext_bits + pack_buffer;
            const size_t n_plaintexts = (size_t) (plaintexts_end - plaintexts_begin);

            if(n_plaintexts > pack_count(plaintext_bits, pai))
                error_exit("trying to pack too many elements!");

            Integer sum = n_plaintexts > 0 ? *plaintexts_begin : 0;
            for (auto iter = plaintexts_begin + 1; iter < plaintexts_end; iter++) {
                if(iter->size_bits() > plaintext_bits)
                    error_exit("plaintext size too large!");
                sum <<= shift;
                sum += *iter;
            }

            return PackedCiphertext(
                    pai.encrypt(sum),
                    (const size_t) n_plaintexts,
                    plaintext_bits
            );
        }

        PackedCiphertext encrypt_pack(const Vec<Integer> &plaintexts, size_t plaintext_bits, const PaillierBase &pai) {
            return encrypt_pack(plaintexts.begin(), plaintexts.end(), plaintext_bits, pai);
        }

        Vec<PackedCiphertext> encrypt_pack_vec(const Vec<Integer> &plaintexts, const size_t plaintext_bits, const PaillierBase &pai) {
            const auto plaintexts_per_pack = pack_count(plaintext_bits, pai);
            const auto n_vectors = plaintexts.length() / plaintexts_per_pack;
            const auto last_vector = plaintexts.length() % plaintexts_per_pack;

            Vec<PackedCiphertext> ret;
            ret.SetLength(n_vectors + (last_vector > 0 ? 1 : 0));
            for(size_t i = 0; i < n_vectors; i++) {
                const auto begin = plaintexts.begin() + i * plaintexts_per_pack;
                const auto end = begin + plaintexts_per_pack;
                ret[i] = encrypt_pack(begin, end, plaintext_bits, pai);
            }

            if(last_vector > 0) {
                ret[n_vectors] = encrypt_pack(plaintexts.end() - last_vector,
                                              plaintexts.end(),
                                              plaintext_bits,
                                              pai);
            }

            return ret;
        }

        Vec<Integer> decrypt_pack(const PackedCiphertext &ciphertext, const PaillierBase &pai) {
            Vec<Integer> ret;
            decrypt_pack(ciphertext, ret, pai);
            return ret;
        }

        void decrypt_pack(const PackedCiphertext &ciphertext, Integer *plaintexts_begin, Integer *plaintexts_end, const PaillierBase &pai) {
            const long n_plaintexts = ciphertext.n_plaintexts;
            const size_t plaintext_bits = ciphertext.plaintext_bits;

            if(n_plaintexts > (long)pack_count(plaintext_bits, pai))
                error_exit("trying to unpack too many elements!");

            const size_t shift = plaintext_bits + pack_buffer;

            if(n_plaintexts != (plaintexts_end - plaintexts_begin))
                error_exit("trying to unpack too many elements!");

            const Integer mask_plus_1 = Integer(1) << shift;
            const Integer mask = mask_plus_1 - 1;

            Integer sum = pai.decrypt(ciphertext.data);

            for(auto i = n_plaintexts; i-- != 0;) {
                mpz_and(plaintexts_begin[i].get_mpz_t(), sum.get_mpz_t(), mask.get_mpz_t());
                const bool is_neg = (bool)mpz_tstbit(sum.get_mpz_t(), shift - 1);
                if(is_neg) {
                    plaintexts_begin[i] -= mask_plus_1;
                    sum -= plaintexts_begin[i];
                }
                sum >>= shift;
            }
        }

        void decrypt_pack(const PackedCiphertext &ciphertext, Vec<Integer> &plaintexts, const PaillierBase &pai) {
            const size_t n_plaintexts = ciphertext.n_plaintexts;
            const size_t plaintext_bits = ciphertext.plaintext_bits;

            if(n_plaintexts > pack_count(plaintext_bits, pai))
                error_exit("trying to unpack too many elements!");

            plaintexts.SetLength(n_plaintexts);
            decrypt_pack(ciphertext, plaintexts.begin(), plaintexts.end(), pai);
        }

        Vec<Integer> decrypt_pack(const Vec<PackedCiphertext> &ciphertexts, const PaillierBase &pai) {
            Vec<Integer> ret;
            decrypt_pack(ciphertexts, ret, pai);
            return ret;
        }

        void decrypt_pack(const Vec<PackedCiphertext> &ciphertexts, Vec<Integer> &plaintexts, const PaillierBase &pai) {
            size_t total = 0;
            for(auto p: ciphertexts) {
                total += p.n_plaintexts;
            }
            plaintexts.SetLength(total);

            size_t decrypted = 0;
            for(const auto c: ciphertexts) {
                const auto begin = plaintexts.begin() + decrypted;
                const auto end = begin + c.n_plaintexts;
                decrypt_pack(c, begin, end, pai);
                decrypted += c.n_plaintexts;
            }
        }

        Vec<Integer> decrypt_fast(const Vec<Ciphertext> &cipher, const PaillierBase &pai, const size_t plaintext_bits) {
            // could be implemented just using pack_ciphertexts_vec()
            // and decrypt_pack(Vec<PackedCiphertext>), but this
            // way we save us some memory allocations and copying.

            const auto plaintexts_per_pack = pack_count(plaintext_bits, pai);
            const auto n_vectors = cipher.length() / plaintexts_per_pack;
            const auto last_vector = cipher.length() % plaintexts_per_pack;

            Vec<Integer> ret;
            ret.SetLength(cipher.length());

            PackedCiphertext tmp;
            for(size_t i = 0; i < n_vectors; i++) {
                const auto begin = cipher.begin() + i * plaintexts_per_pack;
                const auto end = begin + plaintexts_per_pack;
                tmp = pack_ciphertexts(begin, end, plaintext_bits, pai);

                const auto begin_dec = ret.begin() + i * plaintexts_per_pack;
                const auto end_dec = begin_dec + plaintexts_per_pack;
                decrypt_pack(tmp, begin_dec, end_dec, pai);
            }

            if(last_vector > 0) {
                tmp = pack_ciphertexts(cipher.end() - last_vector,
                                       cipher.end(),
                                       plaintext_bits,
                                       pai);
                decrypt_pack(tmp, ret.end() - last_vector, ret.end(), pai);
            }

            return ret;
        }

        Mat<Integer> decrypt_fast(const Mat<Ciphertext> &cipher, const PaillierBase &pai, const size_t plaintext_bits) {
            Mat<Integer> ret;
            ret.SetDims(cipher.NumRows(), cipher.NumCols());
            #pragma omp parallel for
            for(long i = 0; i < cipher.NumRows(); i++) {
                ret[i] = decrypt_fast(cipher[i], pai, plaintext_bits);
            }
            return ret;
        }
    }
}