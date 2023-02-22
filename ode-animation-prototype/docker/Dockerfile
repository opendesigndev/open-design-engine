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
# TODO: remove this once we update to node.js >= 16.10, which includes corepack by default

RUN npm i -g corepack && npm cache clean --force && corepack enable
RUN corepack prepare yarn@1.22.19

COPY .npmrc .gitconfig /home/emscripten/

ENV HOME /home/emscripten
