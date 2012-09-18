// #define BUILDING_V8_SHARED 1

#include "core.hpp"
#include "v8_core.hpp"

#if OS(WINDOWS)
//	#pragma warning ( disable : 4251 )
#if defined(HYDROGEN_EXPORTS)
#define HYDROGEN_API __declspec(dllexport)
#else
#define HYDROGEN_API __declspec(dllimport)
#endif
#elif __GNUC__ >= 4
# define HYDROGEN_API __attribute__((visibility("default")))
#else
#define HYDROGEN_API // nothing, symbols in a shared library are exported by default
#endif

#include "video_modes.hpp"
#include "gui.hpp"
#if OS(WINDOWS)
#include "gui.windows.hpp"
#elif OS(LINUX)
#include "gui.linux.hpp"
#endif

/*

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

*/