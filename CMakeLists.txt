cmake_minimum_required(VERSION 3.20)
project(HoverRace)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

# Enable MFC globally for all projects
set(CMAKE_MFC_FLAG 2)  # Use MFC in a shared DLL

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(Util)
add_subdirectory(ColorTools)
add_subdirectory(Model)
add_subdirectory(ObjFacTools)
add_subdirectory(ObjFac1)
add_subdirectory(VideoServices)
add_subdirectory(MainCharacter)
add_subdirectory(Game2)
