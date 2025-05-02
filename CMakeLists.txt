cmake_minimum_required(VERSION 3.20)
project(HoverRace)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

add_library(sokol INTERFACE)
target_include_directories(sokol INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vendored/sokol)

include(FetchContent)
FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	1.0.1
)
FetchContent_MakeAvailable(glm)

set(SDL_STATIC ON CACHE BOOL "Build SDL3 as a static library" FORCE)
set(SDL_SHARED OFF CACHE BOOL "Disable building SDL3 as a shared library" FORCE)
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(Util)
add_subdirectory(ColorTools)
add_subdirectory(Model)
add_subdirectory(MainCharacter)
add_subdirectory(ObjFacTools)
add_subdirectory(ObjFac1)
add_subdirectory(VideoServices)
add_subdirectory(GameSDL)
