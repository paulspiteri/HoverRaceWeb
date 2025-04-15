add_library(VideoServices STATIC
    2DViewport.cpp           
    3DViewport.cpp           
    3DViewportRendering.cpp  
    Bitmap.cpp               
    ColorPalette.cpp         
    ColorTab.cpp                      
    Patch.cpp
    #SoundServer.cpp          
    StubSoundServer.cpp
    Sprite.cpp               
    VideoBuffer.cpp          
    VideoBuffer2DDraw.cpp    
)

target_compile_definitions(VideoServices PRIVATE
    MR_VIDEO_SERVICES
)

target_link_libraries(VideoServices PRIVATE ColorTools Util SDL3::SDL3-static
)