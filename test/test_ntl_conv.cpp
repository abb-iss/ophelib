#include "ophelib/ntl_conv.h"
#include "ophelib/random.h"
#include "catch.hpp"

#include <iostream>

using namespace std;
using namespace NTL;
using namespace ophelib;

TEST_CASE("NTL Conversions") {
    const int n_rep = 100;

    SECTION( "void conv(ophelib::Integer&, const NTL::RR&)" ) {
        for(int u = 0; u < n_rep; u++) {
            long a =  Random::instance().rand_int_bits(50).to_long(),
                 b = -Random::instance().rand_int_bits(50).to_long();
            Integer a_, b_;
            RR r(a), s(b);
            conv(a_, r);
            conv(b_, s);
            REQUIRE( a_ == a );
            REQUIRE( b_ == b );
            REQUIRE( conv<Integer>(r) == a );
            REQUIRE( conv<Integer>(s) == b );
        }
    }

    SECTION( "void conv(ophelib::Integer&, const NTL::ZZ&)" ) {
        for(int u = 0; u < n_rep; u++) {
            long a =  Random::instance().rand_int_bits(50).to_long(),
                    b = -Random::instance().rand_int_bits(50).to_long();
            Integer a_, b_;
            ZZ r(a), s(b);
            conv(a_, r);
            conv(b_, s);
            REQUIRE( a_ == a );
            REQUIRE( b_ == b );
            REQUIRE( conv<Integer>(r) == a );
            REQUIRE( conv<Integer>(s) == b );
        }
    }

    SECTION( "void conv(NTL::RR& z, const ophelib::Integer& a)" ) {
        for(int u = 0; u < n_rep; u++) {
            long a =  Random::instance().rand_int_bits(50).to_long(),
                    b = -Random::instance().rand_int_bits(50).to_long();
            RR a_, b_;
            Integer r(a), s(b);
            conv(a_, r);
            conv(b_, s);
            REQUIRE( a_ == a );
            REQUIRE( b_ == b );
            REQUIRE( conv<RR>(r) == a );
            REQUIRE( conv<RR>(s) == b );
        }
    }
}
