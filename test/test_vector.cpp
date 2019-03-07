#include "ophelib/vector.h"
#include "ophelib/paillier_fast.h"
#include "catch.hpp"
#include "ophelib/disable_exception_tests.h"

#include <string>
#include <fstream>
#include <streambuf>

using namespace std;
using namespace ophelib;

const int keysize = 1024;
const float eps = 0.0001;

TEST_CASE("Vector Ops") {
    const string data_file = "../test/fixtures/x17.txt";

    NTL::Mat<float> X_;
    NTL::Vec<float> y_;
    Vector::load_data(data_file, X_, y_);
    const NTL::Mat<float> X = X_;
    const NTL::Vec<float> y = y_;

    SECTION("load_data") {
        REQUIRE( y.length() == 81 );
        REQUIRE( X.NumRows() == 81 );
        REQUIRE( X.NumCols() == 3 );
    }

    SECTION("load_data_str") {
        ifstream t(data_file);
        string str;

        t.seekg(0, std::ios::end);
        str.reserve(t.tellg());
        t.seekg(0, std::ios::beg);

        str.assign((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());

        NTL::Mat<float> X2;
        NTL::Vec<float> y2;

        Vector::load_data_str(str, X2, y2);
        REQUIRE( y.length() == 81 );
        REQUIRE( X.NumRows() == 81 );
        REQUIRE( X.NumCols() == 3 );

        y2.SetLength(0);
        X2.SetDims(0, 0);
        Vector::load_data_str(str.c_str(), str.length(), X_, y_);
        REQUIRE( y.length() == 81 );
        REQUIRE( X.NumRows() == 81 );
        REQUIRE( X.NumCols() == 3 );
    }

    SECTION("row_matrix, col_matrix") {
        const auto a = Vector::row_matrix(y);
        const auto b = Vector::col_matrix(y);
        REQUIRE( a.NumCols() == y.length() );
        REQUIRE( a.NumRows() == 1 );
        REQUIRE( b.NumCols() == 1 );
        REQUIRE( b.NumRows() == y.length() );
        for(long i = 0; i < y.length(); i++) {
            REQUIRE( a[0][i] == y[i] );
            REQUIRE( b[i][0] == y[i] );
        }
    }

    SECTION("vector") {
        const auto a = Vector::vector(Vector::row_matrix(y));
        const auto b = Vector::vector(Vector::col_matrix(y));
        REQUIRE( a.length() == y.length() );
        REQUIRE( b.length() == y.length() );
        for(long i = 0; i < y.length(); i++) {
            REQUIRE( a[i] == y[i] );
            REQUIRE( b[i] == y[i] );
        }
        Mat<float> A;
        A.SetDims(2, 5);
        REQUIRE_THROWS_AS( Vector::vector(A), BaseException );
        A.SetDims(5, 2);
        REQUIRE_THROWS_AS( Vector::vector(A), BaseException );
    }

    SECTION("transpose") {
        const Mat<float> X_t = Vector::transpose(X);
        REQUIRE( X_t.NumCols() == X.NumRows() );
        REQUIRE( X_t.NumRows() == X.NumCols() );
        REQUIRE( X[7][2] == X_t[2][7] );
        REQUIRE( X == Vector::transpose(X_t) );
    }

    const string small_x_s = "[[1 2 3 4]\n"
                              "[5 6 6 5]\n"
                              "[4 3 2 1]]";
    const auto small_x = Vector::matrix_string<float>(small_x_s);
    REQUIRE( small_x.NumCols() == 4 );
    REQUIRE( small_x.NumRows() == 3 );

    SECTION("sum") {
        // matrix
        const auto sum1 = Vector::sum(small_x);
        const auto sum2 = Vector::sum(small_x, 1);
        REQUIRE( sum1.length() == 4 );
        REQUIRE( sum2.length() == 3 );
        REQUIRE( sum1 == Vector::vec_string<float>("[10 11 11 10]") );
        REQUIRE( sum1[0] == 10 );
        REQUIRE( sum1[1] == 11 );
        REQUIRE(  sum1[2] == 11 );
        REQUIRE(  sum1[3] == 10 );
        REQUIRE( sum2[0] == 10 );
        REQUIRE( sum2[1] == 22 );
        REQUIRE(  sum2[2] == 10 );

        // vector
        const Vec<float> a = small_x[1];
        REQUIRE(Vector::sum(a) == 22 );
    }

    SECTION("max") {
        // matrix
        const auto max1 = Vector::max(small_x);
        const auto max2 = Vector::max(small_x, 1);
        REQUIRE( max1.length() == 4 );
        REQUIRE( max2.length() == 3 );
        REQUIRE( max1[0] == 5 );
        REQUIRE( max1[1] == 6 );
        REQUIRE(  max1[2] == 6 );
        REQUIRE(  max1[3] == 5 );
        REQUIRE( max2[0] == 4 );
        REQUIRE( max2[1] == 6 );
        REQUIRE(  max2[2] == 4 );

        // vector
        const Vec<float> a = small_x[1];
        REQUIRE(Vector::max(a) == 6 );
    }

    SECTION("min") {
        // matrix
        const auto min1 = Vector::min(small_x);
        const auto min2 = Vector::min(small_x, 1);
        REQUIRE( min1.length() == 4 );
        REQUIRE( min2.length() == 3 );
        REQUIRE( min1[0] == 1 );
        REQUIRE( min1[1] == 2 );
        REQUIRE(  min1[2] == 2 );
        REQUIRE(  min1[3] == 1 );
        REQUIRE( min2[0] == 1 );
        REQUIRE( min2[1] == 5 );
        REQUIRE(  min2[2] == 1 );

        // vector
        const Vec<float> a = small_x[1];
        REQUIRE(Vector::min(a) == 5 );
    }

    SECTION("scalar operator*") {
        using Vector::operator*;
        const Vec<float> a = small_x[0];
        const Vec<float> id = a * (float)1;
        const Vec<float> z = a * (float)0;
        const Vec<float> five = a * (float)5;
        REQUIRE( id == a );
        REQUIRE( z == Vector::zeros<float>(small_x[0].length()) );
        REQUIRE( Vector::sum(five) == 5 * Vector::sum(a) );
        REQUIRE( five[2] == 3 * 5 );

        REQUIRE( Vector::sum(Vector::sum(small_x * (float)1)) == Vector::sum(Vector::sum(small_x)) );
        REQUIRE( Vector::sum(Vector::sum(small_x * (float)0)) == (float)0 );
        const Mat<float> Five = small_x * (float)5;
        REQUIRE( Vector::sum(Vector::sum(Five)) == Vector::sum(Vector::sum(small_x)) * (float)5 );
        REQUIRE( Five[0][2] == 3 * 5 );
    }

    SECTION("scalar operator/") {
        using Vector::operator/;
        const Vec<float> a = small_x[0];
        const Vec<float> id = a / (float)1;
        const Vec<float> five = a / (float)5;
        REQUIRE( id == a );
        REQUIRE( Vector::sum(five) == Vector::sum(a) / 5 );
        REQUIRE( five[2] == (float)3 / 5 );

        REQUIRE( Vector::sum(Vector::sum(small_x / (float)1)) == Vector::sum(Vector::sum(small_x)) );
        const Mat<float> Five = small_x / (float)5;
        REQUIRE( Vector::sum(Vector::sum(Five)) == Vector::sum(Vector::sum(small_x)) / (float)5 );
        REQUIRE( Five[0][2] == (float)3 / 5 );
    }

    SECTION("scalar operator+") {
        using Vector::operator+;
        const Vec<float> a = small_x[0];
        const Vec<float> id = a + (float)0;
        const Vec<float> five = a + (float)5;
        REQUIRE( id == a );
        REQUIRE( Vector::sum(five) == (4 * 5) + Vector::sum(a) );
        REQUIRE( five[2] == 3 + 5 );

        REQUIRE( Vector::sum(Vector::sum(small_x + (float)0)) == Vector::sum(Vector::sum(small_x)) );
        const Mat<float> Five = small_x + (float)5;
        REQUIRE( Vector::sum(Vector::sum(Five)) == Vector::sum(Vector::sum(small_x)) + (float)5 * 4 * 3 );
        REQUIRE( Five[0][2] == 3 + 5 );
    }

    SECTION("scalar operator-") {
        using Vector::operator-;
        const Vec<float> a = small_x[0];
        const Vec<float> id = a - (float)0;
        const Vec<float> five = a - (float)5;
        REQUIRE( id == a );
        REQUIRE( Vector::sum(five) == -(4 * 5) + Vector::sum(a) );
        REQUIRE( five[2] == 3 - 5 );

        REQUIRE( Vector::sum(Vector::sum(small_x - (float)0)) == Vector::sum(Vector::sum(small_x)) );
        const Mat<float> Five = small_x - (float)5;
        REQUIRE( Vector::sum(Vector::sum(Five)) == Vector::sum(Vector::sum(small_x)) - (float)5 * 4 * 3 );
        REQUIRE( Five[0][2] == 3 - 5 );
    }

    SECTION("unary operator-") {
        using Vector::operator-;
        using Vector::operator+;
        const Vec<float> a = small_x[0];
        REQUIRE( Vector::sum(-a) == -Vector::sum(a) );
        REQUIRE( -(-a) == a );
        REQUIRE( a + (-a) == a - a );

        REQUIRE( Vector::sum(-X) == -Vector::sum(X) );
        REQUIRE( -(-X) == X );
        REQUIRE( X + (-X) == X - X );
    }

    SECTION("vec/mat operator+") {
        using Vector::operator+;
        using Vector::operator*;
        const Vec<float> a = small_x[0];
        REQUIRE( (a + a) ==  a * (float)2 );
        REQUIRE( (a + a + a) ==  a * (float)3 );

        REQUIRE( (small_x + small_x) ==  small_x * (float)2 );
        REQUIRE( (small_x + small_x + small_x) ==  small_x * (float)3 );
    }

    SECTION("vec/mat operator-") {
        using Vector::operator+;
        using Vector::operator*;
        const Vec<float> a = small_x[0];
        REQUIRE( (a + a) ==  a * (float)2 );
        REQUIRE( (a + a + a) ==  a * (float)3 );

        REQUIRE( (small_x + small_x) ==  small_x * (float)2 );
        REQUIRE( (small_x + small_x + small_x) ==  small_x * (float)3 );
    }

    SECTION("operator==") {
        SECTION("matrix") {
            Mat<float> small_x_2 = small_x;
            REQUIRE( small_x == small_x_2 );
            small_x_2[1][1] = 10;
            REQUIRE_FALSE( small_x == small_x_2 );
            small_x_2 = small_x;
            REQUIRE( small_x == small_x_2 );
            small_x_2.SetDims(small_x_2.NumRows() + 1, small_x_2.NumCols());
            REQUIRE_FALSE( small_x == small_x_2 );
        }

        SECTION("vectors") {
            const Vec<float> vec = small_x[0];
            Vec<float> vec2 = small_x[0];
            REQUIRE( vec == vec2 );
            vec2[1] = 10;
            REQUIRE_FALSE( vec == vec2 );
            vec2 = vec;
            REQUIRE( vec == vec2 );
            vec2.SetLength(vec.length() + 1);
            REQUIRE_FALSE( vec == vec2 );
        }
    }

    SECTION("Normalizer") {
        Vector::Normalizer norm1, norm2;
        norm1.fit(X);
        const auto Xn = norm1.transform(X);

        SECTION("transform, fit_transform") {
            REQUIRE( norm2.fit_transform(X) == Xn );
            REQUIRE( norm2.fit_transform(X, Vector::transpose(X)) == Xn );
            REQUIRE( norm1.get_intercept() == norm2.get_intercept() );
            REQUIRE( norm1.get_scale() == norm2.get_scale() );
            norm2.clear();
            REQUIRE( norm2.get_intercept().length() == 0 );
            norm2.set_intercept_scale(norm1.get_intercept(), norm1.get_scale());
            REQUIRE( Xn == norm2.transform(X) );
        }

        SECTION("properties of transformed data") {
            auto sum = Vector::sum(Xn);
            auto max = Vector::max(Xn);
            auto min = Vector::min(Xn);

            // mean of the transformed data should be zero
            for(auto m: sum) {
                REQUIRE( m == Approx(0).epsilon(eps) );
            }

            // all values in [-1, 1], and each feature should
            // contain 1 or -1 at least once
            using Vector::operator-;
            Mat<float> test;
            test.SetDims(2, max.length());
            REQUIRE(max.length() > 0);
            test[0] = -min;
            test[1] = max;
            const auto ones = Vector::max(test);
            for(long i = 0; i < max.length(); i++) {
                REQUIRE( ones[i] == 1 );
            }
        }

        SECTION("second transformation should not change anything") {
            const auto Xn2 = norm2.fit_transform(Xn);
            for(long i = 0; i < Xn.NumRows(); i++) {
                for(long j = 0; j < Xn.NumCols(); j++) {
                    REQUIRE( Xn[i][j] == Approx(Xn2[i][j]).epsilon(eps) );
                }
            }
            // mean/scale for second transformation should be 0/1
            for(long i = 0; i < norm1.get_intercept().length(); i++) {
                REQUIRE( norm2.get_intercept()[i] == Approx(0).epsilon(eps) );
                REQUIRE( norm2.get_scale()[i] == Approx(1).epsilon(eps) );
            }
        }

        SECTION("transform back") {
            // transform back
            auto Xn_inv = norm1.inverse_transform(Xn);
            for(long i = 0; i < Xn.NumRows(); i++) {
                for(long j = 0; j < Xn.NumCols(); j++) {
                    REQUIRE( Xn_inv[i][j] == Approx(X[i][j]).epsilon(eps) );
                }
            }
            // transform back for second transformation should not change anything
            norm2.fit(Xn);
            Xn_inv = norm2.inverse_transform(Xn);
            for(long i = 0; i < Xn.NumRows(); i++) {
                for(long j = 0; j < Xn.NumCols(); j++) {
                    REQUIRE( Xn_inv[i][j] == Approx(Xn[i][j]).epsilon(eps) );
                }
            }
        }

        SECTION("Vectors") {
            const auto X_t = Vector::transpose(X);
            const auto v = X_t[0];
            const auto vn = norm1.fit_transform(v);
            REQUIRE( norm1.get_intercept().length() == 1 );
            REQUIRE( vn.length() == v.length() );
            REQUIRE( max(Vector::max(vn), -Vector::min(vn)) == 1 );
            const auto vn_inv = norm1.inverse_transform(vn);
            for(long i = 0; i < vn.length(); i++) {
                REQUIRE( vn_inv[i] == Approx(v[i]).epsilon(eps) );
            }
        }
    }

    SECTION("Integerizer, complete transformation process") {
        Vector::Normalizer norm;
        const auto Xn = norm.fit_transform(X);
        const size_t bits = 22;
        const Vector::Integerizer inter(bits);
        const auto Xi = inter.transform(Xn);

        REQUIRE( Xi.NumRows() == X.NumRows() );
        REQUIRE( Xi.NumCols() == X.NumCols() );
        for(long i = 0; i < Xi.NumRows(); i++) {
            for(long j = 0; j < Xi.NumCols(); j++) {
                REQUIRE( Xi[i][j].size_bits() <= bits );
            }
        }

        SECTION("constructor") {
            Vector::Integerizer a(10);
            Vector::Integerizer b(10, -1);
            Vector::Integerizer c(10, -10);
            Vector::Integerizer d(1337, c.get_factor());
            Vector::Integerizer e(1337, 20);
            Vector::Integerizer f(10, 20);

            REQUIRE( a.get_factor() == b.get_factor() );
            REQUIRE( a.get_factor() == c.get_factor() );
            REQUIRE( a.get_factor() == d.get_factor() );
            REQUIRE( a.double_precision().get_factor() == d.get_factor() * d.get_factor() );

            REQUIRE_FALSE( d.get_factor() == e.get_factor() );
            REQUIRE( e.get_factor() == f.get_factor() );
        }

        SECTION("inverse transform, including normalizer") {
            const auto X_rev = inter.inverse_transform(Xi);
            for(long i = 0; i < Xi.NumRows(); i++) {
                for(long j = 0; j < Xi.NumCols(); j++) {
                    REQUIRE( X_rev[i][j] == Approx(Xn[i][j]).epsilon(eps) );
                }
            }
            const auto X_orig = norm.inverse_transform(X_rev);
            for(long i = 0; i < Xi.NumRows(); i++) {
                for(long j = 0; j < Xi.NumCols(); j++) {
                    REQUIRE( X_orig[i][j] == Approx(X[i][j]).epsilon(eps) );
                }
            }
        }

        SECTION("vectors") {
            Vec<float> test;
            test.SetLength(3);
            test[0] = -1; test[1] = 0; test[2] = 1;
            const Vec<Integer> test_i = inter.transform(test);
            REQUIRE(test_i[0].size_bits() == bits);
            REQUIRE(test_i[2].size_bits() == bits);
            REQUIRE(test_i[0] == -test_i[2]);
            const Vec<float> test_orig = inter.inverse_transform(test_i);
            REQUIRE(test_orig[0] == -1);
            REQUIRE(test_orig[2] == 1);
        }
    }

    SECTION("matrix and vector product") {
        const auto a = Vector::matrix_string<float>("[[1 2 3] [1 2 3] [1 1 1]]");
        const auto b = Vector::matrix_string<float>("[[1 2] [1 2] [1 1]]");
        const auto c = Vector::vec_string<float>("[3 4 2]");
        const auto d = Vector::vec_string<float>("[4 5]");

        SECTION("matrix") {
            REQUIRE( Vector::dot(X, Vector::transpose(X)).NumRows() == X.NumRows() );
            REQUIRE( Vector::dot(Vector::transpose(X), X).NumRows() == X.NumCols() );
            REQUIRE( Vector::dot(X, Vector::id<float>(X.NumCols())) == X );
            REQUIRE_THROWS_AS( Vector::dot(X, X), DimensionMismatchException );

            REQUIRE( Vector::dot(a, b) ==
                             Vector::matrix_string<float>("[[6 9] [6 9] [3 5]]") );
            REQUIRE( Vector::dot(Vector::transpose(b), a) ==
                             Vector::matrix_string<float>("[[3 5 7] [5 9 13]]") );
            REQUIRE_THROWS_AS( Vector::dot(b, a), DimensionMismatchException );
        }

        SECTION("matrix and vector") {
            REQUIRE( Vector::dot(a, c) == Vector::vec_string<float>("[17 17 9]") );
            REQUIRE( Vector::dot(b, d) == Vector::vec_string<float>("[14 14 9]") );

            REQUIRE( Vector::dot(Vector::transpose(b), c) ==
                             Vector::vec_string<float>("[9 16]") );
            REQUIRE( Vector::dot(Vector::transpose(a), c) ==
                             Vector::vec_string<float>("[9 16 23]") );
            REQUIRE( Vector::dot(c, b) ==
                     Vector::vec_string<float>("[9 16]") );
            REQUIRE( Vector::dot(c, a) ==
                     Vector::vec_string<float>("[9 16 23]") );

            REQUIRE( Vector::dot(Vector::col_matrix(c), Vector::row_matrix(c)) ==
                             Vector::matrix_string<float>("[[9 12 6][12 16 8][6 8 4]]") );
            REQUIRE( Vector::dot(Vector::row_matrix(c), Vector::col_matrix(c)) ==
                             Vector::matrix_string<float>("[[29]]") );

            REQUIRE_THROWS_AS( Vector::dot(a, d), DimensionMismatchException );
            REQUIRE_THROWS_AS( Vector::dot(b, a), DimensionMismatchException );
        }

        SECTION("vector product") {
            REQUIRE( Vector::dot(c, c) == 29 );
            REQUIRE( Vector::dot(d, d) == 41 );

            REQUIRE_THROWS_AS( Vector::dot(d, c), DimensionMismatchException );
        }

        SECTION("zero and id") {
            REQUIRE( Vector::dot(a, Vector::zeros<float>(3, 3)) ==
                             Vector::zeros<float>(3, 3) );
            REQUIRE( Vector::dot(a, Vector::id<float>(3)) == a);
            REQUIRE( Vector::dot(b, Vector::zeros<float>(2, 2)) ==
                     Vector::zeros<float>(3, 2) );
            REQUIRE( Vector::dot(b, Vector::zeros<float>(2, 3)) ==
                     Vector::zeros<float>(3, 3) );
            REQUIRE( Vector::dot(b, Vector::id<float>(2)) == b);
        }
    }

    SECTION("zeros, ones, id, rand") {
        SECTION("matrix") {
            auto const a = Vector::zeros<float>(10, 5);
            REQUIRE( a.NumRows() == 10 );
            REQUIRE( a.NumCols() == 5 );
            REQUIRE( Vector::sum(Vector::sum(a, 0)) == 0 );
            REQUIRE( Vector::sum(Vector::sum(a, 1)) == 0 );

            auto const b = Vector::ones<float>(11, 3);
            REQUIRE( b.NumRows() == 11 );
            REQUIRE( b.NumCols() == 3 );
            REQUIRE( Vector::sum(Vector::sum(b, 0)) == 11 * 3 );
            REQUIRE( Vector::sum(Vector::sum(b, 1)) == 11 * 3 );
        }

        SECTION("vector") {
            auto const a = Vector::zeros<float>(10);
            REQUIRE( a.length() == 10 );
            REQUIRE( Vector::sum(a) == 0 );

            auto const b = Vector::ones<float>(11);
            REQUIRE( b.length() == 11 );
            REQUIRE( Vector::sum(b) == 11 );
        }

        SECTION("id (matrix)") {
            auto const a = Vector::id<float>(4);
            REQUIRE( a.NumRows() == 4 );
            REQUIRE( a.NumCols() == 4 );
            REQUIRE( Vector::sum(Vector::sum(a, 0)) == 4 );
            REQUIRE( Vector::sum(Vector::sum(a, 1)) == 4 );
            REQUIRE( Vector::sum(a, 0) == Vector::ones<float>(4) );
            REQUIRE( Vector::sum(a, 1) == Vector::ones<float>(4) );
        }

        SECTION("rand") {
            const int max = 100000;
            const size_t bits = 100, x = 20, y = 30;
            REQUIRE( Vector::rand(x, max).length() == x );
            REQUIRE( Vector::rand_bits(x, bits).length() == x );
            REQUIRE( Vector::rand_bits_neg(x, bits).length() == x );
            REQUIRE( Vector::rand_primes(x, bits).length() == x );

            auto const a = Vector::rand(y, x, max);
            REQUIRE( a.NumRows() == y );
            REQUIRE( a.NumCols() == x );
            for(long i = 0; i < a.NumRows(); i++) {
                for(long j = 0; j < a.NumCols(); j++) {
                    REQUIRE( a[i][j] < max );
                }
            }
            auto const b = Vector::rand_bits(y, x, bits);
            REQUIRE( b.NumRows() == y );
            REQUIRE( b.NumCols() == x );
            for(long i = 0; i < a.NumRows(); i++) {
                for(long j = 0; j < a.NumCols(); j++) {
                    REQUIRE( b[i][j].size_bits() <= bits );
                }
            }
            auto const c = Vector::rand_bits_neg(y, x, bits);
            REQUIRE( c.NumRows() == y );
            REQUIRE( c.NumCols() == x );
            for(long i = 0; i < a.NumRows(); i++) {
                for(long j = 0; j < a.NumCols(); j++) {
                    REQUIRE( c[i][j].size_bits() <= bits );
                }
            }
            auto const d = Vector::rand_primes(y, x, bits);
            REQUIRE( d.NumRows() == y );
            REQUIRE( d.NumCols() == x );
            for(long i = 0; i < a.NumRows(); i++) {
                for(long j = 0; j < a.NumCols(); j++) {
                    REQUIRE( d[i][j].size_bits() <= bits );
                }
            }
        }
    }

    SECTION("matrix_string, vec_string") {
        SECTION("matrix") {
            const auto m = Vector::matrix_string<float>("[[1 2 3] [4 5 6]]");
            REQUIRE( m.NumRows() == 2 );
            REQUIRE( m.NumCols() == 3 );
            REQUIRE( m[0][0] == 1 );
            REQUIRE( m[1][2] == 6 );
            REQUIRE( Vector::sum(m)[0] == 5 );
            REQUIRE( Vector::sum(m)[1] == 7 );
            REQUIRE( Vector::sum(m, 1)[0] == 6 );
            REQUIRE( Vector::sum(m, 1)[1] == 15 );
        }

        SECTION("vector") {
            const auto m = Vector::vec_string<float>("[1 2 3]");
            REQUIRE( m.length() == 3 );
            REQUIRE( m[0] == 1 );
            REQUIRE( m[1] == 2 );
            REQUIRE( m[2] == 3 );
        }
    }

    SECTION("encrypt/decrypt") {
        Vector::Normalizer norm;
        Vector::Integerizer inter(50);
        PaillierFast pai(keysize);
        pai.generate_keys();

        SECTION("matrix") {
            auto const X_tr = inter.transform(norm.fit_transform(X));
            auto const X_enc = Vector::encrypt(X_tr, pai);
            auto const X_dec = Vector::decrypt(X_enc, pai);
            REQUIRE( X_dec == X_tr );
            auto const X_orig = norm.inverse_transform(inter.inverse_transform(X_dec));
            for(long i = 0; i < X.NumRows(); i++) {
                for(long j = 0; j < X.NumCols(); j++) {
                    REQUIRE( X_orig[i][j] == Approx(X[i][j]).epsilon(eps) );
                }
            }
        }

        SECTION("vectors") {
            auto const y_tr = inter.transform(norm.fit_transform(y));
            auto const y_enc = Vector::encrypt(y_tr, pai);
            auto const y_dec = Vector::decrypt(y_enc, pai);
            REQUIRE( y_dec == y_tr );
            auto const y_orig = norm.inverse_transform(inter.inverse_transform(y_dec));
            for(long i = 0; i < y.length(); i++) {
                REQUIRE( y_orig[i] == Approx(y[i]).epsilon(eps) );
            }
        }
    }

    SECTION("scalar and element wise ops on ciphertext") {
        Vector::Normalizer normX, normY;
        Vector::Integerizer inter(50);
        PaillierFast pai(keysize);
        pai.generate_keys();
        auto const X_tr = inter.transform(normX.fit_transform(X));
        auto const y_tr = inter.transform(normY.fit_transform(y));
        auto const X_enc = Vector::encrypt(X_tr, pai);
        auto const y_enc = Vector::encrypt(y_tr, pai);

        // sum
        REQUIRE( Vector::decrypt(Vector::sum(X_enc), pai) == Vector::sum(X_tr) );
        REQUIRE( Vector::decrypt(Vector::sum(X_enc, 1), pai) == Vector::sum(X_tr, 1) );
        REQUIRE( pai.decrypt(Vector::sum(y_enc)) == Vector::sum(y_tr) );

        // mul
        using Vector::operator*;
        REQUIRE( Vector::decrypt(y_enc * Integer(3), pai) == y_tr * Integer(3) );
        REQUIRE( Vector::decrypt(y_enc * Integer(0), pai) == y_tr * Integer(0) );
        REQUIRE( Vector::decrypt(y_enc * Integer(1), pai) == y_tr );
        REQUIRE( Vector::decrypt(X_enc * Integer(3), pai) == X_tr * Integer(3) );
        REQUIRE( Vector::decrypt(X_enc * Integer(0), pai) == X_tr * Integer(0) );
        REQUIRE( Vector::decrypt(X_enc * Integer(1), pai) == X_tr );

        // add
        using Vector::operator+;
        REQUIRE( Vector::decrypt(y_enc + pai.encrypt(3), pai) == y_tr + Integer(3) );
        REQUIRE( Vector::decrypt(y_enc + pai.encrypt(0), pai) == y_tr );
        REQUIRE( Vector::decrypt(X_enc + pai.encrypt(3), pai) == X_tr + Integer(3) );
        REQUIRE( Vector::decrypt(X_enc + pai.encrypt(0), pai) == X_tr );

        REQUIRE( Vector::decrypt(y_enc + y_enc, pai) == y_tr + y_tr );
        REQUIRE( Vector::decrypt(y_enc + y_enc + y_enc, pai) == y_tr * Integer(3) );
        REQUIRE( Vector::decrypt(X_enc + X_enc, pai) == X_tr + X_tr );
        REQUIRE( Vector::decrypt(X_enc + X_enc + X_enc, pai) == X_tr * Integer(3) );

        // sub
        using Vector::operator-;
        REQUIRE( Vector::decrypt(y_enc - pai.encrypt(3), pai) == y_tr - Integer(3) );
        REQUIRE( Vector::decrypt(y_enc - pai.encrypt(0), pai) == y_tr );
        REQUIRE( Vector::decrypt(X_enc - pai.encrypt(3), pai) == X_tr - Integer(3) );
        REQUIRE( Vector::decrypt(X_enc - pai.encrypt(0), pai) == X_tr );

        REQUIRE( Vector::decrypt(y_enc - y_enc, pai) == y_tr - y_tr );
        REQUIRE( Vector::decrypt(y_enc - y_enc, pai) == Vector::zeros<Integer>(y_tr.length()) );
        REQUIRE( Vector::decrypt(y_enc - y_enc - y_enc, pai) == -y_tr );
        REQUIRE( Vector::decrypt(X_enc - X_enc, pai) == X_tr - X_tr );
        REQUIRE( Vector::decrypt(X_enc - X_enc, pai) == Vector::zeros<Integer>(X_tr.NumRows(), X_tr.NumCols()) );
        REQUIRE( Vector::decrypt(X_enc - X_enc - X_enc, pai) == -X_tr );

        // neg
        REQUIRE( Vector::decrypt(-y_enc, pai) == -y_tr );
        REQUIRE( Vector::decrypt(-X_enc, pai) == -X_tr );
    }

    SECTION("vector ops on ciphertext") {
        Vector::Normalizer normX, normY;
        Vector::Integerizer inter(50);
        PaillierFast pai(keysize);
        pai.generate_keys();

        // toy tests
        auto const aa = Vector::matrix_string<Integer>("[[1 1] [1 2] [1 3]]");
        auto const bb = Vector::vec_string<Integer>("[1 2]");
        auto const aaa = Vector::encrypt(aa, pai);
        auto const bbb = Vector::encrypt(bb, pai);

        REQUIRE( pai.decrypt(Vector::dot(bbb, bb)) == 1 + 4 );
        REQUIRE( Vector::decrypt(Vector::dot(aaa, bb), pai) == Vector::dot(aa, bb) );
        REQUIRE( Vector::decrypt(Vector::dot(bbb, Vector::transpose(aa)), pai) == Vector::dot(bb, Vector::transpose(aa)) );

        // larger X
        auto const X_tr = inter.transform(normX.fit_transform(X));
        auto const y_tr = inter.transform(normY.fit_transform(y));
        auto const X_enc = Vector::encrypt(X_tr, pai);
        auto const y_enc = Vector::encrypt(y_tr, pai);
        auto const id = Vector::id<Integer>(y_tr.length());
        auto const zero = Vector::zeros<Integer>(y_tr.length(), y_tr.length());
        auto const id_enc = Vector::encrypt(id, pai);
        auto const zero_enc = Vector::encrypt(zero, pai);

        REQUIRE( Vector::decrypt(Vector::dot(Vector::transpose(X_enc), y_tr), pai) == Vector::dot(y_tr, X_tr) );
        REQUIRE( Vector::decrypt(Vector::dot(id_enc, y_tr), pai) == y_tr );
        REQUIRE( Vector::decrypt(Vector::dot(zero_enc, y_tr), pai) == Vector::zeros<Integer>(y_tr.length()) );

        REQUIRE( Vector::decrypt(Vector::dot(y_enc, X_tr), pai) == Vector::dot(y_tr, X_tr) );
        REQUIRE( Vector::decrypt(Vector::dot(y_enc, id), pai) == y_tr );
        REQUIRE( Vector::decrypt(Vector::dot(y_enc, zero), pai) == Vector::zeros<Integer>(y_tr.length()) );

        auto const id_vec = Vector::ones<Integer>(y_tr.length());
        auto const zero_vec = Vector::zeros<Integer>(y_tr.length());
        REQUIRE( pai.decrypt(Vector::dot(y_enc, y_tr)) == Vector::dot(y_tr, y_tr) );
        REQUIRE( pai.decrypt(Vector::dot(y_enc, id_vec)) == Vector::sum(y_tr) );
        REQUIRE( pai.decrypt(Vector::dot(y_enc, zero_vec)) == 0 );
    }

    SECTION("matrix inversion") {
        auto id = Vector::id<float>(10);
        // inversion of id
        REQUIRE( Vector::inv(id) == id );

        // non-square
        REQUIRE_THROWS_AS( Vector::inv(Vector::zeros<float>(5, 6)), MathException );
        // non-invertible
        id[0][0] = 0;
        id[0][1] = 1;
        REQUIRE_THROWS_AS( Vector::inv(id), MathException );

        // invertible
        const auto X_2 = Vector::dot(Vector::transpose(X), X);
        REQUIRE( X_2.NumCols() == X_2.NumRows() );
        const auto X_2_inv = Vector::inv(X_2);
        REQUIRE( X_2_inv.NumCols() == X_2_inv.NumRows() );
        REQUIRE( X_2.NumCols() == X_2_inv.NumRows() );
        const auto X_id = Vector::id<float>((size_t)X_2_inv.NumRows());
        const auto approx_id = Vector::dot(X_2, X_2_inv);
        for(long i = 0; i < X_id.NumRows(); i++) {
            for(long j = 0; j < X_id.NumRows(); j++) {
                REQUIRE( approx_id[i][j] == Approx(X_id[i][j]).epsilon(eps) );
            }
        }
    }
}
