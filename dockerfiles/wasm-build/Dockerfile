FROM emscripten/emsdk:3.1.43@sha256:e856cd59d54fd143366c2df314043d63a1ca23ab43521e0f30f9d9a7ce6d43e0 as builder
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
