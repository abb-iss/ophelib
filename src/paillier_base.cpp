#include "ophelib/paillier_base.h"
#include "ophelib/error.h"

#include <sstream>
#include <vector>

namespace ophelib {
    Ciphertext::Ciphertext(const Integer &data_, const std::shared_ptr <Integer> &n2_shared_, const std::shared_ptr<FastMod> &fast_mod_)
            : data(data_),
              n2_shared(n2_shared_),
              fast_mod(fast_mod_) { }

    Ciphertext::Ciphertext(const Integer &data_, const std::shared_ptr <Integer> &n2_shared_)
            : data(data_),
              n2_shared(n2_shared_) { }

    Ciphertext::Ciphertext(const Integer &data_)
            : data(data_) { }

    Ciphertext::Ciphertext() { }

    bool Ciphertext::operator==(const Ciphertext &input) const {
        return this->data == input.data;
    }

    bool Ciphertext::operator!=(const Ciphertext &input) const {
        return this->data != input.data;
    }

    Ciphertext Ciphertext::operator-() const {
        if(!this->n2_shared)
            error_exit("no modulus set!");

        Ciphertext ret = *this;
        ret.data = this->data.inv_mod_n(*n2_shared.get());
        return ret;
    }

    Ciphertext Ciphertext::operator+(const Ciphertext &other) const {
        Ciphertext ret = *this;
        ret += other;
        return ret;
    }

    void Ciphertext::operator+=(const Ciphertext &other) {
        if(!this->n2_shared)
            error_exit("no modulus set!");

        if(this->n2_shared.get() != other.n2_shared.get() &&
           *(this->n2_shared.get()) != *(other.n2_shared.get()))
            error_exit("cannot operate on ciphertexts from different keys!");

        this->data = (this->data * other.data) % *n2_shared.get();
    }

    Ciphertext Ciphertext::operator-(const Ciphertext &other) const {
        Ciphertext ret = *this;
        ret -= other;
        return ret;
    }

    void Ciphertext::operator-=(const Ciphertext &other) {
        if(!this->n2_shared)
            error_exit("no modulus set!");

        if(this->n2_shared.get() != other.n2_shared.get() &&
           *(this->n2_shared.get()) != *(other.n2_shared.get()))
            error_exit("cannot operate on ciphertexts from different keys!");

        this->data = (this->data * other.data.inv_mod_n(*n2_shared.get()))  % *n2_shared.get();
    }

    Ciphertext Ciphertext::operator*(const Integer &other) const {
        Ciphertext ret = *this;
        ret *= other;
        return ret;
    }

    void Ciphertext::operator*=(const Integer &other) {
        if(!this->n2_shared)
            error_exit("no modulus set!");

        if(fast_mod) {
            this->data = fast_mod.get()->pow_mod_n2(this->data, other);
        } else {
            this->data = this->data.pow_mod_n(other, *n2_shared.get());
        }
    }

    const std::string Ciphertext::to_string(const bool brief) const {
        std::ostringstream o("");

        o << "<Ciphertext";
        o << " data=" << data.to_string(brief);
        if(n2_shared)
            o << " n2_shared=" << n2_shared.get()->to_string(brief);
        o << ">";

        return o.str();
    }

    std::ostream &operator<<(std::ostream &stream, const Ciphertext &c) {
        stream << c.to_string(false);
        return stream;
    }

    PrivateKey::PrivateKey(const size_t key_size_bits_, const Integer &p_, const Integer &q_)
            : key_size_bits(key_size_bits_),
              p(p_),
              q(q_) { }

    PrivateKey::PrivateKey(const size_t key_size_bits_, const size_t a_bits_, const Integer &p_, const Integer &q_, const Integer &a_)
            : key_size_bits(key_size_bits_),
              a_bits(a_bits_),
              p(p_),
              q(q_),
              a(a_){ }

    PrivateKey::PrivateKey() : key_size_bits(0) { }

    bool PrivateKey::operator==(const PrivateKey &input) const {
        return p == input.p && q == input.q && a == input.a;
    }

