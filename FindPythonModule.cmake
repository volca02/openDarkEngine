# Helper macro that finds if python module of the given name exists 
MACRO(FIND_PYTHON_MODULE _MODULE_NAME _VARNAME)
    FIND_PACKAGE(PythonInterp REQUIRED)
    
    # Execute the one-liner to get the python deps
    IF(PYTHONINTERP_FOUND)
    	SET(${_VARNAME})
	MARK_AS_ADVANCED(${_VARNAME})
	
	EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} "-c" "import ${_MODULE_NAME};"
	    RESULT_VARIABLE PYRET
	    ERROR_QUIET
	)
	
	# The return value should be Zero, otherwise module did not exist...
	IF(NOT PYRET)
	    	SET(${_VARNAME} ON)
	ENDIF(NOT PYRET)
	
    ELSE(PYTHONINTERP_FOUND)
	MESSAGE(FATAL_ERROR "Python interpreter not found, but needed to detect module presence")
    ENDIF(PYTHONINTERP_FOUND)
ENDMACRO(FIND_PYTHON_MODULE _MODULE_NAME _VARNAME)
