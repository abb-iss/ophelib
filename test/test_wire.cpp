#include "ophelib/integer.h"
#include "ophelib/wire.h"
#include "ophelib/paillier_fast.h"
#include "ophelib/util.h"
#include "ophelib/packing.h"
#include "catch.hpp"
#include "ophelib/disable_exception_tests.h"

using namespace std;
using namespace ophelib;

const int keysize = 1024;

/**
 * Tests for serialization and deserialization
 */
TEST_CASE("Wire") {
    const auto fname = temp_name();

    const string data_file = "../test/fixtures/rand.txt";
    NTL::Mat<float> X_;
    NTL::Vec<float> y_;
    Vector::load_data(data_file, X_, y_);
    const Vector::Integerizer inter(30);
    Vector::Normalizer normX;
    const NTL::Mat<Integer> X = inter.transform(normX.fit_transform(X_));

    SECTION("Integer") {
        const Integer a(35468651343LL);
        serialize_to_file(a, fname);
        REQUIRE( deserialize_from_file<Integer>(fname) == a );
        unlink(fname.c_str());

        serialize_to_file(-a, fname);
        REQUIRE( deserialize_from_file<Integer>(fname) == -a );
        unlink(fname.c_str());

        serialize_to_file(Integer(0), fname);
        REQUIRE( deserialize_from_file<Integer>(fname) == 0 );
        unlink(fname.c_str());

        serialize_to_file(Integer(), fname);
        REQUIRE( deserialize_from_file<Integer>(fname) == 0 );
        unlink(fname.c_str());
    }

    SECTION("Ciphertext") {
        PaillierFast pai(keysize);
        pai.generate_keys();

        const Ciphertext c = pai.encrypt(Integer(68441343843LL));

        serialize_to_file(c, fname);
        const auto c_ = deserialize_from_file<Ciphertext>(fname);
        unlink(fname.c_str());
        REQUIRE(c_ == c);
        // does not contain n_shared
        REQUIRE_THROWS_AS( -c_, BaseException );
        REQUIRE_THROWS_AS( c_ * Integer(2), BaseException );
        REQUIRE_THROWS_AS( c_ + c_, BaseException );
        REQUIRE_FALSE( c_.n2_shared );
        REQUIRE_FALSE( c_.fast_mod );

        serialize_to_file(-c, fname);
        REQUIRE( deserialize_from_file<Ciphertext>(fname) == -c );
        unlink(fname.c_str());
    }

    SECTION("PackedCiphertext") {
        PaillierFast pai(keysize);
        pai.generate_keys();
        const auto n_bits = 200;
        const auto plain = Vector::rand_bits(5, n_bits);
        const auto enc = Vector::encrypt_pack(plain, n_bits, pai);

        serialize_to_file(enc, fname);
        const auto enc_ = deserialize_from_file<PackedCiphertext>(fname);
        unlink(fname.c_str());
        REQUIRE( enc_ == enc );
        REQUIRE( Vector::decrypt_pack(enc_, pai) == plain );
    }

    SECTION("VectorFloat") {
        for(long i = 0; i < X_.NumRows(); i++) {
            serialize_to_file(X_[i], fname);
            REQUIRE( deserialize_from_file< Vec<float> >(fname) == X_[i] );
            unlink(fname.c_str());
        }
    }

    SECTION("VectorInteger") {
        for(long i = 0; i < X.NumRows(); i++) {
            serialize_to_file(X[i], fname);
            REQUIRE( deserialize_from_file< Vec<Integer> >(fname) == X[i] );
            unlink(fname.c_str());
        }
    }

    SECTION("VectorCiphertext") {
        PaillierFast pai(keysize);
        pai.generate_keys();
        const auto X_enc = Vector::encrypt(X, pai);

        for(long i = 0; i < X_enc.NumRows(); i++) {
            serialize_to_file(X_enc[i], fname);
            const auto x = deserialize_from_file< Vec<Ciphertext> >(fname);
            REQUIRE( x == X_enc[i] );
            unlink(fname.c_str());
        }
    }

    SECTION("VectorPackedCiphertext") {
        PaillierFast pai(keysize);
        pai.generate_keys();
        const auto n_bits = 64;
        const auto n_plaintexts = 200;
        const auto plain = Vector::rand_bits(n_plaintexts, n_bits);
        const auto enc = Vector::encrypt_pack_vec(plain, n_bits, pai);

        serialize_to_file(enc, fname);
        const auto enc_ = deserialize_from_file< Vec<PackedCiphertext> >(fname);
        unlink(fname.c_str());
        REQUIRE( enc_ == enc );
        REQUIRE( Vector::decrypt_pack(enc_, pai)  == plain );
    }

    SECTION("MatrixFloat") {
        serialize_to_file(X_, fname);
        REQUIRE( deserialize_from_file< Mat<float> >(fname) == X_ );
        unlink(fname.c_str());
    }

    SECTION("MatrixInteger") {
        serialize_to_file(X, fname);
        REQUIRE( deserialize_from_file< Mat<Integer> >(fname) == X );
        unlink(fname.c_str());
    }

    SECTION("MatrixCiphertext") {
        PaillierFast pai(keysize);
        pai.generate_keys();
        const auto X_enc = Vector::encrypt(X, pai);

        serialize_to_file(X_enc, fname);
        const auto x = deserialize_from_file< Mat<Ciphertext> >(fname);
        REQUIRE( x == X_enc );
        unlink(fname.c_str());
    }

    SECTION("PublicKey") {
        PaillierFast pai(keysize);
        pai.generate_keys();
        serialize_to_file(pai.get_pub(), fname);
        REQUIRE( deserialize_from_file<PublicKey>(fname) == pai.get_pub());
    }

    SECTION("PrivateKey") {
        PaillierFast pai(keysize);
        pai.generate_keys();
        serialize_to_file(pai.get_priv(), fname);
        REQUIRE( deserialize_from_file<PrivateKey>(fname) == pai.get_priv());
    }

    SECTION("KeyPair") {
        PaillierFast pai(keysize);
        pai.generate_keys();
        serialize_to_file(pai.get_keypair(), fname);
        REQUIRE( deserialize_from_file<KeyPair>(fname) == pai.get_keypair());
    }
}
