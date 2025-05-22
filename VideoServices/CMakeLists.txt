add_library(VideoServices STATIC
    2DViewport.cpp           
    3DViewport.cpp           
    3DViewportRendering.cpp  
    Bitmap.cpp               
    ColorPalette.cpp         
    ColorTab.cpp                      
    Patch.cpp
    SDLSoundServer.cpp
    Sprite.cpp               
    VideoBuffer.cpp          
    VideoBuffer2DDraw.cpp
    GL/GLRenderer.cpp
        GL/GLViewport.cpp
        GL/GLViewport.h
)

target_compile_definitions(VideoServices PRIVATE
    MR_VIDEO_SERVICES
)

if(NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
    target_link_libraries(VideoServices PRIVATE OpenGL::GL)
endif()


target_link_libraries(VideoServices PRIVATE ColorTools Util SDL3::SDL3-static glm::glm sokol
)