#pragma once

#include "ophelib/integer.h"
#include "ophelib/paillier_fast.h"
#include "ophelib/packing.h"
#include "ophelib/vector.h"

#include <functional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wpedantic"
#include <NTL/vector.h>
#include <NTL/matrix.h>
#include <NTL/RR.h>
#include <NTL/vec_RR.h>
#include <NTL/mat_RR.h>
#include <NTL/mat_ZZ.h>
#include <NTL/tools.h>
#pragma GCC diagnostic pop

using NTL::Vec;
using NTL::Mat;

namespace ophelib {

    /**
     * This namespace contains all the machine learning functionality.
     * Generally, The implementation classes can be used in a fashion
     * similar to the [Scikit-learn](http://scikit-learn.org/stable/)
     * library:
     *
     *     LinregPlain reg;
     *     reg.fit(X_train, y_train);
     *     auto pred = reg.predict(X_test);
     */
    namespace ML {
        /**
         * Classic linear regression, using gradient descent.
         * Features and targets must already be normalized
         * and integerized when using this class.
         */
        class LinregPlain {
            /**
             * Multiplier used during integerize
             */
            const Integer multiplier;

            /**
             * 1/alpha, where alpha is the learning rate
             */
            const Integer alpha_inv;

            /**
             * Maximum iterations for gradient descent
             */
            const size_t n_iter;

            long n_features;

            /**
             * Weight vector
             */
            Vec<Integer> theta;

            /**
             * Do a single gradient descent step
             * Returs true if converged
             */
            bool grad_desc_step(const Mat<Integer> &X, const Vec<Integer> &y);

        public:

            /**
             * Initialize a new linear regressor.
             *
             * @param multiplier which was used for integerizing
             * @param alpha_inv: 1/alpha, where alpha is learning rate
             * @param n_iter: maximum iterations for gradient descent (will stop earlier if converges)
             */
            LinregPlain(const Integer &multiplier, const Integer &alpha_inv = (int)(1 / 0.01), const size_t n_iter = 1000);

            /**
             * Train
             * @param X contains the features and is a n_feature x n_samples matrix
             * @param y contains the target values and has length n_samples.
             * @return number of gradient descent iterations. If smaller than n_iter,
             *         this means that it converged faster than n_iter.
             */
            size_t fit(const Mat<Integer> &X, const Vec<Integer> &y);

            /**
             * Predict target values from feature matrix
             * @return Predicted values
             */
            Vec<Integer> predict(const Mat<Integer> &X) const;

            /**
             * @return Weights, available after fitting
             */
            const Vec<Integer> &get_weights() const;
        };

        /**
         * Linear regression with plaintext X (features), encrypted y (target)
         * and encrypted theta (weights).
         * This implementation uses gradient descent to compute the model.
         *
         * Fit and predict are also offered in a variant where you can pass
         * X_t directly to save some time, if you happen to have X transposed
         * around anyways.
         *
         * Note that y must be integerized with 2 times the precision used
         * for X:
         * - X = integerize( normalize( X_ ) , nbits)
         * - y = encrypt(integerize( normalize( y_ ), nbits * 2))
         */
        class LinregPlainEnc {
        public:
            /**
             * Type for client callback.
             * Arguments:
             * - PackedCiphertext vector, dividends
             * - Plaintext, divisor
             *
             * Should return
             *   encrypt(-decrypt(arg1) / (arg2));
             */
            typedef std::function< Vec<Ciphertext> (const Vec<PackedCiphertext>&, const Integer&)> client_callback_fn_t;

            class client_callback_cls_t {
            public:
                virtual Vec<Ciphertext> call(const Vec<PackedCiphertext> &error, const Integer &divisor) const = 0;
                virtual ~client_callback_cls_t();
            };

        private:

            class client_callback_cls_t_impl: public client_callback_cls_t {
                const client_callback_fn_t client_callback;
            public:
                client_callback_cls_t_impl(const client_callback_fn_t &client_callback);
                virtual Vec<Ciphertext> call(const Vec<PackedCiphertext> &error, const Integer &divisor) const;
            };

