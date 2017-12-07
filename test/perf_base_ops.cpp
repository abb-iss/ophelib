#include "ophelib/paillier_fast.h"
#include "ophelib/util.h"

using namespace std;
using namespace ophelib;

#ifndef PERF_N_ITER
#define PERF_N_ITER 200
#endif

const int n_iter_ = PERF_N_ITER;
const int max_int = 1000000;
const size_t keysize = 2048;

/**
 * Generate keys
 */
void run_keygen() {
    const auto n_iter_keygen = n_iter_ / 10;
    StopWatch watch("KeyGen", n_iter_keygen);
    PaillierFast crypto(keysize);
    watch.start();

    for(int i = 0; i < n_iter_keygen; i++) {
        crypto.generate_keys();
    }

    watch.stop();
}

/**
 * Encryption with randomness
 */
void run_enc(PaillierFast &crypto, vector<Integer> &ints, vector<Ciphertext> &cipher) {
    StopWatch watch("Encrypt", n_iter_);
    watch.start();

    for(int i = 0; i < n_iter_; i++) {
        cipher[i] = crypto.encrypt(ints[i]);
    }

    watch.stop();
}

/**
 * Decryption
 */
void run_dec(PaillierFast &crypto, vector<Integer> &ints, vector<Ciphertext> &cipher) {
    StopWatch watch("Decrypt", n_iter_);
    watch.start();

    for(int i = 0; i < n_iter_; i++) {
        ints[i] = crypto.decrypt(cipher[i]);
    }

    watch.stop();
}

/**
 * Ciphertext addition
 */
void run_add(vector<Ciphertext> &cipher) {
    StopWatch watch("Add", n_iter_);
    watch.start();

    for(int i = 0; i < n_iter_; i++) {
        cipher[i] = cipher[i] + cipher[(i + 1) % n_iter_];
    }

    watch.stop();
}

/**
 * Ciphertext subtraction
 */
void run_sub(vector<Ciphertext> &cipher) {
    StopWatch watch("Sub", n_iter_);
    watch.start();

    for(int i = 0; i < n_iter_; i++) {
        cipher[i] = cipher[i] - cipher[(i + 1) % n_iter_];
    }

    watch.stop();
}

/**
 * Ciphertext scalar multiplication
 */
void run_mul(vector<Ciphertext> &cipher) {
    vector<Integer> scalars(n_iter_);
    for(int i = 0; i < n_iter_; i++) {
        scalars[i] = Integer(rand() % max_int);
        if(scalars[i] == 0)
            scalars[i] = 1;
    }

    StopWatch watch("Mul", n_iter_);
    watch.start();

    for(int i = 0; i < n_iter_; i++) {
        cipher[i] = cipher[i] * scalars[i];
    }

    watch.stop();
}

/**
 * Division with decryption
 */
void run_div_dec(PaillierFast &crypto, vector<Ciphertext> &cipher) {
    vector<Integer> scalars(n_iter_);
    for(int i = 0; i < n_iter_; i++) {
        scalars[i] = Integer(rand() % max_int);
        if(scalars[i] == 0)
            scalars[i] = 1;
    }

    StopWatch watch("DivDec", n_iter_);
    watch.start();

    for(int i = 0; i < n_iter_; i++) {
        cipher[i] = crypto.encrypt(crypto.decrypt(cipher[i]) / scalars[i]);
    }

    watch.stop();
}

int main () {
    PaillierFast crypto(keysize);
    crypto.generate_keys();

    vector<Integer> rand_ints(n_iter_);
    vector<Ciphertext> rand_ciphertexts(n_iter_);

    for(int i = 0; i < n_iter_; i++) {
        rand_ints[i] = Integer(rand() % max_int);
    }

    cerr << "# " << crypto.to_string() << endl;
    StopWatch::header();

    run_keygen();
    run_enc(crypto, rand_ints, rand_ciphertexts);
    run_dec(crypto, rand_ints, rand_ciphertexts);
    run_add(rand_ciphertexts);
    run_sub(rand_ciphertexts);
    run_mul(rand_ciphertexts);
    run_div_dec(crypto, rand_ciphertexts);

    return 0;
}
