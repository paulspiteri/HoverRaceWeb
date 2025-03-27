cmake_minimum_required(VERSION 3.20)
project(HoverRaceCMAKE)

# Enable MFC globally for all projects
set(CMAKE_MFC_FLAG 2)  # Use MFC in a shared DLL

add_subdirectory(Util)
add_subdirectory(ColorTools)
add_subdirectory(Model)
add_subdirectory(VideoServices)
add_subdirectory(ObjFacTools)
