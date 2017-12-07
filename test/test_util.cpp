#include "ophelib/util.h"
#include "catch.hpp"

using namespace std;
using namespace ophelib;

TEST_CASE("util") {

    SECTION("nCr") {
        const Integer a(5), b(3);
        REQUIRE( nCr(a, b) == nCr(a, a - b) );
        REQUIRE( nCr(5, 3) == nCr(5, 5 - 3) );
    }

    SECTION("version and ref") {
        REQUIRE( ophelib_ref() != "unknown-ref" );
        REQUIRE( ophelib_version() != "unknown-version" );
    }
}
