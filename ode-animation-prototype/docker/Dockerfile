FROM emscripten/emsdk:3.1.12
RUN apt-get update && apt-get install -y \
    ninja-build \
    pkg-config \
    autoconf autoconf-archive \
    bison gperf \
    python-is-python3 \
    && rm -rf /var/lib/apt/lists/*

# Install emscripten ports
RUN embuilder build freetype zlib harfbuzz libpng icu libjpeg
# TODO: remove this once we update to node.js >= 16.10, which includes corepack by default
RUN npm i -g corepack && npm cache clean --force && corepack enable
RUN corepack prepare yarn@1.22.19

COPY .npmrc /home/emscripten/.npmrc
