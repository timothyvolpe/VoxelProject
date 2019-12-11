# VoxelProject
3D Voxel Game Project

Building Binaries

CMake 3.3 was used to build the binary files. Point the source directory to the folder containing CMakeLists.txt, and the binaries directory
to a folder of your choosing.

After CMake has been configured, the required libraries which were not detected automatically must have their paths assigned to the proper
variable. The list of required libraries and their verions is listed below.

Finally, if using Visual Studio, make sure to set the startup project to the main project, not ALL_BUILD, or you will get an error
when trying to debug!