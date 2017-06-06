#include <lib/platform.hpp>


#ifdef LIB_PLATFORM_WEB
#include <GLES2/gl2.h>
#define ROV_GLES2
#else
#include <GL/gl3w.h>
#define ROV_GL3
#endif

#define ROV_GL_IMPL
#include <rov/rov.hpp>
