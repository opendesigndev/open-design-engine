set(ICU_FOUND TRUE)
set(ICU_INCLUDE_DIR "${EMSCRIPTEN_SYSROOT}/include")
set(ICU_LIBRARIES "null")
add_library(icu ALIAS null)
target_include_directories(null PUBLIC "${ICU_INCLUDE_DIR}")
