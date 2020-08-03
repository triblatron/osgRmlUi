#pragma once

#ifdef OSGRMLUI_DYNAMIC
   #if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
   #  ifdef OSGRMLUI_LIBRARY
   #    define OSGRMLUI_EXPORT __declspec(dllexport)
   #  else
   #     define OSGRMLUI_EXPORT __declspec(dllimport)
   #   endif
   #else
   #   ifdef OSGRMLUI_LIBRARY
   #      define OSGRMLUI_EXPORT __attribute__ ((visibility("default")))
   #   else
   #      define OSGRMLUI_EXPORT
   #   endif
   #endif
#else
#  define OSGRMLUI_EXPORT
#endif
#ifdef _WIN32
#   pragma warning (disable: 4251)
#   pragma warning(disable : 4355) // 'this' used in initializer list
#endif
