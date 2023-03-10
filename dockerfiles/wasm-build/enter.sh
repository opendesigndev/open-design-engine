#!/bin/sh

set -ex
if [ -d "${0%/*}" ]; then cd "${0%/*}" ; fi # go to directory containing this script
cd ../.. # go to repo root

docker build . -f dockerfiles/wasm-build/Dockerfile --tag ode-napi --target builder
docker run --rm -it --entrypoint /bin/sh --name ode-napi --volume `pwd`:/src ode-napi "$@"
