#ifndef _CONFIG_H
#define _CONFIG_H
	
#cmakedefine HAVE_INTTYPES_H 1

#cmakedefine OPDE_DEBUG
#cmakedefine __STATIC_GEOMETRY
#cmakedefine CUSTOM_IMAGE_HOOKS
#cmakedefine PYTHON_SUPPORT

#define OPDE_VER_MAJOR ${OPDE_VER_MAJOR}
#define OPDE_VER_MINOR ${OPDE_VER_MINOR}
#define OPDE_VER_PATCH ${OPDE_VER_PATCH}

#cmakedefine __OPDE_BIG_ENDIAN ${BIG_ENDIAN}

#if defined (_MSC_VER)
// disable the class needs to have a dll-interface...
#pragma warning(disable:4251)
// No suitable definition for explicit template spec warning disable
#pragma warning(disable:4661)
#endif

// DLL export/import stuff for OpdeLib
// If the build target is OpdeLib, the OpdeLib_EXPORTS is defined,
// otherwise it is not. Trouble is we need it unset for all library targets,
// so that all the static libs build and can be linked together.
// We also want OPDELIB_EXPORT to be dllimport for executables
// Thus, we define our own macros in the CMakeLists.txt files that define the
// contents of the OPDELIB_EXPORT
#if defined (_WIN32) // It is said that mingw also tolerates __declspec
  #if defined(OPDELIB_DLL_TARGET)
    #define OPDELIB_EXPORT __declspec(dllexport)
  #elif defined(OPDE_EXE_TARGET)
    #define OPDELIB_EXPORT __declspec(dllimport)
  #else
    #define OPDELIB_EXPORT
  #endif /* OpdeLib_EXPORTS */
#elif defined(GCC4)
    #if defined(OPDELIB_DLL_TARGET)
        #define OPDELIB_EXPORT __attribute__ ((visibility("default")))
    #endif /* OPDELIB_DLL_TARGET */
#else /* defined (GCC4) */
 #define OPDELIB_EXPORT
#endif

#endif
