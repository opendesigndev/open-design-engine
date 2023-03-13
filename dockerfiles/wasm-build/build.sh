#!/bin/sh

set -ex
if [ -d "${0%/*}" ]; then cd "${0%/*}" ; fi # go to directory containing this script
cd ../.. # go to repo root

cmake --preset wasm-rel
cmake --build --preset wasm-rel
