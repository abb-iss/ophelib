#include "ophelib/integer.h"
#include "ophelib/error.h"

#include <gmpxx.h>

namespace ophelib {
    std::string Integer::to_string_(const unsigned int base) const {
        //get a pointer to GMP's internal memory deallocator function
        void (*deallocator)(void *, size_t);
        mp_get_memory_functions(NULL, NULL, &deallocator);

        //get the string representation of input
        char *data = mpz_get_str(NULL, base, this->get_mpz_t());

        std::string ret(data);

        //deallocate data, including the terminator character
        //calling std::free on the char * returned by mpz_get_str is dangerous,
        // because it is initialized internally by GMP
        (*deallocator)((void *)data, std::char_traits<char>::length(data) + 1);

        return ret;
    }

    std::string Integer::to_string(const bool brief) const {
        std::ostringstream o("");
        o << "<Int[" << size_bits() << "]";
        if(!brief) {
            o << " " << to_string_();
        }
        o << ">";

        return o.str();
    }

    bool Integer::is_prime() const {
        return 0 != mpz_probab_prime_p(this->get_mpz_t(), N_PRIME_CHEKS_FOR_SIZE(size_bits()));
    }

    size_t Integer::size_bits() const {
        return mpz_sizeinbase(this->get_mpz_t(), 2);
    }

    Integer Integer::pow(const long &exponent) const {
        Integer ret;
        if (exponent < 0) {
            mpz_set_si(ret.get_mpz_t(), 0);
        } else {
            mpz_pow_ui(ret.get_mpz_t(), this->get_mpz_t(), (unsigned long) exponent);
        }
        return ret;
    }

    Integer Integer::pow_mod_n(const Integer &exponent, const Integer mod) const {
        Integer ret;

        if(mod == 0) {
            math_error_exit("cannot operate with mod=0");
        }

        mpz_powm(ret.get_mpz_t(), this->get_mpz_t(), exponent.get_mpz_t(), mod.get_mpz_t());
        return ret;
    }

    Integer Integer::inv_mod_n(const Integer mod) const {
        Integer ret;
        int stat = mpz_invert(ret.get_mpz_t(), this->get_mpz_t(), mod.get_mpz_t());

        if(stat == 0) {
            math_error_exit("inverse of n=" + this->to_string() + " does not exist!");
        }

        return ret;
    }

    Integer Integer::lcm(const Integer &op1, const Integer &op2) {
        Integer ret;
        mpz_lcm(ret.get_mpz_t(), op1.get_mpz_t(), op2.get_mpz_t());
        return ret;
    }

    Integer Integer::gcd(const Integer &op1, const Integer &op2) {
        Integer ret;
        mpz_gcd(ret.get_mpz_t(), op1.get_mpz_t(), op2.get_mpz_t());
        return ret;
    }

    Integer Integer::L(const Integer &input, const Integer &d) {
        return Integer((input - 1) / d);
    }

    long Integer::to_long() const {
        if(this->size_bits() > sizeof(long) * 8)
            math_error_exit("cannot convert to long");

        return mpz_get_si(this->get_mpz_t());
    }

    unsigned long Integer::to_ulong() const {
        if(this->size_bits() > sizeof(unsigned long) * 8 || *this < 0)
            math_error_exit("cannot convert to unsigned long");
        return mpz_get_ui(this->get_mpz_t());
    }

    int Integer::to_int() const {
        if(this->size_bits() > sizeof(int) * 8)
            math_error_exit("cannot convert to int");
        return (int)mpz_get_si(this->get_mpz_t());
    }

    unsigned int Integer::to_uint() const {
        if(this->size_bits() > sizeof(unsigned int) * 8 || *this < 0)
            math_error_exit("cannot convert to unsigned int");
        return (unsigned int)mpz_get_ui(this->get_mpz_t());
    }
}
