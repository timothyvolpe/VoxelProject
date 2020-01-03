# VoxelProject
3D Voxel Game Project

Generating Docs

Doxygen 1.8.7 is being used to generate the documentation files. In order to perform this operating, place
doxygen.exe in the root folder where doxyconf.cfg is located and run

doxygen doxyconf.cfg

Building Binaries

CMake 3.3 was used to build the binary files. Point the source directory to the folder containing CMakeLists.txt, and the binaries directory
to a folder of your choosing. The target platform should be 64-bit, in this case Visual Studio 2015 was used.

After CMake has been configured, the required libraries which were not detected automatically must have their paths assigned to the proper
variable. The list of required libraries and their verions is listed below.

Finally, if using Visual Studio, make sure to set the startup project to the main project, not ALL_BUILD, or you will get an error
when trying to debug! Additionally, you will likely need to change the debugging Working Directory to point to the executable output directory.

Libaries

Boost 1.59.0 x64
SDL2 v2.0.3
GLEW 1.13.0


Requires Microsoft Visual Studio Redistributable 2015 (x64)

Notes for Compiling SDL:
For release builds, had to change runtime library to Multi-threaded DLL (/MD)
Had to add vcruntimed.lib, ucrtd.lib to SDL project dependencies for debug build