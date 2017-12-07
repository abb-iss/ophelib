#pragma once

#include "ophelib/paillier_base.h"
#include "ophelib/ntl_conv.h"

#include <fstream>

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
using NTL::RR;

namespace ophelib {

    /**
     * Data matrices have `n_samples` rows and `n_features` columns.
     * Vectors are always assumed to be column vectors.
     *
     * At the moment, all operations are only offered with
     * copy semantics. Maybe in-place should be added for performance.
     *
     * All the scalar operations are only implemented with the scalar
     * on the right hand side and vec/mat left. The other way round does
     * not work, add your own wrapper ops if you need it.
     *
     * Also, all the scalar operations only work with scalars of the same
     * type. This means, to a `Mat<float>` you can only add a `float`. If you want
     * to add an `int`, you have to cast it explicitly: `Mat<float> + (float)x`
     *
     * @TODO
     * Most of the operations use OpenMP for parallelization. However,
     * implementation is very naive and does not bring much advantage in many
     * use cases (1.2x-2x mostly on 4 cores). To make it really useful, it would have
     * to analyze the matrix/vector dimensions and then do the most efficient thing:
     * - parallelize along the longest axis (e.g. along the 1000elm-axis in a 1000x20
     *   dimensional matrix). However, it also has to be paid attention to keep
     *   data locality (as NTL Matrices use a Vec for each row). Maybe transposing
     *   before and after makes sense in some cases.
     * - The operations on encrypted data more often make sense to parallelize,
     *   as they are more expensive. At the moment, operations on all types
     *   (float, Integer, Ciphertext) use the same code through templates.
     * - Parallelize aggregate functions on vectors using recursive splitting
     *   according to the number of threads. At the moment, these operations
     *   are only parallelized for matrices row- or column-wise.
     * - Most of the matrix functions call the vector function for each row.
     *   Most parallelization now is done only in the matrix functions because
     *   if offers better benefits for all use cases. Fastest would be of course,
     *   if the vector functions parallelized their loop, and the matrix functions
     *   inlined the vector functions and parallelization only on the outer loop.
     * - In conclusion, e.g. build logic which says "if vector is of size
     *   at least 20, and data type is ciphertext, then use 2 threads for
     *   scalar multiplication"
     *
     * Similar to NTL, we could add a configuration wizard which figures out
     * the optimal settings (e.g. when to use decrypt_fast() and
     * when to use normal decrypt()), and puts them into a header file.
     */
    namespace Vector {

        /**
         * Make a col matrix (n x 1) from a vector
         */
        template<typename number>
        Mat<number> col_matrix(const Vec<number> &v);

        /**
         * Make a row matrix (1 x n) from a vector
         */
        template<typename number>
        Mat<number> row_matrix(const Vec<number> &v);

        /**
         * Make a vector from a (n x 1) or (1 x n) matrix
         */
        template<typename number>
        Vec<number> vector(const Mat<number> &m);

        /**
         * Transpose a matrix
         */
        template<typename number>
        Mat<number> transpose(const Mat<number> &m);

        /**
         * Calculate matrix sum
         * @param axis along which axis to operate. 0 -> col-wise, 1 -> row-wise
         */
        template<typename number>
        Vec<number> sum(const Mat<number> &m, const int axis = 0);

        /**
         * Calculate vector sum
         */
        template<typename number>
        number sum(const Vec<number> &m);

        /**
         * Calculate matrix max
         * @param axis along which axis to operate. 0 -> col-wise, 1 -> row-wise
         */
        template<typename number>
        Vec<number> max(const Mat<number> &m, const int axis = 0);

        /**
         * Calculate vector max
         */
        template<typename number>
        number max(const Vec<number> &m);

        /**
         * Calculate matrix min
         * @param axis along which axis to operate. 0 -> col-wise, 1 -> row-wise
         */
        template<typename number>
        Vec<number> min(const Mat<number> &m, const int axis = 0);

        /**
         * Calculate vector min
         */
        template<typename number>
        number min(const Vec<number> &m);

        /**
         * Scalar multiplication
         */
        template<typename number, typename scalar>
        Vec<number> operator*(const Vec<number>& a, const scalar& b);

        /**
         * Scalar multiplication
         */
        template<typename number, typename scalar>
        Mat<number> operator*(const Mat<number>& a, const scalar& b);

        /**
         * Scalar division
         */
        template<typename number>
        Vec<number> operator/(const Vec<number>& a, const number& b);

        /**
         * Scalar division
         */
        template<typename number>
        Mat<number> operator/(const Mat<number>& a, const number& b);

        /**
         * Scalar addition
         */
        template<typename number>
        Vec<number> operator+(const Vec<number>& a, const number& b);

        /**
         * Scalar addition
         */
        template<typename number>
        Mat<number> operator+(const Mat<number>& a, const number& b);

        /**
         * Scalar subtraction
         */
        template<typename number>
        Vec<number> operator-(const Vec<number>& a, const number& b);

