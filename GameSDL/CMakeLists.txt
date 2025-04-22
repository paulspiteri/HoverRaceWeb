add_executable(GameSDL
    ClientSession.cpp
    Observer.cpp
    SDLGameApp.cpp
    sdlmain.cpp
)

target_link_libraries(GameSDL PRIVATE MainCharacter Model ObjFacTools Util VideoServices ObjFac1 SDL3::SDL3-static
)

if(EMSCRIPTEN)
    set_target_properties(GameSDL PROPERTIES OUTPUT_NAME "hover" SUFFIX ".html")
    set_target_properties(GameSDL PROPERTIES LINK_FLAGS "--preload-file ${CMAKE_SOURCE_DIR}\\ObjFac1.dat@ObjFac1.dat --preload-file ${CMAKE_SOURCE_DIR}\\Tracks\\ClassicH.trk@ClassicH.trk")
elseif(WIN32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_link_options(GameSDL PRIVATE "/subsystem:windows")
    else()
        target_link_options(GameSDL PRIVATE "-mwindows")
    endif()
    set_target_properties(GameSDL PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
