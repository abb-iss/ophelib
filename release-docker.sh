#!/bin/bash

# This file contains the steps to build OPHELib,
# with the build performed in docker containers.
# The final artifact is the ophelib-release.zip file.
#
# These steps can be run by an CI.
#

IMAGE_NAME="abb/ophelib"
# get the build path from the Dockerfile
BUILD_PATH=$(grep -m 1 BUILD_PATH Dockerfile | cut -d ' ' -f 3)
# get the build dependencies path from the Dockerfile
BUILD_DEPS_PREFIX=$(grep -m 1 BUILD_DEPS_PREFIX Dockerfile | cut -d ' ' -f 3)

# build the docker image
if ! docker build -t "$IMAGE_NAME" .; then
	echo 'Failed to build docker image'
	exit 1
fi

# run tests
if ! docker run --rm -t -i "$IMAGE_NAME" ./test.sh; then
	echo 'Tests failed'
	exit 1
fi

#
# Create the release
#

# generate docs in a container and copy them to docs/ local directory.
container_id=$(docker run -t -i -d "$IMAGE_NAME" doxygen)
docker attach $container_id
docker cp "$container_id:$BUILD_PATH/docs/" .
docker rm $container_id

# test build with disabled openmp
docker run --rm -t -i "$IMAGE_NAME" bash -c 'cmake -DENABLE_OPENMP=OFF . && make -j4'

# test build with disabled exceptions
docker run --rm -t -i "$IMAGE_NAME" bash -c 'cmake -DENABLE_EXCEPTIONS=OFF . && make -j4'

# real build. Copy the resulting artifacts to the local directories.
container_id=$(docker run -t -i -d "$IMAGE_NAME" bash -c './release.sh')
docker attach $container_id
docker cp "$container_id:$BUILD_PATH/bin/" .
docker cp "$container_id:$BUILD_PATH/lib/" .
docker cp "$container_id:$BUILD_DEPS_PREFIX/install" - > "deps.tar"
docker cp "$container_id:$BUILD_PATH/include/ophelib/wire/" .
mv wire/*_generated.h include/ophelib/wire/
rm -rf wire
docker rm $container_id

# copy dependencies
rm -rf "install" "deps"
tar xf "deps.tar"
mv "install" "deps"
rm "deps.tar"
rm deps/lib/libflatbuffers.a
rm deps/bin/flatc

# create a zip file with the release artifacts
zip -r ophelib-release.zip lib include deps docs CHANGELOG