        /**
         * Scalar subtraction
         */
        template<typename number>
        Mat<number> operator-(const Mat<number>& a, const number& b);

        /**
         * Unary minus for vector
         */
        template<typename number>
        Vec<number> operator-(const Vec<number>& a);

        /**
         * Unary minus for matrix
         */
        template<typename number>
        Mat<number> operator-(const Mat<number>& a);

        /**
         * Vector addition
         */
        template<typename number>
        Vec<number> operator+(const Vec<number>& a, const Vec<number>& b);

        /**
         * Matrix addition
         */
        template<typename number>
        Mat<number> operator+(const Mat<number>& a, const Mat<number>& b);

        /**
         * Vector subtraction
         */
        template<typename number>
        Vec<number> operator-(const Vec<number>& a, const Vec<number>& b);

        /**
         * Matrix subtraction
         */
        template<typename number>
        Mat<number> operator-(const Mat<number>& a, const Mat<number>& b);

        template<typename number>
        bool operator==(const Mat<number>& a, const Mat<number>& b);

        template<typename number>
        bool operator==(const Vec<number>& a, const Vec<number>& b);

        /**
         * Normalizes elements of a matrix or vector so that their mean is 0 and they
         * are in range [-1, 1] (inclusive).
         * If a matrix is given, normalization is done column-wise. That is, the mean
         * of every column will be 0.
         *
         * All functions are also offered in a variant where you can pass
         * X_t directly for optimization, if you happen to have X transposed
         * around anyways.
         */
        class Normalizer {
            long n_features;

            Vec<float> intercept;
            Vec<float> scale;
        public:
            Normalizer();

            /**
             * Initialize mean and scale for later transformation
             */
            void fit(const Mat<float> &X);
            void fit(const Mat<float> &X, const Mat<float> &X_t);

            /**
             * Normalize a matrix
             */
            Mat<float> transform(const Mat<float> &X) const;
            Mat<float> transform(const Mat<float> &X, const Mat<float> &X_t) const;

            /**
             * Do initialization and transformation in one step
             */
            Mat<float> fit_transform(const Mat<float> &X);
            Mat<float> fit_transform(const Mat<float> &X, const Mat<float> &X_t);

            /**
             * De-normalize
             */
            Mat<float> inverse_transform(const Mat<float> &X) const;
            Mat<float> inverse_transform(const Mat<float> &X, const Mat<float> &X_t) const;

            /**
             * Initialize mean and scale for later transformation
             */
            void fit(const Vec<float> &X);

            /**
             * Normalize a vector (= Matrix with one column)
             */
            Vec<float> transform(const Vec<float> &X) const;

            /**
             * Do initialization and transformation in one step for vector (= Matrix with one column)
             */
            Vec<float> fit_transform(const Vec<float> &X);

            /**
             * De-normalize, for vector (= Matrix with one column)
             */
            Vec<float> inverse_transform(const Vec<float> &X) const;

            const Vec<float> &get_intercept() const;
            const Vec<float> &get_scale() const;
            void set_intercept_scale(const Vec<float> &intercept, const Vec<float> &scale);
            void clear();
        };

        /**
         * Convert given data into signed integers using a precision of n_bits.
         * Input are float values in the interval [-1, 1]. Values outside this
         * interval will be clipped.
         * You should use Normalizer before this transformation, otherwise
         * everything will be cropped!
         */
        class Integerizer {
            const Integer factor;

        public:
            /**
             * There are two ways to initialize an Integerizer:
             * - pass n_bits, and a negative multiplier (default). Then, the multiplier
             *   will automatically be set correctly to reach the desired precision.
             * - pass a multiplier > 0. Then, n_bits will be ignored.
             *
             * This is not the nicest interface, but an overloaded constructor with only
             * one argument (size_t in one case and Integer in the other) would be much
             * more prone to subtle bugs.
             */
            Integerizer(const size_t n_bits, const Integer &multiplier = -1);

            /**
             * Initialize an integerizer with double the precision as the one
             * you are calling this method on.
             */
            Integerizer double_precision() const;

            /**
             * Initialize an integerizer with triple the precision as the one
             * you are calling this method on.
             */
            Integerizer triple_precision() const;

            Mat<Integer> transform(const Mat<float> &M) const;
            Mat<float> inverse_transform(const Mat<Integer> &M) const;

            Vec<Integer> transform(const Vec<float> &V) const;
            Vec<float> inverse_transform(const Vec<Integer> &V) const;

            Integer get_factor() const;
        };

        /**
         * Matrix product
         * @param A is n x d
         * @param B is d x m
         * @return n x m
         */
        template<typename number>
        Mat<number> dot(const Mat<number> &A, const Mat<number> &B);

        /**
         * Product of matrix with column vector
         * @param A is n x d
         * @param B is d x 1
         * @return n x 1
         */
        template<typename number>
        Vec<number> dot(const Mat<number> &A, const Vec<number> &B);

