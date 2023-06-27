# Open Design Internal Design Editor

The **Internal Design Editor** is a small utility tool that tests the completeness of the [Open Design Engine](https://github.com/opendesigndev/open-design-engine) public API, the engine’s editing capabilities and its performance. It is a C++ desktop application, the user interface is built using the [Dear ImGui](https://github.com/ocornut/imgui) library.

It allows opening and saving binary **Octopus** files (`*.octopus`) as well as **Octopus** component files (`*.json`).

The Editor can be used from the command line:
```
./design-editor [input_file_path] [--fonts fonts_dir_path] [--images images_dir_path]
```
```
input_file_path     Optional argument, initially loaded json/octopus file.

--fonts             Optional argument, custom fonts directory.
                    In not specified, system directories will be used.

--images            Optional argument, custom images directory.
                    If not specified, images will be loaded from the "images"
                    directory relative to the opened file.
```
