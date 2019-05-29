# This Dockerfile contains commands that build an image with
# all ophelib dependencies and with ophelib source code.

FROM ubuntu:18.04

LABEL author="Jonathan Müller <jonathan.mueller@ch.abb.com>"
LABEL maintainer="Ognjen Vukovic <ognjen.vukovic@ch.abb.com>"

## Install necessary supporting tools and clean apt afterwards
RUN apt-get update && apt-get install -yq \
            build-essential               \
            m4                            \
            libtool-bin                   \
            cmake                         \
            doxygen                       \
            graphviz                      \
            libgmp-dev                    \
            libntl-dev                 && \
    apt-get clean                      && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

## Build OPHELib
ADD . /source
WORKDIR /source
RUN mkdir build      && \
    cd build         && \
    cmake ..         && \
    make -j4         && \
    make install

## Build sample
RUN mkdir build-sample   && \
    cd build-sample      && \
    cmake ../sample      && \
    make -j4

## Generate docs
RUN doxygen

## Show version
RUN /source/build/ophelib_version
