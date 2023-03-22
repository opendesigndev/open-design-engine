if(NOT TARGET harfbuzz::harfbuzz)
  add_library(harfbuzz::harfbuzz INTERFACE IMPORTED)
  add_library(harfbuzz ALIAS harfbuzz::harfbuzz)
  set_target_properties(harfbuzz::harfbuzz PROPERTIES
    INTERFACE_COMPILE_OPTIONS "-sUSE_HARFBUZZ=1"
    INTERFACE_LINK_LIBRARIES "-sUSE_HARFBUZZ=1"
    INTERFACE_INCLUDE_DIRECTORIES "${EMSCRIPTEN_SYSROOT}/include/harfbuzz"
  )
endif()
