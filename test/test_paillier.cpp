#include "ophelib/paillier.h"
#include "ophelib/random.h"
#include "ophelib/util.h"
#include "catch.hpp"
#include "ophelib/disable_exception_tests.h"

#ifdef PAILLIER_CLASS
#undef PAILLIER_CLASS
#endif
#define PAILLIER_CLASS Paillier

using namespace std;
using namespace ophelib;

const int keysize = 1024;

TEST_CASE(STR(PAILLIER_CLASS)) {
    SECTION("sanity checks") {
        PAILLIER_CLASS pai1(keysize), pai2(keysize), pai3(keysize);
        Integer i(213), j(872);

        REQUIRE_THROWS_AS( pai1.encrypt(i), BaseException );
        REQUIRE_THROWS_AS( pai1.get_pub(), BaseException );
        REQUIRE_THROWS_AS( pai1.get_priv(), BaseException );

        pai1.generate_keys();
        pai2.generate_keys();

        Ciphertext i_ = pai1.encrypt(i), j_ = pai2.encrypt(j);
        REQUIRE(*i_.n2_shared.get() == *pai1.get_n2().get());

        REQUIRE_THROWS_AS( i_ - j_, BaseException );
        REQUIRE( pai1.decrypt(i_) == i );
        #ifdef DEBUG
        REQUIRE_THROWS_AS( i_ + j_, BaseException );
        REQUIRE_THROWS_AS( pai2.decrypt(i_), BaseException );
        #endif
        REQUIRE_THROWS_AS( pai3.decrypt(i_), BaseException );
    }

    SECTION("constructor from key containers") {
        PAILLIER_CLASS pai(keysize);
        pai.generate_keys();
        const Integer i(34684324);
        const auto ie = pai.encrypt(i);

        PAILLIER_CLASS pai1(pai.get_pub());
        REQUIRE( pai.decrypt(pai1.encrypt(i)) == i );
        REQUIRE_THROWS_AS( pai1.decrypt(ie), BaseException );

        PAILLIER_CLASS pai2(pai.get_pub(), pai.get_priv());
        REQUIRE( pai.decrypt(pai2.encrypt(i)) == i );
        REQUIRE( pai2.decrypt(ie) == i );

        PAILLIER_CLASS pai3(pai.get_keypair());
        REQUIRE( pai.decrypt(pai3.encrypt(i)) == i );
        REQUIRE( pai3.decrypt(ie) == i );

        auto pr = pai.get_priv();
        pr.a = 123;
        REQUIRE_THROWS_AS( PAILLIER_CLASS(pai.get_pub(), pr), BaseException );
        REQUIRE_THROWS_AS( PAILLIER_CLASS(KeyPair(pai.get_pub(), pr)), BaseException );
    }

    SECTION("different key sizes") {
        const size_t keys[] = {2048, 3072, 4096, 7680, 0};
        for(auto k = keys; *k > 0; k++) {
            #ifdef DEBUG
            cout << "testing key size k=" << *k << endl;
            #endif

            PAILLIER_CLASS pai(*k);
            pai.generate_keys();
            REQUIRE( pai.get_pub().key_size_bits == *k );
            REQUIRE( pai.get_priv().key_size_bits == *k );

            Integer m(1234);
            Ciphertext c;

            REQUIRE_FALSE( pai.encrypt(m) == pai.encrypt(m) );
            REQUIRE( pai.encrypt(m).data.size_bits() ==
                     Approx(pai.ciphertext_size_bits() ).epsilon(0.01) );

            REQUIRE( pai.decrypt(pai.encrypt(m)) == m );
        }
    }

    PAILLIER_CLASS paillier(keysize);
    paillier.generate_keys();
    Integer m(1234);
    Ciphertext c;

    SECTION( "encrypt" ) {
        REQUIRE_FALSE( paillier.encrypt(m) == paillier.encrypt(m) );
        REQUIRE( paillier.encrypt(m).data.size_bits() ==
                 Approx( paillier.ciphertext_size_bits() ).epsilon(0.01) );
    }

    SECTION( "decrypt" ) {
        c = paillier.encrypt(m);
        REQUIRE( paillier.decrypt(c) == m );
    }

    SECTION( "Ciphertext constructors" ) {
        c = paillier.encrypt(m);
        REQUIRE( c.n2_shared );

        const auto c2 = Ciphertext(c.data);
        REQUIRE_FALSE( c2.fast_mod );
        REQUIRE_FALSE( c2.n2_shared );
        REQUIRE_THROWS_AS( -c2, BaseException );
        REQUIRE_THROWS_AS( c2 + c2, BaseException );
        REQUIRE_THROWS_AS( c2 * Integer(3), BaseException );
    }

    SECTION( "test with random numbers" ) {
        for(int i = 0; i < 20; i++) {
            Integer r = Random::instance().rand_int(paillier.plaintext_upper_boundary() + 1);
            REQUIRE( paillier.decrypt(paillier.encrypt(r)) == r );
            REQUIRE( paillier.decrypt(paillier.encrypt(-r)) == -r );
        }
    }

    SECTION( "enc/dec edge cases" ) {
        Integer hi = paillier.plaintext_upper_boundary();
        Integer lo = paillier.plaintext_lower_boundary();

        REQUIRE( paillier.decrypt(paillier.encrypt(hi)) == hi );
        REQUIRE( paillier.decrypt(paillier.encrypt(hi + 1)) == -hi );
        REQUIRE( paillier.decrypt(paillier.encrypt(hi - 1)) == hi - 1 );
        REQUIRE( paillier.decrypt(paillier.encrypt(1)) == 1 );
        REQUIRE( paillier.decrypt(paillier.encrypt(0)) == 0 );
        REQUIRE( paillier.decrypt(paillier.encrypt(-1)) == -1 );
        REQUIRE( paillier.decrypt(paillier.encrypt(lo)) == lo );
        REQUIRE( paillier.decrypt(paillier.encrypt(lo + 1)) == lo + 1 );
        REQUIRE( paillier.decrypt(paillier.encrypt(lo - 1)) == -lo );
    }
}

