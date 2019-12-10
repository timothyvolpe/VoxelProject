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
ADD_DEFINITIONS( -DWIN32 )
#ADD_DEFINITIONS( -DGLEW_STATIC )
#ADD_DEFINITIONS( -DSDL_MAIN_HANDLED )
#ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
#ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
