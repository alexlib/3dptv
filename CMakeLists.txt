cmake_minimum_required(VERSION 3.10)

project (3dptv_solution)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
# Add definitions and Find TIFF

if (WIN32)
  add_definitions( -D_CONSOLE -D_CRT_SECURE_NO_WARNINGS )
  set(TIFF_LIBRARY "${CMAKE_SOURCE_DIR}/src_c/libtiff.lib")
else()
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/opt/ActiveTcl-8.6")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/lib")
  find_package( TIFF REQUIRED )
endif()

# Find TCL/TK
set(TCL_INCLUDE_PATH "/usr/include/tcl8.6")
set(TCL_LIBRARY "/usr/lib/x86_64-linux-gnu/libtcl.so")
set(TK_INCLUDE_PATH "/usr/include/tcl8.6")
set(TK_LIBRARY "/usr/lib/x86_64-linux-gnu/libtk8.6.so")
find_package(TCL 8.6 REQUIRED)

if (TCLTK_FOUND )
	message(STATUS "TCL INCLUDE: " ${TCL_INCLUDE_PATH})
	message(STATUS "TCL LIB: " ${TCL_LIBRARY})
	message(STATUS "TK INCLUDE: " ${TCL_INCLUDE_PATH})
	message(STATUS "TK LIB: " ${TK_LIBRARY})
	link_directories( ${TCL_LIBRARY} ${TK_LIBRARY} )
	include_directories( ${TCL_INCLUDE_PATH} ${TK_INCLUDE_PATH} )
endif()

if (WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
endif()

message(STATUS "gathering ptv tcl files:")
file(GLOB ptv_tcl "${CMAKE_SOURCE_DIR}/src_tcl/*.tcl")

# Option 1: Set build type (recommended for overall debug build)
# When running cmake, use: cmake -DCMAKE_BUILD_TYPE=Debug ..

# Option 2: Add -g flag directly (can be combined with build type)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g") # If you have C++ code

add_subdirectory("${CMAKE_SOURCE_DIR}/src_c")