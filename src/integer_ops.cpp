#include "ophelib/integer.h"

namespace ophelib {
    Integer::Integer() { }

    Integer::Integer(const mpz_class &input) {
        mpz_set(this->get_mpz_t(), input.get_mpz_t());
    }

    Integer::Integer(const int &input) {
        mpz_set_si(this->get_mpz_t(), input);
    }

    Integer::Integer(const long &input) {
        mpz_set_si(this->get_mpz_t(), input);
    }

    Integer::Integer(const long long &input) {
        mpz_set_si(this->get_mpz_t(), input);
    }

    Integer::Integer(const unsigned int &input) {
        mpz_set_ui(this->get_mpz_t(), input);
    }

    Integer::Integer(const unsigned long &input) {
        mpz_set_ui(this->get_mpz_t(), input);
    }

    Integer::Integer(const unsigned long long &input) {
        mpz_set_ui(this->get_mpz_t(), input);
    }

    Integer::Integer(const char *input, unsigned int base) {
        mpz_set_str(this->get_mpz_t(), input, base);
    }

    Integer &Integer::operator=(const Integer &input) {
        mpz_set(this->get_mpz_t(), input.get_mpz_t());
        return *this;
    }

    Integer &Integer::operator=(const mpz_class &input) {
        mpz_set(this->get_mpz_t(), input.get_mpz_t());
        return *this;
    }

    Integer &Integer::operator=(const int &input) {
        mpz_set_si(this->get_mpz_t(), input);
        return *this;
    }

    Integer &Integer::operator=(const long &input) {
        mpz_set_si(this->get_mpz_t(), input);
        return *this;
    }

    Integer &Integer::operator=(const unsigned int &input) {
        mpz_set_ui(this->get_mpz_t(), input);
        return *this;
    }

    Integer &Integer::operator=(const unsigned long &input) {
        mpz_set_ui(this->get_mpz_t(), input);
        return *this;
    }

    Integer Integer::operator-() const {
        Integer ret;
        mpz_neg(ret.get_mpz_t(), this->get_mpz_t());
        return ret;
    }

    std::ostream &operator<<(std::ostream &stream, const Integer &i) {
        stream << i.to_string_();
        return stream;
    }

    Integer operator+(const mpz_class &lhs, const mpz_class &rhs) {
        Integer ret;
        mpz_add(ret.get_mpz_t(), lhs.get_mpz_t(), rhs.get_mpz_t());
        return ret;
    }

    Integer operator+(const mpz_class &lhs, const int &rhs) {
        Integer ret;
        if(rhs > 0)
            mpz_add_ui(ret.get_mpz_t(), lhs.get_mpz_t(), (unsigned int)rhs);
        else
            mpz_sub_ui(ret.get_mpz_t(), lhs.get_mpz_t(), (unsigned int)-rhs);
        return ret;
    }

    Integer operator+(const mpz_class &lhs, const unsigned int &rhs) {
        Integer ret;
        mpz_add_ui(ret.get_mpz_t(), lhs.get_mpz_t(), rhs);
        return ret;
    }

    Integer operator-(const mpz_class &lhs, const mpz_class &rhs) {
        Integer ret;
        mpz_sub(ret.get_mpz_t(), lhs.get_mpz_t(), rhs.get_mpz_t());
        return ret;
    }

    Integer operator-(const Integer &lhs, const unsigned int &rhs) {
        Integer ret;
        mpz_sub_ui(ret.get_mpz_t(), lhs.get_mpz_t(), rhs);
        return ret;
    }

    Integer operator-(const Integer &lhs, const int &rhs) {
        Integer ret;
        if(rhs > 0)
            mpz_sub_ui(ret.get_mpz_t(), lhs.get_mpz_t(), (unsigned int)rhs);
        else
            mpz_add_ui(ret.get_mpz_t(), lhs.get_mpz_t(), (unsigned int)-rhs);
        return ret;
    }

    Integer operator*(const Integer &lhs, const Integer &rhs) {
        Integer ret;
        mpz_mul(ret.get_mpz_t(), lhs.get_mpz_t(), rhs.get_mpz_t());
        return ret;
    }

    Integer operator%(const Integer &lhs, const Integer &rhs) {
        Integer ret;
        mpz_mod(ret.get_mpz_t(), lhs.get_mpz_t(), rhs.get_mpz_t());
        return ret;
    }

    Integer operator/(const Integer &lhs, const Integer &rhs) {
        Integer ret;
        mpz_div(ret.get_mpz_t(), lhs.get_mpz_t(), rhs.get_mpz_t());
        return ret;
    }

    Integer operator/(const Integer &lhs, const int &rhs) {
        Integer ret;
        mpz_div(ret.get_mpz_t(), lhs.get_mpz_t(), mpz_class(rhs).get_mpz_t());
        return ret;
    }

    Integer operator<<(const mpz_class &lhs, const size_t &rhs) {
        Integer ret = lhs;
        ret <<= rhs;
        return ret;
    }

    Integer operator>>(const mpz_class &lhs, const size_t &rhs) {
        Integer ret = lhs;
        ret >>= rhs;
        return ret;
    }
}