            /**
             * Callback, implemented by client, which does division.
             * Has to be done on client side as decryption is necessary.
             */
            const client_callback_cls_t *client_callback;

            /**
             * If the std::function constructor was used, it has been wrapped
             * into a client_callback_cls_t internally and we thus also are
             * responsible for deleting it.
             */
            const bool destroy_callback_wrapper;

            const Integer multiplier;

            /**
             * Paillier pubkey instance, necessary for packing
             */
            const PaillierFast paillier;

            /**
             * 1/alpha, where alpha is the learning rate
             */
            const Integer alpha_inv;

            /**
             * N iterations
             */
            const size_t n_iter;

            long n_features;

            /**
             * Weight vector
             */
            Vec<Ciphertext> theta;

        public:
            /**
             * Initialize a new linear regressor.
             *
             * @param client_callback Callback, implemented by client, which does division.
             *        Will get called many times, so make it fast!
             * @param multiplier which was used for integerization of X.
             * @param pubkey needed for packing
             * @param alpha_inv 1/alpha, where alpha is learning rate
             * @param n_iter maximum iterations (will stop earlier on converge)
             */
            LinregPlainEnc(const client_callback_fn_t &client_callback, const Integer &multiplier, const PublicKey &pubkey, const Integer &alpha_inv = (int)(1 / 0.01), const size_t n_iter = 1000);

            /**
             * Initialize a new linear regressor.
             *
             * @param client_callback Callback, implemented by client, which does division.
             *        Will get called many times, so make it fast! This class does not take
             *        ownership of the pointer, it's your task to delete it after use.
             * @param multiplier which was used for integerization of X.
             * @param pubkey needed for packing
             * @param alpha_inv 1/alpha, where alpha is learning rate
             * @param n_iter maximum iterations (will stop earlier on converge)
             */
            LinregPlainEnc(const client_callback_cls_t *client_callback, const Integer &multiplier, const PublicKey &pubkey, const Integer &alpha_inv = (int)(1 / 0.01), const size_t n_iter = 1000);

            ~LinregPlainEnc();

            /**
             * Train.
             * @param X contains the features and is a n_feature x n_samples matrix
             * @param y contains the target values and has length n_samples
             * @return number of gradient descent iterations. Will always be n_iter,
             *         as the weight vector is encrypted and thus the server does not
             *         know when to stop iterations.
             */
            size_t fit(const Mat<Integer> &X, const Vec<Ciphertext> &y);
            size_t fit(const Mat<Integer> &X, const Mat<Integer> &X_t, const Vec<Ciphertext> &y);

            /**
             * Predict target values from feature matrix
             * @return Predicted values
             */
            Vec<Ciphertext> predict(const Mat<Integer> &X) const;
            Vec<Ciphertext> predict(const Mat<Integer> &X, const Mat<Integer> &X_t) const;

            /**
             * @return Weights, available after fitting
             */
            const Vec<Ciphertext> &get_weights() const;

            /**
             * Helper function to construct the client callback
             * function
             */
            static const client_callback_fn_t construct_client_callback(const PaillierBase &paillier);
        };

        /**
         * Linear regression with plaintext X (features), encrypted y (target)
         * and encrypted theta (weights).
         * This implementation solves the normal equation to compute the model.
         *
         * Fit and predict are also offered in a variant where you can pass
         * X_t directly to save some time, if you happen to have X transposed
         * around anyways.
         */
        class LinregPlainEncEqn {
        public:
            /**
             * Type for client callback.
             * Arguments:
             * - Ciphertext vector, dividends
             * - Plaintext, divisor
             *
             * Should return
             *   encrypt(decrypt(arg1) / (arg2));
             */
            typedef std::function< Vec<Ciphertext> (Vec<Ciphertext>, Integer)> client_callback_t;

        private:
            /**
             * Callback, implemented by client, which does division.
             * Has to be done on client side as decryption is necessary.
             */
            const client_callback_t client_callback;

            /**
             * Integerizer which will be used for X
             */
            const Vector::Integerizer inter;

