#include "ophelib/vector.h"
#include "ophelib/packing.h"
#include "ophelib/random.h"
#include "ophelib/paillier.h"
#include "ophelib/util.h"
#include "catch.hpp"
#include "ophelib/disable_exception_tests.h"

using namespace std;
using namespace ophelib;

const int keysize = 1024;

TEST_CASE("PaillierBase::Packing") {
    Paillier paillier(keysize);
    paillier.generate_keys();

    SECTION("pack_count") {
        REQUIRE( Vector::pack_count(128, paillier) == 7 );
        REQUIRE( Vector::pack_count(64, paillier) == 15 );
        REQUIRE( Vector::pack_count(32, paillier) == 31 );
        REQUIRE( Vector::pack_count(30, paillier) == 33 );
        REQUIRE( Vector::pack_count(16, paillier) == 60 );
    }

    SECTION("encrypt") {
        const auto plaintext_bits = 128;
        const auto plain = Vector::rand_bits_neg(6, plaintext_bits);

        REQUIRE( Vector::encrypt_pack(plain, plaintext_bits, paillier).data.data > 0 );
        REQUIRE( Vector::encrypt_pack(plain, plaintext_bits, paillier).n_plaintexts == 6 );
        REQUIRE( Vector::encrypt_pack(plain, plaintext_bits, paillier).plaintext_bits == plaintext_bits );
        REQUIRE( Vector::encrypt_pack(plain, plaintext_bits, paillier).data.data !=
                         Vector::encrypt_pack(plain, plaintext_bits, paillier).data.data );
    }

    SECTION("try to encrypt too many") {
        const auto plaintext_bits = 128;
        const auto plain = Vector::rand_bits_neg(Vector::pack_count(plaintext_bits, paillier) + 1, plaintext_bits);

        REQUIRE_THROWS_AS( Vector::encrypt_pack(plain, plaintext_bits, paillier), BaseException );
    }

    SECTION("try to encrypt too big") {
        const auto plaintext_bits = 128;
        auto plain = Vector::rand_bits_neg(Vector::pack_count(plaintext_bits, paillier), plaintext_bits);
        REQUIRE( Vector::encrypt_pack(plain, plaintext_bits, paillier).data.data > 0 );
        plain[5] = Random::instance().rand_int_bits(plaintext_bits * 2);
        REQUIRE_THROWS_AS( Vector::encrypt_pack(plain, plaintext_bits, paillier), BaseException );
        REQUIRE_THROWS_AS( Vector::encrypt_pack(Vector::rand_bits(1, 3000), 3000, paillier), BaseException );
    }

    SECTION("decrypt") {
        const auto plaintext_bits = 128;
        const auto n_plaintexts = Vector::pack_count(plaintext_bits, paillier);
        const auto plain = Vector::rand_bits_neg(n_plaintexts, plaintext_bits);

        const auto enc = Vector::encrypt_pack(plain, plaintext_bits, paillier);
        const auto dec = Vector::decrypt_pack(enc, paillier);
        REQUIRE( dec.length() == Vector::pack_count(plaintext_bits, paillier) );
        REQUIRE( dec == plain );
    }

    SECTION("decrypt in-place") {
        const auto plaintext_bits = 128;
        const auto plain = Vector::rand_bits(7, plaintext_bits);

        const auto enc = Vector::encrypt_pack(plain, plaintext_bits, paillier);
        Vec<Integer> dec;
        Vector::decrypt_pack(enc, dec, paillier);
        REQUIRE( dec.length() == 7 );
        REQUIRE( dec == plain );
    }

    SECTION("pack/unpack, length 0") {
        const auto plaintext_bits = 128;
        const auto plain = Vector::zeros<Integer>(0);

        const auto enc = Vector::encrypt_pack(plain, plaintext_bits, paillier);
        const auto dec = Vector::decrypt_pack(enc, paillier);
        REQUIRE( dec.length() == 0 );
    }

    SECTION("pack arbitrary size plaintext vector") {
        const auto plaintext_bits = 64;
        const auto n_plaintexts = 500;
        const auto plain = Vector::rand_bits_neg(n_plaintexts, plaintext_bits);

        const auto enc = Vector::encrypt_pack_vec(plain, plaintext_bits, paillier);

        REQUIRE( enc[0].data.data > 0 );
        size_t sum = 0;
        for(auto p: enc) {
            sum += p.n_plaintexts;
            REQUIRE( p.plaintext_bits == plaintext_bits );
        }
        REQUIRE( sum == n_plaintexts );
    }

    SECTION("pack arbitrary size plaintext, length 0") {
        const auto plaintext_bits = 64;
        const auto plain = Vector::zeros<Integer>(0);

        const auto enc = Vector::encrypt_pack_vec(plain, plaintext_bits, paillier);
        REQUIRE( enc.length() == 0 );
        REQUIRE( Vector::decrypt_pack(enc, paillier).length() == 0 );
    }

    SECTION("decrypt arbitrary size plaintext vector") {
        const auto plaintext_bits = 64;
        const auto n_plaintexts = 500;
        const auto plain = Vector::rand_bits_neg(n_plaintexts, plaintext_bits);

        const auto enc = Vector::encrypt_pack_vec(plain, plaintext_bits, paillier);
        REQUIRE( Vector::decrypt_pack(enc, paillier) == plain );
    }

    SECTION("decrypt arbitrary size plaintext vector, no odd packed ciphertext") {
        const auto plaintext_bits = 64;
        const auto n_plaintexts = Vector::pack_count(plaintext_bits, paillier) * 5;
        const auto plain = Vector::rand_bits_neg(n_plaintexts, plaintext_bits);

        const auto enc = Vector::encrypt_pack_vec(plain, plaintext_bits, paillier);
        REQUIRE( enc.length() == 5 );
        REQUIRE( Vector::decrypt_pack(enc, paillier) == plain );
    }

    SECTION("decrypt arbitrary size plaintext vector, only odd packed ciphertext") {
        const auto plaintext_bits = 64;
        const auto n_plaintexts = Vector::pack_count(plaintext_bits, paillier) / 2;
        const auto plain = Vector::rand_bits_neg(n_plaintexts, plaintext_bits);

        const auto enc = Vector::encrypt_pack_vec(plain, plaintext_bits, paillier);
        REQUIRE( enc.length() == 1 );
        REQUIRE( Vector::decrypt_pack(enc, paillier) == plain );
    }

    SECTION("pack ciphertexts") {
        const auto n_bits = 64;
        const auto n_ciphertexts = Vector::pack_count(n_bits, paillier);
        const auto plain = Vector::rand_bits(n_ciphertexts, n_bits);

        Vec<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);

        const auto packed = Vector::pack_ciphertexts(ciphertexts, n_bits, paillier);
        const auto decrypted = Vector::decrypt_pack(packed, paillier);

        REQUIRE( decrypted.length() == n_ciphertexts );
        REQUIRE( plain == decrypted );
    }

    SECTION("pack ciphertexts, length 0") {
        const auto n_bits = 64;
        const auto plain = Vector::zeros<Integer>(0);
        Vec<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);

        REQUIRE_THROWS_AS( Vector::pack_ciphertexts(ciphertexts, n_bits, paillier), BaseException );
    }

    SECTION("pack arbitrary size ciphertext vector") {
        const auto n_bits = 64;
        const auto n_ciphertexts = 500;
        const auto plain = Vector::rand_bits(n_ciphertexts, n_bits);

        Vec<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);

        const auto packed = Vector::pack_ciphertexts_vec(ciphertexts, n_bits, paillier);
        const auto decrypted = Vector::decrypt_pack(packed, paillier);

        REQUIRE( decrypted.length() == n_ciphertexts );
        REQUIRE( plain == decrypted );
    }

    SECTION("pack arbitrary size ciphertext vector, no odd packed ciphertext") {
        const auto n_bits = 64;
        const auto n_ciphertexts = Vector::pack_count(n_bits, paillier) * 5;
        const auto plain = Vector::rand_bits(n_ciphertexts, n_bits);

        Vec<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);

        const auto packed = Vector::pack_ciphertexts_vec(ciphertexts, n_bits, paillier);
        const auto decrypted = Vector::decrypt_pack(packed, paillier);

        REQUIRE( decrypted.length() == n_ciphertexts );
        REQUIRE( plain == decrypted );
    }

    SECTION("pack arbitrary size ciphertext vector, only odd packed ciphertext") {
        const auto n_bits = 64;
        const auto n_ciphertexts = Vector::pack_count(n_bits, paillier) / 2;
        const auto plain = Vector::rand_bits(n_ciphertexts, n_bits);

        Vec<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);

        const auto packed = Vector::pack_ciphertexts_vec(ciphertexts, n_bits, paillier);
        const auto decrypted = Vector::decrypt_pack(packed, paillier);

        REQUIRE( packed.length() == 1 );
        REQUIRE( plain == decrypted );
    }

    SECTION("pack arbitrary size ciphertext vector, length 0") {
        const auto n_bits = 64;
        const auto plain = Vector::rand_bits(0, n_bits);
        Vec<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);

        const auto packed = Vector::pack_ciphertexts_vec(ciphertexts, n_bits, paillier);
        REQUIRE( Vector::decrypt_pack(packed, paillier) == plain );
    }

    SECTION("decrypt_fast") {
        const auto n_bits = 64;
        const auto n_ciphertexts = 500;
        const auto plain = Vector::rand_bits(n_ciphertexts, n_bits);

        Vec<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);
        REQUIRE( Vector::decrypt_fast(ciphertexts, paillier, n_bits) == plain );
    }

    SECTION("decrypt_fast, length 0") {
        const auto n_bits = 64;
        const auto plain = Vector::zeros<Integer>(0);

        Vec<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);
        REQUIRE( Vector::decrypt_fast(ciphertexts, paillier, n_bits) == plain );
    }

    SECTION("decrypt_fast, no odd packed ciphertext") {
        const auto n_bits = 64;
        const auto n_ciphertexts = Vector::pack_count(n_bits, paillier) * 5;
        const auto plain = Vector::rand_bits(n_ciphertexts, n_bits);

        Vec<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);
        REQUIRE( Vector::decrypt_fast(ciphertexts, paillier, n_bits) == plain );
    }

    SECTION("decrypt_fast, only odd packed ciphertext") {
        const auto n_bits = 64;
        const auto n_ciphertexts = Vector::pack_count(n_bits, paillier) / 2;
        const auto plain = Vector::rand_bits(n_ciphertexts, n_bits);

        Vec<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);
        REQUIRE( Vector::decrypt_fast(ciphertexts, paillier, n_bits) == plain );
    }

    SECTION("decrypt_fast matrix") {
        const auto n_bits = 64;
        const auto plain = Vector::rand_bits(10, 50, n_bits);

        Mat<Ciphertext> ciphertexts = Vector::encrypt(plain, paillier);
        REQUIRE( Vector::decrypt_fast(ciphertexts, paillier, n_bits) == plain );
    }
}
