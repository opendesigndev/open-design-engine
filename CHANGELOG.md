
## Version 0.4.0 (2023-05-10)

- New real-time text renderer based on signed distance fields, which greatly improves performance of re-rendering text, even at different scales
- Added Design Editor module - an internal GUI-based tool
- Fixed layer identification by position for inverted layers
- Fixed an OpenGL synchronization bug that manifested on some systems
- Updated Text Renderer submodule to `fba1cdd`

## Version 0.3.1 (2023-04-12)

- Added new functions to ODE API:
  - `ode_design_listComponents`
  - `ode_destroyComponentList`
- Fixed positioning of group members after changes to group's transform matrix
- Fixed invalidation of reported layer bounds following layer modifications
- Updated baseline version of vcpkg dependencies to `4116148` (2023-04-11)

## Version 0.3.0 (2023-03-30)

- Added Node-API bindings for the ODE API (ode-napi module)
- Embind version of JavaScript bindings removed
- Expanded ODE API
  - Output arguments marked with `ODE_OUT` / `ODE_INOUT` / `ODE_OUT_RETURN`
  - New enumeration `ODE_TransformationBasis`
  - New API function `ode_component_transformLayer`
  - Additional `ODE_Result` codes
- Generated source code no longer checked in in the repository
- Moved animation prototype module to tools
- Minor version formatting changes

## Version 0.2.3 (2023-03-13)

- "Textify" submodule replaced by overhauled Open Design Text Renderer

## Version 0.2.2 (2023-03-07)

- Octopus 3.0.2 compatibility (glows not removed yet)
  - `GAUSSIAN_BLUR` and `BOUNDED_BLUR` effect support
  - Modification of `maskChannels` via layer changes
- Fixed upside down output to screen
- Computation of effect margins more accurate
- Parsers and serializers generated with new version of json-cpp-gen
- Removed usage of deprecated `sprintf`, warning fixes
- Added CMake presets
- Emscripten binding fixes

## Version 0.2.1 (2023-02-28)

- Added support for "bounded blur" (equivalent to box blur applied twice)
- Bounded blur now used for shadows
- Fixed regression bugs of the new renderer (introduced in 0.2.0)
  - Inner shadow no longer rendered as drop shadow
  - Fixed out-of-bounds mapping of image fills
  - Fixed effect behavior when `ODE_SKIA_GPU` is disabled
  - Fixed fill color premultiplication
  - Fixed shadow blur being too large
  - Output image files correctly saved as unpremultiplied
- Rendering empty components now produces a blank image rather than an error
- Fixed an issue related to wrong OpenGL state left by Skia call

## Version 0.2.0 (2023-02-17)

- Complete overhaul of core renderer which now operates in alpha-premultiplied color space
- Removed all legacy code left over from "Old Rendering" project
- Added Renderer Unit Tests subproject

## Version 0.1.2 (2023-02-15)

- Updated Text Renderer
- Support for some layer modifications via Octopus `LayerChange` object
- Expanded ODE API
  - New API functions:
    - `ode_component_modifyLayer`
    - `ode_pr1_component_getAnimationValueAtTime`
  - New API type `ODE_ConstCharPtr`
  - `ODE_StringRef` now bound as value object, its `data` member changed to `ODE_ConstCharPtr`
  - `ODE_String` now provides binding for `getData()` getter for its `data` member as `ODE_VarDataPtr`
  - New `ODE_Result` value - `ODE_RESULT_ITEM_NOT_FOUND`
  - Some integer values of `ODE_Result` enumeration changed
- Fixed Emscripten bindings of `ODE_Transformation` type
- Improvements to Render Graph Inspector

## Version 0.1.1 (2023-02-01)

- Updated to Octopus 3.0.0-alpha.41 specs
- Major overhaul of ODE API
  - API headers extensively documented
  - Switched from integer constants to `enum`s:
    - `ODE_Result` - new return type of all functions
    - `ODE_LayerType` (for `ODE_LayerMetrics::Entry::type`)
  - New API types:
    - `ODE_VarDataPtr`
    - `ODE_ConstDataPtr`
    - `ODE_StringList`
    - `ODE_MemoryBuffer`
    - `ODE_ParseError`
  - New API functions:
    - `ode_component_getOctopus`
    - `ode_component_listMissingFonts`
    - `ode_design_listMissingFonts`
    - `ode_destroyMissingFontList`
    - `ode_design_loadFontFile`
    - `ode_design_loadFontBytes`
    - `ode_allocateMemoryBuffer`
    - `ode_reallocateMemoryBuffer`
    - `ode_destroyMemoryBuffer`
  - Detailed JSON parse errors reported through the API - changed interface of functions:
    - `ode_loadDesignFromManifestFile`
    - `ode_loadDesignFromManifestString`
    - `ode_design_loadManifestFile`
    - `ode_design_loadManifestString`
    - `ode_design_addComponentFromOctopusString`
    - `ode_component_addLayer`
    - `ode_pr1_component_loadAnimation`
  - Changed type of `pixels` member in `ODE_Bitmap` and `ODE_BitmapRef`
  - `ODE_Transformation` type changed to structure
  - New pixel format constant `ODE_PIXEL_FORMAT_PREMULTIPLIED_RGBA`
  - Non-Emscripten functions now marked by `ODE_NATIVE_API`
  - Renamed `ode_designLoadImagePixels` to `ode_design_loadImagePixels`
- Fixed component's `addLayer` and `setAnimation` when called early
- Emscripten bindings and TypeScript bindings now machine-generated by script
- Fixed edge case bug in JSON parser (liboctopus submodule to `b427dc7d`)
- Updated Text Renderer submodule to `cdc32a45` (load font from bytes in memory)
- Updated baseline version of vcpkg dependencies to `ac12118` (2023-01-26)
- npm package renamed to `@opendesign/engine`

## Version 0.1.0 (2023-01-09)
