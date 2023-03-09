FROM emscripten/emsdk:3.1.12@sha256:9c83e2dbe9b9a76fc7bb9f892df0ca67757c7b04527e8f722eea46c85f6df603 as builder
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

RUN npm i -g corepack && npm cache clean --force && corepack enable
RUN corepack prepare yarn@1.22.19
WORKDIR /src
COPY third-party /pre/third-party
RUN /pre/third-party/skia/build.sh
ENV ODE_PREBUILT_THIRDPARTY /pre/third-party/wasm-cmake

FROM builder

COPY . /src
RUN emcmake cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DNAPI_BINDINGS=ON -DODE_TEST_MODULES=OFF -DODE_USE_WEBGL2=ON -DEMSCRIPTEN=ON -DODE_NAPI=ON -B build/wasm-napi .
RUN cmake --build build/wasm-napi

# docker build . -f dockerfiles/wasm-build/Dockerfile
