#include "ophelib/util.h"

#include <iostream>
#include <cmath>

using namespace ophelib;
using namespace std;

double compute_bits(const size_t n_gn_pow_r, const size_t n_gn_use) {
    Integer combinations = nCr(Integer(n_gn_pow_r + n_gn_use - 1), Integer(n_gn_use));
    cout << "nCr(" << n_gn_pow_r + n_gn_use - 1 << ", " << n_gn_use << ") = " << combinations.to_string_();
    double c = mpz_get_d(combinations.get_mpz_t());
    cout << " (" << c << ")" << endl;
    return log2(c);
}

/**
 * Computes n_gn_use
 * given n_gn_pow_r (lut size)
 * and r_bits (randomness of r)
 */
int main() {
    const size_t r_bits = 112;
    const size_t r_lut = 1 << 12;
    cout << "r_bits=" << r_bits << endl;
    cout << "r_lut=" << r_lut << endl;

    for(size_t r_use = 1;; r_use++) {
        double b = compute_bits(r_lut, r_use);

        if(b >= r_bits) {
            cout << "\nr_use=" << r_use << " offers " << b << " bits" << endl;
            break;
        }
    }

    return EXIT_SUCCESS;
}