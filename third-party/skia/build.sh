#!/bin/sh

if [ -d "${0%/*}" ]; then cd "${0%/*}" ; fi # go to directory containing this script

echo running build_skia.sh: $*
set -xe
cd src

if [ ! -f bin/gn ]; then python3 ./bin/fetch-gn; fi

EXTRA_CFLAGS="-Wno-undef-prefix"

# debug | release
BUILD_TYPE=release
IS_DEBUG=false
IS_OFFICIAL_BUILD=true

if [[ $@ == *debug* ]]; then
    BUILD_TYPE=debug
    IS_DEBUG=true
    IS_OFFICIAL_BUILD=false
    EXTRA_CFLAGS="${EXTRA_CFLAGS} -DSK_FORCE_8_BYTE_ALIGNMENT"
fi
echo "build type: ${BUILD_TYPE}"

BUILD_PATH=../out/wasm/${BUILD_TYPE}

mkdir -p "${BUILD_PATH}"

git checkout . || true
patch -u BUILD.gn -i ../patches/03-build-gn-no-modules-deps.patch

./bin/gn gen ${BUILD_PATH} --args="\
    target_cpu=\"wasm\" \
    cc=\"`which emcc`\" \
    cxx=\"`which em++`\" \
    ar=\"`which emar`\" \
    is_debug=${IS_DEBUG} \
    is_official_build=${IS_OFFICIAL_BUILD} \
    extra_cflags=[
        \"${EXTRA_CFLAGS}\",
    ] \
    skia_enable_gpu=true \
    skia_use_gl=true \
    skia_use_webgl=true \
    skia_gl_standard=\"webgl\" \
    skia_use_x11=false \
    skia_enable_pdf=false \
    skia_use_dng_sdk=false \
    skia_use_expat=false \
    skia_use_icu=false \
    skia_use_zlib=false \
    skia_use_piex=false \
    skia_use_vulkan=false \
    skia_enable_tools=false \
    skia_use_freetype=false \
    skia_use_fontconfig=false \
    skia_use_libjpeg_turbo_decode=false \
    skia_use_libjpeg_turbo_encode=false \
    skia_use_libpng_decode=false \
    skia_use_libpng_encode=false \
    skia_use_libwebp_decode=false \
    skia_use_libwebp_encode=false \
    skia_use_ffmpeg=false \
    skia_enable_skottie=false \
    skia_use_fonthost_mac=false \
    "

ninja -C ${BUILD_PATH} libskia.a && echo ${BUILD_PATH}/libskia.a

git checkout . || true

cd ..
mkdir -p out/include
cp -r src/include out/include/skia
cp -r src/include out/include/include
