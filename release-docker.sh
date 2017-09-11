#!/bin/bash

#
# This file uses the same steps and variables as .gitlab-ci.yml
#

IMAGE_NAME="CH-S-GitLab.ch.abb.com:SecreDS/ophelib:latest"
BUILD_PATH=$(grep -m 1 BUILD_PATH Dockerfile | cut -d ' ' -f 3)
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
# release
#

# generate docs
container_id=$(docker run -t -i -d "$IMAGE_NAME" doxygen)
docker attach $container_id
docker cp "$container_id:$BUILD_PATH/docs/" .
docker rm $container_id

# test build with disabled openmp
docker run --rm -t -i "$IMAGE_NAME" bash -c 'cmake -DENABLE_OPENMP=OFF . && make -j4'

# test build with disabled exceptions
docker run --rm -t -i "$IMAGE_NAME" bash -c 'cmake -DENABLE_EXCEPTIONS=OFF . && make -j4'

# real build
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

# create zip
zip -r ophelib-release.zip lib include deps docs CHANGELOG
