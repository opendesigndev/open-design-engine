
# Cachables

ARG VCPKG_COMMIT_SOURCE=manifest

FROM public.ecr.aws/debian/debian:11.6-slim AS ode-debian-build-config-from-arg

# When setting VCPKG_COMMIT, also set VCPKG_COMMIT_SOURCE=arg
ARG VCPKG_COMMIT=
RUN echo $VCPKG_COMMIT > /home/vcpkg-commit

# Optional intermediate target that can be skipped when vcpkg commit is explicitly specified
# If the vcpkg commit is not set, it will be deduced from vcpkg.json
# The reason this can be skipped is so that other changes in vcpkg.json (e.g. ODE version) don't invalidate the ode-debian-build-base target
FROM ode-debian-build-config-from-arg AS ode-debian-build-config-from-manifest

ONBUILD COPY vcpkg.json /tmp/vcpkg.json
ONBUILD RUN apt update -qq && apt install -qqy jq
ONBUILD RUN jq -r '."builtin-baseline"' /tmp/vcpkg.json > /home/vcpkg-commit

FROM ode-debian-build-config-from-${VCPKG_COMMIT_SOURCE} AS ode-debian-build-base

ENV VCPKG_DEFAULT_TRIPLET x64-linux-release
ENV VCPKG_DEFAULT_HOST_TRIPLET x64-linux-release

RUN apt-get update \
    && apt-get install -y curl zip unzip tar clang make cmake wget git python3 bison automake autoconf autoconf-archive libosmesa6-dev pkg-config python3-distutils patchelf nodejs \
    && rm -rf /var/lib/apt/lists/*

# Vckpg
RUN git clone https://github.com/microsoft/vcpkg /opt/vcpkg && cd /opt/vcpkg && git checkout `cat /home/vcpkg-commit`
RUN /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics && ln -s /opt/vcpkg/vcpkg /usr/bin/vcpkg
RUN vcpkg --triplet x64-linux-release --host-triplet x64-linux-release install freetype harfbuzz[icu] icu libpng libjpeg-turbo tiff giflib libwebp skia[core,freetype] node-api[core] node-addon-api
ENV VCPKG_ROOT=/opt/vcpkg

# Build GLFW with OSMesa support
RUN wget -c https://github.com/glfw/glfw/archive/refs/tags/3.3.7.tar.gz -O - | tar -xz --directory /home
RUN cmake -S /home/glfw-3.3.7 -B /home/glfw-3.3.7/build -DGLFW_USE_OSMESA=ON -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
RUN cmake --build /home/glfw-3.3.7/build && cmake --install /home/glfw-3.3.7/build

# Build GLEW with OSMesa support
RUN wget -c https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz -O - | tar -xz --directory /home
RUN make install SYSTEM=linux-osmesa -C /home/glew-2.2.0

# Build image

FROM ode-debian-build-base AS ode-debian-build

ARG ODE_BUILD_COMMIT_TAG

RUN mkdir -p /home/ode
COPY . /home/ode
# Provide dummy implementation of GLX functions used by Skia
COPY ./dockerfiles/debian-build/glx-dummy.cpp /home/ode/ode-graphics/ode/glx-dummy.cpp

# Configure and build ODE
RUN cmake \
    -S /home/ode \
    -B /home/ode/build \
    -DCMAKE_BUILD_TYPE=Release \
    -DOVERRIDE_OPENGL_LIBRARY=ON \
    -DOVERRIDE_GLFW_LIBRARY=ON \
    -DOVERRIDE_GLEW_LIBRARY=ON \
    -DGLEW_USE_STATIC_LIBS=ON \
    -DODE_RENDERER_ENABLED=ON \
    -DODE_SKIA_GPU=OFF \
    -DODE_TEST_MODULES=OFF \
    -DODE_BUILD_COMMIT_TAG=$ODE_BUILD_COMMIT_TAG \
    -DCMAKE_CXX_STANDARD_LIBRARIES=-lOSMesa \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_MAKE_PROGRAM=make \
    -DVCPKG_TARGET_TRIPLET=x64-linux-release
RUN cmake --build /home/ode/build --target ode-renderer-cli --config Release

RUN mkdir -p /opt/ode-renderer/bin /opt/ode-renderer/lib
RUN cp /home/ode/build/ode-renderer/ode-renderer-cli /opt/ode-renderer/bin/ode-renderer

# Find system libraries used by the compiled binaries and copy them as redistributables
RUN ldd /opt/ode-renderer/bin/* | awk '{ print $1 }' | awk -F'/' '{ print $NF }' | xargs -I'{}' find /lib/x86_64-linux-gnu /usr/lib/x86_64-linux-gnu -name '{}' -print -quit | tee /tmp/libs.txt | xargs -I{} cp {} /opt/ode-renderer/lib

WORKDIR /opt/ode-renderer
RUN patchelf --set-interpreter /opt/ode-renderer/lib/ld-linux-x86-64.so.2 bin/* \
    && for v in $(patchelf --print-needed bin/*); do patchelf --replace-needed $v /opt/ode-renderer/lib/$v bin/*; done \
    && for v in $(patchelf --print-needed lib/* | sort | uniq); do patchelf --replace-needed $v /opt/ode-renderer/lib/$v lib/*; done

# Image with portable ODE executables
FROM scratch
ENV PATH="${PATH}:/opt/ode-renderer/bin"
COPY --from=ode-debian-build /opt/ode-renderer /opt/ode-renderer

# Build locally using:
# bash -c 'docker build . -f dockerfiles/debian-build/Dockerfile --build-arg ODE_BUILD_COMMIT_TAG=`git describe --tags --always` --tag ode --platform linux/amd64'
