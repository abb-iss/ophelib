#include "ophelib/integer.h"
#include "ophelib/fast_mod.h"
#include "ophelib/error.h"

#include <future>
#include <vector>

namespace ophelib {
    FastMod::FastMod(const Integer &p_, const Integer &q_)
            : p(p_),
              q(q_),
              p2(p * p),
              q2(q * q),
              n(p * q),
              n2(n * n) { }

    FastMod::FastMod(const Integer &p_, const Integer &q_, const Integer &p2_, const Integer &q2_, const Integer &n_, const Integer &n2_)
            : p(p_),
              q(q_),
              p2(p2_),
              q2(q2_),
              n(n_),
              n2(n2_) { }

    const Integer &FastMod::get_n2() const {
        return n2;
    }

    Integer FastMod::pow_mod_n2(const Integer &base, const Integer &exp) const {
        const Integer p_ = base.pow_mod_n(exp, p2);
        const Integer q_ = base.pow_mod_n(exp, q2);

        Integer gcd, r, s;
        mpz_gcdext(gcd.get_mpz_t(), r.get_mpz_t(), s.get_mpz_t(), p2.get_mpz_t(), q2.get_mpz_t());

        return (p_ * s * q2 + q_ * r * p2) % n2;
    }

    Integer FastMod::pow_mod_n2_par(const Integer &base, const Integer &exp) const {
        auto p_ = std::async(std::launch::async, [&](){return base.pow_mod_n(exp, p2);});
        auto q_ = std::async(std::launch::async, [&](){return base.pow_mod_n(exp, q2);});

        Integer gcd, r, s;
        mpz_gcdext(gcd.get_mpz_t(), r.get_mpz_t(), s.get_mpz_t(), p2.get_mpz_t(), q2.get_mpz_t());

        return (p_.get() * s * q2 + q_.get() * r * p2) % n2;
    }
}
