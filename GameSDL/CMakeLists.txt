add_executable(GameSDL
    ClientSession.cpp
    Observer.cpp
    GLObserver.cpp
    SDLGameApp.cpp
    sdlmain.cpp
    GLLevelLoader.cpp
    WebPeerInterface.cpp
    NetworkSession.cpp
)

target_link_libraries(GameSDL PRIVATE MainCharacter Model ObjFacTools Util VideoServices ObjFac1 SDL3::SDL3-static sokol glm::glm imgui
)

if(EMSCRIPTEN)
    set_target_properties(GameSDL PROPERTIES 
        OUTPUT_NAME "hoverrace"
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Web/Client/public"
        LINK_FLAGS "-s MIN_WEBGL_VERSION=2 -s USE_WEBGL2=1 -sASSERTIONS -s EXPORTED_FUNCTIONS=['_main','_ChangeWindowSize','_ReceivePeerMessage','_SetPlayerId','_SetPeerStatus','_malloc','_free'] -s EXPORTED_RUNTIME_METHODS=['ccall','HEAPU8']")
    target_link_options(GameSDL PRIVATE
    "--shell-file=${CMAKE_SOURCE_DIR}/Emscripten/Hover.html"
    "--preload-file=${CMAKE_SOURCE_DIR}/Assets/ObjFac1.dat@ObjFac1.dat"
    "--preload-file=${CMAKE_SOURCE_DIR}/Assets/Fonts/Roboto-MediumItalic.ttf@Roboto-MediumItalic.ttf"
    "--preload-file=${CMAKE_SOURCE_DIR}/Assets/Tracks/ClassicH.trk@ClassicH.trk"
    "--preload-file=${CMAKE_SOURCE_DIR}/Assets/Tracks/Steeplechase.trk@Steeplechase.trk"
    "--preload-file=${CMAKE_SOURCE_DIR}/Assets/Tracks/The Alley2.trk@The Alley2.trk"
    "--preload-file=${CMAKE_SOURCE_DIR}/Assets/Tracks/The River.trk@The River.trk"
  )
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(GameSDL PRIVATE OpenGL::GL)

    if(WIN32)
        if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
            target_link_options(GameSDL PRIVATE "/subsystem:windows")
        else()
            target_link_options(GameSDL PRIVATE "-mwindows")
        endif()
        set_target_properties(GameSDL PROPERTIES WIN32_EXECUTABLE TRUE)
    endif()
endif()
