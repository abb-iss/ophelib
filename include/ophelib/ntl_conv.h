#pragma once

#include "ophelib/integer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wparentheses"
#include <NTL/RR.h>
#include <NTL/ZZ.h>
#pragma GCC diagnostic pop

namespace ophelib {
    /**
     * @file
     * Conversions from to NTL/GMP number format.
     * NTL actually uses GMPs low-level representation and
     * functions internally, however does not offer a way
     * to access those representations for import/export
     * in any way. So the only thing we can do is to convert
     * over string representation.
     */

    /**
     * From NTL::RR float to Integer
     * @param z output parameter
     * @param a input value
     */
    void conv(ophelib::Integer& z, const NTL::RR& a);

    /**
     * From NTL::ZZ BigInt to Integer
     * @param z output parameter
     * @param a input value
     */
    void conv(ophelib::Integer& z, const NTL::ZZ& a);

    /**
     * From Integer to NTL::ZZ BigInt
     * @param z output parameter
     * @param a input value
     */
    void conv(NTL::RR& z, const ophelib::Integer& a);
}
