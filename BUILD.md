# Building the project

First, clone the source tree with git:

    git clone git@CH-S-GitLab.ch.abb.com:SecreDS/ophelib.git
    cd ophelib

The fastest way to build the project from scratch is using Docker (which is also used for CI). After you have set up docker properly, run the script which will create the docker container on your host machine:

    ./release-docker.sh

What this does:

* Download and compile GMP and NTL in a container (defined in `Dockerfile`)
* Add the contents of this folder to the container (defined in `Dockerfile`)
* Call `./tests.sh` inside the container
* Call `./release.sh` inside the container
* Copy artifacts to the host (defined in `release-docker.sh`)

The first time, it will need quite long to build GMP and NTL. On subsequent builds, this is cached and the build will be much faster.

## Build options
Several options can be passed to CMake. Some concern compiler flags (optimization), whilst others allow to enable and disable features. Passing options to cmake: `-D<name>=value`, e.g. `-DENABLE_OPENMP=OFF` disabled OpenMP.

| Name                 | Values              | Default    | Description |
|----------------------|---------------------|------------|-------------|
| CMAKE_BUILD_TYPE     | "" , Debug, Release | "" (empty) | [See CMake Documentation](https://cmake.org/cmake/help/v3.0/variable/CMAKE_BUILD_TYPE.html) |
| PROFILING_BUILD      | ON, OFF             | OFF        | Enable profiling in binaries                                  |
| ENABLE_OPENMP        | ON, OFF             | ON         | Use OpenMP for parallelization                                |
| ENABLE_EXCEPTIONS    | ON, OFF             | ON         | If off, replaces C++ exceptions and stack traces by `exit(1)` |
| BUILD_RUN_TESTS      | ON, OFF             | OFF        | Run tests in Makefile after compilation                       |
| ADDITIONAL_CXX_FLAGS | e.g. "-g"           | "" (empty) | Additional flags to pass to C++ compiler                      |
| ADDITIONAL_LD_FLAGS  | e.g. "-lto"         | "" (empty) | Additional flags to pass to linker                            |

So, running `cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_OPENMP=OFF` would build the project in release mode without OpenMP, but with Exceptions enabled.

## The build system explained
Generally, there are two ways to build the project. First: manually on your host machine, which also allows integration with an IDE. This is explained below. The second way is the build in a Docker container, as explained above.

CMake is used as build system. It is used for both methods. The difference is that when building locally, you need a recent CMake version on your host system wheres when using Docker to build, only the CMake from inside the container is used. The build artifacts are the copied from within the container to the build artifact folders on your host machine.

Files relevant to the build system:

    # build artifact output directories
    bin
    include
    lib

    # cmake build definition file
    CMakeLists.txt

    # cmake cache files
    CMakeCache.txt
    CMakeFiles
    cmake_install.cmake
    Makefile

    # used for docker build
    Dockerfile
    .dockerignore

    # job description for Gitlab CI
    .gitlab-ci.yml

    # in here, the libraries we depend on are
    # built and stored if using manual build
    lib_

    # helper scripts, which build from scratch:

    release-docker.sh
    release.sh
    # also runs profiler
    profile.sh
    # also runs tests
    test.sh

## Gitlab CI
The file `.gitlab-ci.yml` basically does the same as `./release-docker.sh`, but is executed on the build server and split into different stages.
