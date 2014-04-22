# Helper. Statically linked python needs some deps, and the FindPython.cmake script does not set those
MACRO(FIND_PYTHON_DEPS _LIBS_TO_ADD)
    # SET(${_LIBS_TO_ADD})

    FIND_PACKAGE(PythonInterp REQUIRED)

    # Execute the one-liner to get the python deps
    IF(PYTHONINTERP_FOUND)
	EXEC_PROGRAM(${PYTHON_EXECUTABLE} ARGS "-c 'from distutils.sysconfig import get_config_var; print(get_config_var(\"LIBS\") + \" \" + get_config_var(\"SYSLIBS\"))'"
	    OUTPUT_VARIABLE PYDEPS
	    RETURN_VALUE PYRET
	)

	# The return value should be Zero, otherwise, an error is indicated
	IF(NOT PYRET)
	    # Ok, execution went fine
	    # Remove the -l parts, and do other small changes needed as well...
	    SET(_LIBNAMES)

	    STRING(REGEX REPLACE "-l" "" _LIBNAMES "${PYDEPS}")
	    STRING(REGEX REPLACE "[\n\r]" " " _LIBNAMES "${_LIBNAMES}")
	    STRING(REGEX REPLACE "  " " " _LIBNAMES "${_LIBNAMES}")
	    STRING(REGEX REPLACE " " ";" _LIBNAMES "${_LIBNAMES}")

	    # Now. For each of the listed libs, try to find it using FIND_LIBRARY (pthread -> /usr/lib/libpthread.so, etc)
	    FOREACH(_LIB ${_LIBNAMES})
		# As the CMake caches things, it is important to have a different name for each lib path, otherwise
		# the FIND_LIBRARY won't search

		# Var Name will contain the lib name
		SET(_VARNAME "_LIB_${_LIB}")

		SET(${_VARNAME})

		# Find it, concantenate the result list
		FIND_LIBRARY(${_VARNAME} ${_LIB} PATHS /usr/lib /lib)

		IF(${_VARNAME})
		    # Semicolon is the THING. It took me a while to find this out...
		    SET(${_LIBS_TO_ADD} "${${_LIBS_TO_ADD}};${${_VARNAME}}")
		ELSE(${_VARNAME})
		    MESSAGE(FATAL_ERROR "Python static linking dependency ${_LIB} not found!")
		ENDIF(${_VARNAME})

		# SET(${_VARNAME})
		MARK_AS_ADVANCED(${_VARNAME})

	    ENDFOREACH(_LIB)

	    # Done...
	ELSE(NOT PYRET)
	    MESSAGE(FATAL_ERROR "Python static linking dependencies could not be evaluated - return value non-zero")
	ENDIF(NOT PYRET)

    ELSE(PYTHONINTERP_FOUND)
	MESSAGE(FATAL_ERROR "Python interpreter not found, but needed to determine static linking dependencies")
    ENDIF(PYTHONINTERP_FOUND)
ENDMACRO(FIND_PYTHON_DEPS _LIBS_TO_ADD)
