#ifndef MUS_CONFIG_H
#define MUS_CONFIG_H

#define USE_SND 0

#ifdef _MSC_VER

#ifndef SIZEOF_VOID_P
  #define SIZEOF_VOID_P 8
#endif
#ifndef HAVE_SCHEME
  #define HAVE_SCHEME 1
#endif

  typedef long off_t;
  #define ssize_t int 
  #define snprintf _snprintf 
  #define strtoll strtol

  #if _MSC_VER > 1200
    #ifndef _CRT_DEFINED
      #define _CRT_DEFINED
      #define _CRT_SECURE_NO_DEPRECATE 1
      #define _CRT_NONSTDC_NO_DEPRECATE 1
      #define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
    #endif
  #endif
#else

#if (!HAVE_PREMAKE)
/* if premake4, all settings are passed in the command line
 *    otherwise the configure script writes unix-config.h
 */
  #include "unix-config.h"
#endif

#endif

/* ---------------------------------------- */

#define HAVE_EXTENSION_LANGUAGE (HAVE_SCHEME || HAVE_RUBY || HAVE_FORTH)

#define HAVE_COMPLEX_NUMBERS    ((!_MSC_VER) && ((!HAVE_FORTH) || HAVE_COMPLEX))
#define HAVE_COMPLEX_TRIG       ((!_MSC_VER) && (!__cplusplus) && (!__FreeBSD__))
#define HAVE_MAKE_RATIO ((HAVE_SCHEME) || (HAVE_FORTH))

#endif
