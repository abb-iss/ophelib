#include "ophelib/paillier.h"
#include "ophelib/random.h"
#include "ophelib/error.h"

#include <gmpxx.h>
#include <memory>

namespace ophelib {

    Paillier::Paillier(const PublicKey &pub_)
            : PaillierBase(pub_) {
        precompute();
    }

    Paillier::Paillier(const PublicKey &pub_, const PrivateKey &priv_)
            : PaillierBase(pub_, priv_) {
        if(priv_.a != 0 || priv_.a_bits == 0)
            error_exit("invalid private key, not from a Paillier instance!");
        precompute();
    }

    Paillier::Paillier(const KeyPair &pair)
            : PaillierBase(pair) {
        if(pair.priv.a != 0 || pair.priv.a_bits == 0)
            error_exit("invalid private key, not from a Paillier instance!");
        precompute();
    }

    void Paillier::generate_keys() {
        const size_t prime_size_bits = key_size_bits / 2;
        Integer p, q, n, g;
        Random& rand = Random::instance();

        do {
            p = rand.rand_prime(prime_size_bits);
            q = rand.rand_prime(prime_size_bits);

            while(p == q) {
                p = rand.rand_prime(prime_size_bits);
            }

            n = p * q;
        }
        while(n.size_bits() != key_size_bits);

        g = rand.rand_int_bits(this->key_size_bits * this->key_size_bits);

        priv = PrivateKey(key_size_bits, p, q);
        pub = PublicKey(key_size_bits, n, g);

        have_priv = true;
        have_pub = true;
        this->precompute();
    }

    void Paillier::precompute() {
        if(!have_pub)
            error_exit("don't have a public key!");

        n_minus_one = pub.n - 1;
        n2 = pub.n.pow(2);

        if(have_priv) {
            if(Integer::gcd(priv.p, priv.q) != 1) {
                math_error_exit("p and q are not coprime!");
            }

            lambda = Integer::lcm(priv.p - 1, priv.q - 1);
            mu = Integer::L(pub.g.pow_mod_n(lambda, n2), pub.n).inv_mod_n(pub.n);
        }

        n2_shared = std::make_shared<Integer>(n2);
        pos_neg_boundary = pub.n / 2;
        plaintxt_upper_boundary = pos_neg_boundary;
        plaintxt_lower_boundary = -pos_neg_boundary;
    }

    Integer Paillier::decrypt(const Ciphertext &ciphertext) const {
        if(!have_priv)
            error_exit("don't have a private key!");

        #ifdef DEBUG
        /* If they have the same pointer, they are the same. If not, it might
         * still be the same number, but initialized seperately. */
        if(this->n2_shared.get() != ciphertext.n2_shared.get() &&
           *(this->n2_shared.get()) != *(ciphertext.n2_shared.get()))
            error_exit("cannot decrypt a ciphertext from another n!");
        #endif

        Integer ret = (
            Integer::L(
                ciphertext.data.pow_mod_n(lambda, n2),
                pub.n
            ) * mu
        ) % pub.n;

        if(ret > pos_neg_boundary) {
            ret = ret - pub.n;
        }

        return ret;
    }

    Ciphertext Paillier::encrypt(const Integer &plaintext) const {
        return randomize(encrypt_no_rand(plaintext));
    }

    Ciphertext Paillier::encrypt_no_rand(const Integer &plaintext) const {
        Integer ret;

        if(!have_pub)
            error_exit("don't have a public key!");

        // c = g^m (mod n^2)
        if(plaintext < 0) {
            ret = pub.g.pow_mod_n(pub.n + plaintext, n2);
        } else {
            ret = pub.g.pow_mod_n(plaintext, n2);
        }

        return Ciphertext(ret, n2_shared);
    }

    Ciphertext Paillier::randomize(Ciphertext ciphertext) const {
        Integer ret = (ciphertext.data * randomizer_val()) % *n2_shared.get();

        return Ciphertext(ret, n2_shared);
    }

    Integer Paillier::randomizer_val() const {
        Integer val = Random::instance().rand_int(n_minus_one) + Integer(1);
        return val.pow_mod_n(pub.n, n2);
    }

    const std::string Paillier::to_string(bool brief) const {
        std::ostringstream o("");
        o << "<Paillier[" << key_size_bits << "]";
        o << " n_minus_one=" << n_minus_one.to_string(brief);
        o << " n2=" << n2.to_string(brief);
        if(have_pub) {
            o << " pub=" << pub.to_string(brief);
        }
        if(have_priv) {
            o << " priv=" << priv.to_string(brief);
            o << " lambda=" << lambda.to_string(brief);
            o << " mu=" << mu.to_string(brief);
        }
        o << ">";

        return o.str();
    }
}
