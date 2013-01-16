cmake_minimum_required(VERSION 2.8) 

project(hydrogen.jsx)
set(CMAKE_BUILD_TYPE debug)
file(GLOB SRC "src/*.cpp")

set(JSX ${PROJECT_SOURCE_DIR}/../jsx)
set(EXTERN ${JSX}/extern)

set(EXECUTABLE_OUTPUT_PATH ${JSX}/bin)
set(LIBRARY_OUTPUT_PATH ${JSX}/bin)
set(CMAKE_CURRENT_BINARY_DIR ${JSX}/bin)

set(CMAKE_SHARED_LINKER_FLAG "${CMAKE_SHARED_LINKER_FLAGS} lt_sys_lib_dlsearch_path_spec=${EXECUTABLE_OUTPUT_PATH}")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(CMAKE_CXX_FLAGS "-Wno-parentheses-equality -DNDEBUG")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        set(CMAKE_CXX_FLAGS "-std=c++0x -fPIC -DNDEBUG")
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(${EXTERN})

include_directories(${EXTERN}/boost/include)
link_directories(${EXTERN}/boost/lib)

include_directories(${EXTERN}/v8/include)
link_directories(${EXTERN}/b8/lib)

include_directories(${EXTERN}/v8-juice/src/include)
link_directories(${EXTERN}/v8-juice/lib)

include_directories(${JSX}/sdk/core/src)
link_directories(${JSX}/bin)

set(MATH ${PROJECT_SOURCE_DIR}/../math)
include_directories(${MATH}/src)

set(IMAGE ${PROJECT_SOURCE_DIR}/../image)
include_directories(${IMAGE}/src)

set(OXYGEN ${PROJECT_SOURCE_DIR}/../oxygen)
include_directories(${OXYGEN}/src)

set(BOOST_LIBRARIES boost_thread boost_filesystem boost_system boost_iostreams)
set(V8_LIBRARIES v8 v8-juice)

add_library(hydrogen.jsx SHARED ${SRC})

target_link_libraries(hydrogen.jsx core.jsx oxygen.jsx v8 v8-juice ${BOOST_LIBRARIES} X11 Xrandr)
