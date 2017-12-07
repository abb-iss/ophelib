#include "ophelib/vector.h"
#include "ophelib/integer.h"
#include "ophelib/paillier_base.h"
#include "ophelib/packing.h"
#include "ophelib/omp_wrap.h"

#include <fstream>
#include <ophelib/random.h>

namespace ophelib {
    namespace Vector {

        template<typename number>
        Mat<number> col_matrix(const Vec<number> &v) {
            Mat<number> ret;
            const long n = v.length();
            ret.SetDims(n, 1);
            for(long i = 0; i < n; i++) {
                ret[i][0] = v[i];
            }
            return ret;
        }

        template Mat<float> col_matrix(const Vec<float>&);
        template Mat<Integer> col_matrix(const Vec<Integer>&);
        template Mat<Ciphertext> col_matrix(const Vec<Ciphertext>&);

        template<typename number>
        Mat<number> row_matrix(const Vec<number> &v) {
            Mat<number> ret;
            ret.SetDims(1, v.length());
            ret[0] = v;
            return ret;
        }

        template Mat<float> row_matrix(const Vec<float>&);
        template Mat<Integer> row_matrix(const Vec<Integer>&);
        template Mat<Ciphertext> row_matrix(const Vec<Ciphertext>&);

        template<typename number>
        Vec<number> vector(const Mat<number> &m) {
            if(m.NumCols() != 1 && m.NumRows() != 1)
                dimension_mismatch();
            Vec<number> ret;
            if(m.NumCols() == 1) {
                ret.SetLength(m.NumRows());
                #pragma omp parallel for
                for(long i = 0; i < ret.length(); i++) {
                    ret[i] = m[i][0];
                }
            } else {
                ret.SetLength(m.NumCols());
                ret = m[0];
            }
            return ret;
        }

        template Vec<float> vector(const Mat<float>&);
        template Vec<Integer> vector(const Mat<Integer>&);
        template Vec<Ciphertext> vector(const Mat<Ciphertext>&);

        template<typename number>
        Mat<number> transpose(const Mat<number> &M) {
            Mat<number> ret;

            const long n = M.NumRows();
            const long m = M.NumCols();

            ret.SetDims(m, n);
            #pragma omp parallel for
            for (long i = 0; i < n; i++)
                for (long j = 0; j < m; j++)
                    ret[j][i] = M[i][j];

            return ret;
        }

        template Mat<float> transpose(const Mat<float>&);
        template Mat<Integer> transpose(const Mat<Integer>&);
        template Mat<Ciphertext> transpose(const Mat<Ciphertext>&);

        template<typename number>
        Vec<number> sum(const Mat<number> &m_, const int axis) {
            if(axis > 1)
                error_exit("invalid axis");
            const Mat<number> &m = (axis == 0) ? transpose(m_) : m_;
            const long n = m.NumRows();
            if(n == 0)
                error_exit("empty matrix!");
            Vec<number> ret;
            ret.SetLength(n);

            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = sum(m[i]);
            }
            return ret;
        }

        template Vec<float> sum(const Mat<float>&, const int axis);
        template Vec<Integer> sum(const Mat<Integer>&, const int axis);
        template Vec<Ciphertext> sum(const Mat<Ciphertext>&, const int axis);

        template<typename number>
        number sum(const Vec<number> &m) {
            const long n = m.length();
            if(n == 0)
                error_exit("empty vector!");
            number ret = m[0];

            // openmp reduce() does not work here,
            // because it does not know about Integer/mpz_class
            for(long i = 1; i < n; i++) {
                ret += m[i];
            }
            return ret;
        }

        template float sum(const Vec<float>&);
        template Integer sum(const Vec<Integer>&);
        template Ciphertext sum(const Vec<Ciphertext>&);

