add_library(Model STATIC
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

    ../ObjFac1/BallElement.cpp
    ../ObjFac1/BumperGate.cpp
    ../ObjFac1/DefaultSurface.cpp
    ../ObjFac1/DoubleSpeedSource.cpp
    ../ObjFac1/FinishLine.cpp
    ../ObjFac1/FuelSource.cpp
    ../ObjFac1/GenericRenderer.cpp
    ../ObjFac1/HoverRender.cpp
    ../ObjFac1/main.cpp
    ../ObjFac1/Mine.cpp
    ../ObjFac1/Missile.cpp
    ../ObjFac1/PowerUp.cpp
    ../ObjFac1/stdafx.cpp
    ../ObjFac1/TestElement.cpp
    ../ObjFac1/WoodSurface.cpp  
    ../ObjFac1/ObjectFromStaticFactory.cpp  
)

target_compile_definitions(Model PRIVATE
    _WINDLL
    _AFXDLL
    MR_MODEL
)

target_link_libraries(Model PRIVATE Util
    winmm.lib
)