        /**
         * Product of row vector with matrix
         * @param A is 1 x d
         * @param B is d x n
         * @return 1 x n
         */
        template<typename number>
        Vec<number> dot(const Vec<number> &A, const Mat<number> &B);

        /**
         * Vector product
         */
        template<typename number>
        number dot(const Vec<number> &A, const Vec<number> &B);

        /**
         * Product of matrix with column vector
         * @param A is n x d, Ciphertext
         * @param B is d x 1, Plaintext
         * @return n x 1
         */
        Vec<Ciphertext> dot(const Mat<Ciphertext> &A, const Vec<Integer> &B);

        /**
         * Product of row vector with matrix
         * @param A is 1 x d, Ciphertext
         * @param B is d x n, Plaintext
         * @return 1 x n
         */
        Vec<Ciphertext> dot(const Vec<Ciphertext> &A, const Mat<Integer> &B);

        /**
         * Vector product.
         * @param A Ciphertext
         * @param B Plaintext
         */
        Ciphertext dot(const Vec<Ciphertext> &A, const Vec<Integer> &B);

        /**
         * Make n x m matrix, filled with zeros
         */
        template<typename number>
        Mat<number> zeros(const size_t n, const size_t m);

        /**
         * Make vector of length n, filled with zeros
         */
        template<typename number>
        Vec<number> zeros(const size_t n);

        /**
         * Make n x m matrix, filled with ones
         */
        template<typename number>
        Mat<number> ones(const size_t n, const size_t m);

        /**
         * Make vector of length n, filled with ones
         */
        template<typename number>
        Vec<number> ones(const size_t n);

        /**
         * Make n x m matrix, filled with random numbers <= max
         */
        Mat<Integer> rand(const size_t n, const size_t m, const Integer &max);

        /**
         * Make vector of length n, filled with random numbers <= max
         */
        Vec<Integer> rand(const size_t n, const Integer &max);

        /**
         * Make n x m matrix, filled with random numbers
         * at most n_bits long.
         */
        Mat<Integer> rand_bits(const size_t n, const size_t m, const size_t n_bits);

        /**
         * Make vector of length n, filled with random
         * numbers at most n_bits long.
         */
        Vec<Integer> rand_bits(const size_t n, const size_t n_bits);

        /**
         * Make n x m matrix, filled with random numbers
         * at most n_bits long. Also contains negative numbers.
         */
        Mat<Integer> rand_bits_neg(const size_t n, const size_t m, const size_t n_bits);

        /**
         * Make vector of length n, filled with random
         * numbers at most n_bits long. Also contains negative numbers.
         */
        Vec<Integer> rand_bits_neg(const size_t n, const size_t n_bits);

        /**
         * Make n x m matrix, filled with random primes
         * which are n_bits long.
         */
        Mat<Integer> rand_primes(const size_t n, const size_t m, const size_t n_bits);

        /**
         * Make vector of length n, filled with random primes
         * which are n_bits long.
         */
        Vec<Integer> rand_primes(const size_t n, const size_t n_bits);

        /**
         * Make n x n id matrix (diagonally 1)
         */
        template<typename number>
        Mat<number> id(const size_t n);

        /**
         * Matrix from string.
         * String format: "[[1 2 3] [4 5 6]]" (can include newlines)
         */
        template <typename number>
        Mat<number> matrix_string(std::string in);

        /**
         * Vector from string.
         * String format: "[1 2 3]" (can include newlines)
         */
        template <typename number>
        Vec<number> vec_string(std::string in);

        /**
         * Decrypt each ciphertext in a vector
         */
        Vec<Integer> decrypt(const Vec<Ciphertext> &cipher, const PaillierBase &pai);

        /**
         * Decrypt each ciphertext in a matrix
         */
        Mat<Integer> decrypt(const Mat<Ciphertext> &cipher, const PaillierBase &pai);

        /**
         * Encrypt each plaintext in a vector
         */
        Vec<Ciphertext> encrypt(const Vec<Integer> &plain, const PaillierBase &pai);

        /**
         * Encrypt each plaintext in a matrix
         */
        Mat<Ciphertext> encrypt(const Mat<Integer> &plain, const PaillierBase &pai);

        /**
         * Invert a matrix
         */
        Mat<float> inv(const Mat<float> &M);

        /**
         * Read data from file. The last column of the matrix in file
         * is assumed to be the target values. Everything but the last column
         * will thus be stored in X, and the last column in y.
         */
        void load_data(const std::string &filename, Mat<float> &X, Vec<float> &y);

        /**
         * Read data from string. The last column of the matrix in file
         * is assumed to be the target values. Everything but the last column
         * will thus be stored in X, and the last column in y.
         */
        void load_data_str(const std::string &data, Mat<float> &X, Vec<float> &y);

        /**
         * Read data from string. The last column of the matrix in file
         * is assumed to be the target values. Everything but the last column
         * will thus be stored in X, and the last column in y.
         * @param data pointer to string data
         * @param len of string data
         */
        void load_data_str(const char *data, const size_t len, Mat<float> &X, Vec<float> &y);
    }
}
