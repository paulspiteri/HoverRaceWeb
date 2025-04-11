add_library(ObjFacTools STATIC
    BitmapSurface.cpp           
    FreeElementBase.cpp         
    FreeElementBaseRenderer.cpp 
    ObjectFactoryData.cpp       
    ResActor.cpp                
    ResBitmap.cpp               
    ResourceLib.cpp             
    ResSound.cpp                
    ResSprite.cpp               
    SpriteHandle.cpp            
    stdafx.cpp                     
)

target_compile_definitions(ObjFacTools PRIVATE
    _WINDLL
    _AFXDLL
    MR_OBJ_FAC_TOOLS
)

target_link_libraries(ObjFacTools PRIVATE Model Util
)