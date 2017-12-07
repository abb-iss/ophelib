#include "ophelib/paillier_fast.h"
#include "ophelib/random.h"
#include "ophelib/omp_wrap.h"

namespace ophelib {
    PaillierFast::PaillierFast(const size_t key_size_bits_, const size_t a_bits_, const size_t r_bits_)
            : PaillierBase(key_size_bits_),
              a_bits(a_bits_),
              r_bits(r_bits_),
              randomizer(this, param_r_lut_size(r_bits), param_r_use_count(r_bits)) { }

    PaillierFast::PaillierFast(const size_t key_size_bits_)
            : PaillierFast(key_size_bits_,
                param_a_bits(key_size_bits_),
                param_r_bits(key_size_bits_)) { }

    PaillierFast::PaillierFast(const PublicKey &pub_)
            : PaillierFast(pub_.key_size_bits,
                param_a_bits(pub_.key_size_bits),
                param_r_bits(pub_.key_size_bits)) {
        pub = pub_;
        have_pub = true;
        precompute();
    }

    PaillierFast::PaillierFast(const PublicKey &pub_, const PrivateKey &priv_)
            : PaillierFast(pub_.key_size_bits,
                param_a_bits(pub_.key_size_bits),
                param_r_bits(pub_.key_size_bits)) {
        if(priv_.a == 0 || priv_.a_bits == 0)
            error_exit("invalid private key, not from a PaillierFast instance!");
        pub = pub_;
        priv = priv_;
        have_priv = true;
        have_pub = true;
        precompute();
    }

    PaillierFast::PaillierFast(const KeyPair &pair)
            : PaillierFast(pair.pub.key_size_bits,
                           param_a_bits(pair.pub.key_size_bits),
                           param_r_bits(pair.pub.key_size_bits)) {
        if(pair.priv.a == 0 || pair.priv.a_bits == 0)
            error_exit("invalid private key, not from a PaillierFast instance!");
        pub = pair.pub;
        priv = pair.priv;
        have_priv = true;
        have_pub = true;
        precompute();
    }

    void PaillierFast::check_valid_key_size(size_t key_size_bits_) const {
        #ifdef DEBUG
        if(key_size_bits_ == 1024)
            std::cerr << "WARNING: Key size of 1024 used, insecure for production!\n";
        #endif

        if(key_size_bits_ != 1024 &&
           key_size_bits_ != 2048 &&
           key_size_bits_ != 3072 &&
           key_size_bits_ != 4096 &&
           key_size_bits_ != 7680)
            error_exit("supported key_size_bits are: 2048, 3072, 4096, 7680!");
    }

    void PaillierFast::check_valid_r_bits(size_t r_bits_) const {
        if(r_bits_ != 80 &&
           r_bits_ != 112 &&
           r_bits_ != 128 &&
           r_bits_ != 140 &&
           r_bits_ != 192)
            error_exit("supported r_bits are: 112, 128, 140, 192!");
    }

    size_t PaillierFast::param_a_bits(size_t key_size_bits_) const {
        check_valid_key_size(key_size_bits_);
        switch(key_size_bits_) {
            case 1024: return 320;
            case 2048: return 512;
            case 3072: return 512;
            case 4096: return 512;
            case 7680: return 1024;
            default: return 0;
        }
    }

    size_t PaillierFast::param_r_bits(size_t key_size_bits_) const {
        check_valid_key_size(key_size_bits_);
        switch(key_size_bits_) {
            case 1024: return 80;
            case 2048: return 112;
            case 3072: return 128;
            case 4096: return 140;
            case 7680: return 192;
            default: return 0;
        }
    }

    size_t PaillierFast::param_r_lut_size(size_t r_bits_) const {
        check_valid_r_bits(r_bits_);
        switch(r_bits_) {
            case 80: return 256;
            case 112: return 4096;
            case 128: return 4096;
            case 140: return 8192;
            case 192: return 16384;
            default: return 0;
        }
    }

    size_t PaillierFast::param_r_use_count(size_t r_bits_) const {
        check_valid_r_bits(r_bits_);
        switch(r_bits_) {
            case 80: return 15;
            case 112: return 12;
            case 128: return 14;
            case 140: return 14;
            case 192: return 18;
            default: return 0;
        }
    }

