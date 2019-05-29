# Building and installing OPHELib

## Building on your machine
This was tested on Ubuntu 18.04.2.

    # needed for build
    sudo apt-get install build-essential m4 libtool-bin libgmp-dev libntl-dev cmake

    # needed to build docs
    sudo apt-get install doxygen graphviz

    # clone repo
    git clone https://github.com/abb-iss/ophelib
    cd ophelib

    # compile and install (from ophelib/)
    mkdir build && cd build
    cmake ..
    make -j

    # run tests (in ophelib/build/)
    make test

    # generate docs (from ophelib/ophelib/)
    doxygen
    open docs/html/index.html

    # install on your system (in ophelib/build/)
    sudo make install

    # compile the sample binary (from ophelib/)
    mkdir build-sample && cd build-sample
    cmake ../sample
    make
    ./hello_world

    # to uninstall (in ophelib/build/)
    sudo xargs rm < install_manifest.txt

## Building in Docker image

    # in ophelib/
    docker build -t ophelib:latest .

    # now, you can run any of the binaries in a container
    docker run -it --rm ophelib:latest build/ophelib_version
    docker run -it --rm ophelib:latest /bin/bash -c 'cd build; ./ophelib_test'

## Integrating into your own project
1. Build and install the library as described above
2. Copy and adapt <sample/CMakeLists.txt>
