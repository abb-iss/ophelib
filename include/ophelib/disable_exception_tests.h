#ifndef OPHELIB_ENABLE_EXCEPTIONS
    /* Disable tests involving exceptions.
       Must be included *after* catch.hpp. */
    #undef REQUIRE_THROWS_AS
    #define REQUIRE_THROWS_AS(x, y)
#endif
