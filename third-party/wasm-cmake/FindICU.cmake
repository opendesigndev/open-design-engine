if(NOT TARGET icu)
  add_library(icu INTERFACE IMPORTED)
  set(ICU_LIBRARY icu)
  set_target_properties(icu PROPERTIES
    INTERFACE_COMPILE_OPTIONS "-sUSE_ICU=1"
    INTERFACE_LINK_LIBRARIES "-sUSE_ICU=1"
  )
endif()
