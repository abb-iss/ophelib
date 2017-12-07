#include "ophelib/ml.h"
#include "ophelib/packing.h"

namespace ophelib {
    namespace ML {

        LinregPlain::LinregPlain(const Integer &multiplier_, const Integer &alpha_inv_, const size_t n_iter_)
                : multiplier(multiplier_),
                  alpha_inv(alpha_inv_),
                  n_iter(n_iter_),
                  n_features(0) { }

        bool LinregPlain::grad_desc_step(const Mat<Integer> &X, const Vec<Integer> &y) {
            using namespace Vector;
            const auto n = Integer(X.NumRows());

            const auto hypothesis = dot(X, theta) / multiplier;
            const auto loss = hypothesis - y;
            const auto grad = dot(transpose(X), loss) / n / multiplier;

            #ifdef DEBUG
            const auto cost = dot(loss, loss) / n;
            std::cerr << "cost = "<< cost.to_string_() << std::endl;
            #endif

            if(dot(grad, grad) == 0)
                return true;

            theta = theta - grad / alpha_inv;
            return false;
        }

        size_t LinregPlain::fit(const Mat<Integer> &X, const Vec<Integer> &y) {
            const long n = X.NumRows(), m = X.NumCols();
            if(n != y.length())
                dimension_mismatch();
            if(n < 1)
                error_exit("no features!");
            if(m < 1)
                error_exit("no samples!");
            n_features = m;

            // initialize weights
            theta = Vector::zeros<Integer>((size_t)n_features);

            for(size_t i = 0; i < n_iter; i++)
                if(grad_desc_step(X, y))
                    return i;

            return n_iter;
        }

        Vec<Integer> LinregPlain::predict(const Mat<Integer> &X) const {
            if(n_features == 0)
                error_exit("not fitted!");
            if(X.NumCols() != n_features)
                dimension_mismatch();

            using Vector::operator/;

            return Vector::dot(X, theta) /  multiplier;
        }

        const Vec<Integer> &LinregPlain::get_weights() const {
            return theta;
        }

        LinregPlainEnc::client_callback_cls_t::~client_callback_cls_t() { }

        LinregPlainEnc::client_callback_cls_t_impl::client_callback_cls_t_impl(const client_callback_fn_t &client_callback_):
                client_callback(client_callback_) { }

        Vec<Ciphertext> LinregPlainEnc::client_callback_cls_t_impl::call(const Vec<PackedCiphertext> &error, const Integer &divisor) const {
            return client_callback(error, divisor);
        }

        LinregPlainEnc::LinregPlainEnc(const client_callback_fn_t &client_callback_, const Integer &multiplier_, const PublicKey &pubkey_, const Integer &alpha_inv_, const size_t n_iter_)
                : client_callback(new LinregPlainEnc::client_callback_cls_t_impl(client_callback_)),
                  destroy_callback_wrapper(true),
                  multiplier(multiplier_),
                  paillier(pubkey_),
                  alpha_inv(alpha_inv_),
                  n_iter(n_iter_),
                  n_features(0) { }

        LinregPlainEnc::LinregPlainEnc(const client_callback_cls_t *client_callback_, const Integer &multiplier_, const PublicKey &pubkey_, const Integer &alpha_inv_, const size_t n_iter_)
                : client_callback(client_callback_),
                  destroy_callback_wrapper(false),
                  multiplier(multiplier_),
                  paillier(pubkey_),
                  alpha_inv(alpha_inv_),
                  n_iter(n_iter_),
                  n_features(0) { }

        LinregPlainEnc::~LinregPlainEnc() {
            if(destroy_callback_wrapper)
                delete client_callback;
        }

        size_t LinregPlainEnc::fit(const Mat<Integer> &X, const Vec<Ciphertext> &y) {
            return fit(X, Vector::transpose(X), y);
        }

