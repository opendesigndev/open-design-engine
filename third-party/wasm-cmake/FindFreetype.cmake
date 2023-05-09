if(NOT TARGET freetype)
  add_library(freetype INTERFACE IMPORTED)
  add_library(Freetype::Freetype ALIAS freetype)
  set_target_properties(freetype PROPERTIES
    INTERFACE_COMPILE_OPTIONS "-sUSE_FREETYPE=1"
    INTERFACE_LINK_LIBRARIES "-sUSE_FREETYPE=1"
    INTERFACE_INCLUDE_DIRECTORIES "${EMSCRIPTEN_SYSROOT}/include/freetype2"
  )
endif()
