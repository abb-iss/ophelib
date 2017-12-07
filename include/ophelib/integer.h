#pragma once

#include <gmpxx.h>

/**
 * Namespace containing all functionality of the library. Nothing is defined
 * outside of this namespace.
 */
namespace ophelib {

/*
 * number of Miller-Rabin iterations for an error rate of less than 2^-80 for
 * random 'b'-bit input, b >= 100 (taken from table 4.4 in the Handbook of
 * Applied Cryptography [Menezes, van Oorschot, Vanstone; CRC Press 1996];
 * original paper: Damgaard, Landrock, Pomerance: Average case error
 * estimates for the strong probable prime test. -- Math. Comp. 61 (1993)
 * 177-194)
 *
 * Copied from OpenSSL include/openssl/bn.h
 */
#define N_PRIME_CHEKS_FOR_SIZE(b) ((b) >= 1300 ?  2 : \
                                   (b) >=  850 ?  3 : \
                                   (b) >=  650 ?  4 : \
                                   (b) >=  550 ?  5 : \
                                   (b) >=  450 ?  6 : \
                                   (b) >=  400 ?  7 : \
                                   (b) >=  350 ?  8 : \
                                   (b) >=  300 ?  9 : \
                                   (b) >=  250 ? 12 : \
                                   (b) >=  200 ? 15 : \
                                   (b) >=  150 ? 18 : \
                                   /* b >= 100 */ 27)

    /**
     * Big Integer class. This is just a thin wrapper around
     * GMPs mpz_class. All mpz_.. functions should only be used inside
     * this class and be provided under an abstraction layer to the
     * rest of the library. However, in some places (although not a lot),
     * we break this rule. This is only a problem if we ever want to
     * replace the Integer implementation with a library other than GMP.
     *
     * Note that most of the operator overloading functionality is
     * implemented in integer_ops.cpp, while the rest is in integer.cpp.
     */
    class Integer : public mpz_class {
    public:
        /* Constructors */
        Integer();
        Integer(const mpz_class &input);
        Integer(const long &input);
        Integer(const long long &input);
        Integer(const int &input);
        Integer(const unsigned long &input);
        Integer(const unsigned long long &input);
        Integer(const unsigned int &input);
        Integer(const char *input, unsigned int base = 10);

        Integer &operator=(const Integer &input);
        Integer &operator=(const mpz_class &input);
        Integer &operator=(const long &input);
        Integer &operator=(const int &input);
        Integer &operator=(const unsigned int &input);
        Integer &operator=(const unsigned long &input);

        /**
         * Unary -
         */
        Integer operator-() const;

        /* conversions */
        long to_long() const;
        unsigned long to_ulong() const;
        int to_int() const;
        unsigned int to_uint() const;

        /**
         * Size of this Integer in bits. size(0) = 1.
         */
        size_t size_bits() const;

        /**
         * Power with long exponent
         */
        Integer pow(const long &exponent) const;

        /**
         * Power modulo mod
         */
        Integer pow_mod_n(const Integer &exponent, const Integer mod) const;

        /**
         * Multiplicative inverse modulo mod
         */
        Integer inv_mod_n(const Integer mod) const;

        /**
         * String representation. If brief = true, only size
         * of the integer is included in the representation.
         * Brief = false will include both size and the number
         * itself as string.
         */
        std::string to_string(const bool brief = true) const;

        /**
         * Get number as string, in a given base.
         */
        std::string to_string_(const unsigned int base = 10) const;

        /**
         * Test if prime, using Miller-Rabin. See the macro
         * N_PRIME_CHEKS_FOR_SIZE for more details.
         */
        bool is_prime() const;

        /**
         * Least common multiple
         */
        static Integer lcm(const Integer &op1, const Integer &op2);

        /**
         * Greatest common divisor
         */
        static Integer gcd(const Integer &op1, const Integer &op2);

        /**
         * Used for decryption
         * Maybe move this to Paillier/PaillierBase class.
         *
         * @param input the function variable
         * @param d the divisor
         * @return `L(input) = (input - 1) / d`
         */
        static Integer L(const Integer &input, const Integer &d);
    };

    /* All kinds of helper operators */
    std::ostream& operator<<(std::ostream& stream, const Integer& i);
    Integer operator+(const mpz_class &lhs, const mpz_class &rhs);
    Integer operator+(const mpz_class &lhs, const int &rhs);
    Integer operator+(const mpz_class &lhs, const unsigned int &rhs);
    Integer operator-(const mpz_class &lhs, const mpz_class &rhs);
    Integer operator-(const Integer &lhs, const int &rhs);
    Integer operator-(const Integer &lhs, const unsigned int &rhs);
    Integer operator*(const Integer &lhs, const Integer &rhs);
    Integer operator%(const Integer &lhs, const Integer &rhs);
    Integer operator/(const Integer &lhs, const Integer &rhs);
    Integer operator/(const Integer &lhs, const int &rhs);
    Integer operator<<(const mpz_class &lhs, const size_t &rhs);
    Integer operator>>(const mpz_class &lhs, const size_t &rhs);
}
