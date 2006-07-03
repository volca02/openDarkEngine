#ifndef __COMPAT_H
#define __COMPAT_H

#ifdef _MSC_VER
#define snprintf _snprintf
#define M_PI_2 1.57079632679489661923
#endif


#ifndef _MSC_VER
#define strnicmp strncasecmp
#define stricmp strcasecmp
#define _wcsnicmp wcsncasecmp
#define _wcsicmp wcscasecmp
#endif

#endif
