message("hints: ${CMAKE_CURRENT_SOURCE_DIR}/third-party/skia/out/wasm/release")
add_library(skia "${CMAKE_BINARY_DIR}/null.cpp")
target_link_libraries(skia "${CMAKE_CURRENT_SOURCE_DIR}/third-party/skia/out/wasm/release/libskia.a")
target_include_directories(skia PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/third-party/skia/out/include"
)
