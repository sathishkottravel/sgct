# Copyright Linkoping University 2011
# SGCT Project Authors see Authors.txt

cmake_minimum_required(VERSION 2.8)

SET(BUILD_SHARED_LIBS OFF)
SET(LIB_NAME tinythreadpp)

SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")

PROJECT(${LIB_NAME})
set(LibPath ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${LibPath})
	
find_path(TINYTHREAD_INCLUDE_DIR REQUIRED)
	
add_library(${LIB_NAME} STATIC tinythread.cpp)
include_directories(${TINYTHREAD_INCLUDE_DIR})

if( NOT WIN32 ) #MINGW or similar
	find_library(Threads REQUIRED)
endif()

if(MSVC AND NOT "${MSVC_VERSION}" LESS 1400)
	add_definitions( "/MP" )
endif()

if( WIN32 )
	add_definitions(-D__WIN32__)
elseif( APPLE  )
	add_definitions(-D__APPLE__)
else()
	add_definitions(-D__LINUX__)
endif()
	
if( CMAKE_COMPILER_IS_GNUCXX )
	set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -s")
endif()