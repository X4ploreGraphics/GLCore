

#ifndef GLCORE_EXPORT_H
#define GLCORE_EXPORT_H

#if defined (WIN32) && !defined (GLCORE_STATIC)
  #if defined(glCORE_EXPORTS)
    #define  GLCORE_EXPORT __declspec(dllexport)
  #else
    #define  GLCORE_EXPORT __declspec(dllimport)
  #endif 
#else /* defined (WIN32) && !defined (GLCORE_STATIC) */
 #define GLCORE_EXPORT
#endif

#ifndef PLATFORMS_DEFINE
#define PLATFORMS_DEFINE

#if defined (WIN32) 
  #define PLATFORM_WINDOWS
#elif defined(__ANDROID__)
  #define PLATFORM_ANDROID
#elif defined(__EMSCRIPTEN__)
  #define PLATFORM_EMSCRIPTEN
#else 
 #define PLATFORM_LINUX
#endif

#endif

#endif