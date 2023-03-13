# Node.js bindings

Bindings using [node-addon-api](https://github.com/nodejs/node-addon-api) and
[emnapi](https://github.com/toyobayashi/emnapi) (for web build).

## Packages

Every package is in @opendesign/ scope. I'll not be prefixing package names with
this.

Main package is `engine`. This package does not contain much by itself, but instead
it acts as a loader and reexports correct package to be used.

On the web, `engine-wasm` package is used. On node.js `engine-$platform-$arch`
(eg. engine-darwin-arm64 for MacOS on Apple Silicon) package is used, or if such
package is not available `engine-source` package is used instead. `engine-source`
contains full Engine source code and instructions to build it using
[cmake-js](https://github.com/cmake-js/cmake-js).

Every package except `engine-source` is built using cmake and is output into
`build/$PRESET/ode-napi/(wrapper|package)` directory. Real (published) wrapper
is output from wasm build, but other builds also produce this package in configuration
that is useful for local development using yarn v3's [portal:](https://yarnpkg.com/advanced/lexicon/#portal)
protocol. `engine-source` is published directly from root of this repository as
that is basically a tarball of all source files required for compilation of node.js
addon.

This setup is similar (but not identical) to how esbuild is packaged and is 
conceptually similar to [Package Distributions npm RFC](https://github.com/npm/rfcs/pull/519).
We'd like to switch to it once it becomes available.
