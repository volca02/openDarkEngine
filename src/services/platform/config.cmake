LIST(APPEND OPDE_SERVICE_SOURCES ${OPDE_SOURCE_DIR}/src/services/platform/PlatformService.cpp)
LIST(APPEND OPDE_SERVICE_HEADERS ${OPDE_SOURCE_DIR}/src/services/platform/PlatformService.h)
LIST(APPEND OPDE_SERVICE_SOURCES ${OPDE_SOURCE_DIR}/src/services/platform/Platform.cpp)
LIST(APPEND OPDE_SERVICE_HEADERS ${OPDE_SOURCE_DIR}/src/services/platform/Platform.h)


# OS Specific code follows. Example: OS X could have a specific condition here
IF(UNIX)
	IF(APPLE)
		# Apple specific platform
		# TODO: LIST(APPEND OPDE_SERVICE_SOURCES ${OPDE_SOURCE_DIR}/src/services/platform/ApplePlatform.cpp)
		# TODO: LIST(APPEND OPDE_SERVICE_HEADERS ${OPDE_SOURCE_DIR}/src/services/platform/ApplePlatform.h)
		MESSAGE(FATAL_ERROR "Apple platform currently unsupported! Please consider helping us by providing the platform specific code!")
	ELSE(APPLE)
		# Generic unix platform
		LIST(APPEND OPDE_SERVICE_SOURCES ${OPDE_SOURCE_DIR}/src/services/platform/UnixPlatform.cpp)
		LIST(APPEND OPDE_SERVICE_HEADERS ${OPDE_SOURCE_DIR}/src/services/platform/UnixPlatform.h)
	ENDIF(APPLE)
ELSE(UNIX)
	IF(WIN32)
		LIST(APPEND OPDE_SERVICE_SOURCES ${OPDE_SOURCE_DIR}/src/services/platform/Win32Platform.cpp)
		LIST(APPEND OPDE_SERVICE_HEADERS ${OPDE_SOURCE_DIR}/src/services/platform/Win32Platform.h)
	ELSE(WIN32)
		MESSAGE(FATAL_ERROR "Unknown platform encountered! Please consider helping us by providing your platform specific code!")
	ENDIF(WIN32)
ENDIF(UNIX)