        size_t LinregPlainEnc::fit(const Mat<Integer> &X, const Mat<Integer> &X_t, const Vec<Ciphertext> &y) {
            const long n = X.NumRows(), m = X.NumCols();
            if(n != y.length())
                dimension_mismatch();
            if(n < 1)
                error_exit("no features!");
            if(m < 1)
                error_exit("no samples!");
            n_features = m;

            using namespace Vector;

            const Mat<Integer> A = dot(X_t, X);
            const Vec<Ciphertext> b = dot(y, -X);

            // initialize weights with cipher zero
            theta.SetLength(n_features);
            for(long i = 0 ; i < m; i++) {
                theta[i] = Ciphertext(1, y[0].n2_shared);
            }

            const auto div = alpha_inv * Integer(n) * multiplier * multiplier;

            for(size_t i = 0; i < n_iter; i++) {
                const Vec<Ciphertext> errors = dot(theta, A) + b;
                const auto errors_packed = Vector::pack_ciphertexts_vec(
                        errors,
                        div.size_bits() + multiplier.size_bits(),
                        paillier);
                const auto errors_div = client_callback->call(errors_packed, div);
                using Vector::operator+;
                theta = theta + errors_div;
            }
            return n_iter;
        }

        Vec<Ciphertext> LinregPlainEnc::predict(const Mat<Integer> &X) const {
            return predict(X, Vector::transpose(X));
        }

        Vec<Ciphertext> LinregPlainEnc::predict(const Mat<Integer> &X, const Mat<Integer> &X_t) const {
            if(n_features == 0)
                error_exit("not fitted!");
            if(X.NumCols() != n_features)
                dimension_mismatch();

            return Vector::dot(theta, X_t);
        }

        const Vec<Ciphertext> &LinregPlainEnc::get_weights() const {
            return theta;
        }

        const LinregPlainEnc::client_callback_fn_t LinregPlainEnc::construct_client_callback(const PaillierBase &paillier) {
            return [&paillier](const Vec<PackedCiphertext> &error, const Integer &divisor) {
                using Vector::operator/;
                return Vector::encrypt(Vector::decrypt_pack(error, paillier) / -divisor, paillier);
            };
        }

        LinregPlainEncEqn::LinregPlainEncEqn(const client_callback_t &client_callback_, const Vector::Integerizer &inter_)
                : client_callback(client_callback_),
                  inter(inter_),
                  n_features(0) { }

        void LinregPlainEncEqn::fit(const Mat<float> &X, const Vec<Ciphertext> &y) {
            return fit(X, Vector::transpose(X), y);
        }

        void LinregPlainEncEqn::fit(const Mat<float> &X, const Mat<float> &X_t, const Vec<Ciphertext> &y) {
            const long n = X.NumRows(), m = X.NumCols();
            if(n != y.length())
                dimension_mismatch();
            if(n < 1)
                error_exit("no features!");
            if(m < 1)
                error_exit("no samples!");
            n_features = m;

            const auto A = Vector::dot(Vector::inv(Vector::dot(X_t, X)), X_t);
            const auto A_i = inter.transform(A);

            theta = Vector::dot(y, Vector::transpose(A_i));
            theta = client_callback(theta, inter.get_factor());
        }

        Vec<Ciphertext> LinregPlainEncEqn::predict(const Mat<Integer> &X) const {
            return predict(X, Vector::transpose(X));
        }

        Vec<Ciphertext> LinregPlainEncEqn::predict(const Mat<Integer> &X, const Mat<Integer> &X_t) const {
            if(n_features == 0)
                error_exit("not fitted!");
            if(X.NumCols() != n_features)
                dimension_mismatch();

            return Vector::dot(theta, X_t);
        }

        const Vec<Ciphertext> &LinregPlainEncEqn::get_weights() const {
            return theta;
        }