        template<typename number>
        Vec<number> max(const Mat<number> &m_, const int axis) {
            if(axis > 1)
                error_exit("invalid axis");
            const Mat<number> &m = (axis == 0) ? transpose(m_) : m_;
            const long n = m.NumRows();
            if(n == 0)
                error_exit("empty matrix!");
            Vec<number> ret;
            ret.SetLength(n);
            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = max(m[i]);
            }
            return ret;
        }

        template Vec<float> max(const Mat<float>&, const int axis);
        template Vec<Integer> max(const Mat<Integer>&, const int axis);

        template<typename number>
        number max(const Vec<number> &m) {
            const long n = m.length();
            if(n == 0)
                error_exit("empty vector!");
            number ret = m[0];
            // openmp reduce() does not work here,
            // because it does not know about Integer/mpz_class
            for(long i = 1; i < n; i++) {
                if(m[i] > ret)
                    ret = m[i];
            }
            return ret;
        }

        template float max(const Vec<float>&);
        template Integer max(const Vec<Integer>&);

        template<typename number>
        Vec<number> min(const Mat<number> &m_, const int axis) {
            if(axis > 1)
                error_exit("invalid axis");
            const Mat<number> &m = (axis == 0) ? transpose(m_) : m_;
            const long n = m.NumRows();
            if(n == 0)
                error_exit("empty matrix!");
            Vec<number> ret;
            ret.SetLength(n);
            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = min(m[i]);
            }
            return ret;
        }

        template Vec<float> min(const Mat<float>&, const int axis);
        template Vec<Integer> min(const Mat<Integer>&, const int axis);

        template<typename number>
        number min(const Vec<number> &m) {
            const long n = m.length();
            if(n == 0)
                error_exit("empty vector!");
            number ret = m[0];
            // openmp reduce() does not work here,
            // because it does not know about Integer/mpz_class
            for(long i = 1; i < n; i++) {
                if(m[i] < ret)
                    ret = m[i];
            }
            return ret;
        }

        template float min(const Vec<float>&);
        template Integer min(const Vec<Integer>&);

        template<typename number, typename scalar>
        Vec<number> operator*(const Vec<number>& a, const scalar& b) {
            Vec<number> ret;
            const long n = a.length();
            ret.SetLength(n);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] * b;
            }
            return ret;
        }

        template Vec<float> operator*(const Vec<float>& a, const float& b);
        template Vec<Integer> operator*(const Vec<Integer>& a, const Integer& b);
        template Vec<Ciphertext> operator*(const Vec<Ciphertext>& a, const Integer& b);

        template<typename number, typename scalar>
        Mat<number> operator*(const Mat<number>& a, const scalar& b) {
            const long n = a.NumRows();
            Mat<number> ret;
            ret.SetDims(a.NumRows(), a.NumCols());
            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] * b;
            }
            return ret;
        }

        template Mat<float> operator*(const Mat<float>& a, const float& b);
        template Mat<Integer> operator*(const Mat<Integer>& a, const Integer& b);
        template Mat<Ciphertext> operator*(const Mat<Ciphertext>& a, const Integer& b);

        template<typename number>
        Vec<number> operator/(const Vec<number>& a, const number& b) {
            Vec<number> ret;
            const long n = a.length();
            ret.SetLength(n);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] / b;
            }
            return ret;
        }

        template Vec<float> operator/(const Vec<float>& a, const float& b);
        template Vec<Integer> operator/(const Vec<Integer>& a, const Integer& b);

        template<typename number>
        Mat<number> operator/(const Mat<number>& a, const number& b) {
            const long n = a.NumRows();
            Mat<number> ret;
            ret.SetDims(a.NumRows(), a.NumCols());
            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] / b;
            }
            return ret;
        }

        template Mat<float> operator/(const Mat<float>& a, const float& b);
        template Mat<Integer> operator/(const Mat<Integer>& a, const Integer& b);

        template<typename number>
        Vec<number> operator+(const Vec<number>& a, const number& b) {
            Vec<number> ret;
            const long n = a.length();
            ret.SetLength(n);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] + b;
            }
            return ret;
        }

        template Vec<float> operator+(const Vec<float>& a, const float& b);
        template Vec<Integer> operator+(const Vec<Integer>& a, const Integer& b);
        template Vec<Ciphertext> operator+(const Vec<Ciphertext>& a, const Ciphertext& b);

        template<typename number>
        Mat<number> operator+(const Mat<number>& a, const number& b) {
            const long n = a.NumRows();
            Mat<number> ret;
            ret.SetDims(a.NumRows(), a.NumCols());
            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] + b;
            }
            return ret;
        }

        template Mat<float> operator+(const Mat<float>& a, const float& b);
        template Mat<Integer> operator+(const Mat<Integer>& a, const Integer& b);
        template Mat<Ciphertext> operator+(const Mat<Ciphertext>& a, const Ciphertext& b);

        template<typename number>
        Vec<number> operator-(const Vec<number>& a, const number& b) {
            Vec<number> ret;
            const long n = a.length();
            ret.SetLength(n);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] - b;
            }
            return ret;
        }

        template Vec<float> operator-(const Vec<float>& a, const float& b);
        template Vec<Integer> operator-(const Vec<Integer>& a, const Integer& b);
        template Vec<Ciphertext> operator-(const Vec<Ciphertext>& a, const Ciphertext& b);

        template<typename number>
        Mat<number> operator-(const Mat<number>& a, const number& b) {
            const long n = a.NumRows();
            Mat<number> ret;
            ret.SetDims(a.NumRows(), a.NumCols());
            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] - b;
            }
            return ret;
        }

        template Mat<float> operator-(const Mat<float>& a, const float& b);
        template Mat<Integer> operator-(const Mat<Integer>& a, const Integer& b);
        template Mat<Ciphertext> operator-(const Mat<Ciphertext>& a, const Ciphertext& b);

        template<typename number>
        Vec<number> operator-(const Vec<number>& a) {
            Vec<number> ret;
            const long n = a.length();
            ret.SetLength(n);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = -a[i];
            }
            return ret;
        }

        template Vec<float> operator-(const Vec<float>& a);
        template Vec<Integer> operator-(const Vec<Integer>& a);
        template Vec<Ciphertext> operator-(const Vec<Ciphertext>& a);

        template<typename number>
        Mat<number> operator-(const Mat<number>& a) {
            const long n = a.NumRows();
            Mat<number> ret;
            ret.SetDims(a.NumRows(), a.NumCols());
            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = -a[i];
            }
            return ret;
        }

        template Mat<float> operator-(const Mat<float>& a);
        template Mat<Integer> operator-(const Mat<Integer>& a);
        template Mat<Ciphertext> operator-(const Mat<Ciphertext>& a);

        template<typename number>
        Vec<number> operator+(const Vec<number>& a, const Vec<number>& b) {
            Vec<number> ret;
            const long n = a.length();
            ret.SetLength(n);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] + b[i];
            }
            return ret;
        }

        template Vec<float> operator+(const Vec<float>& a, const Vec<float>& b);
        template Vec<Integer> operator+(const Vec<Integer>& a, const Vec<Integer>& b);
        template Vec<Ciphertext> operator+(const Vec<Ciphertext>& a, const Vec<Ciphertext>& b);

        template<typename number>
        Mat<number> operator+(const Mat<number>& a, const Mat<number>& b) {
            const long n = a.NumRows();
            Mat<number> ret;
            ret.SetDims(a.NumRows(), a.NumCols());
            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] + b[i];
            }
            return ret;
        }

        template Mat<float> operator+(const Mat<float>& a, const Mat<float>& b);
        template Mat<Integer> operator+(const Mat<Integer>& a, const Mat<Integer>& b);
        template Mat<Ciphertext> operator+(const Mat<Ciphertext>& a, const Mat<Ciphertext>& b);

        template<typename number>
        Vec<number> operator-(const Vec<number>& a, const Vec<number>& b) {
            Vec<number> ret;
            const long n = a.length();
            ret.SetLength(n);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] - b[i];
            }
            return ret;
        }

        template Vec<float> operator-(const Vec<float>& a, const Vec<float>& b);
        template Vec<Integer> operator-(const Vec<Integer>& a, const Vec<Integer>& b);
        template Vec<Ciphertext> operator-(const Vec<Ciphertext>& a, const Vec<Ciphertext>& b);

        template<typename number>
        Mat<number> operator-(const Mat<number>& a, const Mat<number>& b) {
            const long n = a.NumRows();
            Mat<number> ret;
            ret.SetDims(a.NumRows(), a.NumCols());
            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = a[i] - b[i];
            }
            return ret;
        }

        template Mat<float> operator-(const Mat<float>& a, const Mat<float>& b);
        template Mat<Integer> operator-(const Mat<Integer>& a, const Mat<Integer>& b);
        template Mat<Ciphertext> operator-(const Mat<Ciphertext>& a, const Mat<Ciphertext>& b);

        template<typename number>
        bool operator==(const Mat<number>& a, const Mat<number>& b) {
            const long n = a.NumRows(), m = a.NumCols();

            if(n != b.NumRows() || m != b.NumCols())
                return false;

            for(long i = 0; i < n; i++)
                for(long j = 0; j < n; j++)
                    if(a[i][j] != b[i][j])
                        return false;

            return true;
        }

        template bool operator==(const Mat<float>& a, const Mat<float>& b);
        template bool operator==(const Mat<Integer>& a, const Mat<Integer>& b);
        template bool operator==(const Mat<Ciphertext>& a, const Mat<Ciphertext>& b);

        template<typename number>
        bool operator==(const Vec<number>& a, const Vec<number>& b) {
            const long n = a.length();

            if(n != b.length())
                return false;

            for(long i = 0; i < n; i++)
                if(a[i] != b[i])
                    return false;

            return true;
        }

        template bool operator==(const Vec<float>& a, const Vec<float>& b);
        template bool operator==(const Vec<Integer>& a, const Vec<Integer>& b);
        template bool operator==(const Vec<Ciphertext>& a, const Vec<Ciphertext>& b);
        template bool operator==(const Vec<PackedCiphertext>& a, const Vec<PackedCiphertext>& b);

        Normalizer::Normalizer() {
            intercept.SetLength(0);
            scale.SetLength(0);
        }

        void Normalizer::fit(const Mat<float> &X, const Mat<float> &X_t) {
            n_features = X.NumCols();
            const auto n_samples = X.NumRows();

            const auto mean = Vector::sum(X_t, 1) / (float)n_samples;
            const auto min_ = mean - min(X_t, 1);
            const auto max_ = max(X_t, 1) - mean;
            intercept = mean;
            scale.SetLength(n_features);

            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n_features; i++) {
                scale[i] = max_[i] > min_[i] ? max_[i] : min_[i];
                if(scale[i] == 0) scale[i] = 1;
            }
        }

        void Normalizer::fit(const Mat<float> &X) {
            fit(X, transpose(X));
        }

        Mat<float> Normalizer::transform(const Mat<float> &X, const Mat<float> &X_t) const {
            if(n_features == 0)
                error_exit("not fitted!");

            if(X.NumCols() != n_features || X_t.NumRows() != n_features)
                dimension_mismatch();

            Mat<float> ret;
            ret.SetDims(X_t.NumRows(), X_t.NumCols());

            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n_features; i++) {
                ret[i] = (X_t[i] - intercept[i]) / scale[i];
            }

            return transpose(ret);
        }

        Mat<float> Normalizer::transform(const Mat<float> &X) const {
            return transform(X, transpose(X));
        }

        Mat<float> Normalizer::fit_transform(const Mat<float> &X, const Mat<float> &X_t) {
            fit(X, X_t);
            return transform(X, X_t);
        }

        Mat<float> Normalizer::fit_transform(const Mat<float> &X) {
            return fit_transform(X, transpose(X));
        }

        Mat<float> Normalizer::inverse_transform(const Mat<float> &X, const Mat<float> &X_t) const {
            if(n_features == 0)
                error_exit("not fitted!");

            if(X.NumCols() != n_features || X_t.NumRows() != n_features)
                dimension_mismatch();

            Mat<float> ret;
            ret.SetDims(X_t.NumRows(), X_t.NumCols());

            for(long i = 0; i < n_features; i++) {
                ret[i] = X_t[i] * scale[i] + intercept[i];
            }

            return transpose(ret);
        }

        Mat<float> Normalizer::inverse_transform(const Mat<float> &X) const {
            return inverse_transform(X, transpose(X));
        }

        void Normalizer::fit(const Vec<float> &X) {
            fit(col_matrix(X));
        }

        Vec<float> Normalizer::transform(const Vec<float> &X) const {
            return vector(transform(col_matrix(X)));
        }

        Vec<float> Normalizer::fit_transform(const Vec<float> &X) {
            return vector(fit_transform(col_matrix(X)));
        }

        Vec<float> Normalizer::inverse_transform(const Vec<float> &X) const {
            return vector(inverse_transform(col_matrix(X)));
        }

        const Vec<float> &Normalizer::get_intercept() const {
            return intercept;
        }

        const Vec<float> &Normalizer::get_scale() const {
            return scale;
        }

        void Normalizer::set_intercept_scale(const Vec<float> &intercept_, const Vec<float> &scale_) {
            const long size = intercept_.length();
            if(size < 1 || size != scale_.length())
                dimension_mismatch();

            n_features = size;
            intercept = intercept_;
            scale = scale_;
        }

        void Normalizer::clear() {
            n_features = 0;
            intercept.SetLength(0);
            scale.SetLength(0);
        }

        Integerizer::Integerizer(const size_t n_bits_, const Integer &multiplier_)
                : factor((multiplier_ < 0) ? Integer(Integer(1) << (n_bits_ - 1)) : multiplier_) { }

        Integerizer Integerizer::double_precision() const {
            return Integerizer(0, factor * factor);
        }

        Integerizer Integerizer::triple_precision() const {
            return Integerizer(0, factor * factor * factor);
        }

        Mat<Integer> Integerizer::transform(const Mat<float> &M) const {
            Mat<Integer> ret;
            const long n = M.NumRows();
            const long m = M.NumCols();
            ret.SetDims(n, m);
            const RR factor_ = NTL::conv<RR>(factor);

            for (long i = 0; i < n; i++) {
                for (long j = 0; j < m; j++) {
                    float x = M[i][j];
                    if(x > 1) x = 1;
                    if(x < -1) x = -1;
                    ophelib::conv(ret[i][j], NTL::conv<RR>(x) * factor_);
                }
            }

            return ret;
        }

        Mat<float> Integerizer::inverse_transform(const Mat<Integer> &M) const {
            Mat<float> ret;
            const long n = M.NumRows();
            const long m = M.NumCols();
            ret.SetDims(n, m);
            const RR factor_ = NTL::conv<RR>(factor);

            for (long i = 0; i < n; i++) {
                for (long j = 0; j < m; j++) {
                    ret[i][j] = NTL::conv<float>(NTL::conv<RR>(M[i][j]) / factor_);
                }
            }

            return ret;
        }

        Vec<Integer> Integerizer::transform(const Vec<float> &V) const {
            return vector(transform(col_matrix(V)));
        }

        Vec<float> Integerizer::inverse_transform(const Vec<Integer> &V) const {
            return vector(inverse_transform(row_matrix(V)));
        }

        Integer Integerizer::get_factor() const {
            return factor;
        }

        template<typename number>
        Mat<number> dot(const Mat<number> &A, const Mat<number> &B) {
            const long n = A.NumRows(),
                       d = A.NumCols(),
                       m = B.NumCols();

            if(d != B.NumRows())
                dimension_mismatch();
            if(n == 0 || d == 0 || m == 0)
                error_exit("empty matrix");

            Mat<number> ret;
            ret.SetDims(n, m);

            #pragma omp parallel for
            for (long i = 0; i < n; i++) {
                for (long j = 0; j < m; j++) {
                    ret[i][j] = A[i][0] * B[0][j];
                    for(long k = 1; k < d; k++) {
                        ret[i][j] += A[i][k] * B[k][j];
                    }
                }
            }

            return ret;
        }

        template Mat<float> dot(const Mat<float> &A, const Mat<float> &B);
        template Mat<Integer> dot(const Mat<Integer> &A, const Mat<Integer> &B);

        template<typename number>
        Vec<number> dot(const Mat<number> &A, const Vec<number> &B) {
            return vector(dot(A, col_matrix(B)));
        }

        template Vec<float> dot(const Mat<float> &A, const Vec<float> &B);
        template Vec<Integer> dot(const Mat<Integer> &A, const Vec<Integer> &B);

        template<typename number>
        Vec<number> dot(const Vec<number> &A, const Mat<number> &B) {
            return vector(dot(row_matrix(A), B));
        }

        template Vec<float> dot(const Vec<float> &A, const Mat<float> &B);
        template Vec<Integer> dot(const Vec<Integer> &A, const Mat<Integer> &B);

        template<typename number>
        number dot(const Vec<number> &A, const Vec<number> &B) {
            const long n = A.length();
            if (n != B.length())
                dimension_mismatch();
            if (n == 0)
                error_exit("empty vector");

            number ret = A[0] * B[0];
            for(long i = 1; i < n; i++) {
                ret += A[i] * B[i];
            }

            return ret;
        }

        template float dot(const Vec<float> &A, const Vec<float> &B);
        template Integer dot(const Vec<Integer> &A, const Vec<Integer> &B);

        Vec<Ciphertext> dot(const Mat<Ciphertext> &A, const Vec<Integer> &B) {
            const long n = A.NumRows(),
                    d = A.NumCols();

            if(d != B.length())
                dimension_mismatch();
            if(n == 0 || d == 0)
                error_exit("empty matrix");

            Vec<Ciphertext> ret;
            ret.SetLength(n);

            omp_set_nested(0);
            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = dot(A[i], B);
            }

            return ret;
        }

        Vec<Ciphertext> dot(const Vec<Ciphertext> &A, const Mat<Integer> &B) {
            const long n = B.NumCols(),
                    d = B.NumRows();

            if(d != A.length())
                dimension_mismatch();
            if(n == 0 || d == 0)
                error_exit("empty matrix");

            Vec<Ciphertext> ret;
            ret.SetLength(n);

            #pragma omp parallel for
            for(long i = 0; i < n; i++) {
                ret[i] = A[0] * B[0][i];
                for(long j = 1; j < d; j++) {
                    ret[i] += A[j] * B[j][i];
                }
            }

            return ret;
        }

        Ciphertext dot(const Vec<Ciphertext> &A, const Vec<Integer> &B) {
            const long n = A.length();
            if (n != B.length())
                dimension_mismatch();
            if (n == 0)
                error_exit("empty vector");

            Ciphertext ret = A[0] * B[0];
            for(long i = 1; i < n; i++) {
                ret += A[i] * B[i];
            }

            return ret;
        }

        template<typename number>
        Mat<number> zeros(const size_t n, const size_t m) {
            Mat<number> ret;
            ret.SetDims(n, m);
            for(size_t i = 0; i < n; i++) {
                for(size_t j = 0; j < m; j++) {
                    ret[i][j] = 0;
                }
            }
            return ret;
        }

        template Mat<float> zeros(const size_t n, const size_t m);
        template Mat<Integer> zeros(const size_t n, const size_t m);

        template<typename number>
        Vec<number> zeros(const size_t n) {
            Vec<number> ret;
            ret.SetLength(n);
            for(size_t i = 0; i < n; i++) {
                ret[i] = 0;
            }
            return ret;
        }

        template Vec<float> zeros(const size_t n);
        template Vec<Integer> zeros(const size_t n);

        template<typename number>
        Mat<number> ones(const size_t n, const size_t m) {
            Mat<number> ret;
            ret.SetDims(n, m);
            for(size_t i = 0; i < n; i++) {
                for(size_t j = 0; j < m; j++) {
                    ret[i][j] = 1;
                }
            }
            return ret;
        }

        template Mat<float> ones(const size_t n, const size_t m);
        template Mat<Integer> ones(const size_t n, const size_t m);

        template<typename number>
        Vec<number> ones(const size_t n) {
            Vec<number> ret;
            ret.SetLength(n);
            for(size_t i = 0; i < n; i++) {
                ret[i] = 1;
            }
            return ret;
        }

        template Vec<float> ones(const size_t n);
        template Vec<Integer> ones(const size_t n);

        Mat<Integer> rand(const size_t n, const size_t m, const Integer &max) {
            Mat<Integer> ret;
            Random &rand = Random::instance();
            ret.SetDims(n, m);
            for(size_t i = 0; i < n; i++) {
                for(size_t j = 0; j < m; j++) {
                    ret[i][j] = rand.rand_int(max);
                }
            }
            return ret;
        }

        Vec<Integer> rand(const size_t n, const Integer &max) {
            Vec<Integer> ret;
            Random &rand = Random::instance();
            ret.SetLength(n);
            for(size_t i = 0; i < n; i++) {
                ret[i] = rand.rand_int(max);
            }
            return ret;
        }

        Mat<Integer> rand_bits(const size_t n, const size_t m, const size_t n_bits) {
            Mat<Integer> ret;
            Random &rand = Random::instance();
            ret.SetDims(n, m);
            for(size_t i = 0; i < n; i++) {
                for(size_t j = 0; j < m; j++) {
                    ret[i][j] = rand.rand_int_bits(n_bits);
                }
            }
            return ret;
        }

        Vec<Integer> rand_bits(const size_t n, const size_t n_bits) {
            Vec<Integer> ret;
            Random &rand = Random::instance();
            ret.SetLength(n);
            for(size_t i = 0; i < n; i++) {
                ret[i] = rand.rand_int_bits(n_bits);
            }
            return ret;
        }

        /**
         * Make n x m matrix, filled with random numbers
         * at most n_bits long. Also contains negative numbers.
         */
        Mat<Integer> rand_bits_neg(const size_t n, const size_t m, const size_t n_bits) {
            return rand_bits(n, m, n_bits + 1) - (Integer(1) << n_bits);
        }

        /**
         * Make vector of length n, filled with random
         * numbers at most n_bits long. Also contains negative numbers.
         */
        Vec<Integer> rand_bits_neg(const size_t n, const size_t n_bits) {
            return rand_bits(n, n_bits + 1) - (Integer(1) << n_bits);
        }

        Mat<Integer> rand_primes(const size_t n, const size_t m, const size_t n_bits) {
            Mat<Integer> ret;
            Random &rand = Random::instance();
            ret.SetDims(n, m);
            for(size_t i = 0; i < n; i++) {
                for(size_t j = 0; j < m; j++) {
                    ret[i][j] = rand.rand_prime(n_bits);
                }
            }
            return ret;
        }

        Vec<Integer> rand_primes(const size_t n, const size_t n_bits) {
            Vec<Integer> ret;
            Random &rand = Random::instance();
            ret.SetLength(n);
            for(size_t i = 0; i < n; i++) {
                ret[i] = rand.rand_prime(n_bits);
            }
            return ret;
        }

        template<typename number>
        Mat<number> id(const size_t n) {
            Mat<number> ret = zeros<number>(n, n);
            for(size_t i = 0; i < n; i++) {
                ret[i][i] = 1;
            }
            return ret;
        }

        template Mat<float> id(const size_t n);
        template Mat<Integer> id(const size_t n);

        template <typename number>
        Mat<number> matrix_string(std::string in) {
            std::stringstream ss(in);
            Mat<RR> s;
            ss >> s;
            return NTL::conv< Mat<number> >(s);
        }

        template Mat<float> matrix_string<float>(std::string in);
        template Mat<Integer> matrix_string<Integer>(std::string in);

        template <typename number>
        Vec<number> vec_string(std::string in) {
            std::stringstream ss(in);
            Vec<RR> s;
            ss >> s;
            return NTL::conv< Vec<number> >(s);
        }

        template Vec<float> vec_string<float>(std::string in);
        template Vec<Integer> vec_string<Integer>(std::string in);

        Vec<Integer> decrypt(const Vec<Ciphertext> &cipher, const PaillierBase &pai) {
            Vec<Integer> ret;
            ret.SetLength(cipher.length());
            #pragma omp parallel for
            for(long i = 0; i < cipher.length(); i++) {
                ret[i] = pai.decrypt(cipher[i]);
            }
            return ret;
        }

        Mat<Integer> decrypt(const Mat<Ciphertext> &cipher, const PaillierBase &pai) {
            Mat<Integer> ret;
            ret.SetDims(cipher.NumRows(), cipher.NumCols());
            #pragma omp parallel for
            for(long i = 0; i < cipher.NumRows(); i++) {
                for(long j = 0; j < cipher.NumCols(); j++) {
                    ret[i][j] = pai.decrypt(cipher[i][j]);
                }
            }
            return ret;
        }

        Vec<Ciphertext> encrypt(const Vec<Integer> &plain, const PaillierBase &pai) {
            Vec<Ciphertext> ret;
            ret.SetLength(plain.length());
            #pragma omp parallel for
            for(long i = 0; i < plain.length(); i++) {
                ret[i] = pai.encrypt(plain[i]);
            }
            return ret;
        }

        Mat<Ciphertext> encrypt(const Mat<Integer> &plain, const PaillierBase &pai) {
            Mat<Ciphertext> ret;
            ret.SetDims(plain.NumRows(), plain.NumCols());
            #pragma omp parallel for
            for(long i = 0; i < plain.NumRows(); i++) {
                for(long j = 0; j < plain.NumCols(); j++) {
                    ret[i][j] = pai.encrypt(plain[i][j]);
                }
            }
            return ret;
        }

        Mat<float> inv(const Mat<float> &A) {
            const long n = A.NumRows();
            if (A.NumCols() != n)
                math_error_exit("matrix not square!");

            if (n == 0)
                math_error_exit("empty matrix!");

            Mat<float> M;
            M.SetDims(n, 2*n);

            for(long i = 0; i < n; i++) {
                for(long j = 0; j < n; j++) {
                    M[i][j] = A[i][j];
                    M[i][n+j] = 0;
                }
                M[i][n+i] = 1;
            }

            float det = 1;

            for(long k = 0; k < n; k++) {
                long pos = -1;
                float maxval = 0;
                for (long i = k; i < n; i++) {
                    const float tmp = M[i][k] < 0 ? -M[i][k] : M[i][k];
                    if (tmp > maxval) {
                        pos = i;
                        maxval = tmp;
                    }
                }

                if(pos != -1) {
                    if (k != pos) {
                        swap(M[pos], M[k]);
                        det = -det;
                    }

                    det *= M[k][k];
                    const float tmp = - 1 / M[k][k];
                    for(long j = k+1; j < 2*n; j++)
                        M[k][j] *= tmp;

                    for(long i = k+1; i < n; i++) {
                        float       *x = M[i].elts() + (k+1);
                        const float *y = M[k].elts() + (k+1);

                        for(long j = k+1; j < 2*n; j++, x++, y++)
                            *x += *y * M[i][k];
                    }
                }
                else {
                    math_error_exit("non-invertible matrix");
                }
            }

            Mat <float> X;
            X.SetDims(n, n);
            for(long k = 0; k < n; k++) {
                for(long i = n-1; i >= 0; i--) {
                    float tmp = 0;
                    for(long j = i + 1; j < n; j++) {
                        tmp += X[j][k] * M[i][j];
                    }
                    X[i][k] = tmp - M[i][n+k];
                }
            }

            if (det == 0)
                math_error_exit("non-invertible matrix");

            return X;
        }

        void load_data(const std::string &filename, Mat<float> &X, Vec<float> &y) {
            std::fstream file(filename, std::fstream::in);

            if(!file.good())
                error_exit("cannot open file! - " + filename);

            Mat<RR> data;
            file >> data;

            const long n = data.NumRows(),
                    m = data.NumCols() - 1;

            X.SetDims(n, m);
            y.SetLength(n);

            for(long i = 0; i < n; i++) {
                for(long j = 0; j < m; j++) {
                    X[i][j] = NTL::conv< float >(data[i][j]);
                }
                y[i] = NTL::conv<float>(data[i][m]);
            }
        }

        void load_data_str(const std::string &data_in, Mat<float> &X, Vec<float> &y) {
            const auto data = matrix_string<float>(data_in);

            const long n = data.NumRows(),
                    m = data.NumCols() - 1;

            X.SetDims(n, m);
            y.SetLength(n);

            for(long i = 0; i < n; i++) {
                for(long j = 0; j < m; j++) {
                    X[i][j] = data[i][j];
                }
                y[i] = data[i][m];
            }
        }

        void load_data_str(const char *data, const size_t len, Mat<float> &X, Vec<float> &y) {
            const auto data_str = std::string(data);
            if(data_str.length() != len)
                error_exit("invalid string length!");
            return load_data_str(data_str, X, y);
        }
    }
}