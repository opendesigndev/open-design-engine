FROM emscripten/emsdk:3.1.12@sha256:9c83e2dbe9b9a76fc7bb9f892df0ca67757c7b04527e8f722eea46c85f6df603 as wasm-base
RUN apt-get update && apt-get install -y \
    ninja-build \
    pkg-config \
    autoconf autoconf-archive \
    bison gperf \
    python-is-python3 \
    jq \
    && rm -rf /var/lib/apt/lists/*

# Install emscripten ports
RUN embuilder build freetype zlib harfbuzz libpng icu libjpeg
# TODO: remove this once we update to node.js >= 16.10, which includes corepack by default

RUN npm i -g corepack && npm cache clean --force && corepack enable
RUN corepack prepare yarn@1.22.19

COPY .npmrc .gitconfig /home/emscripten/

ENV HOME /home/emscripten

FROM wasm-base as wasm-build

RUN ./third-party/skia/build.sh

RUN emcmake cmake \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
    -DODE_TEST_MODULES=OFF \
    -DEMSCRIPTEN=ON \
    -B $BUILD_DIRECTORY \
    .
RUN cmake --build $BUILD_DIRECTORY