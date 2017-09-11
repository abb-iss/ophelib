FROM CH-S-GitLab.ch.abb.com:7443/docker-images/ubuntu16-build:latest

MAINTAINER Jonathan Müller <jonathan.mueller@ch.abb.com>

RUN mkdir /build-deps
WORKDIR /build-deps

ENV LIBGMP_VERSION 6.1.0
ENV LIBNTL_VERSION 9.8.0

RUN curl https://gmplib.org/download/gmp/gmp-$LIBGMP_VERSION.tar.bz2 \
        > gmp.tar.bz2                                             && \
    mkdir gmp install                                             && \
    tar xfj gmp.tar.bz2 -C gmp --strip-components=1               && \
    cd gmp                                                        && \
    ./configure                                                      \
        --prefix "$(pwd)/../install"                                 \
        --enable-cxx                                                 \
        CPPFLAGS='-D_GLIBCXX_USE_CXX11_ABI=0 -fPIC'               && \
    make -j4                                                      && \
    make -j4 check                                                && \
    make install

RUN curl http://www.shoup.net/ntl/ntl-$LIBNTL_VERSION.tar.gz         \
        > ntl.tar.gz                                              && \
    mkdir ntl                                                     && \
    tar xf ntl.tar.gz -C ntl --strip-components=1                 && \
    cd ntl/src                                                    && \
    ./configure                                                      \
        DEF_PREFIX="$(pwd)/../../install"                            \
        SHARED=on                                                    \
        NATIVE=on                                                    \
        WIZARD=on                                                    \
        CXXFLAGS='-D_GLIBCXX_USE_CXX11_ABI=0 -fPIC'               && \
    make -j 4                                                     && \
    make install

ENV FLATBUFFERS_VERSION 1.3.0

RUN curl -L https://github.com/google/flatbuffers/archive/v$FLATBUFFERS_VERSION.tar.gz \
        > flatbuffers.tar.gz                                                        && \
    mkdir flatbuffers                                                               && \
    tar xf flatbuffers.tar.gz -C flatbuffers --strip-components=1                   && \
    cd flatbuffers                                                                  && \
    cmake                                                                              \
        -G "Unix Makefiles"                                                            \
        -DCMAKE_INSTALL_PREFIX:PATH="$(pwd)/../install"                             && \
    make -j4                                                                        && \
    make install

RUN apt-get update && apt-get install -yq \
        doxygen                           \
        graphviz                       && \
        apt-get clean                  && \
        rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ENV BUILD_DEPS_PREFIX /build-deps
ENV BUILD_PATH /build
ADD . /build
WORKDIR /build