            long n_features;

            /**
             * Weight vector
             */
            Vec<Ciphertext> theta;

        public:
            /**
             * Initialize a new linear regressor.
             *
             * @param client_callback Callback, implemented by client, which does division.
             *                        Will get called only once during fit.
             * @param inter  Integerizer which was used for y, will be used to integerize
             *               `(X^T * X)^-1 * X^T`
             */
            LinregPlainEncEqn(const client_callback_t &client_callback, const Vector::Integerizer &inter);

            /**
             * Train. Solves Ax=b with A=X'X, b=X'y.
             *
             * @param X contains the features and is a n_feature x n_samples matrix.
             *          Must be normalized, but not integerized.
             * @param y contains the target values and has length n_samples.
             */
            void fit(const Mat<float> &X, const Vec<Ciphertext> &y);
            void fit(const Mat<float> &X, const Mat<float> &X_t, const Vec<Ciphertext> &y);

            /**
             * Predict target values from feature matrix
             * Important: The predicted target values have to be decoded
             * with double the precision of the target values passed to fit.
             *
             * @return Predicted values
             */
            Vec<Ciphertext> predict(const Mat<Integer> &X) const;
            Vec<Ciphertext> predict(const Mat<Integer> &X, const Mat<Integer> &X_t) const;

            /**
             * @return Weights, available after fitting
             */
            const Vec<Ciphertext> &get_weights() const;

            /**
             * Helper function to construct the client callback
             * function
             */
            static const client_callback_t construct_client_callback(const PaillierBase &paillier);
        };

        /**
         * Linear regression with plaintext X (features), encrypted y (target)
         * and encrypted theta (weights).
         * This implementation first uses some preprocessing on client side, and
         * then solves the normal equation to compute the model.
         *
         * Fit and predict are also offered in a variant where you can pass
         * X_t directly to save some time, if you happen to have X transposed
         * around anyways.
         */
        class LinregPlainEncUsers {
        public:
            /**
             * Type for client callback.
             * Arguments:
             * - Ciphertext vector, dividends
             * - Plaintext, divisor
             *
             * Should return
             *   encrypt(decrypt(arg1) / (arg2));
             */
            typedef std::function< Vec<Ciphertext> (Vec<Ciphertext>, Integer)> client_callback_t;

        private:

            /**
             * Callback, implemented by client, which does division.
             * Has to be done on client side as decryption is necessary.
             */
            const client_callback_t client_callback;

            /**
             * Integerizer which will be used for X
             */
            const Vector::Integerizer inter;

            long n_features;

            /**
             * Weight vector
             */
            Vec<Ciphertext> theta;

        public:
            /**
             * Initialize a new linear regressor.
             *
             * @param client_callback Callback, implemented by client, which does division.
             *        Will get called only once during fit.
             * @param inter Integerizer which was used for y
             *        will be used for integerizing (X^T * X)^-1 * X^T
             */
            LinregPlainEncUsers(const client_callback_t &client_callback, const Vector::Integerizer &inter);

            /**
             * Train.
             * @param X features and is a n_feature x n_samples matrix. Must
             *        be normalized, but not yet integerized.
             * @param B encrypted values as generated by client_preprocess.
             */
            void fit(const Mat<float> &X, const Mat<Ciphertext> &B);
            void fit(const Mat<float> &X, const Mat<float> &X_t, const Mat<Ciphertext> &B);

            /**
             * Predict target values from feature matrix
             * @return Predicted values
             */
            Vec<Ciphertext> predict(const Mat<Integer> &X) const;
            Vec<Ciphertext> predict(const Mat<Integer> &X, const Mat<Integer> &X_t) const;

            /**
             * @return Weights, available after fitting
             */
            const Vec<Ciphertext> &get_weights() const;

            /**
             * Do the preprocessing on client side. This returns the b which can then be
             * passed to fit.
             *
             * Also offered in a variation where you can pass X_t directly to save some time.
             */
            static Mat<Ciphertext> client_preprocess(const Mat<float> &X, const Vec<float> &y, const Vector::Integerizer &inter, const PaillierBase &paillier);

