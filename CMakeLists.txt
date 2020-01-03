cmake_minimum_required( VERSION 3.0 )

set( ProjectName "VoxelProject" )

project( ${ProjectName} )

# C++ configurations, debug, release, minsizerel, relwithdebinfo
set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_MINSIZEREL
		CMAKE_CXX_FLAGS_RELWITHDEBINFO
        )
		
# Set configurations, use static linked 
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

# Definitons
ADD_DEFINITIONS( -DUNICODE )
ADD_DEFINITIONS( -D_UNICODE )
ADD_DEFINITIONS( -DGLEW_STATIC )
ADD_DEFINITIONS( -DSDL_MAIN_HANDLED )
#ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
#ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )

# Project Include Files
file( GLOB Project_INC "${PROJECT_SOURCE_DIR}/include/*.h" )
file( GLOB Project_INC_GFX "${PROJECT_SOURCE_DIR}/include/gfx/*.h" )
# Include Path
include_directories( "${PROJECT_SOURCE_DIR}/include" )

# Project Source Files
file( GLOB Project_SRC "${PROJECT_SOURCE_DIR}/src/*.cpp" )
file( GLOB Project_SRC_GFX "${PROJECT_SOURCE_DIR}/src/gfx/*.cpp" )

# Project File Filters
source_group( "Header Files\\gfx" FILES ${Project_INC_GFX} )

source_group( "Source Files\\gfx" FILES ${Project_SRC_GFX} )

#########
# Boost #
#########

set( BOOST_INCLUDE_DIR "BOOST_INCLUDE_DIR-NOTFOUND" CACHE PATH "" )
set( BOOST_LIBRARY_DIR "BOOST_INCLUDE_DIR-NOTFOUND" CACHE PATH "" )

if( EXISTS ${BOOST_INCLUDE_DIR} )
	include_directories( ${BOOST_INCLUDE_DIR} )
else()
	message( FATAL_ERROR "BOOST_INCLUDE_DIR was not a valid directory." )
endif()

if( EXISTS ${BOOST_LIBRARY_DIR} )
	link_directories( ${BOOST_LIBRARY_DIR} )
else()
	message( FATAL_ERROR "BOOST_LIBRARY_DIR was not a valid directory." )
endif()

# Executable File to Generate
add_executable( ${ProjectName} ${Project_INC} ${Project_INC_GFX} ${Project_SRC} ${Project_SRC_GFX} )

if( MSVC )
	set_target_properties( ${ProjectName} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/debug" )
	
	if(${CMAKE_VERSION} VERSION_LESS "3.8.0") 
		message( "Be sure to change the Debugger Working Directory in the generated solution to point to the build folder!" )
	endif()
endif()

#########################################
############### Libraries ###############
#########################################

# OpenGL

find_package( OpenGL REQUIRED )
if( OPENGL_FOUND )
	include_directories( ${OPENGL_INCLUDE_DIR} )
	target_link_libraries( ${ProjectName} ${OPENGL_LIBRARIES} )
endif()

# SDL (2.0.3)

set( SDL2_INCLUDE_DIR "SDL2_INCLUDE_DIR-NOTFOUND" CACHE PATH "" )
set( SDL2_LIBRARY "SDL2_LIBRARY-NOTFOUND" CACHE PATH "" )
set( SDL2_LIBRARY_DEBUG "SDL2_LIBRARY_DEBUG-OPTIONAL-NOTFOUND" CACHE PATH "" )
set( SDL2_MAIN "SDL2_MAIN-NOTFOUND" CACHE PATH "" )
set( SDL2_MAIN_DEBUG "SDL2_MAIN_DEBUG-OPTIONAL-NOTFOUND" CACHE PATH "" )
if( EXISTS ${SDL2_INCLUDE_DIR} )
	include_directories( ${SDL2_INCLUDE_DIR} )
else()
	message( FATAL_ERROR "Invalid SDL2_INCLUDE_DIR." )
endif()
if( EXISTS ${SDL2_LIBRARY} )
	if( EXISTS ${SDL2_LIBRARY_DEBUG} )
		target_link_libraries( ${ProjectName} optimized ${SDL2_LIBRARY} debug ${SDL2_LIBRARY_DEBUG} )
	else()
		target_link_libraries( ${ProjectName} ${SDL2_LIBRARY} )
	endif()
else()
	message( FATAL_ERROR "Invalid SDL2_LIBRARY file" )
endif()
if( EXISTS ${SDL2_MAIN} )
	if( EXISTS ${SDL2_MAIN_DEBUG} )
		target_link_libraries( ${ProjectName} optimized ${SDL2_MAIN} debug ${SDL2_MAIN_DEBUG} )
	else()
		target_link_libraries( ${ProjectName} ${SDL2_MAIN} )
	endif()
else()
	message( FATAL_ERROR "Invalid SDL2_MAIN file" )
endif()

# GLEW
set( GLEW_INCLUDE_DIR "GLEW_INCLUDE_DIR-NOTFOUND" CACHE PATH "" )
set( GLEW_LIBRARY "GLEW_LIBRARY-NOTFOUND" CACHE FILEPATH "" )
set( GLEW_LIBRARY_DEBUG "GLEW_LIBRARY_DEBUG-OPTIONAL-NOTFOUND" CACHE FILEPATH "" )
if( EXISTS ${GLEW_INCLUDE_DIR} )
	include_directories( ${GLEW_INCLUDE_DIR} )
else()
	message( FATAL_ERROR "Invalid GLEW_INCLUDE_DIR." )
endif()
if( EXISTS ${GLEW_LIBRARY} )
	if( EXISTS ${GLEW_LIBRARY_DEBUG} )
		target_link_libraries( ${ProjectName} optimized ${GLEW_LIBRARY} debug ${GLEW_LIBRARY_DEBUG} )
	else()
		target_link_libraries( ${ProjectName} ${GLEW_LIBRARY} )
	endif()
else()
	message( FATAL_ERROR "Invalid GLEW_LIBRARY file" )
endif()

