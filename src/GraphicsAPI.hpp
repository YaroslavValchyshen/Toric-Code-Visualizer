#pragma once

// ---------------------------------------------------------------------------
// The one place GL headers are included, in the one order that works.
//
// GLEW insists on being included before anything that pulls in <GL/gl.h> --
// and <GLFW/glfw3.h> does exactly that. Getting the order wrong anywhere in
// the project produces a wall of "gl.h included before glew.h" errors, so no
// other header includes GLEW or GLFW directly; they all include this instead.
// ---------------------------------------------------------------------------

#ifdef __EMSCRIPTEN__
  #include <GLES3/gl3.h>
#else
  #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>