            /**
             * Helper function to construct the client callback
             * function
             */
            static const client_callback_t construct_client_callback(const PaillierBase &paillier);
        };

        /**
         * Linear regression with encrypted X (features), encrypted y (target)
         * and plaintext model (weights/theta).
         * This implementation first uses some preprocessing on client side, and
         * then employs gradient descent.
         */
        class LinregEncEncUsers {
        public:
            /**
             * Type for client callback.
             * Arguments:
             * - PackedCiphertext vector, dividends
             * - Plaintext, divisor
             *
             * Should return
             *   encrypt(decrypt(arg1) / (arg2));
             */
            typedef std::function< Vec<Integer> (Vec<PackedCiphertext>)> client_callback_t;

        private:
            /**
             * Callback, implemented by client, which does decryption.
             */
            const client_callback_t client_callback;

            /**
             * Multiplier used during integerize
             */
            const Integer multiplier;

            /**
             * Paillier pubkey instance, necessary for packing
             */
            const PaillierFast paillier;

            /**
             * 1/alpha, where alpha is the learning rate
             */
            const Integer alpha_inv;

            /**
             * Maximum iterations for gradient descent
             */
            const size_t n_iter;

            /**
             * How many iterations we actually did
             */
            size_t n_iter_done = 0;

            long n_features;

            /**
             * Weight vector
             */
            Vec<Integer> theta;

            /**
             * Implements gradient descent, used by fit()
             * @param AA -sum of all Ai
             * @param bb sum of all bi
             * @return computed weight vector theta
             */
            Vec<Integer> gradient_descent(const Mat<Ciphertext> &AA, const Vec<Ciphertext> bb);

        public:
            /**
             * Initialize a new linear regressor.
             *
             * @param client_callback: Callback, implemented by client, which does decryption.
             *                         We promise to not misuse it for, err, decrypting the
             *                         actual data **lolol**.
             * @param multiplier which was used for integerizing
             * @param pubkey needed for packing
             * @param alpha_inv: 1/alpha, where alpha is learning rate
             * @param n_iter: maximum iterations for gradient descent (will stop earlier if converges)
             */
            LinregEncEncUsers(const client_callback_t &client_callback, const Integer &multiplier, const PublicKey &pubkey, const Integer &alpha_inv = (int)(1 / 0.1), const size_t n_iter = 1000);

            /**
             * Do the preprocessing step 1 on client side.
             * @param X features, normalized and integerized
             * @return A to pass to fit()
             */
            static Vec<Mat<Ciphertext>> client_preprocess_A(const Mat<Integer> &X, const PaillierBase &paillier);

            /**
             * Do the preprocessing step 2 on client side.
             * @param X features, normalized and integerized
             * @param y target, normalized and integerized
             * @return b to pass to fit()
             */
            static Mat<Ciphertext> client_preprocess_b(const Mat<Integer> &X, const Vec<Integer> &y, const PaillierBase &paillier);

            /**
             * Train.
             * @param A encrypted values as generated by client_preprocess_A()
             * @param b encrypted values as generated by client_preprocess_b()
             * @return number of gradient descent iterations. If smaller than n_iter,
             *         this means that it converged faster than n_iter.
             */
            size_t fit(const Vec<Mat<Ciphertext>> &A, const Mat<Ciphertext> &b);

            /**
             * Predict target values from feature matrix
             */
            Vec<Integer> predict(const Mat<Integer> &X) const;
            Vec<Integer> predict(const Mat<Integer> &X, const Mat<Integer> &X_t) const;
            Vec<Ciphertext> predict(const Mat<Ciphertext> &X) const;

            /**
             * Weights, available after fitting
             */
            const Vec<Integer> &get_weights() const;

            /**
             * Helper function to construct the client callback
             * function
             */
            static const client_callback_t construct_client_callback(const PaillierBase &paillier);
        };

        /**
         * Calculate square loss on predicted data
         */
        template<typename number>
        number cost(const Vec<number> &y_pred, const Vec<number> &y_real);
    }
}
