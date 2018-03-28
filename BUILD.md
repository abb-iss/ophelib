# Building OPHELib

The following instructions and build scripts assume that the user is running a Linux based OS with the APT package manager, e.g, Ubuntu or Debian.

The first step in building `OPHELib` is to clone its git repository containing the source code:

    git clone https://github.com/abb/ophelib
    cd ophelib

There are two ways to build `OPHELib`:
1. Automatically using `Docker`, where the entire build process is performed in a container and the resulting artifacts are saved in the local workspace. This is the fastest and the least intrusive way to build the library, since all the steps are automated and all dependencies are only built and installed in the auxiliary container. The build using `Docker` can also be used for CI.
2. Manually performing the build in the local workspace. This way also allows integration with an IDE.

## Building using Docker
The prerequisite is to have `Docker` installed, which can be verified by executing in the terminal:

```
docker --version
```

The build can be done by executing the script in the terminal:

```
./release-docker.sh
```
The resulting artifact is the file `ophelib-release.zip`.

The script:
* downloads and compiles the dependencies GMP, NTL, and FlatBuffers in a container based on Ubuntu, as defined in the file `Dockerfile`,
* adds the source files from the workspace to the container, as defined in the files `Dockerfile` and `.dockerignore`,
* executes `./tests.sh` inside the container,
* executes `./release.sh` inside the container, and
* copies the resulting artifacts from the container to the workspace as a zip file.

## Building manually in local workspace

The manual build requires the following tools and dependencies to be installed first:
* G++ compailer
* Make
* Curl
* GNU m4
* GNU libtool
* doxygen
* graphviz
* CMake version 3.2 or later
* GMP
* NTL
* Flatbuffers

All the tools and dependencies, except GMP, NTL, and Flatbuffers, are available in the Ubuntu official repository, and can be installed by executing:

```
sudo apt-get install  build-essential \
                      curl            \
                      m4              \
                      libtool-bin     \
                      doxygen         \
                      graphviz        \
                      cmake
```

CMake version can be checked by executing:

```
cmake --version
```

If the version is below 3.2 and a newer one is not available in the official repository, the following repository can be added:

```
sudo apt-get install software-properties-common
sudo -E add-apt-repository ppa:george-edison55/cmake-3.x
sudo apt-get update && sudo apt-get install cmake
```

GMP can be installed by executing in the terminal:

```
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
# cleanup
rm -f gmp.tar.bz2
rm -rf gmp
```
NTL can be installed by executing in the terminal (while still in the *lib_* directory):

```
curl http://www.shoup.net/ntl/ntl-$(grep -m 1 LIBNTL_VERSION ../Dockerfile | cut -d ' ' -f 3).tar.gz > ntl.tar.gz
mkdir ntl
tar xfv ntl.tar.gz -C ntl --strip-components=1
cd ntl/src
./configure DEF_PREFIX="$(pwd)/../../install" SHARED=on NATIVE=on WIZARD=on CXXFLAGS='-fPIC'
make -j4
make install
cd ../..
# cleanup
rm -f ntl.tar.gz
rm -rf ntl
```

Flatbuffers can be installed by executing in the terminal (while still in the *lib_* directory):

```
curl -L https://github.com/google/flatbuffers/archive/v$(grep -m 1 FLATBUFFERS_VERSION ../Dockerfile | cut -d ' ' -f 3).tar.gz > flatbuffers.tar.gz
mkdir flatbuffers
tar xfv flatbuffers.tar.gz -C flatbuffers --strip-components=1
cd flatbuffers
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX:PATH="$(pwd)/../install"
make -j4
make install
cd ..
# cleanup
rm -f flatbuffers.tar.gz
cd ..
```

Finally, `OPHELib` can be built by executing in the terminal:

```
cmake .
make -j4
```

# Apendix

## The Build System Explained
CMake is used as the build system. It is used for both methods described above. The difference is that when building locally, you need a recent CMake version on your host system, whereas when using Docker to build, only the CMake from inside the container is used. The build artifacts are then copied from within the container to the build artifact folders on your host machine.

Files and folders relevant to the build system:
* Scripts for building `OPHELib`
    * ```release-docker.sh```
    * ```release.sh```
* Build artifact output directories
    * ```bin/```
    * ```include/```
    * ```lib/```
* Files used for docker build
    * ```Dockerfile```
    * ```.dockerignore```
* CMake build definition file
    * ```CMakeLists.txt```
* CMake cache files
    * ```CMakeCache.txt```
    * ```CMakeFiles```
    * ```cmake_install.cmake```
    * ```Makefile```
* Directory where the dependencies are built and stored if using manual build
    * ```lib_```
* Script for running profiler
    * ```profile.sh```
* Script for running tests
    * ```test.sh```
* Script for cleaning the build artifacts
    * ```clean.sh```

## Build Options
Several options can be passed to CMake, some of which concern compiler flags (optimization), while others enable and disable features. Options can be passed to CMake using the format `-D<name>=value`, e.g. `-DENABLE_OPENMP=OFF` disables OpenMP.

| Name                 | Values              | Default    | Description |
|----------------------|---------------------|------------|-------------|
| CMAKE_BUILD_TYPE     | "" , Debug, Release | "" (empty) | [See CMake Documentation](https://cmake.org/cmake/help/v3.0/variable/CMAKE_BUILD_TYPE.html) |
| PROFILING_BUILD      | ON, OFF             | OFF        | Enable profiling in binaries                                  |
| ENABLE_OPENMP        | ON, OFF             | ON         | Use OpenMP for parallelization                                |
| ENABLE_EXCEPTIONS    | ON, OFF             | ON         | If off, replaces C++ exceptions and stack traces by `exit(1)` |
| BUILD_RUN_TESTS      | ON, OFF             | OFF        | Run tests in Makefile after compilation                       |
| ADDITIONAL_CXX_FLAGS | e.g. "-g"           | "" (empty) | Additional flags to pass to C++ compiler                      |
| ADDITIONAL_LD_FLAGS  | e.g. "-lto"         | "" (empty) | Additional flags to pass to linker                            |

Thus, running `cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENMP=OFF` builds the project in release mode without OpenMP but with exceptions enabled.
