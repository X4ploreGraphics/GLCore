

#ifndef _NITROS_GLCORE_GL_HPP
#define _NITROS_GLCORE_GL_HPP

#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    #include <GLES3/gl3.h>
    #define OPENGL_ES 30000
#else
    #if defined(OPENGL_V_40500)
        #include "glad/glad_4_5.h"
        #define OPENGL_CORE 40500
    #elif defined(OPENGL_V_40300)
        #include "glad/glad_4_3.h"
        #define OPENGL_CORE 40300
    #endif
#endif

#endif