    void PaillierFast::generate_keys() {
        Integer p, q, n, g, a;
        const size_t prime_size_bits = key_size_bits / 2 - a_bits;
        Random& rand = Random::instance();

        do {
            Integer cp, cq;

            cp = rand.rand_int_bits(prime_size_bits);
            if(cp.size_bits() != prime_size_bits)
                continue;
            cq = rand.rand_int_bits(prime_size_bits);
            if(cq.size_bits() != prime_size_bits)
                continue;

            a = rand.rand_prime(a_bits);

            p = a * cp + 1;
            while(!p.is_prime())
                p = p + a;

            q = a * cq + 1;
            while(!q.is_prime())
                q = q + a;

            n = p * q;
        }
        while(n.size_bits() != key_size_bits || p == q);

        if(p > q)
            swap(p, q);

        Integer lambda = Integer::lcm(p - 1, q - 1);
        g = Integer(2).pow_mod_n(lambda / a, n);

        priv = PrivateKey(key_size_bits, a_bits, p, q, a);
        pub = PublicKey(key_size_bits, n, g);

        have_priv = true;
        have_pub = true;
        this->precompute();
    }

    void PaillierFast::precompute() {
        if(!have_pub)
            error_exit("don't have a public key!");

        n2 = pub.n * pub.n;
        n2_shared = std::make_shared<Integer>(n2);

        if(have_priv) {
            fast_mod = std::make_shared<FastMod>(priv.p, priv.q, priv.p * priv.p, priv.q * priv.q, pub.n, n2);
            mu = Integer::L(fast_mod.get()->pow_mod_n2(pub.g, priv.a), pub.n).inv_mod_n(pub.n);
        }

        pos_neg_boundary = pub.n / 2;
        plaintxt_upper_boundary = pos_neg_boundary;
        plaintxt_lower_boundary = -pos_neg_boundary;

        randomizer.precompute();
        precomputed_zero = encrypt(0);
    }

    Integer PaillierFast::decrypt(const Ciphertext &ciphertext) const {
        if(!have_priv)
            error_exit("don't have a private key!");

        #ifdef DEBUG
        /* If they have the same pointer, they are the same. If not, it might
         * still be the same number, but initialized seperately. */
        if(ciphertext.n2_shared && this->n2_shared.get() != ciphertext.n2_shared.get() &&
                *(this->n2_shared.get()) != *(ciphertext.n2_shared.get()))
            error_exit("cannot decrypt a ciphertext from another n!");
        #endif

        Integer ret = (
            Integer::L(
                fast_mod.get()->pow_mod_n2(ciphertext.data, priv.a),
                pub.n
            ) * mu
        ) % pub.n;

        if(ret > pos_neg_boundary) {
            ret -= pub.n;
        }

        return ret;
    }

    Integer PaillierFast::check_plaintext(const Integer &plaintext) const {
        if(!have_pub)
            error_exit("don't have a public key!");

        if(plaintext < 0) {
            return pub.n + plaintext;
        } else {
            return plaintext;
        }
    }

    Ciphertext PaillierFast::encrypt(const Integer &plaintext) const {
        if(!have_pub)
            error_exit("don't have a public key!");

        Integer m = check_plaintext(plaintext),
                tmp;

        if(have_priv) {
            tmp = fast_mod.get()->pow_mod_n2(pub.g, m) * randomizer.get_noise();
        } else {
            tmp = pub.g.pow_mod_n(m, n2) * randomizer.get_noise();
        }
        return Ciphertext(tmp % n2, n2_shared, fast_mod);
    }

    Ciphertext PaillierFast::zero_ciphertext() const {
        if(!have_pub)
            error_exit("don't have a public key!");
        return precomputed_zero;
    }

    const std::string PaillierFast::to_string(bool brief) const {
        std::ostringstream o("");

        o << "<Paillier[" << key_size_bits << ", " << a_bits <<"]";
        o << " n2=" << n2.to_string(brief);
        if(have_pub) {
            o << " pub=" << pub.to_string(brief);
        } else {
            o << " have_pub=" << have_pub;
        }
        if(have_priv) {
            o << " priv=" << priv.to_string(brief);
            o << " mu=" << mu.to_string(brief);
        } else {
            o << " have_priv=" << have_priv;
        }
        o << " a_bits=" << a_bits;
        o << " r_bits=" << r_bits;
        o << " randomizer=" << randomizer.to_string(brief);
        o << ">";

        return o.str();
    }

