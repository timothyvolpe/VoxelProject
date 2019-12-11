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
#ADD_DEFINITIONS( -DGLEW_STATIC )
#ADD_DEFINITIONS( -DSDL_MAIN_HANDLED )
#ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
#ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )

# Project Include Files
file( GLOB Project_INC "${PROJECT_SOURCE_DIR}/include/*.h" )
# Include Path
include_directories( "${PROJECT_SOURCE_DIR}/include" )

# Project Source Files
file( GLOB Project_SRC "${PROJECT_SOURCE_DIR}/src/*.cpp" )

# Executable File to Generate
add_executable( ${ProjectName} ${Project_INC} ${Project_SRC} )

#########################################
############### Libraries ###############
#########################################

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