        const LinregPlainEncEqn::client_callback_t LinregPlainEncEqn::construct_client_callback(const PaillierBase &paillier) {
            return [&paillier](const Vec<Ciphertext> &error, const Integer &divisor) {
                using Vector::operator/;
                return Vector::encrypt(Vector::decrypt(error, paillier) / divisor, paillier);
            };
        }

        LinregPlainEncUsers::LinregPlainEncUsers(const client_callback_t &client_callback_, const Vector::Integerizer &inter_)
                : client_callback(client_callback_),
                  inter(inter_),
                  n_features(0) { }

        Mat<Ciphertext> LinregPlainEncUsers::client_preprocess(const Mat<float> &X, const Vec<float> &y, const Vector::Integerizer &inter, const PaillierBase &paillier) {
            const long n = X.NumRows(),
                       m = X.NumCols();

            Mat<float> B;
            B.SetDims(n, m);

            for(long i = 0; i < n; i++) {
                using Vector::operator*;
                B[i] = X[i] * y[i];
            }

            return Vector::encrypt(inter.transform(B), paillier);
        }

        void LinregPlainEncUsers::fit(const Mat<float> &X, const Mat<Ciphertext> &B) {
            return fit(X, Vector::transpose(X), B);
        }

        void LinregPlainEncUsers::fit(const Mat<float> &X, const Mat<float> &X_t, const Mat<Ciphertext> &B) {
            const long n = X.NumRows(),
                       m = X.NumCols();
            if(m != B.NumCols() || n != B.NumRows())
                dimension_mismatch();
            if(n < 1)
                error_exit("no features!");
            if(m < 1)
                error_exit("no samples!");
            n_features = m;

            const auto A = Vector::inv(Vector::dot(X_t, X));
            const auto A_i = inter.transform(A);
            auto b = Vector::sum(B);

            theta = Vector::dot(b, Vector::transpose(A_i));
            theta = client_callback(theta, inter.get_factor());
        }

        Vec<Ciphertext> LinregPlainEncUsers::predict(const Mat<Integer> &X) const {
            return predict(X, Vector::transpose(X));
        }

        Vec<Ciphertext> LinregPlainEncUsers::predict(const Mat<Integer> &X, const Mat<Integer> &X_t) const {
            if(n_features == 0)
                error_exit("not fitted!");
            if(X.NumCols() != n_features)
                dimension_mismatch();

            return Vector::dot(theta, X_t);
        }

        const Vec<Ciphertext> &LinregPlainEncUsers::get_weights() const {
            return theta;
        }

        const LinregPlainEncUsers::client_callback_t LinregPlainEncUsers::construct_client_callback(const PaillierBase &paillier) {
            return [&paillier](const Vec<Ciphertext> &error, const Integer &divisor) {
                using Vector::operator/;
                return Vector::encrypt(Vector::decrypt(error, paillier) / divisor, paillier);
            };
        }

        LinregEncEncUsers::LinregEncEncUsers(const client_callback_t &client_callback_, const Integer &multiplier_, const PublicKey &pubkey_, const Integer &alpha_inv_, const size_t n_iter_)
                : client_callback(client_callback_),
                  multiplier(multiplier_),
                  paillier(pubkey_),
                  alpha_inv(alpha_inv_),
                  n_iter(n_iter_),
                  n_features(0) { }

        Vec<Mat<Ciphertext>> LinregEncEncUsers::client_preprocess_A(const Mat<Integer> &X, const PaillierBase &paillier) {
            const long n = X.NumRows();

            Vec<Mat<Ciphertext>> A;
            A.SetLength(n);
            for(long i = 0; i < n; i++) {
                const auto sample = X[i];
                const auto Ai = Vector::dot(Vector::col_matrix(sample),
                                            Vector::row_matrix(sample));
                A[i] = Vector::encrypt(Ai, paillier);
            }
            return A;
        }

