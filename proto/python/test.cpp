// TEMPORARY CODE FOR FUNCTIONALITY TESTING, will be ERASED!
#include "bindings.h"

using Opde::PythonLanguage;

int main(void) {
	// Only call the python script. that's all we need now
	PythonLanguage::init();
	// TODO: Pass arguments to python
	// TODO: Handle exceptions gracefully
	PythonLanguage::runScript("test.py");
	PythonLanguage::term();
}
