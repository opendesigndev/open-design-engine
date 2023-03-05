
# ![Open Design Engine](https://user-images.githubusercontent.com/114005855/219663138-d71d2423-0d12-46a6-834f-c0b93a3e9125.png)

The Open Design Engine processes and renders designs and provides interface to query and manipulate their contents.

- [Changelog](CHANGELOG.md)

## How to build

1. Make sure you have installed [CMake](https://cmake.org/), [vcpkg](https://vcpkg.io/en/getting-started.html), and a C++ compiler (e.g. Visual Studio, XCode, or g++).
2. Linux & Mac only: Using APT or Homebrew, install `automake autoconf autoconf-archive`
3. (Optional) I would recommend setting your vcpkg directory as persistent system variable `VCPKG_ROOT`. If you do, you may omit the `-DCMAKE_TOOLCHAIN_FILE` option in the following step.
4. Generate CMake build. There are presets available (list them with `cmake --list-presets`), for example
    - `cmake --preset win64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake`
5. Build generated project, e.g. `cmake --build build/win64`

## Maintainer guidelines

- The version of ODE is defined in [vcpkg.json](vcpkg.json) and automatically parsed from this file for other purposes.
- Some committed files are machine-generated. These are (should be) marked as such at the very beginning. Apart from submodules, currently all such files are generated by [generate-api-bindings.py](generate-api-bindings.py). Do not modify any generated files by hand and make sure they are up-to-date before committing by re-running the script. This is only needed when the `ODE_API` headers are changed. I am considering either having the script run automatically by CMake or having the CI check that the generated files are up-to-date in the future.
- All commit messages must be capitalized

## Project module hierarchy

![Open Design Engine architecture diagram](https://user-images.githubusercontent.com/114005855/219659131-73f8ad53-a1da-4211-a002-d81003cb6d73.png)
