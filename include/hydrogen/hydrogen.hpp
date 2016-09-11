#pragma once

#include "image/image.hpp"
#include "math/math.hpp"
#include "oxygen/oxygen.hpp"

#if _MSC_VER
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


#if defined(_WIN32)
#include "glew.h"
//#include <GL/glew.h>
#include "hydrogen/gl.iface.wgl.hpp"
#elif defined(__APPLE__)
#include <OpenGL/gl.h>
#include "hydrogen/gl.iface.nsgl.hpp"
#else
#include <GL/glxew.h>
#include <GL/glew.h>
#include "hydrogen/gl.iface.glx.hpp"
#endif

#ifdef None
#undef None
#endif

#include "hydrogen/gl.color.hpp"
#include "hydrogen/gl.shader.hpp"
#include "hydrogen/gl.texture.hpp"
#include "hydrogen/gl.transform.hpp"
#include "hydrogen/gl.entity.hpp"
#include "hydrogen/gl.camera.hpp"

// #define BT_USE_DOUBLE_PRECISION
#include "hydrogen/physics.bullet.hpp"

#include "hydrogen/gl.engine.hpp"
