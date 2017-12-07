#include "ophelib/random.h"
#include "ophelib/error.h"
#include "catch.hpp"
#include "ophelib/disable_exception_tests.h"

using namespace std;
using namespace ophelib;

TEST_CASE("Random") {
    Random& r = Random::instance();
    size_t n_bits = 2048;
    Integer max = 1;
    max <<= n_bits;

    REQUIRE_FALSE( r.rand_int_bits(n_bits) == r.rand_int_bits(n_bits) );
    REQUIRE( r.rand_int_bits(n_bits) > 0 );
    REQUIRE_FALSE( r.rand_int(max) == r.rand_int(max) );
    REQUIRE( r.rand_int(max) > 0 );

    SECTION( "Generating prime numbers " ) {
        REQUIRE_FALSE( r.rand_prime(n_bits) == r.rand_prime(n_bits) );
        REQUIRE( r.rand_prime(n_bits) > 1 );
        REQUIRE( r.rand_prime(n_bits).is_prime() );
    }

    SECTION( "invalid arguments" ) {
        REQUIRE_THROWS_AS( r.rand_int(0), BaseException );
        REQUIRE_THROWS_AS( r.rand_int(1), BaseException );
        REQUIRE( r.rand_int(2) < 2 );
        REQUIRE_THROWS_AS( r.rand_int_bits(0), BaseException );
        REQUIRE( r.rand_int_bits(1) < 2 );
        REQUIRE_THROWS_AS( r.rand_prime(0), BaseException );
        REQUIRE_THROWS_AS( r.rand_prime(1), BaseException );
        REQUIRE( r.rand_prime(2) < 4 );
    }
}
