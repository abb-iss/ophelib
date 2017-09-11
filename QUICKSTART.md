# Quickstart

In the Gitlab project view, go to the `Pipelines -> Tags` section and download the latest build release/artifact file. It contains the header files and the compiled library, for ophelib as well for the libraries we depend on.

## Demo program
\include "hello_world.cpp"

There is a small hello world demo program which shows basic usage (shown above).

Although compilation works with GCC 4.x, **libgcc 5 is needed for compilation**. Also we need build-essential (and maybe unzip to unpack the release file). For being able to fetch the key for the new repository, you may need to update your `/etc/environment` with the ABB proxy (don't forget to reload the environment after, i.e. start a new login shell).

    sudo -E add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt-get update
    sudo apt-get install build-essential unzip gcc-5 g++-5

To build it, use the following `Makefile` (also included in the downloaded zip):

\include "Makefile"

To see more code samples, look at the code in the `test` directory (in the source tree, not in the build release). Also the performance measurement tool in `test/perf.cpp` shows the usage of some components.

## Short API introduction
Load data:

    const string data_file = "test/fixtures/x17.txt";
    NTL::Mat<float> X;
    NTL::Vec<float> y;
    Vector::load_data(data_file, X, y);

Generate Keys:

    PaillierFast pai(2048);
    pai.generate_keys();

Normalize and integerize:

    Vector::Normalizer normX, normY;
    Vector::Integerizer inter(30); // 30 bits precision

    auto const X_tr = inter.transform(normX.fit_transform(X));
    auto const y_tr = inter.transform(normY.fit_transform(y));

Encrypt/decrypt a vector/matrix:

    auto const X_enc = Vector::encrypt(X_tr, pai);
    auto const X_dec = Vector::decrypt(X_enc, pai);

Deintegerize and denormalize:

    auto const X_orig = normX.inverse_transform(inter.inverse_transform(X_dec));
