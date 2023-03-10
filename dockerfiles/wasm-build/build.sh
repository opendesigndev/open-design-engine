#!/bin/sh

set -ex
if [ -d "${0%/*}" ]; then cd "${0%/*}" ; fi # go to directory containing this script

./enter.sh -c "python3 generate-api-bindings.py && cmake --build build/wasm-napi"
