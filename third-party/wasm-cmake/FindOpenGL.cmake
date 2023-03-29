if(NOT TARGET OpenGL::GL)
  add_library(OpenGL::GL INTERFACE IMPORTED)
endif()
