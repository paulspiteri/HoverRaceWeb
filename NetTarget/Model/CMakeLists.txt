add_library(Model SHARED
    ConcreteShape.cpp          
    ContactEffect.cpp          
    FreeElementMovingHelper.cpp
    GameSession.cpp            
    Level.cpp                  
    MazeElement.cpp            
    PhysicalCollision.cpp      
    ShapeCollisions.cpp        
    Shapes.cpp                 
    stdafx.cpp                 
)

target_compile_definitions(Model PRIVATE
    _WINDLL
    _AFXDLL
    MR_MODEL
)

target_link_libraries(Model PRIVATE Util
    winmm.lib
)