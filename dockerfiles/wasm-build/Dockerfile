FROM emscripten/emsdk:3.1.41@sha256:270ed8b0045b14593817e0c2d751a04ff1da2aea70e17e56a3f36baab6b470e1 as builder
RUN apt-get update && apt-get install -y \
    pkg-config \
    autoconf autoconf-archive \
    bison gperf \
    python-is-python3 \
    jq curl \
    && rm -rf /var/lib/apt/lists/*

# Upgrade CMake to allow us to use --preset
RUN mkdir -p /cmake \
    && cd /cmake \
    && curl -L https://github.com/Kitware/CMake/releases/download/v3.25.3/cmake-3.25.3-linux-x86_64.sh -o cmake.sh \
    && bash cmake.sh --skip-license \
    && rm cmake.sh
ENV PATH=/cmake/bin:$PATH

# Install emscripten ports
RUN embuilder build freetype zlib harfbuzz libpng icu libjpeg

RUN npm i -g corepack && npm cache clean --force && corepack enable
RUN corepack prepare yarn@1.22.19
WORKDIR /src
COPY third-party /pre/third-party
RUN /pre/third-party/skia/build.sh
ENV ODE_PREBUILT_THIRDPARTY /pre/third-party/wasm-cmake

# docker build . -f dockerfiles/wasm-build/Dockerfile
