#define BUILDING_V8_SHARED 1

#include "core.hpp"
#include "v8_core.hpp"

// Template Project for creation of SDK library
// Contains basic test_class class that gets
// instantiated within v8.

namespace aspect
{

class __declspec(dllexport) test_class
{
	public:

		V8_DECLARE_CLASS_BINDER(test_class);


		void test_function_binding(void) { printf("TEST FUNCTION BINDING INVOKED!\n"); }
};


} // ::aspect

#define WEAK_CLASS_TYPE aspect::test_class
#define WEAK_CLASS_NAME test_class
#include <v8/juice/WeakJSClassCreator-Decl.h>
