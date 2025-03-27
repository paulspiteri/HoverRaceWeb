add_library(VideoServices SHARED
    2DViewPort.cpp           
    3DViewport.cpp           
    3DViewportRendering.cpp  
    Bitmap.cpp               
    ColorPalette.cpp         
    ColorTab.cpp                      
    SoundServer.cpp          
    Sprite.cpp               
    StdAfx.cpp               
    VideoBuffer.cpp          
    VideoBuffer2DDraw.cpp    
)

target_compile_definitions(VideoServices PRIVATE
    _WINDLL
    _AFXDLL
    MR_VIDEO_SERVICES
)

target_link_libraries(VideoServices PRIVATE ColorTools Util
    ddraw.lib
    dsound.lib
    winmm.lib
)