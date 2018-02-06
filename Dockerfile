# This Dockerfile contains commands that build an image with
# all ophelib dependencies and with ophelib source code.
#  
# The commands:
#  0. set environment variables, such as build path,
#     and install supporting tools.
#  1. build and install ophelib dependencies
#  2. copy ophelib source files

FROM ubuntu:16.04

LABEL author="Jonathan Müller <jonathan.mueller@ch.abb.com>"
LABEL maintainer="Ognjen Vukovic <ognjen.vukovic@ch.abb.com>"

## 0.1 specify the versions of the required libraries to be installed
ENV LIBGMP_VERSION 6.1.0
ENV LIBNTL_VERSION 9.8.0
ENV FLATBUFFERS_VERSION 1.3.0

## 0.2 specify the build paths
ENV BUILD_DEPS_PREFIX /build-deps
ENV BUILD_PATH /build

## 0.3 Install necessary supporting tools and clean apt afterwards
RUN apt-get update && apt-get install -yq \
            build-essential               \
            curl                          \
            m4                            \
            libtool-bin                   \
            cmake                         \
            doxygen                       \
            graphviz                   && \
    apt-get clean                      && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# 1. BUILD & INSTALL DEPENDENCIES

## 1.1 Create the directory for building dependencies
RUN mkdir $BUILD_DEPS_PREFIX
WORKDIR $BUILD_DEPS_PREFIX

## 1.2 Install GMP: The GNU Multiple Precision Arithmetic Library
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

## 1.3 Install NTL: A Library for doing Number Theory
RUN curl http://www.shoup.net/ntl/ntl-$LIBNTL_VERSION.tar.gz \
        > ntl.tar.gz                                      && \
    mkdir ntl                                             && \
    tar xf ntl.tar.gz -C ntl --strip-components=1         && \
    cd ntl/src                                            && \
    ./configure                                              \
        DEF_PREFIX="$(pwd)/../../install"                    \
        SHARED=on                                            \
        NATIVE=on                                            \
        WIZARD=on                                            \
        CXXFLAGS='-D_GLIBCXX_USE_CXX11_ABI=0 -fPIC'       && \
    make -j 4                                             && \
    make install

## 1.4 Install FlatBuffers: an efficient serialization library for memory constrained apps
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

# 2. PREPARE FOR OPHELIB BUILD

## 2.1 copy all source files and set the working directory
ADD . $BUILD_PATH
WORKDIR $BUILD_PATH
