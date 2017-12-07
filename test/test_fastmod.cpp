#include "ophelib/fast_mod.h"
#include "ophelib/paillier.h"
#include "ophelib/random.h"
#include "catch.hpp"

using namespace std;
using namespace ophelib;

const int keysize = 1024;

TEST_CASE("FastMod") {
    const int n_rep = 10;
    Paillier paillier(keysize);
    paillier.generate_keys();
    const Integer n = paillier.get_pub().n,
            n2 = n * n,
            p = paillier.get_priv().p,
            q = paillier.get_priv().q,
            p2 = p * p;
    const FastMod mod(p, q);

    Random& rand = Random::instance();
    Integer a = rand.rand_int(n2),
            b = rand.rand_int(n2);

    SECTION( "fast pow parallel" ) {
        REQUIRE( mod.pow_mod_n2_par(a, b) == a.pow_mod_n(b, n2) );
        for(int i = 0; i < n_rep; i++)
            REQUIRE( mod.pow_mod_n2_par(a, b) );
    }

    SECTION( "fast pow" ) {
        REQUIRE( mod.pow_mod_n2(a, b) == a.pow_mod_n(b, n2) );
        for(int i = 0; i < n_rep; i++)
            REQUIRE( mod.pow_mod_n2(a, b) );
    }

    SECTION( "pow" ) {
        for(int i = 0; i < n_rep; i++)
            REQUIRE( a.pow_mod_n(b, n2) );
    }

    SECTION("negative numbers") {
        REQUIRE( mod.pow_mod_n2(-a, b) == (-a).pow_mod_n(b, n2) );
        REQUIRE( mod.pow_mod_n2(a, -b) == a.pow_mod_n(-b, n2) );
        REQUIRE( mod.pow_mod_n2(-a, -b) == (-a).pow_mod_n(-b, n2) );
    }
}