    const std::string PrivateKey::to_string(const bool brief) const {
        std::ostringstream o("");
        o << "<PrivateKey[";
        o << key_size_bits << "]";
        o << " p=" << p.to_string(brief);
        o << " q=" << q.to_string(brief);
        o << " a=" << a.to_string(brief);
        o << ">";
        return o.str();
    }

    PublicKey::PublicKey(const size_t key_size_bits_, const Integer &n_, const Integer &g_)
            : key_size_bits(key_size_bits_),
              n(n_),
              g(g_) { }

    PublicKey::PublicKey() : key_size_bits(0) { }

    bool PublicKey::operator==(const PublicKey &input) const {
        return n == input.n && g == input.g;
    }

    const std::string PublicKey::to_string(const bool brief) const {
        std::ostringstream o("");
        o << "<PublicKey[";
        o << key_size_bits << "]";
        o << " g=" << g.to_string(brief);
        o << " n=" << n.to_string(brief);
        o << ">";
        return o.str();
    }

    KeyPair::KeyPair(const ophelib::PublicKey &pub_, const ophelib::PrivateKey &priv_)
            : pub(pub_),
              priv(priv_) { }

    KeyPair::KeyPair() { }

    bool KeyPair::operator==(const KeyPair &input) const {
        return priv == input.priv && pub == input.pub;
    }

    const std::string KeyPair::to_string(const bool brief) const {
        std::ostringstream o("");
        o << "<KeyPair";
        o << " priv=" << priv.to_string(brief);
        o << " pub=" << pub.to_string(brief);
        o << ">";
        return o.str();
    }

    PaillierBase::PaillierBase(const size_t key_size_bits_)
            : key_size_bits(key_size_bits_),
              plaintxt_size_bits(key_size_bits_) { }

    PaillierBase::PaillierBase(const PublicKey &pub_)
            : key_size_bits(pub_.key_size_bits),
              plaintxt_size_bits(pub_.key_size_bits),
              pub(pub_),
              have_pub(true) { }

    PaillierBase::PaillierBase(const PublicKey &pub_, const PrivateKey &priv_)
            : key_size_bits(priv_.key_size_bits),
              plaintxt_size_bits(priv_.key_size_bits),
              priv(priv_),
              pub(pub_),
              have_priv(true),
              have_pub(true) { }

    PaillierBase::PaillierBase(const KeyPair &pair)
            : key_size_bits(pair.priv.key_size_bits),
              plaintxt_size_bits(pair.priv.key_size_bits),
              priv(pair.priv),
              pub(pair.pub),
              have_priv(true),
              have_pub(true) { }

    const PublicKey &PaillierBase::get_pub() const {
        if(!have_pub)
            error_exit("don't have a public key!");

        return pub;
    }

    const PrivateKey &PaillierBase::get_priv() const {
        if(!have_priv)
            error_exit("don't have a private key!");

        return priv;
    }

    const KeyPair PaillierBase::get_keypair() const {
        return KeyPair(get_pub(), get_priv());
    }

    const std::string PaillierBase::to_string(bool brief) const {
        std::ostringstream o("");
        o << "<PaillierBase[" << key_size_bits << "]";
        if(have_pub) {
            o << " pub=" << pub.to_string(brief);
        }
        if(have_priv) {
            o << " priv=" << priv.to_string(brief);
        }
        o << ">";
        return o.str();
    }

    size_t PaillierBase::ciphertext_size_bits() const {
        return key_size_bits * 2;
    }

    size_t PaillierBase::plaintext_size_bits() const {
        return plaintxt_size_bits;
    }

    const std::shared_ptr<FastMod> PaillierBase::get_fast_mod() const {
        return fast_mod;
    }

    const std::shared_ptr<Integer> PaillierBase::get_n2() const {
        return n2_shared;
    }

    const Integer PaillierBase::plaintext_lower_boundary() const {
        return plaintxt_lower_boundary;
    }

    const Integer PaillierBase::plaintext_upper_boundary() const {
        return plaintxt_upper_boundary;
    }
}
