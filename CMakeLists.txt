cmake_minimum_required(VERSION 2.8) 

project(hydrogen.jsx)

set(JSX_ROOT $ENV{JSX_ROOT})
if(NOT DEFINED JSX_ROOT)
	set(JSX_ROOT ~/dev/jsx)
	message("Environment variable JSX_ROOT not defined, using ${JSX_ROOT}")
endif(NOT DEFINED JSX_ROOT)

include(${JSX_ROOT}/CMakeLists.txt.global)

file(GLOB SRC "src/*.cpp")

set(MATH ${PROJECT_SOURCE_DIR}/../math)
include_directories(${MATH}/src)

set(IMAGE ${PROJECT_SOURCE_DIR}/../image)
include_directories(${IMAGE}/src)

set(OXYGEN ${PROJECT_SOURCE_DIR}/../oxygen)
include_directories(${OXYGEN}/src)

include_directories(extern)
include_directories(extern/bullet/src)

add_subdirectory(extern/bullet bin/bullet)

add_library(hydrogen.jsx SHARED ${SRC})
set(BULLET_LIBS BulletSoftBody BulletDynamics BulletCollision LinearMath)
target_link_libraries(hydrogen.jsx core.jsx math.jsx image.jsx oxygen.jsx v8 X11 Xrandr GL GLEW ${BULLET_LIBS})
 
