#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/ui/text/TestRunner.h"

#include "Trace.h"

int main(int argc, char *argv[])
{
	createTrace();
	output2Console();

	CppUnit::TextUi::TestRunner runner;
	runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
	runner.run();

	closeTrace();

	exit(0);
}
