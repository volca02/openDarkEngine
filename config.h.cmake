#ifndef _CONFIG_H
#define _CONFIG_H
	
#cmakedefine HAVE_INTTYPES_H 1

#cmakedefine OPDE_DEBUG

#define OPDE_VER_MAJOR ${OPDE_VER_MAJOR}
#define OPDE_VER_MINOR ${OPDE_VER_MINOR}
#define OPDE_VER_PATCH ${OPDE_VER_PATCH}
#define OPDE_CODE_NAME "${REL_CODE_NAME}"

#cmakedefine __OPDE_BIG_ENDIAN ${BIG_ENDIAN}

#if defined (_MSC_VER)
// disable the class needs to have a dll-interface...
#pragma warning(disable:4251)
// No suitable definition for explicit template spec warning disable
#pragma warning(disable:4661)
#endif

// obsolete
#define OPDELIB_EXPORT

// platforms
#cmakedefine WIN32
#cmakedefine UNIX
#cmakedefine APPLE

// data installation path
#define OPDE_SHARE_DIR "${OPDE_FULL_DATA_INSTALL_DIR}"

// Script compilers present
#cmakedefine SCRIPT_COMPILERS

#endif
