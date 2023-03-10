FROM emscripten/emsdk:3.1.32@sha256:d0c17c0843510f3e306e607603f72af17834795c44b73b0c7ebc0302d6f028ee
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