TEST_CASE(STR(PAILLIER_CLASS)"::Ciphertext") {
    PAILLIER_CLASS paillier(keysize);
    paillier.generate_keys();
    Integer i(5), j(8), z(0), o(1);
    Ciphertext i_ = paillier.encrypt(i),
            j_ = paillier.encrypt(j),
            z_ = paillier.encrypt(z),
            o_ = paillier.encrypt(o);

    SECTION( "operator+" ) {
        REQUIRE( paillier.decrypt(i_ + j_) == i + j );
        REQUIRE( paillier.decrypt(i_ + z_) == i );
    }

    SECTION( "operator+=" ) {
        Ciphertext i__ = i_;
        i__ += j_;
        REQUIRE( paillier.decrypt(i__) == i + j );
    }

    SECTION( "unary operator-" ) {
        REQUIRE( paillier.decrypt(-i_) == -i );
        REQUIRE( paillier.decrypt(-o_) == -o );
        REQUIRE( paillier.decrypt(-z_) == -z );
        REQUIRE( paillier.decrypt(-z_) == z );
        REQUIRE( paillier.decrypt(z_) == -z );
    }

    SECTION( "binary operator-" ) {
        REQUIRE( paillier.decrypt(i_ - j_) == i - j );
        REQUIRE( paillier.decrypt(i_ - z_) == i );
    }

    SECTION( "operator-=" ) {
        Ciphertext i__ = i_;
        i__ -= j_;
        REQUIRE( paillier.decrypt(i__) == i - j );
    }

    SECTION( "operator*" ) {
        REQUIRE( paillier.decrypt(i_ * j) == i * j);
        REQUIRE( paillier.decrypt(i_ * -j) == i * -j);
        REQUIRE( paillier.decrypt(i_ * o) == i );
        REQUIRE( paillier.decrypt(i_ * z) == 0 );
    }

    SECTION( "operator*=" ) {
        Ciphertext i__ = i_;
        i__ *= j;
        REQUIRE( paillier.decrypt(i__) == i * j);
    }

    SECTION( "test with random numbers" ) {
        for(int u = 0; u < 10; u++) {
            Integer a =  Random::instance().rand_int_bits(50),
                    b = -Random::instance().rand_int_bits(50);
            Ciphertext a_ = paillier.encrypt(a),
                    b_ = paillier.encrypt(b);
            REQUIRE( paillier.decrypt(-a_) == -a );
            REQUIRE( paillier.decrypt(a_ + b_) == a + b );
            REQUIRE( paillier.decrypt(a_ - b_) == a - b );
            REQUIRE( paillier.decrypt(b_ - a_) == b - a );
            REQUIRE( paillier.decrypt(a_ * b) == a * b );
            REQUIRE( paillier.decrypt(b_ * a) == a * b );
        }
    }
}
