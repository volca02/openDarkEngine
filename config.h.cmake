#ifndef _CONFIG_H
#define _CONFIG_H
	
#cmakedefine HAVE_INTTYPES_H 1

#cmakedefine OPDE_DEBUG
#cmakedefine __STATIC_GEOMETRY

#define OPDE_VER_MAJOR ${OPDE_VER_MAJOR}
#define OPDE_VER_MINOR ${OPDE_VER_MINOR}
#define OPDE_VER_PATCH ${OPDE_VER_PATCH}

#cmakedefine __OPDE_BIG_ENDIAN ${BIG_ENDIAN}

#endif
