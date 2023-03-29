if(NOT TARGET skia)
  add_library(skia STATIC IMPORTED)
  set_target_properties(skia PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../skia/out/wasm/release/libskia.a"
    INTERFACE_COMPILE_OPTIONS ""
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../skia/out/include"
  )
endif()
