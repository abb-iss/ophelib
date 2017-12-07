#include "ophelib/paillier_fast.h"
#include "ophelib/packing.h"
#include "ophelib/util.h"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

using namespace std;
using namespace ophelib;

const int keysize = 2048;

/**
 * Measurements for ciphertext packing. All measurements are per sample.
 *
 * * First: Server sends vector of ciphertexts, client decrypts
 * * Second: Server sends ciphertexts packed, client decrypts and unpacks
 */
void run_ciphertext_packing(const Vec<Ciphertext> &v, const size_t plaintext_bits, const PaillierBase &paillier) {
    StopWatch t0("run_ciphertext_packing 1 total", v.length());
    StopWatch t1("run_ciphertext_packing 1 server", v.length());
    t0.start();
    t1.start();
    // server has nothing to do
    t1.stop();

    StopWatch t2("run_ciphertext_packing 1 client", v.length());
    t2.start();
    const auto v_dec = Vector::decrypt(v, paillier);
    t2.stop();
    t0.stop();

    StopWatch t3("run_ciphertext_packing 2 total", v.length());
    StopWatch t4("run_ciphertext_packing 2 server", v.length());
    t3.start();
    t4.start();
    const auto packed = Vector::pack_ciphertexts_vec(v, plaintext_bits, paillier);
    t4.stop();

    StopWatch t5("run_ciphertext_packing 2 client", v.length());
    t5.start();
    const auto v_dec2 = Vector::decrypt_pack(packed, paillier);
    t5.stop();
    t3.stop();

    assert( v_dec == v_dec2 );
}

/**
 * Measurements for encrypt_pack und decrypt_pack. All measurements are per sample.
 *
 * * First: Encrypt and decrypt using Vector::encrypt() and Vector::decrypt()
 */
void run_enc_dec_pack(const Vec<Integer> &v, const size_t plaintext_bits, const PaillierBase &paillier) {
    StopWatch t0("run_enc_dec_pack 1 total", v.length());
    StopWatch t1("run_enc_dec_pack 1 encrypt", v.length());
    t0.start();
    t1.start();
    const auto v_enc = Vector::encrypt(v, paillier);
    t1.stop();

    StopWatch t2("run_enc_dec_pack 1 decrypt", v.length());
    t2.start();
    const auto v_dec = Vector::decrypt(v_enc, paillier);
    t2.stop();
    t0.stop();

    StopWatch t3("run_enc_dec_pack 2 total", v.length());
    StopWatch t4("run_enc_dec_pack 2 encrypt", v.length());
    t3.start();
    t4.start();
    const auto packed = Vector::encrypt_pack_vec(v, plaintext_bits, paillier);
    t4.stop();

    StopWatch t5("run_enc_dec_pack 2 decrypt", v.length());
    t5.start();
    const auto v_dec2 = Vector::decrypt_pack(packed, paillier);
    t5.stop();
    t3.stop();

    assert( v_dec == v_dec2 );
}

/**
 * Test if it is faster to just decrypt, or to first pack and
 * then decrypt.
 */
void run_fast_decrypt(const Vec<Ciphertext> &v, const size_t plaintext_bits, const PaillierBase &paillier) {
    const int n_iter = 100;

    StopWatch t0("run_fast_decrypt plain decrypt", v.length() * n_iter);
    Vec<Integer> v_dec;
    t0.start();
    for(int i = 0; i < n_iter; i++) {
        v_dec = Vector::decrypt(v, paillier);
    }
    t0.stop();

    StopWatch t1("run_fast_decrypt fast decrypt with copying", v.length() * n_iter);
    Vec<Integer> v_dec2;
    t1.start();
    for(int i = 0; i < n_iter; i++) {
        const auto packed = Vector::pack_ciphertexts_vec(v, plaintext_bits, paillier);
        v_dec2 = Vector::decrypt_pack(packed, paillier);
    }
    t1.stop();

    StopWatch t2("run_fast_decrypt fast_decrypt", v.length() * n_iter);
    Vec<Integer> v_dec3;
    t2.start();
    for(int i = 0; i < n_iter; i++) {
        v_dec3 = Vector::decrypt_fast(v, paillier, plaintext_bits);
    }
    t2.stop();

    assert( v_dec == v_dec2 );
    assert( v_dec == v_dec3 );
}

int main () {
    PaillierFast paillier(keysize);
    paillier.generate_keys();

    cout << "key size: " << keysize << endl;

    StopWatch::header();

    const size_t n_values = 500;
    const size_t plaintext_bits = 10;

    const auto x = Vector::rand_bits_neg(n_values, plaintext_bits);
    const auto x_enc = Vector::encrypt(x, paillier);
    run_ciphertext_packing(x_enc, plaintext_bits, paillier);
    run_enc_dec_pack(x, plaintext_bits, paillier);
    run_fast_decrypt(x_enc, plaintext_bits, paillier);

    return 0;
}
