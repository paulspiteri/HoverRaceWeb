#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(GameSDL
    ClientSession.cpp
    Observer.cpp
    SDLGameApp.cpp
    sdlmain.cpp
)

target_link_libraries(GameSDL PRIVATE MainCharacter Model ObjFacTools Util VideoServices ObjFac1 SDL3::SDL3-static
)
