#!/bin/bash
set -ex
if [ -d "${0%/*}" ]; then cd "${0%/*}" ; fi # go to directory containing this script

# Parse switches: --docker, --debug
DOCKER_BUILD=false
CMAKE_BUILD_TYPE=Release
BUILD_DIRECTORY=build/wasm-release
DOCKER_BUILD_ARGS=
DOCKER_RUN_ARGS=
NPM_RELEASE=false
while [[ $# -gt 0 ]]; do
  case $1 in
    docker|-docker|--docker)
      DOCKER_BUILD=true
      echo "Docker build"
      shift
      ;;
    debug|-debug|--debug)
      CMAKE_BUILD_TYPE=Debug
      BUILD_DIRECTORY=build/wasm-debug
      DOCKER_BUILD_ARGS=--debug
      shift
      ;;
    publish|-publish|--publish)
      NPM_RELEASE=true
      DOCKER_RUN_ARGS="-v $HOME/.npmrc:/host_npmrc:ro"
      DOCKER_BUILD_ARGS=--publish
      shift
      ;;
    *)
      echo "Unknown option $1"
      exit 1
      ;;
  esac
done

# if first argument is docker:
# 1. build docker image with correct deps
# 2. run self in docker
if [ $DOCKER_BUILD = true ]; then
    DOCKER_TAG=ode-animation-wasm-builder
    docker build docker -t $DOCKER_TAG
    cd .. # project root
    docker run \
        --rm \
        -v $(pwd):/src \
        $DOCKER_RUN_ARGS \
        -u $(id -u):$(id -g) \
        $DOCKER_TAG \
        bash ode-animation-prototype/wasm-build.sh $DOCKER_BUILD_ARGS
    exit 0
fi

cd .. # project root

./third-party/skia/build.sh

# Make sure that local changes in package directory do not make it to npm
PACKAGE_DIRECTORY="${BUILD_DIRECTORY}/ode-animation-prototype/package"
if [ $NPM_RELEASE = true ]; then
  rm -rf "$PACKAGE_DIRECTORY"
fi

# Build the project
emcmake cmake \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
    -DODE_TEST_MODULES=OFF \
    -DEMSCRIPTEN=ON \
    -B $BUILD_DIRECTORY \
    .
cmake --build $BUILD_DIRECTORY

# Set version field to be 0.0.0-githash-YYYYMMDD
VERSION="0.0.0-`git log -n 1 '--pretty=format:%h_%ci' | cut -d ' ' -f 1 | tr -d '-' | tr '_' '-'`"
if [ $CMAKE_BUILD_TYPE = "Debug" ]; then
  VERSION=${VERSION}-debug
fi
sed -i -E 's/"version": "[0-9.]+"/"version": "'$VERSION'"/' "${PACKAGE_DIRECTORY}/package.json"
echo 'export const version = "'$VERSION'"' >> "${PACKAGE_DIRECTORY}/ode.js"
cp typescript-bindings/* ./build/wasm-release/ode-animation-prototype/package/

# Publish
if [ $NPM_RELEASE = true ]; then
  if [ -f /host_npmrc ]; then
    export NPM_TOKEN=`cat /host_npmrc | grep '//registry.npmjs.org/:_authToken' | cut -d = -f 2`
  fi
  npm publish --tag alpha "${PACKAGE_DIRECTORY}"
fi
