#include "ophelib/ml.h"
#include "ophelib/paillier_fast.h"
#include "catch.hpp"
#include "ophelib/disable_exception_tests.h"

using namespace std;
using namespace ophelib;

const size_t keysize = 1024;

TEST_CASE("ML") {
    const string data_file = "../test/fixtures/rand.txt";
    NTL::Mat<float> X_;
    NTL::Vec<float> y_;
    Vector::load_data(data_file, X_, y_);
    const Vector::Integerizer inter(30);
    Vector::Normalizer normX, normY;
    const NTL::Mat<Integer> X = inter.transform(normX.fit_transform(X_));

    SECTION("LinregPlain") {
        const NTL::Vec<Integer> y = inter.transform(normY.fit_transform(y_));

        ML::LinregPlain reg(inter.get_factor(), 1, 100);
        // not yet fitted
        REQUIRE_THROWS_AS( reg.predict(X), BaseException );
        REQUIRE( reg.fit(X, y) == 100 );
        REQUIRE_THROWS_AS( reg.predict(Vector::id<Integer>(X.NumCols() + 1)),
                           DimensionMismatchException );
        const auto y_pred = normY.inverse_transform(inter.inverse_transform(reg.predict(X)));
        REQUIRE( y_pred.length() == X.NumRows() );
        REQUIRE( ML::cost(y_, y_pred) < 26 );

        #ifdef DEBUG
        const auto weights = inter.inverse_transform(reg.get_weights());
        cout << "> LinregPlain weights: ";
        for(auto w: weights)
            cout << w << " ";
        cout << endl;
        cout << "> cost = " << ML::cost(y_, y_pred) << endl;
        #endif
    }

    SECTION("LinregPlainEnc") {
        const auto interY = inter.double_precision();
        const NTL::Vec<Integer> y = interY.transform(normY.fit_transform(y_));

        PaillierFast paillier(keysize);
        paillier.generate_keys();

        const auto y_enc = Vector::encrypt(y, paillier);
        const auto callback = ML::LinregPlainEnc::construct_client_callback(paillier);
        ML::LinregPlainEnc reg(callback, inter.get_factor(), paillier.get_pub(), 1, 100);
        // not yet fitted
        REQUIRE_THROWS_AS( reg.predict(X), BaseException );
        REQUIRE( reg.fit(X, y_enc) == 100 );
        REQUIRE_THROWS_AS( reg.predict(Vector::id<Integer>(X.NumCols() + 1)),
                           DimensionMismatchException );
        const auto y_pred = normY.inverse_transform(interY.inverse_transform(
                Vector::decrypt(reg.predict(X), paillier)));
        REQUIRE( y_pred.length() == X.NumRows() );
        REQUIRE( ML::cost(y_, y_pred) < 26 );

        #ifdef DEBUG
        const auto weights = inter.inverse_transform(Vector::decrypt(reg.get_weights(), paillier));
        cout << "> LinregPlain weights: ";
        for(auto w: weights)
            cout << w << " ";
        cout << endl;
        cout << "> cost = " << ML::cost(y_, y_pred) << endl;
        #endif
    }

    SECTION("LinregPlainEnc - with wrapper class callback") {
        const auto interY = inter.double_precision();
        const NTL::Vec<Integer> y = interY.transform(normY.fit_transform(y_));

        PaillierFast paillier(keysize);
        paillier.generate_keys();

        class clbk: public ML::LinregPlainEnc::client_callback_cls_t {
        public:
            const PaillierFast p;
            clbk(PaillierFast p_): p(p_) {}

            virtual Vec<Ciphertext> call(const Vec<PackedCiphertext> &error, const Integer &divisor) const {
                using Vector::operator/;
                return Vector::encrypt(Vector::decrypt_pack(error, p) / -divisor, p);
            }
        };

        const auto callback = clbk(paillier);
        const auto y_enc = Vector::encrypt(y, paillier);
        ML::LinregPlainEnc reg(&callback, inter.get_factor(), paillier.get_pub(), 1, 100);
        reg.fit(X, y_enc);
        const auto y_pred = normY.inverse_transform(interY.inverse_transform(
                Vector::decrypt(reg.predict(X), paillier)));
        REQUIRE( ML::cost(y_, y_pred) < 26 );
    }

    SECTION("LinregPlainEncEqn") {
        const auto X_flt = normX.transform(X_);
        const auto y = inter.transform(normY.fit_transform(y_));

        PaillierFast paillier(keysize);
        paillier.generate_keys();
        const auto y_enc = Vector::encrypt(y, paillier);
        const auto callback = ML::LinregPlainEncEqn::construct_client_callback(paillier);
        ML::LinregPlainEncEqn reg(callback, inter);
        // not yet fitted
        REQUIRE_THROWS_AS( reg.predict(X), BaseException );
        reg.fit(X_flt, y_enc);
        REQUIRE_THROWS_AS( reg.predict(Vector::id<Integer>(X.NumCols() + 1)),
                           DimensionMismatchException );
        const auto y_pred = normY.inverse_transform(inter.double_precision().inverse_transform(
                Vector::decrypt(reg.predict(X), paillier)));
        REQUIRE( y_pred.length() == X.NumRows() );
        REQUIRE( ML::cost(y_, y_pred) < 26 );

        #ifdef DEBUG
        const auto weights = inter.inverse_transform(Vector::decrypt(reg.get_weights(), paillier));
        cout << "> LinregPlainEncEqn weights: ";
        for(auto w: weights) {
            cout << w << " ";
        }
        cout << endl;
        cout << "> cost = " << ML::cost(y_, y_pred) << endl;
        #endif
    }

    SECTION("LinregPlainEncUsers") {
        const auto X_flt = normX.transform(X_);
        const auto y_flt = normY.fit_transform(y_);

        PaillierFast paillier(keysize);
        paillier.generate_keys();
        const auto callback = ML::LinregPlainEncUsers::construct_client_callback(paillier);
        ML::LinregPlainEncUsers reg(callback, inter);
        const auto b = ML::LinregPlainEncUsers::client_preprocess(X_flt, y_flt, inter, paillier);

        // not yet fitted
        REQUIRE_THROWS_AS( reg.predict(X), BaseException );
        reg.fit(X_flt, b);
        REQUIRE_THROWS_AS( reg.predict(Vector::id<Integer>(X.NumCols() + 1)),
                           DimensionMismatchException );
        const auto y_pred = normY.inverse_transform(inter.double_precision().inverse_transform(
                Vector::decrypt(reg.predict(X), paillier)));

        REQUIRE( y_pred.length() == X.NumRows() );
        REQUIRE( ML::cost(y_, y_pred) < 26 );

        #ifdef DEBUG
        const auto weights = inter.inverse_transform(Vector::decrypt(reg.get_weights(), paillier));
        cout << "> LinregPlainEncUsers weights: ";
        for(auto w: weights) {
            cout << w << " ";
        }
        cout << endl;
        cout << "> cost = " << ML::cost(y_, y_pred) << endl;
        #endif
    }

    SECTION("LinregEncEncUsers") {
        const auto interY = inter.triple_precision();
        const NTL::Vec<Integer> y = interY.transform(normY.fit_transform(y_));

        PaillierFast paillier(keysize);
        paillier.generate_keys();
        const auto callback = ML::LinregEncEncUsers::construct_client_callback(paillier);
        ML::LinregEncEncUsers reg(callback, inter.get_factor(), paillier.get_pub(), 1, 100);

        const auto A = ML::LinregEncEncUsers::client_preprocess_A(X, paillier);
        const auto b = ML::LinregEncEncUsers::client_preprocess_b(X, y, paillier);

        // not yet fitted
        REQUIRE_THROWS_AS( reg.predict(X), BaseException );
        REQUIRE( reg.fit(A, b) == 100 );
        REQUIRE_THROWS_AS( reg.predict(Vector::id<Integer>(X.NumCols() + 1)),
                           DimensionMismatchException );
        const auto y_pred = normY.inverse_transform(inter.inverse_transform(reg.predict(X)));
        const auto y_pred2 = normY.inverse_transform(interY.inverse_transform(
                Vector::decrypt(reg.predict(Vector::encrypt(X, paillier)), paillier)));

        REQUIRE( y_pred.length() == X.NumRows() );
        REQUIRE( y_pred2.length() == X.NumRows() );
        REQUIRE( ML::cost(y_, y_pred) < 26 );
        REQUIRE( ML::cost(y_, y_pred2) < 26 );

        #ifdef DEBUG
        const auto weights = inter.inverse_transform(reg.get_weights());
        cout << "> LinregEncEncUsers weights: ";
        for(auto w: weights) {
            cout << w << " ";
        }
        cout << endl;
        cout << "> cost = " << ML::cost(y_, y_pred) << endl;
        #endif
    }

    SECTION("cost") {
        using Vector::operator*;
        REQUIRE(ML::cost(y_, y_) == 0 );
        REQUIRE( ML::cost(Vector::zeros<float>(10), Vector::ones<float>(10)) == 1 );
        REQUIRE( ML::cost(Vector::zeros<float>(10), Vector::ones<float>(10) * (float)2) == 4 );
    }
}
