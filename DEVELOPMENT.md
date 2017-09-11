# Development

## Dev env setup
If you want to set up a local dev environment, or just build the project manually, follow this guide.

If you want to build the project automatically in a Docker container, read [Build](BUILD.md).

First, we need a recent CMake version. Skip this if your OS already ships with CMake version >= 3.2 (e.g., Ubuntu 16.04).

    sudo apt-get install software-properties-common
    sudo -E add-apt-repository ppa:george-edison55/cmake-3.x
    sudo apt-get update
    sudo apt-get upgrade cmake

Clone the source tree:

    git clone git@CH-S-GitLab.ch.abb.com:SecreDS/ophelib.git
    cd ophelib

Then, we have to prepare [NTL](http://www.shoup.net/ntl/doc/tour.html) and [GMP](https://gmplib.org/).

    # GMP
    cd lib_
    curl https://gmplib.org/download/gmp/gmp-$(grep -m 1 LIBGMP_VERSION ../Dockerfile | cut -d ' ' -f 3).tar.bz2 > gmp.tar.bz2
    mkdir gmp install
    tar xfjv gmp.tar.bz2 -C gmp --strip-components=1
    cd gmp
    ./configure --prefix "$(pwd)/../install" --enable-cxx CXXFLAGS='-fPIC'
    make -j4
    make -j4 check
    make install
    cd ..

    # NTL
    curl http://www.shoup.net/ntl/ntl-$(grep -m 1 LIBNTL_VERSION ../Dockerfile | cut -d ' ' -f 3).tar.gz > ntl.tar.gz
    mkdir ntl
    tar xfv ntl.tar.gz -C ntl --strip-components=1
    cd ntl/src
    ./configure DEF_PREFIX="$(pwd)/../../install" SHARED=on NATIVE=on WIZARD=on CXXFLAGS='-fPIC'
    make -j 4
    make install
    cd ../..

    # Flatbuffers
    curl -L https://github.com/google/flatbuffers/archive/v$(grep -m 1 FLATBUFFERS_VERSION ../Dockerfile | cut -d ' ' -f 3).tar.gz > flatbuffers.tar.gz
    mkdir flatbuffers
    tar xfv flatbuffers.tar.gz -C flatbuffers --strip-components=1
    cd flatbuffers
    cmake -G "Unix Makefiles"
    make -j4
    cd ../..

Now, we can build ophelib:

    cmake .
    make -j 4

There exist some additional helper scripts. They all clean the build and then call cmake/make with special arguments (they work directly on the host machine as well inside a Docker container):

    # build in debug mode, run tests
    ./tests.sh
    # build in release mode
    ./release.sh
    # build in profile mode, and then call gprof on the results
    ./profile.sh

## Run tests
To run the tests manually, execute `bin/tests`.

[Catch](https://github.com/philsquared/Catch) is used as test framework. This means that `bin/tests` accepts the options listed [here](https://github.com/philsquared/Catch/blob/master/docs/command-line.md).

## Release a new version
Update the `CHANGELOG` file to contain the version you want to release like this (here, releasing `0.2.3`):

    v 0.2.3
      - Better packing
      ...

Then, tag (when tagging, the top line in `CHANGELOG` must be `v <tag-name>`:

    git tag <version>
    git push --tags

Then, wait until CI has generated the build artifacts. Done!
Alternatively, you can run `./release.sh` or `./release-docker.sh` locally and the pack the build artifacts manually.
To continue, add a new line on top of the `CHANGELOG`:

    v 0.2.4 (unreleased)
      - Did something new
      ...

    v 0.2.3
      - Better packing
      ...

## Profiling

    sudo apt-get install valgrind
    ./profile.sh

Find the results in `analysis.txt`.

## Documentation
You can extract documentation using doxygen:

    sudo apt-get install doxygen
    doxygen

You will find the generated html docs in `docs/html/index.html`. These files are also included in the CI build artifacts.