        Mat<Ciphertext> LinregEncEncUsers::client_preprocess_b(const Mat<Integer> &X, const Vec<Integer> &y, const PaillierBase &paillier) {
            const long n = X.NumRows();
            if(n != y.length())
                dimension_mismatch();

            using Vector::operator*;

            Mat<Ciphertext> b;
            b.SetDims(n, X.NumCols());
            for(long i = 0; i < n; i++) {
                const auto sample = X[i];
                const auto bi = sample * y[i];
                b[i] = Vector::encrypt(bi, paillier);
            }
            return b;
        }

        Vec<Integer> LinregEncEncUsers::gradient_descent(const Mat<Ciphertext> &AA, const Vec<Ciphertext> bb) {
            using Vector::operator-;
            using Vector::operator/;
            auto theta = Vector::zeros<Integer>((size_t)n_features);

            const auto divisor = alpha_inv * multiplier * multiplier * Integer(n_features);

            for(size_t k = 0; k < n_iter; k++, n_iter_done = k) {
                const auto tmp = bb - Vector::dot(AA, theta);
                const auto n_bits = divisor.size_bits() + multiplier.size_bits() * 2;
                const auto packed = Vector::pack_ciphertexts_vec(tmp, n_bits, paillier);
                const auto loss = client_callback(packed) / divisor;

                if(Vector::dot(loss, loss) == 0)
                    break;
                theta = theta - loss;

                #ifdef DEBUG
                const auto cost = Vector::dot(loss, loss);
                std::cerr << "cost(" << k << ") = "<< cost.to_string_() << std::endl;
                #endif
            }

            return -theta;
        }

        size_t LinregEncEncUsers::fit(const Vec<Mat<Ciphertext>> &A, const Mat<Ciphertext> &b) {
            using Vector::operator+;
            using Vector::operator-;

            if(A.length() < 1)
                error_exit("A empty!");
            if(b.NumRows() < 1)
                error_exit("A empty!");
            const long n = A[0].NumRows(),
                       m = A.length();
            if(n < 1)
                error_exit("no features!");
            if(m < 1)
                error_exit("no samples!");
            n_features = n;

            Mat<Ciphertext> AA = A[0];
            for(long i = 1; i < m; i++)
                AA = AA + A[i];

            Vec<Ciphertext> bb = Vector::sum(b);

            theta = gradient_descent(-AA, bb);
            return n_iter_done;
        }

        Vec<Integer> LinregEncEncUsers::predict(const Mat<Integer> &X) const {
            return predict(X, Vector::transpose(X));
        }

        Vec<Integer> LinregEncEncUsers::predict(const Mat<Integer> &X, const Mat<Integer> &X_t) const {
            if(n_features == 0)
                error_exit("not fitted!");
            if(X.NumCols() != n_features)
                dimension_mismatch();

            using Vector::operator/;
            const auto divisor = multiplier * multiplier;
            return Vector::dot(theta, X_t) / divisor;
        }

        Vec<Ciphertext> LinregEncEncUsers::predict(const Mat<Ciphertext> &X) const {
            if(n_features == 0)
                error_exit("not fitted!");
            if(X.NumCols() != n_features)
                dimension_mismatch();

            return Vector::dot(X, theta);
        }

        const LinregEncEncUsers::client_callback_t LinregEncEncUsers::construct_client_callback(const PaillierBase &paillier) {
            return [&paillier](const Vec<PackedCiphertext> &v) {
                return Vector::decrypt_pack(v, paillier);
            };
        }

        const Vec<Integer> &LinregEncEncUsers::get_weights() const {
            return theta;
        }

        template<typename number>
        number cost(const Vec<number> &y_pred, const Vec<number> &y_real) {
            using Vector::operator-;
            const auto loss = y_pred - y_real;
            return Vector::dot(loss, loss) / (number)loss.length();
        }

        template float cost(const Vec<float> &y_pred, const Vec<float> &y_real);
        template Integer cost(const Vec<Integer> &y_pred, const Vec<Integer> &y_real);
    }
}
