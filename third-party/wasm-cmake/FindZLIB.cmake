if(NOT TARGET zlib)
  add_library(zlib INTERFACE IMPORTED)
  set_target_properties(zlib PROPERTIES
    INTERFACE_COMPILE_OPTIONS "-sUSE_ZLIB=1"
    INTERFACE_LINK_LIBRARIES "-sUSE_ZLIB=1"
  )
endif()
