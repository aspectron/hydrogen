#pragma once

#include "core.hpp"
#include "v8_core.hpp"
#include "geometry.hpp"
#include "image.hpp"
#include "math.hpp"
#include "oxygen.hpp"

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


#if OS(WINDOWS)
#include "glew/include/glew.h"
//#include <GL/glew.h>
#elif OS(LINUX)
#include <GL/glxew.h>
#include <GL/glew.h>
#endif

#include "gl.color.hpp"

#include "gl.shader.hpp"
#include "gl.iface.hpp"
#include "gl.texture.hpp"

#include "gl.transform.hpp"

#include "gl.pipeline.hpp"
#include "gl.context.hpp"
#include "gl.entity.hpp"
#include "gl.viewport.hpp"
#include "gl.camera.hpp"
#include "gl.layer.hpp"

// #define BT_USE_DOUBLE_PRECISION
#include "physics.hpp"
#include "physics.bullet.hpp"

#include "gl.engine.hpp"
