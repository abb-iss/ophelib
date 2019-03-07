#include "ophelib/vector.h"
#include "ophelib/paillier_fast.h"
#include "catch.hpp"

using namespace std;
using namespace ophelib;

const float eps = 0.0001;

#define REPEAT(x) for(size_t r = 0; r < x; r ++)
#define CRYPTO_INIT()   PaillierFast pai(2048); \
                        pai.generate_keys(); \
                        const auto X_enc = Vector::encrypt(X_i, pai); \
                        const auto y_enc = Vector::encrypt(y_i, pai);

/**
 * These tests are used for measuring parallelization performance
 * during implementation. For this, a big matrix is used. However,
 * the correctness of the results is not checked, for this we have
 * test_vector_ops.cpp. To run:
 *
 * make -j4 && OMP_NUM_THREADS=2 bin/perf_vector_parallel --durations yes
 */
TEST_CASE("Vector Ops Parallelization Performance") {
    const string data_file = "../test/fixtures/rand_big.txt";

    NTL::Mat<float> X_;
    NTL::Vec<float> y_;
    Vector::load_data(data_file, X_, y_);
    const NTL::Mat<float> X = X_;
    const NTL::Vec<float> y = y_;
    Vector::Normalizer normX;
    Vector::Normalizer normY;
    Vector::Integerizer inter(30);
    const auto X_i = inter.transform(normX.fit_transform(X));
    const auto y_i = inter.transform(normY.fit_transform(y));

    /* 2 threads: 1.6x faster, 4 threads: 1.6x faster (2 cores) */
    SECTION("vector") {
        const auto b = Vector::col_matrix(y);

        REPEAT(500000)
            Vector::vector(b);
    }

    /* 2 threads: 1.6x faster, 4 threads: 2.4x faster (2 cores) */
    SECTION("transpose") {
        REPEAT(100000)
            Vector::transpose(X);
    }

    SECTION("sum") {
        /* 2 threads: 1.7x faster, 4 threads: 2.5x faster (2 cores) */
        SECTION("matrix sum 0") {
            REPEAT(50000)
                Vector::sum(X);
        }
        /* 2 threads: same, 4 threads: same (2 cores) */
        SECTION("matrix sum 1") {
            REPEAT(50000)
                Vector::sum(X, 1);
        }
        /* not implemented */
        SECTION("vector sum") {
            REPEAT(1000000)
                Vector::sum(y);
        }
    }

    SECTION("max") {
        /* 2 threads: 1.7x faster, 4 threads: 2.5x faster (2 cores) */
        SECTION("matrix max 0") {
            REPEAT(50000)
                Vector::max(X);
        }
        /* 2 threads: same, 4 threads: same (2 cores) */
        SECTION("matrix max 1") {
            REPEAT(50000)
                Vector::max(X, 1);
        }
        /* not implemented */
        SECTION("vector max") {
            REPEAT(1000000)
                Vector::max(y);
        }
    }

    /* 2 threads: x faster, 4 threads: x faster (2 cores) */
    SECTION("min") {
        /* 2 threads: 1.7x faster, 4 threads: 2.5x faster (2 cores) */
        SECTION("matrix min 0") {
            REPEAT(50000)
                Vector::max(X);
        }
        /* 2 threads: same, 4 threads: same (2 cores) */
        SECTION("matrix min 1") {
            REPEAT(50000)
                Vector::max(X, 1);
        }
        /* not implemented */
        SECTION("vector min") {
            REPEAT(1000000)
                Vector::max(y);
        }
    }

    SECTION("scalar operator*") {
        CRYPTO_INIT();

        using Vector::operator*;

        /* 2 threads: 2x faster, 4 threads: 4x faster (2 cores) */
        SECTION("matrix *") {
            REPEAT(100)
                X_enc * Integer(234);
        }
        /* 2 threads: 2x faster, 4 threads: 3x faster (2 cores) */
        SECTION("vector *") {
            REPEAT(10 * 20)
                y_enc * Integer(234);
        }
    }

    SECTION("scalar operator/") {
        using Vector::operator/;

        /* 2 threads: 1.5x faster, 4 threads: 1.7x faster (2 cores) */
        SECTION("matrix /") {
            REPEAT(10000)
                X_i / Integer(234);
        }
        /* 2 threads: 1.2x faster, 4 threads: 1.5x faster (2 cores) */
        SECTION("vector /") {
            REPEAT(10000 * 20)
                y_i / Integer(234);
        }
    }

    SECTION("scalar operator+") {
        using Vector::operator+;

        /* 2 threads: 1.3x faster, 4 threads: 1.8x faster (2 cores) */
        SECTION("matrix +") {
            REPEAT(5000)
                X_i + Integer(234);
        }
        /* 2 threads: 1.1x faster, 4 threads: 1.4x faster (2 cores) */
        SECTION("vector +") {
            REPEAT(1000 * 20)
                y_i + Integer(234);
        }
    }

    SECTION("scalar operator-") {
        using Vector::operator-;

        /* 2 threads: 1.3x faster, 4 threads: 2x faster (2 cores) */
        SECTION("matrix -") {
            REPEAT(5000)
                X_i - Integer(234);
        }
        /* 2 threads: 1.1x faster, 4 threads: 1.4x faster (2 cores) */
        SECTION("vector -") {
            REPEAT(1000 * 20)
                y_i - Integer(234);
        }
    }

    SECTION("unary operator-") {
        CRYPTO_INIT();

        using Vector::operator-;

        /* 2 threads: 1.2x faster, 4 threads: 1.6x faster (2 cores) */
        SECTION("matrix -") {
            REPEAT(5000)
                -X_i;
        }
        /* 2 threads: same, 4 threads: same (2 cores) */
        SECTION("vector -") {
            REPEAT(3000 * 20)
                -y_i;
        }
        /* 2 threads: 2x faster, 4 threads: 4x faster (2 cores) */
        SECTION("matrix -") {
            REPEAT(30)
                -X_enc;
        }
        /* 2 threads: 2x faster, 4 threads: 4x faster (2 cores) */
        SECTION("vector -") {
            REPEAT(10 * 20)
                -y_enc;
        }
    }

    SECTION("vec/mat operator+") {
        CRYPTO_INIT();

        using Vector::operator+;
        /* 2 threads: 1.8x faster, 4 threads: 3x faster (2 cores) */
        SECTION("matrix +") {
            REPEAT(200)
                X_enc + X_enc;
        }
        /* 2 threads: 1.8x faster, 4 threads: 3x faster (2 cores) */
        SECTION("vector +") {
            REPEAT(100 * 20)
                y_enc + y_enc;
        }
    }

    SECTION("vec/mat operator-") {
        CRYPTO_INIT();

        using Vector::operator-;
        /* 2 threads: 2x faster, 4 threads: 4x faster (2 cores) */
        SECTION("matrix -") {
            REPEAT(20)
                X_enc - X_enc;
        }
        /* 2 threads: 2x faster, 4 threads: 4x faster (2 cores) */
        SECTION("vector -") {
            REPEAT(20 * 20)
                y_enc - y_enc;
        }
    }

    SECTION("Normalizer") {
        const int n = 30000;

        /* 2 threads: 1.4x faster, 4 threads: 1.65x faster (2 cores) */
        SECTION("fit_transform matrix") {
            REPEAT(n)
                normX.fit_transform(X);
        }
        normX.fit(X);
        /* 2 threads: x1.5 faster, 4 threads: 1.7x faster (2 cores) */
        SECTION("transform matrix") {
            REPEAT(n)
                normX.transform(X);
        }

        /* 2 threads: same, 4 threads: same (2 cores) */
        SECTION("fit_transform vector") {
            REPEAT(n)
                normY.fit_transform(y);
        }
        normY.fit(y);
        /* 2 threads: same, 4 threads: same (2 cores) */
        SECTION("transform vector") {
            REPEAT(n)
                normY.transform(y);
        }

        const auto Xn = normX.fit_transform(X);
        /* 2 threads: 1.35x faster, 4 threads: 1.55x faster (2 cores) */
        SECTION("transform back matrix") {
            REPEAT(n)
                normX.inverse_transform(Xn);
        }
        const auto yn = normY.fit_transform(y);
        /* not implemented */
        SECTION("transform back vector") {
            REPEAT(n)
                normY.inverse_transform(yn);
        }
    }

    /* no parallelization implemented */
    SECTION("Integerizer") {
        Vector::Integerizer intX(30), intY(30);
        const int n = 1000;

        SECTION("transform matrix") {
            REPEAT(n)
                intX.transform(X);
        }

        SECTION("transform vector") {
            REPEAT(n * 4)
                intY.transform(y);
        }

        const auto Xn = intX.transform(X);
        SECTION("transform back matrix") {
            REPEAT(n)
                intX.inverse_transform(Xn);
        }
        const auto yn = intY.transform(y);
        SECTION("transform back vector") {
            REPEAT(n * 4)
                intY.inverse_transform(yn);
        }
    }

    SECTION("matrix and vector product, plaintext") {
        const int n = 10;
        const auto X_t = Vector::transpose(X_i);

        /* 2 threads: 1.3x faster, 4 threads: 1.7x faster (2 cores) */
        SECTION("matrix matrix 1") {
            REPEAT(n)
                Vector::dot(X_i, X_t);
        }
        /* 2 threads: 1.7x faster, 4 threads: 2.5x faster (2 cores) */
        SECTION("matrix matrix 2") {
            REPEAT(n * 100)
                Vector::dot(X_t, X_i);
        }
        /* 2 threads: 1.3x faster, 4 threads: 1.6x faster (2 cores) */
        SECTION("matrix vector") {
            REPEAT(n * 1000)
                Vector::dot(X_t, y_i);
        }
        /* not implemented */
        SECTION("vector vector") {
            REPEAT(n * 2000)
                Vector::dot(y_i, y_i);
        }
    }

    SECTION("matrix and vector product, ciphertext") {
        CRYPTO_INIT();

        const int n = 5;
        const auto X_et = Vector::transpose(X_enc);
        /* 2 threads: 2x faster, 4 threads: 3x faster (2 cores) */
        SECTION("matrix vector") {
            REPEAT(n)
                Vector::dot(X_et, y_i);
        }
        /* 2 threads: 2x faster, 4 threads: 3x faster (2 cores) */
        SECTION("vector matrix") {
            REPEAT(n)
                Vector::dot(y_enc, X_i);
        }
        /* not implemented */
        SECTION("vector vector") {
            REPEAT(n * 6)
                Vector::dot(y_enc, y_i);
        }
    }

    SECTION("enc/dec") {
        CRYPTO_INIT();

        const int n = 1;
        /* 2 threads: 2x faster, 4 threads: 4x faster (2 cores) */
        SECTION("enc matrix") {
            REPEAT(n)
                Vector::encrypt(X_i, pai);
        }
        /* 2 threads: 2x faster, 4 threads: 3.5x faster (2 cores) */
        SECTION("enc vector") {
            REPEAT(n * 10)
                Vector::encrypt(y_i, pai);
        }
        /* 2 threads: 2x faster, 4 threads: 3.5x faster (2 cores) */
        SECTION("dec matrix") {
            REPEAT(n)
                Vector::decrypt(X_enc, pai);
        }
        /* 2 threads: 2x faster, 4 threads: 4x faster (2 cores) */
        SECTION("dec vector") {
            REPEAT(n * 10)
                Vector::decrypt(y_enc, pai);
        }
    }

    /* not implemented (because only small matrices are
     * inverted in this library */
    SECTION("matrix inv") {
        const int n = 1000000;
        const auto X_sq = Vector::dot(Vector::transpose(X), X);

        SECTION("inv") {
            REPEAT(n)
                Vector::inv(X_sq);
        }
    }

    /* 2 threads: 2x faster, 4 threads: 2.5x faster (2 cores) */
    SECTION("PaillierFast") {
        const int n = 3;
        PaillierFast pai(2048);

        SECTION("generate_keys") {
            REPEAT(n)
                pai.generate_keys();
        }
    }
}
