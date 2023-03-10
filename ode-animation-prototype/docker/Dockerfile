FROM emscripten/emsdk:3.1.33@sha256:aa50e3dfb1cfe0d31091708d5ce1dd02d4827c4124a98eb4f22d665b0db084b6
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

COPY third-party/skia/src/bin/fetch-gn /pre/third-party/skia/src/bin/fetch-gn
RUN python3 /pre/third-party/skia/src/bin/fetch-gn

# TODO: remove this once we update to node.js >= 16.10, which includes corepack by default
RUN node -v && npm i -g corepack && npm cache clean --force && corepack enable
RUN corepack prepare yarn@1.22.19

COPY ode-animation-prototype/docker/.npmrc ode-animation-prototype/docker/.gitconfig /home/emscripten/

ENV HOME /home/emscripten

ENV ODE_PREBUILT_THIRDPARTY /pre/third-party/wasm-cmake
COPY third-party /pre/third-party
RUN /pre/third-party/skia/build.sh

# docker build . -f ode-animation-prototype/docker/Dockerfile