    const Integer PaillierFast::Randomizer::get_noise() const {
        if(!precomputed)
            error_exit("lookup table not precomputed!");

        if(paillier->fast_mod) {
            return paillier->fast_mod.get()->pow_mod_n2(g_pow_n, r());
        } else {
            return g_pow_n.pow_mod_n(r(), paillier->n2);
        }
    }

    PaillierFast::Randomizer::Randomizer(const PaillierFast *paillier_)
            : paillier(paillier_) { }

    void PaillierFast::Randomizer::precompute() {
        if(paillier->fast_mod) {
            g_pow_n = paillier->fast_mod.get()->pow_mod_n2(paillier->pub.g, paillier->pub.n);
        } else {
            g_pow_n = paillier->pub.g.pow_mod_n(paillier->pub.n, paillier->n2);
        }
        precomputed = true;
    }

    const Integer PaillierFast::Randomizer::r() const {
        return Random::instance().rand_int_bits(paillier->r_bits);
    }

    const std::string PaillierFast::Randomizer::to_string(const bool brief) const {
        std::ostringstream o("");
        o << "<Randomizer";
        o << " g_pow_n=" << g_pow_n.to_string(brief);
        o << " precomputed=" << precomputed;
        o << ">";

        return o.str();
    }

    PaillierFast::FastRandomizer::FastRandomizer(const PaillierFast *paillier_, const size_t r_lut_size_, const size_t r_use_count_)
            : Randomizer(paillier_),
              r_lut_size(r_lut_size_),
              r_use_count(r_use_count_) { }

    void PaillierFast::FastRandomizer::precompute() {
        Randomizer::precompute();

        #ifdef DEBUG
        std::cerr << "PaillierFast::FastRandomizer::precompute: precomputing " << r_lut_size<< std::endl;
        #endif

        gn_pow_r.reserve(r_lut_size);

        omp_declare_lock(writelock);
        omp_init_lock(&writelock);
        if(paillier->fast_mod) {
            paillier->fast_mod.get()->pow_mod_n2(g_pow_n, r());

            #pragma omp parallel for
            for(auto i = 0u; i < r_lut_size; i++) {
                const auto rand = paillier->fast_mod.get()->pow_mod_n2(g_pow_n, r());

                omp_set_lock(&writelock);
                gn_pow_r.push_back(rand);
                omp_unset_lock(&writelock);
            }
        } else {
            #pragma omp parallel for
            for(auto i = 0u; i < r_lut_size; i++) {
                const auto rand = g_pow_n.pow_mod_n(r(), paillier->n2);

                omp_set_lock(&writelock);
                gn_pow_r.push_back(rand);
                omp_unset_lock(&writelock);
            }
        }
        omp_destroy_lock(&writelock);

        #ifdef DEBUG
        size_t size = 0;
        for(auto i = 0u; i < r_lut_size; i++) {
            size += gn_pow_r[i].size_bits();
        }
        std::cerr << "PaillierFast::FastRandomizer::precompute: computed r, size=" << size / 8 << " bytes" << std::endl;
        #endif
    }

    const Integer PaillierFast::FastRandomizer::get_noise() const {
        if(!precomputed)
            error_exit("lookup table not precomputed!");
        Integer ret = 1;
        Random &rand = Random::instance();
        for(auto i = 0u; i < r_use_count; i++) {
            unsigned int ix = rand.rand_int(r_lut_size).to_uint();
            ret = (ret * gn_pow_r[ix]) % paillier->n2;
        }

        return ret;
    }

    const std::string PaillierFast::FastRandomizer::to_string(const bool brief) const {
        std::ostringstream o("");
        o << "<FastRandomizer";
        o << " g_pow_n=" << g_pow_n.to_string(brief);
        o << " r_lut_size=" << r_lut_size;
        o << " r_use_count=" << r_use_count;
        o << " precomputed=" << precomputed;
        o << ">";

        return o.str();
    }
}
