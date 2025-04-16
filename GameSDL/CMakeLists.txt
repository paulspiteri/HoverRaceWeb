add_executable(GameSDL
    ClientSession.cpp
    Observer.cpp
    SDLGameApp.cpp
    sdlmain.cpp
)

target_link_libraries(GameSDL PRIVATE MainCharacter Model ObjFacTools Util VideoServices ObjFac1 SDL3::SDL3-static
)

if(WIN32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_link_options(GameSDL PRIVATE "/subsystem:windows")
    else()
        target_link_options(GameSDL PRIVATE "-mwindows")
    endif()
    set_target_properties(GameSDL PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
