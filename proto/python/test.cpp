// TEMPORARY CODE FOR FUNCTIONALITY TESTING, will be ERASED!
// $Id$
 
#include "bindings.h"

using Opde::PythonLanguage;

int main(void) {
	// Only call the python script. that's all we need now
	PythonLanguage::init(0, NULL);
	// TODO: Pass arguments to python
	// TODO: Handle exceptions gracefully
	PythonLanguage::runScript("test.py");
	PythonLanguage::term();
}
