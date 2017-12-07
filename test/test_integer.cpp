#include "ophelib/integer.h"
#include "ophelib/error.h"
#include "ophelib/random.h"
#include "catch.hpp"
#include "ophelib/disable_exception_tests.h"

using namespace std;
using namespace ophelib;

TEST_CASE("Integer") {
    SECTION( "assignment and constructors" ) {
        Integer i, j;

        i = 1234;
        REQUIRE( i == 1234 );
        i = 1234L;
        REQUIRE( i == 1234 );
        i = (unsigned int)1234;
        REQUIRE( i == 1234 );
        i = (unsigned long)1234L;
        REQUIRE( i == 1234 );

        i = Integer(1234);
        REQUIRE( i == 1234 );
        i = Integer(1234L);
        REQUIRE( i == 1234 );
        i = Integer((unsigned int)1234);
        REQUIRE( i == 1234 );
        i = Integer((unsigned long)1234L);
        REQUIRE( i == 1234 );

        j = i;
        REQUIRE( j == 1234 );
        mpz_class k(12345);
        i = k;
        REQUIRE( i == 12345 );

        i = Integer("123456");
        REQUIRE( i == 123456 );
        REQUIRE( "123456" == i.to_string_() );
    }

    SECTION( "operator/, operator*" )  {
        Integer i(48976587145LL), j(123);
        REQUIRE( (i * j ) / j == i );
        REQUIRE( Integer(12) / 2 == 6 );
    }

    SECTION( "to_int, to_uint, to_long, to_ulong" ) {
        REQUIRE( Integer(~(1 << 31)).to_int() == ~(1 << 31) );
        REQUIRE( Integer(1 << 31).to_int() == (1 << 31) );
        REQUIRE_THROWS_AS( Integer(1LL << 32).to_int(), MathException );

        REQUIRE( Integer(~(1LL << 63)).to_long() == ~(1LL << 63) );
        REQUIRE( Integer(1LL << 63).to_long() == (1LL << 63) );
        REQUIRE_THROWS_AS( Integer("18446744073709551617").to_long(), MathException );

        REQUIRE( Integer(~(1 << 31)).to_uint() == ~(1 << 31) );
        REQUIRE_THROWS_AS( Integer(1 << 31).to_uint(), MathException );

        REQUIRE( Integer(~(1LL << 63)).to_ulong() == ~(1LL << 63) );
        REQUIRE_THROWS_AS( Integer(1LL << 63).to_ulong(), MathException );
    }

    SECTION( "to_string" ) {
        REQUIRE( Integer(0).to_string() == "<Int[1]>" );
        REQUIRE( Integer(0).to_string(false) == "<Int[1] 0>" );
        REQUIRE( Integer(10).to_string() == "<Int[4]>" );
        REQUIRE( Integer(10).to_string(false) == "<Int[4] 10>" );

        Integer number(834942652345LL);
        REQUIRE( number.to_string() == "<Int[40]>" );
        REQUIRE( number.to_string(false) == "<Int[40] 834942652345>" );
    }

    SECTION( "to_string_" ) {
        REQUIRE( Integer(0).to_string_() == "0" );
        REQUIRE( Integer(10).to_string_() == "10" );

        Integer number(834942652345LL);
        REQUIRE( number.to_string_() == "834942652345" );
        REQUIRE( number.to_string_(10) == "834942652345" );

        REQUIRE( Integer(0x64).to_string_(16) == "64" );
        REQUIRE( Integer(0xDEADBEEF).to_string_(16) == "deadbeef" );
        REQUIRE( Integer(100).to_string_(2) == "1100100" );
    }

    SECTION( "is_prime" ) {
        REQUIRE( Integer(2).is_prime() );
        REQUIRE( Integer(3).is_prime() );
        REQUIRE( Integer(5).is_prime() );
        REQUIRE( Integer(97).is_prime() );
        REQUIRE( Integer(131071).is_prime() );
        REQUIRE( Integer(524287).is_prime() );
        REQUIRE( Integer(2147483647).is_prime() );
        REQUIRE( Integer("170141183460469231731687303715884105727").is_prime() );

        REQUIRE_FALSE( Integer(1).is_prime() );
        REQUIRE_FALSE( Integer(4).is_prime() );
        REQUIRE_FALSE( Integer(6).is_prime() );
        REQUIRE_FALSE( Integer(100).is_prime() );
        REQUIRE_FALSE( Integer(2147483646).is_prime() );
        REQUIRE_FALSE( Integer("170141183460469231731687303715884105728").is_prime() );
    }

    SECTION( "pow" ) {
        REQUIRE( Integer(2).pow(0) == 1 );
        REQUIRE( Integer(2).pow(1) == 2 );
        REQUIRE( Integer(2).pow(8) == 0x100 );
        REQUIRE( Integer(2).pow(8 + 4) == 0x1000 );
        REQUIRE( Integer(2).pow(8 + 8) == 0x10000 );
        REQUIRE( Integer(2).pow(8 + 12) == 0x100000 );
        REQUIRE( Integer(2).pow(150) == Integer("1427247692705959881058285969449495136382746624") );
    }

    SECTION( "lcm" )  {
        Integer i(100000000000LL), j(1000);
        REQUIRE( Integer::lcm(i, j) == i );
        REQUIRE( Integer::lcm(2, 8) == 8 );
        REQUIRE( Integer::lcm(7, 5) == 35 );
    }

    SECTION( "L" )  {
        Integer i(11), j(5);
        REQUIRE( Integer::L(i, j) == 2 );
        REQUIRE( Integer::L(100001, 1000) == 100 );
    }

    SECTION( "pow_mod_n" )  {
        REQUIRE( Integer(2).pow_mod_n(10, 1000) == 24 );
        REQUIRE( Integer(1351235).pow_mod_n(6234, 82467) == 75601 );
        size_t keysize = 2048 * 2;
        Integer base = Random::instance().rand_int_bits(keysize * keysize);
        Integer exp_ = Random::instance().rand_int_bits(keysize);
        Integer mod = Random::instance().rand_int_bits(keysize * 2);
        REQUIRE( base.pow_mod_n(exp_, mod).size_bits() <= mod.size_bits() );
    }

    SECTION( "inv_mod_n" )  {
        REQUIRE( Integer(2).inv_mod_n(11) == 6 );
        REQUIRE_THROWS_AS( Integer(5).inv_mod_n(10), MathException );
        REQUIRE_THROWS_AS( Integer(0).inv_mod_n(11), MathException );
        REQUIRE_THROWS_AS( Integer(5).inv_mod_n(0), MathException );
    }
}
