add_executable(GameSDL
    ClientSession.cpp
    Observer.cpp
    SDLGameApp.cpp
    sdlmain.cpp
)

target_link_libraries(GameSDL PRIVATE MainCharacter Model ObjFacTools Util VideoServices ObjFac1 SDL3::SDL3-static sokol
)

if(EMSCRIPTEN)
    set_target_properties(GameSDL PROPERTIES OUTPUT_NAME "index" SUFFIX ".html"   
    LINK_FLAGS "-s EXPORTED_FUNCTIONS=['_main','_ChangeToTrack'] -s EXPORTED_RUNTIME_METHODS=['ccall']")
    target_link_options(GameSDL PRIVATE
    "--shell-file=${CMAKE_SOURCE_DIR}/Emscripten/Hover.html"
    "--preload-file=${CMAKE_SOURCE_DIR}/ObjFac1.dat@ObjFac1.dat"
    "--preload-file=${CMAKE_SOURCE_DIR}/Tracks/ClassicH.trk@ClassicH.trk"
    "--preload-file=${CMAKE_SOURCE_DIR}/Tracks/Steeplechase.trk@Steeplechase.trk"
    "--preload-file=${CMAKE_SOURCE_DIR}/Tracks/The Alley2.trk@The Alley2.trk"
    "--preload-file=${CMAKE_SOURCE_DIR}/Tracks/The River.trk@The River.trk"
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
